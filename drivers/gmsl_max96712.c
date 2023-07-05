
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <media/camera_common.h>
#include <linux/module.h>

#include "gmsl_max96712.h"

#define MAX96712_CHIP_ID				0xA0
#define MAX96712_REG_CHIP_ID			0x0D

struct max96712 {
	struct i2c_client *i2c_client;
	struct regmap *regmap;
	struct mutex lock;
};

static int max96712_write_reg(struct device *dev,
	u16 addr, u8 val)
{
	struct max96712 *priv;
	int err;

	priv = dev_get_drvdata(dev);

	err = regmap_write(priv->regmap, addr, val);
	if (err)
		dev_err(dev, "%s:i2c write failed, 0x%x = %x\n",
		__func__, addr, val);

	dev_info(dev, "%s:i2c-w, 0x%x = %x\n",
		__func__, addr, val);

	/* delay before next i2c command as required for SERDES link */
	usleep_range(100, 110);

	return err;
}

static int max96712_read_reg(struct device *dev,
			u16 addr, u32 *val)
{
	struct max96712 *priv;
	int err;

	priv = dev_get_drvdata(dev);

	err = regmap_read(priv->regmap, addr, val);
	if (err)
		dev_err(dev, "%s:i2c read failed, 0x%x = %x\n",
			__func__, addr, *val);

	dev_info(dev, "%s:i2c-r, 0x%x = %x\n",
		__func__, addr, *val);

	return err;
}

static int max96712_update_bits(struct device *dev, u16 addr,
			u8 mask, u8 bits)
{
	struct max96712 *priv;
	int err;

	priv = dev_get_drvdata(dev);

	err = regmap_update_bits(priv->regmap, addr, mask, bits);
	if (err)
		dev_err(dev, "update 0x%04x failed\n", addr);

	return err;
}

static int max96712_link_check(struct device *dev)
{
	struct max96712 *priv;
	int ret, count = 0;
	u32 tmp;

	priv = dev_get_drvdata(dev);

	ret = max96712_read_reg(dev, MAX96712_REG_CHIP_ID, &tmp);
	if (tmp != MAX96712_CHIP_ID) {
		dev_err(dev, "Unexpected MAX96712 chip id(%02x), ret(%d)\n", tmp, ret);
		return -ENODEV;
	}

	dev_info(dev, "Detected MAX96712 chipid: %02x\n", tmp);

	/* IF VDD = 1.2V: Enable REG_ENABLE and REG_MNL
	 *	CTRL0: Enable REG_ENABLE
	 *	CTRL2: Enable REG_MNL
	 */
	//max96712_update_bits(dev, 0x0017, BIT(2), BIT(2));
	//max96712_update_bits(dev, 0x0019, BIT(4), BIT(4));

	/* CSI output disabled. */
	max96712_write_reg(dev, 0x040B, 0x00);

	/* All links select GMSL2 mode and beginning. */
	max96712_write_reg(dev, 0x0006, 0xFF);

	/* Link A ~ Link D Transmitter Rate: 187.5Mbps, Receiver Rate: 3Gbps */
	max96712_write_reg(dev, 0x0010, 0x11);
	max96712_write_reg(dev, 0x0011, 0x11);

	/* Link A ~ Link D One-Shot Reset */
	//max96712_write_reg(dev, 0x0018, 0x0F);
	//max96712_read_reg(dev, 0x0018, &tmp);
	//max96712_write_reg(dev, 0x0006, 0xFF);

	msleep(100);

	for (count = 0; count < 20; count++) {
		max96712_read_reg(dev, 0x001a, &tmp);
		if (tmp & BIT(3)) {
			dev_info(dev, "LinkA lock success: 0x%x, count: %d\n", tmp, count);
		}

		max96712_read_reg(dev, 0x000a, &tmp);
		if (tmp & BIT(3)) {
			dev_info(dev, "LinkB lock success: 0x%x, count: %d\n", tmp, count);
		}

		max96712_read_reg(dev, 0x000b, &tmp);
		if (tmp & BIT(3)) {
			dev_info(dev, "LinkC lock success: 0x%x, count: %d\n", tmp, count);
		}

		max96712_read_reg(dev, 0x000c, &tmp);
		if (tmp & BIT(3)) {
			dev_info(dev, "LinkD lock success: 0x%x, count: %d\n", tmp, count);
		}
		msleep(10);
	}

	return ret;
}

