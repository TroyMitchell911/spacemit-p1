// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024 Troy Mitchell <troymitchell988@gmail.com>
 */
#include "linux/err.h"
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/of_regulator.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>
#include <linux/mfd/spacemit/spacemit-pmic.h>

static const struct regulator_ops pmic_dcdc_ldo_ops = {
	.list_voltage		= regulator_list_voltage_linear_range,
	.map_voltage		= regulator_map_voltage_linear_range,
	.get_voltage_sel	= regulator_get_voltage_sel_regmap,
	.set_voltage_sel	= regulator_set_voltage_sel_regmap,
	.set_voltage_time_sel	= regulator_set_voltage_time_sel,
	.enable			= regulator_enable_regmap,
	.disable		= regulator_disable_regmap,
	.is_enabled		= regulator_is_enabled_regmap,
};

static const struct regulator_ops pmic_switch_ops = {
	.enable			= regulator_enable_regmap,
	.disable		= regulator_disable_regmap,
	.is_enabled		= regulator_is_enabled_regmap,
};

static int spacemit_regulator_probe(struct platform_device *pdev)
{
	const struct spacemit_regulator_match_data *match_data;
	struct regulator_config config = {};
	struct i2c_client *client;
	struct regulator_dev *regulator_dev;
	struct spacemit_pmic *pmic;
	int i;

	pmic = dev_get_drvdata(pdev->dev.parent);
	
	match_data = of_device_get_match_data(&pdev->dev);
	if (WARN_ON(!match_data))
		return -EINVAL;

	client = pmic->i2c;
	config.dev = &client->dev;
	config.regmap = pmic->regmap;

	for (i = 0; i < match_data->nr_desc; ++i) {
		regulator_dev = devm_regulator_register(&pdev->dev, match_data->desc + i, &config);
		if (IS_ERR(regulator_dev))
			return dev_err_probe(&client->dev, PTR_ERR(regulator_dev), "failed to register %d regulator", i);
	}

	return 0;
}

static const struct of_device_id spacemit_regulator_of_match[] = {
	/*just NULL now*/
	{ .compatible = "pmic,regulator,p1", .data = (void *)NULL},
	{ },
};
MODULE_DEVICE_TABLE(of, spacemit_regulator_of_match);

static struct platform_driver spacemit_regulator_driver = {
	.probe = spacemit_regulator_probe,
	.driver = {
		.name = "spacemit-regulator",
		.of_match_table = spacemit_regulator_of_match,
	},
};

static int spacemit_regulator_init(void)
{
	return platform_driver_register(&spacemit_regulator_driver);
}
subsys_initcall(spacemit_regulator_init);

MODULE_DESCRIPTION("regulator drivers for the SpacemiT PMIC");
MODULE_LICENSE("GPL");
