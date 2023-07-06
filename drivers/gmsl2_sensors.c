
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

enum {
	GMSL2_SENSOR_MODE_CROP_NFPS,
	GMSL2_SENSOR_MODE_START_STREAM,
	GMSL2_SENSOR_MODE_STOP_STREAM,
};

static const int gmsl2_sensor_30fps[] = {
	30,
};

static const struct camera_common_frmfmt gmsl2_sensor_frmfmt[] = {
	{{2880, 1860}, gmsl2_sensor_30fps, 1, 0, GMSL2_SENSOR_MODE_CROP_NFPS},
};

static const struct of_device_id gmsl2_sensor_of_match[] = {
	{ .compatible = "sensing,gmsl2_sensor",},
	{ },
};
MODULE_DEVICE_TABLE(of, gmsl2_sensor_of_match);

static const u32 ctrl_cid_list[] = {
	TEGRA_CAMERA_CID_GAIN,
	TEGRA_CAMERA_CID_EXPOSURE,
	TEGRA_CAMERA_CID_EXPOSURE_SHORT,
	TEGRA_CAMERA_CID_FRAME_RATE,
	TEGRA_CAMERA_CID_HDR_EN,
};

struct gmsl2_sensor {
	struct i2c_client	*i2c_client;
	const struct i2c_device_id *id;
	struct v4l2_subdev	*subdev;
	struct device		*ser_dev;
	struct device		*dser_dev;
	u32	frame_length;
	struct camera_common_data	*s_data;
	struct tegracam_device		*tc_dev;
	int power_supply;
	int pwdn_gpio;
	int sync_gpio;
	int pwdn_gpio0;
	int pwdn_gpio1;
};

static const struct regmap_config sensor_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.cache_type = REGCACHE_RBTREE,
};

static int test_mode;
module_param(test_mode, int, 0644);

static inline int gmsl2_sensor_read_reg(struct camera_common_data *s_data,
				u16 addr, u8 *val)
{
	int err = 0;
	u32 reg_val = 0;

	err = regmap_read(s_data->regmap, addr, &reg_val);
	*val = reg_val & 0xFF;

	return err;
}

