
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/module.h>

#include <linux/seq_file.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>

#include <media/camera_common.h>
#include <media/tegracam_core.h>

#include "gmsl_link_priv.h"
#include "gmsl_max9296.h"
#include "gmsl_max9295.h"

#define IMX490_MIN_GAIN         (0)
#define IMX490_MAX_GAIN         (30)
#define IMX490_MAX_GAIN_REG     ((IMX490_MAX_GAIN - IMX490_MIN_GAIN) * 10 / 3)
#define IMX490_DEFAULT_FRAME_LENGTH    (2000)
#define IMX490_FRAME_LENGTH_ADDR_MSB    0x200A
#define IMX490_FRAME_LENGTH_ADDR_MID    0x2009
#define IMX490_FRAME_LENGTH_ADDR_LSB    0x2008
#define IMX490_COARSE_TIME_SHS1_ADDR_MSB    0x000E
#define IMX490_COARSE_TIME_SHS1_ADDR_MID    0x000D
#define IMX490_COARSE_TIME_SHS1_ADDR_LSB    0x000C
#define IMX490_COARSE_TIME_SHS2_ADDR_MSB    0x0012
#define IMX490_COARSE_TIME_SHS2_ADDR_MID    0x0011
#define IMX490_COARSE_TIME_SHS2_ADDR_LSB    0x0010
#define IMX490_GROUP_HOLD_ADDR			0x0008
#define IMX490_ANALOG_GAIN_SP1H_ADDR	0x0018
#define IMX490_ANALOG_GAIN_SP1L_ADDR	0x001A

enum {
	IMX490_MODE_2880X1860_CROP_30FPS,
	IMX490_MODE_START_STREAM,
	IMX490_MODE_STOP_STREAM,
};

static const int imx490_30fps[] = {
	30,
};

static const struct camera_common_frmfmt imx490_frmfmt[] = {
	{{2880, 1860}, imx490_30fps, 1, 0, IMX490_MODE_2880X1860_CROP_30FPS},
};

static const struct of_device_id imx490_of_match[] = {
	{ .compatible = "sony,imx490",},
	{ },
};
MODULE_DEVICE_TABLE(of, imx490_of_match);

static const u32 ctrl_cid_list[] = {
	TEGRA_CAMERA_CID_GAIN,
	TEGRA_CAMERA_CID_EXPOSURE,
	TEGRA_CAMERA_CID_EXPOSURE_SHORT,
	TEGRA_CAMERA_CID_FRAME_RATE,
	TEGRA_CAMERA_CID_HDR_EN,
};

struct imx490 {
	struct i2c_client	*i2c_client;
	const struct i2c_device_id *id;
	struct v4l2_subdev	*subdev;
	struct device		*ser_dev;
	struct device		*dser_dev;
	struct gmsl_link_ctx	g_ctx;
	u32	frame_length;
	struct camera_common_data	*s_data;
	struct tegracam_device		*tc_dev;
	int vdig_supply;
	int pwdn_gpio;
};

static const struct regmap_config sensor_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.cache_type = REGCACHE_RBTREE,
};

static int test_mode;
module_param(test_mode, int, 0644);

static inline int imx490_read_reg(struct camera_common_data *s_data,
				u16 addr, u8 *val)
{
	int err = 0;
	u32 reg_val = 0;

	err = regmap_read(s_data->regmap, addr, &reg_val);
	*val = reg_val & 0xFF;

	return err;
}

static int imx490_write_reg(struct camera_common_data *s_data,
				u16 addr, u8 val)
{
	int err;
	struct device *dev = s_data->dev;

	err = regmap_write(s_data->regmap, addr, val);
	if (err)
		dev_err(dev, "%s:i2c write failed, 0x%x = %x\n",
			__func__, addr, val);

	return err;
}

static struct mutex serdes_lock__;

