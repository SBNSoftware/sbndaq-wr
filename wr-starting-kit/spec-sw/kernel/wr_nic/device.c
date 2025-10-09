/*
 * Device initialization and cleanup for White-Rabbit switch network interface
 *
 * Copyright (C) 2010 CERN (www.cern.ch)
 * Author: Alessandro Rubini <rubini@gnudd.com>
 * Partly from previous work by Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
 * Partly from previous work by  Emilio G. Cota <cota@braap.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/fmc.h>
#include <linux/io.h>

#include "wr-nic.h"
#include "nic-mem.h"
#include "../spec-nic.h"

/* The remove function is used by probe, so it's not __devexit
   This is referred to as "the platform driver's remove (wrn_remove)" */
static int wrn_remove(struct platform_device *pdev)
{
	struct wrn_drvdata *drvdata = pdev->dev.platform_data;
	struct wrn_dev *wrn = drvdata->wrn;
	int i;
# define STRx( x ) #x
# define XSTRx( x ) #x
	char *ss = XSTRx(WRN_IRQ_NUMBERS);

	printk("device.c:wrn_remove: (start) before stop any transmission\n");

	/* 1. stop the NIC (i.e stop any transmission) */
	writel(0, &wrn->regs->CR);

    /* 2. free the network interfaces that were created */
	printk("device.c:wrn_remove: before wrn_mez....\n");
	/* Then remove devices, memory maps, interrupts */
	for (i = 0; i < WRN_NR_ENDPOINTS; i++) {
		if (wrn->dev[i]) {
			wrn_mezzanine_exit(wrn->dev[i]);
			wrn_endpoint_remove(wrn->dev[i]);
			free_netdev(wrn->dev[i]);
			wrn->dev[i] = NULL;
		}
	}

    /* 3. iounmap the memory regions */
	printk("device.c:wrn_remove: before iounmap\n");
	for (i = 0; i < ARRAY_SIZE(wrn->bases); i++) {
		if (wrn->bases[i]) {
			printk("device.c:wrn_remove: iounmap(%p)\n", (void*)wrn->bases[i]);
			iounmap(wrn->bases[i]);
		}
	}

    /* 4. free the **IRQ** that was requested in wrn_eth_init() */
	printk("device.c:wrn_remove: before Unregister all interrupts\n");
	/* Unregister all interrupts that were registered */
	printk("device.c:wrn_remove: before Unregister... WRN_IRQ_NUMBERS is %s\n", ss );
	printk("device.c:wrn_remove: before Unregister... wrn->irq_registered=0x%x\n",wrn->irq_registered );
    if (drvdata->irq_owned && drvdata->fmc) {
        printk("device.c:wrn_remove: freeing FMC IRQ\n");
        drvdata->fmc->op->irq_free(drvdata->fmc);
		drvdata->irq_owned = false;           /* clear the flag */
    }

	/* 5. remove the GPIO chip --------------------------- */
	printk("device.c:wrn_remove: SHOULD call wrn_gpio_exit(fmc) (%p)drvdata->gc(%p) (SHOULD BE NON-0)\n",
	       drvdata, drvdata->gc);
	if (drvdata->gc) {
        wrn_gpio_exit(drvdata->fmc);   /* strong implementation in wr?nic?gpio.c */
        drvdata->gc = NULL;
    }

    /* 6. tear down the tasklet */
	printk("device.c:wrn_remove: before tasklet_kill\n");
	tasklet_kill( &wrn->rx_tlet );

	printk("device.c:wrn_remove: before return\n");
	return 0;
}

/* This helper is used by probe below */
static int __wrn_map_resources(struct platform_device *pdev)
{
	int i;
	struct resource *res;
	void __iomem *ptr;
	struct wrn_drvdata *drvdata = pdev->dev.platform_data;
	struct wrn_dev *wrn = drvdata->wrn;

	/*
	 * The memory regions are mapped once for all endpoints.
	 * We don't populate the whole array, but use the resource list
	 */
	for (i = 0; i < pdev->num_resources; i++) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (!res || !res->start)
			continue;
		ptr = ioremap(res->start, res->end + 1 - res->start);
		if (!ptr) {
			dev_err(&pdev->dev, "Remap for res %i (%08lx) failed\n",
				i, (long)res->start);
			return -ENOMEM;
		}
		/* Hack: find the block number and fill the array */
		pr_debug("Remapped %08lx (block %i) to %p\n",
			 (long)res->start, i, ptr);
		printk("device.c:__wrn_map_resources: ioremap(...)=%p\n",(void*)ptr);
		wrn->bases[i] = ptr;
	}
	return 0;
}