static int gmsl2_sensor_write_reg(struct camera_common_data *s_data,
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

static int gmsl2_sensor_power_on(struct camera_common_data *s_data)
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

static int gmsl2_sensor_power_off(struct camera_common_data *s_data)
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

static int gmsl2_sensor_power_get(struct tegracam_device *tc_dev)
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

static int gmsl2_sensor_power_put(struct tegracam_device *tc_dev)
{
	struct camera_common_data *s_data = tc_dev->s_data;
	struct camera_common_power_rail *pw = s_data->power;

	if (unlikely(!pw))
		return -EFAULT;

	return 0;
}

static int gmsl2_sensor_set_group_hold(struct tegracam_device *tc_dev, bool val)
{
	struct device *dev = tc_dev->dev;

	dev_info(dev, "%s\n",  __func__);

	return 0;
}

static int gmsl2_sensor_set_gain(struct tegracam_device *tc_dev, s64 val)
{
	struct device *dev = tc_dev->dev;

	dev_info(dev, "[%s]: Gain control is not avilable yet.\n",  __func__);

	return 0;
}

static int gmsl2_sensor_set_frame_rate(struct tegracam_device *tc_dev, s64 val)
{
	struct device *dev = tc_dev->dev;

	dev_info(dev, "%s, %lld\n",  __func__, val);

	return 0;
}

static int gmsl2_sensor_set_exposure(struct tegracam_device *tc_dev, s64 val)
{
	struct device *dev = tc_dev->dev;

	dev_info(dev, "[%s]: Setting auto exposure mode is not avilable.\n",  __func__);

	return 0;
}

static struct tegracam_ctrl_ops gmsl2_sensor_ctrl_ops = {
	.numctrls = ARRAY_SIZE(ctrl_cid_list),
	.ctrl_cid_list = ctrl_cid_list,
	.set_gain = gmsl2_sensor_set_gain,
	.set_exposure = gmsl2_sensor_set_exposure,
	.set_exposure_short = gmsl2_sensor_set_exposure,
	.set_frame_rate = gmsl2_sensor_set_frame_rate,
	.set_group_hold = gmsl2_sensor_set_group_hold,
};

static struct camera_common_pdata *gmsl2_sensor_parse_dt(struct tegracam_device *tc_dev)
{
	struct device *dev = tc_dev->dev;
	struct device_node *node = dev->of_node;
	struct camera_common_pdata *board_priv_pdata;
	const struct of_device_id *match;
	int err;

	if (!node)
		return NULL;

	match = of_match_device(gmsl2_sensor_of_match, dev);
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

static int gmsl2_sensor_set_mode(struct tegracam_device *tc_dev)
{
	struct camera_common_data *s_data = tc_dev->s_data;
	struct device *dev = tc_dev->dev;

	dev_info(dev, "%s: %d\n", __func__, s_data->mode_prop_idx);

	return 0;
}

static int gmsl2_sensor_start_streaming(struct tegracam_device *tc_dev)
{
	struct device *dev = tc_dev->dev;

	dev_info(dev, "sensors_start_streaming.\n");

	return 0;
}

static int gmsl2_sensor_stop_streaming(struct tegracam_device *tc_dev)
{
	struct device *dev = tc_dev->dev;

	dev_info(dev, "%s:\n", __func__);

	return 0;
}

static struct camera_common_sensor_ops gmsl2_sensor_common_ops = {
	.numfrmfmts = ARRAY_SIZE(gmsl2_sensor_frmfmt),
	.frmfmt_table = gmsl2_sensor_frmfmt,
	.power_on = gmsl2_sensor_power_on,
	.power_off = gmsl2_sensor_power_off,
	.write_reg = gmsl2_sensor_write_reg,
	.read_reg = gmsl2_sensor_read_reg,
	.parse_dt = gmsl2_sensor_parse_dt,
	.power_get = gmsl2_sensor_power_get,
	.power_put = gmsl2_sensor_power_put,
	.set_mode = gmsl2_sensor_set_mode,
	.start_streaming = gmsl2_sensor_start_streaming,
	.stop_streaming = gmsl2_sensor_stop_streaming,
};

static int gmsl2_sensor_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	dev_dbg(&client->dev, "%s:\n", __func__);

	return 0;
}

static const struct v4l2_subdev_internal_ops gmsl2_sensor_subdev_internal_ops = {
	.open = gmsl2_sensor_open,
};

static int gmsl2_sensor_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct device_node *node = dev->of_node;
	struct tegracam_device *tc_dev;
	struct gmsl2_sensor *priv;
	int err;

	dev_info(dev, "probing v4l2 sensor.\n");

	if (!IS_ENABLED(CONFIG_OF) || !node)
		return -EINVAL;

	priv = devm_kzalloc(dev, sizeof(struct gmsl2_sensor), GFP_KERNEL);
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
	strncpy(tc_dev->name, "gmsl2_sensor", sizeof(tc_dev->name));
	tc_dev->dev_regmap_config = &sensor_regmap_config;
	tc_dev->sensor_ops = &gmsl2_sensor_common_ops;
	tc_dev->v4l2sd_internal_ops = &gmsl2_sensor_subdev_internal_ops;
	tc_dev->tcctrl_ops = &gmsl2_sensor_ctrl_ops;

	err = tegracam_device_register(tc_dev);
	if (err) {
		dev_err(dev, "tegra camera driver registration failed\n");
		return err;
	}

	priv->tc_dev = tc_dev;
	priv->s_data = tc_dev->s_data;
	priv->subdev = &tc_dev->s_data->subdev;

	tegracam_set_privdata(tc_dev, (void *)priv);

	err = tegracam_v4l2subdev_register(tc_dev, true);
	if (err) {
		dev_err(dev, "tegra camera subdev registration failed\n");
		return err;
	}

	dev_info(&client->dev, "Detected GMSL2_SENSOR sensor\n");

	return 0;
}

static int gmsl2_sensor_remove(struct i2c_client *client)
{
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct gmsl2_sensor *priv = (struct gmsl2_sensor *)s_data->priv;

	tegracam_v4l2subdev_unregister(priv->tc_dev);
	tegracam_device_unregister(priv->tc_dev);

	return 0;
}

static const struct i2c_device_id gmsl2_sensor_id[] = {
	{ "gmsl2_sensor", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, gmsl2_sensor_id);

static struct i2c_driver gmsl2_sensor_i2c_driver = {
	.driver = {
		.name = "gmsl2_sensor",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(gmsl2_sensor_of_match),
	},
	.probe = gmsl2_sensor_probe,
	.remove = gmsl2_sensor_remove,
	.id_table = gmsl2_sensor_id,
};

static int __init gmsl2_sensor_init(void)
{
	mutex_init(&serdes_lock__);

	return i2c_add_driver(&gmsl2_sensor_i2c_driver);
}

static void __exit gmsl2_sensor_exit(void)
{
	mutex_destroy(&serdes_lock__);

	i2c_del_driver(&gmsl2_sensor_i2c_driver);
}

module_init(gmsl2_sensor_init);
module_exit(gmsl2_sensor_exit);

MODULE_DESCRIPTION("Media Controller driver for Sensing GMSL sensors");
MODULE_AUTHOR("Autra.tech Corporation");
MODULE_LICENSE("GPL v2");