static int imx490_gmsl_serdes_setup(struct imx490 *priv)
{
	int err = 0;
	int des_err = 0;
	struct device *dev;

	if (!priv || !priv->ser_dev || !priv->dser_dev || !priv->i2c_client)
		return -EINVAL;

	dev = &priv->i2c_client->dev;

	mutex_lock(&serdes_lock__);

	/* For now no separate power on required for serializer device */
	gmsl_max9296_power_on(priv->dser_dev);

	/* setup serdes addressing and control pipeline */
	err = gmsl_max9296_setup_link(priv->dser_dev, &priv->i2c_client->dev);
	if (err) {
		dev_err(dev, "gmsl deserializer link config failed\n");
		goto error;
	}

	err = gmsl_max9295_setup_control(priv->ser_dev);

	/* proceed even if ser setup failed, to setup deser correctly */
	if (err)
		dev_err(dev, "gmsl serializer setup failed\n");

	des_err = gmsl_max9296_setup_control(priv->dser_dev, &priv->i2c_client->dev);
	if (des_err) {
		dev_err(dev, "gmsl deserializer setup failed\n");
		/* overwrite err only if deser setup also failed */
		err = des_err;
	}

error:
	mutex_unlock(&serdes_lock__);
	return err;
}

static void imx490_gmsl_serdes_reset(struct imx490 *priv)
{
	mutex_lock(&serdes_lock__);

	/* reset serdes addressing and control pipeline */
	gmsl_max9295_reset_control(priv->ser_dev);
	gmsl_max9296_reset_control(priv->dser_dev, &priv->i2c_client->dev);

	gmsl_max9296_power_off(priv->dser_dev);

	mutex_unlock(&serdes_lock__);
}

static int imx490_power_on(struct camera_common_data *s_data)
{
	int err = 0;
	struct camera_common_power_rail *pw = s_data->power;
	struct camera_common_pdata *pdata = s_data->pdata;
	struct device *dev = s_data->dev;

	dev_info(dev, "%s: power on\n", __func__);
	if (pdata && pdata->power_on) {
		err = pdata->power_on(pw);
		if (err)
			dev_err(dev, "%s failed.\n", __func__);
		else
			pw->state = SWITCH_ON;
		return err;
	}

	pw->state = SWITCH_ON;

	return 0;
}

static int imx490_power_off(struct camera_common_data *s_data)
{
	int err = 0;
	struct camera_common_power_rail *pw = s_data->power;
	struct camera_common_pdata *pdata = s_data->pdata;
	struct device *dev = s_data->dev;

	dev_info(dev, "%s:\n", __func__);

	if (pdata && pdata->power_off) {
		err = pdata->power_off(pw);
		if (!err)
			goto power_off_done;
		else
			dev_err(dev, "%s failed.\n", __func__);
		return err;
	}

power_off_done:
	pw->state = SWITCH_OFF;

	return 0;
}

static int imx490_power_get(struct tegracam_device *tc_dev)
{
	struct device *dev = tc_dev->dev;
	struct camera_common_data *s_data = tc_dev->s_data;
	struct camera_common_power_rail *pw = s_data->power;
	struct camera_common_pdata *pdata = s_data->pdata;
	const char *mclk_name;
	const char *parentclk_name;
	struct clk *parent;
	int err = 0;

	mclk_name = pdata->mclk_name ?
		    pdata->mclk_name : "cam_mclk1";
	pw->mclk = devm_clk_get(dev, mclk_name);
	if (IS_ERR(pw->mclk)) {
		dev_err(dev, "unable to get clock %s\n", mclk_name);
		return PTR_ERR(pw->mclk);
	}

	parentclk_name = pdata->parentclk_name;
	if (parentclk_name) {
		parent = devm_clk_get(dev, parentclk_name);
		if (IS_ERR(parent)) {
			dev_err(dev, "unable to get parent clcok %s",
				parentclk_name);
		} else
			clk_set_parent(pw->mclk, parent);
	}

	pw->state = SWITCH_OFF;

	return err;
}

static int imx490_power_put(struct tegracam_device *tc_dev)
{
	struct camera_common_data *s_data = tc_dev->s_data;
	struct camera_common_power_rail *pw = s_data->power;

	if (unlikely(!pw))
		return -EFAULT;

	return 0;
}

