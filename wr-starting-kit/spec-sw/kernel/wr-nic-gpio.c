/*
 * Copyright (C) 2012 CERN (www.cern.ch)
 * Author: Alessandro Rubini <rubini@gnudd.com>
 *
 * Released according to the GNU GPL, version 2 or any later version.
 *
 * This work is part of the White Rabbit project, a research effort led
 * by CERN, the European Institute for Nuclear Research.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/gpio/driver.h>  // Needed for gpiochip_get_dev()
#include <linux/fmc.h>
#include <linux/version.h>
#include "spec-nic.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,14,0)
static inline struct device *gpiochip_get_dev(struct gpio_chip *gc)
{
	return gc->parent;
}
#endif

static inline struct fmc_device *gc_to_fmc(struct gpio_chip *gc)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,14,0)
	struct device *dev = gpiochip_get_dev(gc);
#else
	struct device *dev = gc->dev;
#endif
	return container_of(dev, struct fmc_device, dev);
}

static int wrn_gpio_input(struct gpio_chip *chip, unsigned offset)
{
	//struct fmc_device *fmc = gc_to_fmc(chip);
	//struct wrn_drvdata *dd = fmc_get_drvdata(fmc);

	//fmc_writel(fmc, ...); /*  FIXME  */
	return -EAGAIN;
}

static int wrn_gpio_output(struct gpio_chip *chip, unsigned offset, int value)
{
	return -EAGAIN;
}

int wrn_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	return -EAGAIN;
}

void wrn_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	return;
}

static const char *wrn_gpio_names[] = {
	"dire", "fare", "baciare", "lettera", "testamento"
};

static struct gpio_chip wrn_gpio_template = {
	.label = "wr-nic",
	.owner = THIS_MODULE,
	/* FIXME: request, free, for multi-function operation */
	.direction_input = wrn_gpio_input,
	.direction_output = wrn_gpio_output,
	.get = wrn_gpio_get,
	.set = wrn_gpio_set,
	.base = -1, /* request dynamic */
	.ngpio = 5,
	.names = wrn_gpio_names,
};

int wrn_gpio_init(struct fmc_device *fmc)
{
	struct wrn_drvdata *dd = fmc_get_drvdata(fmc);
	struct gpio_chip *gc;
	int ret;
	printk("wr-nic-gpio.c:%s: start &drvdata=%p\n",__func__,(void*)dd);

	gc = devm_kzalloc(&fmc->dev, sizeof(*gc), GFP_KERNEL);
	if (!gc)
		return -ENOMEM;
	*gc = wrn_gpio_template;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,14,0)
	gc->parent = &fmc->dev;
#else
	gc->dev = &fmc->dev;
#endif

	printk("wr-nic-gpio.c:%s: before gpiochip_add(gc=%p)\n",__func__,(void*)gc);
	ret = gpiochip_add(gc);
	if (ret < 0) {
		printk("wr-nic-gpio.c:%s: gpiochip_add ERROR\n",__func__);
		goto out_free;
	}
	dd->gc = gc;

	/* FIXME: program the DAC for each port (sysfs attributes?) */
	printk("wr-nic-gpio.c:%s: finished OK with (%p)dd->gc=gc(%p)\n",__func__,(void*)dd,(void*)gc);
	return 0;

out_free:
	printk("wr-nic-gpio.c:%s: finished ERROR ret=%d gc=%p\n",__func__,ret,(void*)gc);
	kfree(gc);
	return ret;
}

void wrn_gpio_exit(struct fmc_device *fmc)
{
	struct wrn_drvdata *dd = fmc_get_drvdata(fmc);
	struct gpio_chip *gc = dd->gc;
	printk("wr-nic-gpio.c:%s: start - trying gpiochip_remove - NEED TO SEE\n",__func__);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,14,0)
	gpiochip_remove(gc);
#else
	int ret = gpiochip_remove(gc);
	if (ret)
		dev_err(fmc->hwdev, "DANGER %i! gpio chip can't be removed\n",
			ret);
#endif
	return;
}
