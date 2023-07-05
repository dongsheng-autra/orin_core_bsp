
#ifndef __GMSL_MAX96712_H__
#define __GMSL_MAX96712_H__

#include "gmsl_link_priv.h"

int gmsl_max96712_setup_link(struct device *dev, struct device *s_dev);

int gmsl_max96712_setup_control(struct device *dev, struct device *s_dev);

int gmsl_max96712_reset_control(struct device *dev, struct device *s_dev);

int gmsl_max96712_sdev_register(struct device *dev, struct gmsl_link_ctx *g_ctx);

int gmsl_max96712_sdev_unregister(struct device *dev, struct device *s_dev);

int gmsl_max96712_setup_streaming(struct device *dev, struct device *s_dev);

int gmsl_max96712_start_streaming(struct device *dev, struct device *s_dev);

int gmsl_max96712_stop_streaming(struct device *dev, struct device *s_dev);

int gmsl_max96712_power_on(struct device *dev);

void gmsl_max96712_power_off(struct device *dev);

#endif  /* __GMSL_MAX96712_H__ */