static int imx490_set_group_hold(struct tegracam_device *tc_dev, bool val)
{
	struct device *dev = tc_dev->dev;

	dev_info(dev, "%s\n",  __func__);

	return 0;
}

static int imx490_set_gain(struct tegracam_device *tc_dev, s64 val)
{
	struct device *dev = tc_dev->dev;

	dev_info(dev, "[%s]: Gain control is not avilable yet.\n",  __func__);

	return 0;
}

static int imx490_set_frame_rate(struct tegracam_device *tc_dev, s64 val)
{
	struct imx490 *priv = (struct imx490 *)tegracam_get_privdata(tc_dev);
	struct device *dev = tc_dev->dev;

	dev_info(dev, "%s, %lld\n",  __func__, val);

	/* fixed 30fps */
	priv->frame_length = IMX490_DEFAULT_FRAME_LENGTH;

	return 0;
}

static int imx490_set_exposure(struct tegracam_device *tc_dev, s64 val)
{
	struct device *dev = tc_dev->dev;

	dev_info(dev, "[%s]: Setting auto exposure mode is not avilable.\n",  __func__);

	return 0;
}

static struct tegracam_ctrl_ops imx490_ctrl_ops = {
	.numctrls = ARRAY_SIZE(ctrl_cid_list),
	.ctrl_cid_list = ctrl_cid_list,
	.set_gain = imx490_set_gain,
	.set_exposure = imx490_set_exposure,
	.set_exposure_short = imx490_set_exposure,
	.set_frame_rate = imx490_set_frame_rate,
	.set_group_hold = imx490_set_group_hold,
};

static struct camera_common_pdata
*imx490_parse_dt(struct tegracam_device *tc_dev)
{
	struct device *dev = tc_dev->dev;
	struct device_node *node = dev->of_node;
	struct camera_common_pdata *board_priv_pdata;
	const struct of_device_id *match;
	int err;

	if (!node)
		return NULL;

	match = of_match_device(imx490_of_match, dev);
	if (!match) {
		dev_err(dev, "Failed to find matching dt id\n");
		return NULL;
	}

	board_priv_pdata = devm_kzalloc(dev,
		sizeof(*board_priv_pdata), GFP_KERNEL);

	err = of_property_read_string(node, "mclk",
				      &board_priv_pdata->mclk_name);
	if (err)
		dev_err(dev, "mclk not in DT\n");

	return board_priv_pdata;
}

static int imx490_set_mode(struct tegracam_device *tc_dev)
{
	struct camera_common_data *s_data = tc_dev->s_data;
	struct device *dev = tc_dev->dev;

	dev_info(dev, "%s: %d\n", __func__, s_data->mode_prop_idx);

	return 0;
}

static int imx490_start_streaming(struct tegracam_device *tc_dev)
{
	struct imx490 *priv = (struct imx490 *)tegracam_get_privdata(tc_dev);
	struct device *dev = tc_dev->dev;
	int err;

	dev_info(dev, "imx490_start_streaming.\n");

	return 0;

	/* enable serdes streaming */
	err = gmsl_max9295_setup_streaming(priv->ser_dev);
	if (err)
		goto exit;
	err = gmsl_max9296_setup_streaming(priv->dser_dev, dev);
	if (err)
		goto exit;
	err = gmsl_max9296_start_streaming(priv->dser_dev, dev);
	if (err)
		goto exit;

	msleep(20);

	dev_info(dev, "imx490_start_streaming success.\n");
	return 0;

exit:
	dev_err(dev, "%s: error setting stream\n", __func__);

	return err;
}

static int imx490_stop_streaming(struct tegracam_device *tc_dev)
{
	struct device *dev = tc_dev->dev;
	struct imx490 *priv = (struct imx490 *)tegracam_get_privdata(tc_dev);

	dev_info(dev, "%s:\n", __func__);

	return 0;

	/* disable serdes streaming */
	gmsl_max9296_stop_streaming(priv->dser_dev, dev);

	return 0;
}

