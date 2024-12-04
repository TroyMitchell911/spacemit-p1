// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2024 Troy Mitchell <troymitchell988@gmail.com>
 */

#include "linux/mfd/spacemit/spacemit-p1.h"
#include <linux/mfd/core.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/of_device.h>
#include <linux/pm_wakeirq.h>
#include <linux/mfd/spacemit/spacemit-pmic.h>

P1_REGMAP_CONFIG;
P1_IRQS_DESC;
P1_IRQ_CHIP_DESC;
P1_POWER_KEY_RESOURCES_DESC;
P1_RTC_RESOURCES_DESC;
P1_MFD_CELL;
P1_MFD_MATCH_DATA;

static struct spacemit_pmic *pmic;

static int spacemit_pmic_probe(struct i2c_client *client)
{
	int nr_cells, ret;
	struct spacemit_pmic_match_data *match_data;
	const struct mfd_cell *cells;
	
	match_data = (struct spacemit_pmic_match_data*)of_device_get_match_data(&client->dev);
	if (WARN_ON(!match_data))
		return -EINVAL;
	
	pmic = devm_kzalloc(&client->dev, sizeof(*pmic), GFP_KERNEL);
	if (!pmic)
		return -ENOMEM;
	
	cells = match_data->mfd_cells;
	nr_cells = match_data->nr_cells;
	
	pmic->regmap_cfg = match_data->regmap_cfg;
	pmic->regmap_irq_chip = match_data->regmap_irq_chip;
	pmic->i2c = client;
	pmic->regmap = devm_regmap_init_i2c(client, pmic->regmap_cfg);
	if (IS_ERR(pmic->regmap))
		return dev_err_probe(&client->dev, PTR_ERR(pmic->regmap), "regmap initialization failed");

	regcache_cache_bypass(pmic->regmap, true);

	i2c_set_clientdata(client, pmic);
	
	if (!client->irq) {
		dev_warn(&client->dev, "No interrupt supported");
	} else {
		if (pmic->regmap_irq_chip) {
			ret = regmap_add_irq_chip(pmic->regmap, client->irq, IRQF_ONESHOT, -1,
						  pmic->regmap_irq_chip, &pmic->irq_data);
			if (ret)
				return dev_err_probe(&client->dev, ret, "failed to add irqchip %d", ret);
		}

		dev_pm_set_wake_irq(&client->dev, client->irq);
		device_init_wakeup(&client->dev, true);
	}

	ret = devm_mfd_add_devices(&client->dev, PLATFORM_DEVID_NONE,
			      cells, nr_cells, NULL, 0,
			      regmap_irq_get_domain(pmic->irq_data));
	if (ret)
		return dev_err_probe(&client->dev, ret, "failed to add MFD devices %d", ret);

	return 0;
}

static const struct of_device_id spacemit_p1_of_match[] = {
	{ .compatible = "spacemit,p1" , .data = &pmic_p1_match_data},
	{ },
};
MODULE_DEVICE_TABLE(of, spacemit_p1_of_match);

static struct i2c_driver spacemit_p1_i2c_driver = {
	.driver = {
		.name = "spacemit-pmic",
		.of_match_table = spacemit_p1_of_match,
	},
	.probe    = spacemit_pmic_probe,
};

static int spacemit_mfd_init(void)
{
	return i2c_add_driver(&spacemit_p1_i2c_driver);
}
subsys_initcall(spacemit_mfd_init);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("driver for the SpacemiT P1 PMIC chip");