static int wrn_probe(struct platform_device *pdev)
{
	struct net_device *netdev;
	struct wrn_ep *ep;
	struct wrn_drvdata *drvdata = pdev->dev.platform_data;
	struct wrn_dev *wrn = drvdata->wrn;
	int i, err = 0;

	/* Map our resource list and instantiate the shortcut pointers */
	printk("device.c:wrn_probe: calling __wrn_map_resources(pdev) w/pdev->num_resources=%d\n",
		pdev->num_resources);
	if ( (err = __wrn_map_resources(pdev)) )
		goto out;
	wrn->regs = wrn->bases[WRN_FB_NIC];
	wrn->txtsu_regs = wrn->bases[WRN_FB_TS];
	wrn->ppsg_regs = wrn->bases[WRN_FB_PPSG];
	wrn->txd = ((void *)wrn->regs) + 0x80; /* was: TX1_D1 */
	wrn->rxd = ((void *)wrn->regs) + 0x100; /* was: RX1_D1 */
	wrn->databuf = (void *)wrn->regs + offsetof(struct NIC_WB, MEM);
	tasklet_init(&wrn->rx_tlet, wrn_rx_interrupt, (unsigned long)wrn);
	if (0)
		printk("regs %p, txd %p, rxd %p, buffer %p\n",
		       wrn->regs, wrn->txd, wrn->rxd, wrn->databuf);

	/* Reset the device, just to be sure, before making anything */
	writel(0, &wrn->regs->CR);
	mdelay(10);

	/* Finally, register one interface per endpoint */
	memset(wrn->dev, 0, sizeof(wrn->dev));
	for (i = 0; i < WRN_NR_ENDPOINTS; i++) {
		printk("device.c:wrn_probe: before alloc_etherdev(...)\n");
		netdev = alloc_etherdev(sizeof(struct wrn_ep));
		netdev->dev.parent = &pdev->dev;
		if (!netdev) {
			dev_err(&pdev->dev, "Etherdev alloc failed.\n");
			err = -ENOMEM;
			goto out;
		}
		/* The ep structure is filled before calling ep_probe */
		ep = netdev_priv(netdev);
		ep->wrn = wrn;
		ep->ep_regs = wrn->bases[WRN_FB_EP] + i * FPGA_SIZE_EACH_EP;
		ep->ep_number = i;

		/* The netdevice thing is registered from the endpoint */
		err = wrn_endpoint_probe(netdev);
		if (err == -ENODEV)
			break;
		if (err)
			goto out;
		/* This endpoint went in properly */
		wrn->dev[i] = netdev;
		err = wrn_mezzanine_init(netdev);
		if (err)
			dev_err(&pdev->dev, "Init mezzanine code: "
				    "error %i\n", err);
	}
	if (i == 0)
		return -ENODEV; /* no endpoints */

	for (i = 0; i < WRN_NR_TXDESC; i++) { /* Clear all tx descriptors */
		struct wrn_txd *tx;
		tx = wrn->txd + i;
		writel(0, &tx->tx1);
	}

	/* Now, prepare RX descriptors */
	for (i = 0; i < WRN_NR_RXDESC; i++) {
		struct wrn_rxd *rx;
		int offset;

		rx = wrn->rxd + i;
		offset = __wrn_desc_offset(wrn, WRN_DDIR_RX, i);
		writel( (2000 << 16) | offset, &rx->rx3);
		writel(NIC_RX1_D1_EMPTY, &rx->rx1);
	}

	/*
	 * make sure all head/tail are 0 -- not needed here, but if we
	 * disable and then re-enable, this _is_ needed
	 */
	wrn->next_tx_head = wrn->next_tx_tail = wrn->next_rx = 0;

	writel(NIC_CR_RX_EN | NIC_CR_TX_EN, &wrn->regs->CR);
	writel(WRN_IRQ_ALL, (void *)wrn->regs + 0x24 /* EIC_IER */);

	wrn_tstamp_init(wrn);
	printk("device.c:wrn_probe: done, returning success\n");msleep(100);
	err = 0;
out:
	if (err) {
		/* Call the remove function to avoid duplicating code */
		wrn_remove(pdev);
	} else {
		dev_info(&pdev->dev, "White Rabbit NIC driver\n");
	}
	return err;
}


/* This is not static as ./module.c is going to register it */
struct platform_driver wrn_driver = {
	.probe		= wrn_probe,
	.remove		= wrn_remove, /* not __exit_p as probe calls it */
	/* No suspend or resume by now */
	.driver		= {
		.name		= KBUILD_MODNAME,
		.owner		= THIS_MODULE,
	},
};