static struct camera_common_sensor_ops imx490_common_ops = {
	.numfrmfmts = ARRAY_SIZE(imx490_frmfmt),
	.frmfmt_table = imx490_frmfmt,
	.power_on = imx490_power_on,
	.power_off = imx490_power_off,
	.write_reg = imx490_write_reg,
	.read_reg = imx490_read_reg,
	.parse_dt = imx490_parse_dt,
	.power_get = imx490_power_get,
	.power_put = imx490_power_put,
	.set_mode = imx490_set_mode,
	.start_streaming = imx490_start_streaming,
	.stop_streaming = imx490_stop_streaming,
};

static int imx490_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	dev_dbg(&client->dev, "%s:\n", __func__);

	return 0;
}

static const struct v4l2_subdev_internal_ops imx490_subdev_internal_ops = {
	.open = imx490_open,
};

static int imx490_board_setup(struct imx490 *priv)
{
	struct tegracam_device *tc_dev = priv->tc_dev;
	struct device *dev = tc_dev->dev;
	struct device_node *node = dev->of_node;
	struct device_node *ser_node;
	struct i2c_client *ser_i2c = NULL;
	struct device_node *dser_node;
	struct i2c_client *dser_i2c = NULL;
	struct device_node *gmsl;
	int value = 0xFFFF;
	const char *str_value;
	const char *str_value1[2];
	int  i;
	int err;

	err = of_property_read_u32(node, "reg", &priv->g_ctx.sdev_reg);
	if (err < 0) {
		dev_err(dev, "reg not found\n");
		goto error;
	}

	err = of_property_read_u32(node, "def-addr",
					&priv->g_ctx.sdev_def);
	if (err < 0) {
		dev_err(dev, "def-addr not found\n");
		goto error;
	}

	ser_node = of_parse_phandle(node, "nvidia,gmsl-ser-device", 0);
	if (ser_node == NULL) {
		dev_err(dev,
			"missing %s handle\n",
				"nvidia,gmsl-ser-device");
		goto error;
	}

	err = of_property_read_u32(ser_node, "reg", &priv->g_ctx.ser_reg);
	if (err < 0) {
		dev_err(dev, "serializer reg not found\n");
		goto error;
	}

	ser_i2c = of_find_i2c_device_by_node(ser_node);
	of_node_put(ser_node);

	if (ser_i2c == NULL) {
		err = -EPROBE_DEFER;
		goto error;
	}
	if (ser_i2c->dev.driver == NULL) {
		dev_err(dev, "missing serializer driver\n");
		goto error;
	}

	priv->ser_dev = &ser_i2c->dev;

	dser_node = of_parse_phandle(node, "nvidia,gmsl-dser-device", 0);
	if (dser_node == NULL) {
		dev_err(dev,
			"missing %s handle\n",
				"nvidia,gmsl-dser-device");
		goto error;
	}

	priv->vdig_supply = of_get_named_gpio(node, "vdig-supply", 0);
	if (priv->vdig_supply < 0) {
		dev_err(dev, "vdig-supply not found %d\n", err);
		err = -EINVAL;
		goto error;
	}
	gpio_set_value(priv->vdig_supply, 0);
	msleep(20);

	priv->pwdn_gpio = of_get_named_gpio(node, "pwdn-gpio", 0);
	if (priv->pwdn_gpio < 0) {
		dev_err(dev, "pwdn-gpio not found %d\n", err);
		err = -EINVAL;
		goto error;
	}
	gpio_set_value(priv->pwdn_gpio, 1);
	msleep(20);

	dser_i2c = of_find_i2c_device_by_node(dser_node);
	of_node_put(dser_node);

	if (dser_i2c == NULL) {
		err = -EPROBE_DEFER;
		goto error;
	}
	if (dser_i2c->dev.driver == NULL) {
		dev_err(dev, "missing deserializer driver\n");
		goto error;
	}

	priv->dser_dev = &dser_i2c->dev;

	/* populate g_ctx from DT */
	gmsl = of_get_child_by_name(node, "gmsl-link");
	if (gmsl == NULL) {
		dev_err(dev, "missing gmsl-link device node\n");
		err = -EINVAL;
		goto error;
	}

	err = of_property_read_string(gmsl, "dst-csi-port", &str_value);
	if (err < 0) {
		dev_err(dev, "No dst-csi-port found\n");
		goto error;
	}
	priv->g_ctx.dst_csi_port =
		(!strcmp(str_value, "a")) ? GMSL_CSI_PORT_A : GMSL_CSI_PORT_B;

	err = of_property_read_string(gmsl, "src-csi-port", &str_value);
	if (err < 0) {
		dev_err(dev, "No src-csi-port found\n");
		goto error;
	}
	priv->g_ctx.src_csi_port =
		(!strcmp(str_value, "a")) ? GMSL_CSI_PORT_A : GMSL_CSI_PORT_B;

	err = of_property_read_string(gmsl, "csi-mode", &str_value);
	if (err < 0) {
		dev_err(dev, "No csi-mode found\n");
		goto error;
	}

	if (!strcmp(str_value, "1x4")) {
		priv->g_ctx.csi_mode = GMSL_CSI_1X4_MODE;
	} else if (!strcmp(str_value, "2x4")) {
		priv->g_ctx.csi_mode = GMSL_CSI_2X4_MODE;
	} else if (!strcmp(str_value, "4x2")) {
		priv->g_ctx.csi_mode = GMSL_CSI_4X2_MODE;
	} else if (!strcmp(str_value, "2x2")) {
		priv->g_ctx.csi_mode = GMSL_CSI_2X2_MODE;
	} else {
		dev_err(dev, "invalid csi mode\n");
		goto error;
	}

	err = of_property_read_string(gmsl, "serdes-csi-link", &str_value);
	if (err < 0) {
		dev_err(dev, "No serdes-csi-link found\n");
		goto error;
	}
	priv->g_ctx.serdes_csi_link =
		(!strcmp(str_value, "a")) ?
			GMSL_SERDES_CSI_LINK_A : GMSL_SERDES_CSI_LINK_B;

	err = of_property_read_u32(gmsl, "st-vc", &value);
	if (err < 0) {
		dev_err(dev, "No st-vc info\n");
		goto error;
	}
	priv->g_ctx.st_vc = value;

	err = of_property_read_u32(gmsl, "vc-id", &value);
	if (err < 0) {
		dev_err(dev, "No vc-id info\n");
		goto error;
	}
	priv->g_ctx.dst_vc = value;

	err = of_property_read_u32(gmsl, "num-lanes", &value);
	if (err < 0) {
		dev_err(dev, "No num-lanes info\n");
		goto error;
	}
	priv->g_ctx.num_csi_lanes = value;

	priv->g_ctx.num_streams =
			of_property_count_strings(gmsl, "streams");
	if (priv->g_ctx.num_streams <= 0) {
		dev_err(dev, "No streams found\n");
		err = -EINVAL;
		goto error;
	}

	for (i = 0; i < priv->g_ctx.num_streams; i++) {
		of_property_read_string_index(gmsl, "streams", i,
						&str_value1[i]);
		if (!str_value1[i]) {
			dev_err(dev, "invalid stream info\n");
			goto error;
		}
		if (!strcmp(str_value1[i], "yuv8")) {
			priv->g_ctx.streams[i].st_data_type =
							GMSL_CSI_DT_YUV422;
		} else if (!strcmp(str_value1[i], "embed")) {
			priv->g_ctx.streams[i].st_data_type =
							GMSL_CSI_DT_EMBED;
		} else if (!strcmp(str_value1[i], "ued-u1")) {
			priv->g_ctx.streams[i].st_data_type =
							GMSL_CSI_DT_UED_U1;
		} else {
			dev_err(dev, "invalid stream data type\n");
			goto error;
		}
	}

	priv->g_ctx.s_dev = dev;

	return 0;

error:
	return err;
}