int gmsl_max96712_setup_link(struct device *dev, struct device *s_dev)
{
	int err;

	err = max96712_link_check(dev);
	if (err) {
		return -EFAULT;
	}

	return 0;
}
EXPORT_SYMBOL(gmsl_max96712_setup_link);

int gmsl_max96712_setup_control(struct device *dev, struct device *s_dev)
{
	return 0;
}
EXPORT_SYMBOL(gmsl_max96712_setup_control);

int gmsl_max96712_reset_control(struct device *dev, struct device *s_dev)
{
	return 0;
}
EXPORT_SYMBOL(gmsl_max96712_reset_control);

int gmsl_max96712_sdev_register(struct device *dev, struct gmsl_link_ctx *g_ctx)
{
	return 0;
}
EXPORT_SYMBOL(gmsl_max96712_sdev_register);

int gmsl_max96712_sdev_unregister(struct device *dev, struct device *s_dev)
{
	return 0;
}
EXPORT_SYMBOL(gmsl_max96712_sdev_unregister);

int gmsl_max96712_setup_streaming(struct device *dev, struct device *s_dev)
{
	return 0;
}
EXPORT_SYMBOL(gmsl_max96712_setup_streaming);

int gmsl_max96712_start_streaming(struct device *dev, struct device *s_dev)
{
	return 0;
}
EXPORT_SYMBOL(gmsl_max96712_start_streaming);

int gmsl_max96712_stop_streaming(struct device *dev, struct device *s_dev)
{
	return 0;
}
EXPORT_SYMBOL(gmsl_max96712_stop_streaming);

int gmsl_max96712_power_on(struct device *dev)
{
	return 0;
}
EXPORT_SYMBOL(gmsl_max96712_power_on);

void gmsl_max96712_power_off(struct device *dev)
{
}
EXPORT_SYMBOL(gmsl_max96712_power_off);

static  struct regmap_config max96712_regmap_config = {
	.reg_bits = 16,
	.val_bits = 8,
	.cache_type = REGCACHE_RBTREE,
};

static int max96712_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct max96712 *priv;
	int err = 0;

	dev_info(&client->dev, "%s: enter\n", __func__);

	priv = devm_kzalloc(&client->dev, sizeof(*priv), GFP_KERNEL);
	priv->i2c_client = client;
	priv->regmap = devm_regmap_init_i2c(priv->i2c_client,
				&max96712_regmap_config);
	if (IS_ERR(priv->regmap)) {
		dev_err(&client->dev,
			"regmap init failed: %ld\n", PTR_ERR(priv->regmap));
		return -ENODEV;
	}

	mutex_init(&priv->lock);

	dev_set_drvdata(&client->dev, priv);

	dev_info(&client->dev, "%s:  success\n", __func__);

	return err;
}


static int max96712_remove(struct i2c_client *client)
{
	struct max96712 *priv;

	if (client != NULL) {
		priv = dev_get_drvdata(&client->dev);
		mutex_destroy(&priv->lock);
		i2c_unregister_device(client);
		client = NULL;
	}

	return 0;
}

static const struct i2c_device_id max96712_id[] = {
	{ "gmsl_max96712", 0 },
	{ },
};

const struct of_device_id max96712_of_match[] = {
	{ .compatible = "maxim,gmsl_max96712", },
	{ },
};

MODULE_DEVICE_TABLE(i2c, max96712_id);

static struct i2c_driver max96712_i2c_driver = {
	.driver = {
		.name = "gmsl_max96712",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(max96712_of_match),
	},
	.probe = max96712_probe,
	.remove = max96712_remove,
	.id_table = max96712_id,
};

static int __init max96712_init(void)
{
	return i2c_add_driver(&max96712_i2c_driver);
}

static void __exit max96712_exit(void)
{
	i2c_del_driver(&max96712_i2c_driver);
}

module_init(max96712_init);
module_exit(max96712_exit);

MODULE_DESCRIPTION("GMSL Deserializer driver gmsl_max96712");
MODULE_AUTHOR("Autra Corporation");
MODULE_LICENSE("GPL v2");