static int imx490_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct device_node *node = dev->of_node;
	struct tegracam_device *tc_dev;
	struct imx490 *priv;
	int err;

	dev_info(dev, "probing v4l2 sensor.\n");

	if (!IS_ENABLED(CONFIG_OF) || !node)
		return -EINVAL;

	priv = devm_kzalloc(dev, sizeof(struct imx490), GFP_KERNEL);
	if (!priv) {
		dev_err(dev, "unable to allocate memory!\n");
		return -ENOMEM;
	}
	tc_dev = devm_kzalloc(dev,
			sizeof(struct tegracam_device), GFP_KERNEL);
	if (!tc_dev)
		return -ENOMEM;

	priv->i2c_client = tc_dev->client = client;
	tc_dev->dev = dev;
	strncpy(tc_dev->name, "imx490", sizeof(tc_dev->name));
	tc_dev->dev_regmap_config = &sensor_regmap_config;
	tc_dev->sensor_ops = &imx490_common_ops;
	tc_dev->v4l2sd_internal_ops = &imx490_subdev_internal_ops;
	tc_dev->tcctrl_ops = &imx490_ctrl_ops;

	err = tegracam_device_register(tc_dev);
	if (err) {
		dev_err(dev, "tegra camera driver registration failed\n");
		return err;
	}

	priv->tc_dev = tc_dev;
	priv->s_data = tc_dev->s_data;
	priv->subdev = &tc_dev->s_data->subdev;

	tegracam_set_privdata(tc_dev, (void *)priv);

	err = imx490_board_setup(priv);
	if (err) {
		tegracam_device_unregister(tc_dev);
		dev_err(dev, "board setup failed\n");
		return err;
	}

	/* Pair sensor to serializer dev */
	err = gmsl_max9295_sdev_pair(priv->ser_dev, &priv->g_ctx);
	if (err) {
		dev_err(&client->dev, "gmsl ser pairing failed\n");
		return err;
	}

	/* Register sensor to deserializer dev */
	err = gmsl_max9296_sdev_register(priv->dser_dev, &priv->g_ctx);
	if (err) {
		dev_err(&client->dev, "gmsl deserializer register failed\n");
		return err;
	}

	/*
	 * gmsl serdes setup
	 *
	 * Sensor power on/off should be the right place for serdes
	 * setup/reset. But the problem is, the total required delay
	 * in serdes setup/reset exceeds the frame wait timeout, looks to
	 * be related to multiple channel open and close sequence
	 * issue (#BUG 200477330).
	 * Once this bug is fixed, these may be moved to power on/off.
	 * The delays in serdes is as per guidelines and can't be reduced,
	 * so it is placed in probe/remove, though for that, deserializer
	 * would be powered on always post boot, until 1.2v is supplied
	 * to deserializer from CVB.
	 */
	err = imx490_gmsl_serdes_setup(priv);
	if (err) {
		dev_err(&client->dev,
			"%s gmsl serdes setup failed\n", __func__);
		return err;
	}

	err = tegracam_v4l2subdev_register(tc_dev, true);
	if (err) {
		dev_err(dev, "tegra camera subdev registration failed\n");
		return err;
	}

	dev_info(&client->dev, "Detected IMX490 sensor\n");

	return 0;
}

static int imx490_remove(struct i2c_client *client)
{
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct imx490 *priv = (struct imx490 *)s_data->priv;

	imx490_gmsl_serdes_reset(priv);

	tegracam_v4l2subdev_unregister(priv->tc_dev);
	tegracam_device_unregister(priv->tc_dev);

	return 0;
}

static const struct i2c_device_id imx490_id[] = {
	{ "imx490", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, imx490_id);

static struct i2c_driver imx490_i2c_driver = {
	.driver = {
		.name = "imx490",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(imx490_of_match),
	},
	.probe = imx490_probe,
	.remove = imx490_remove,
	.id_table = imx490_id,
};

static int __init imx490_init(void)
{
	mutex_init(&serdes_lock__);

	return i2c_add_driver(&imx490_i2c_driver);
}

static void __exit imx490_exit(void)
{
	mutex_destroy(&serdes_lock__);

	i2c_del_driver(&imx490_i2c_driver);
}

module_init(imx490_init);
module_exit(imx490_exit);

MODULE_DESCRIPTION("Media Controller driver for Sony IMX490");
MODULE_AUTHOR("Autra.tech Corporation");
MODULE_LICENSE("GPL v2");
