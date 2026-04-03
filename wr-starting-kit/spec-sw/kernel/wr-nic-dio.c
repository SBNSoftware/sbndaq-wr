/*
 * Copyright (C) 2012 CERN (www.cern.ch)
 * Author: Alessandro Rubini <rubini@gnudd.com>
 *
 * Released according to the GNU GPL, version 2 or any later version.
 *
 * This work is part of the White Rabbit project, a research effort led
 * by CERN, the European Institute for Nuclear Research.
 */
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/ktime.h>
#include <linux/atomic.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/fmc.h>
#include <linux/fmc-sdb.h>
#include <linux/rtnetlink.h>
#include <linux/version.h>
#include "spec-nic.h"
#include "wr_nic/wr-nic.h"
#include "wr-dio.h"
#include "wbgen-regs/vic-regs.h"
#ifdef DO_TRACE
# include "TRACE/trace.h"
#else
# define TRACE(...)
#endif

#ifdef DIO_STAT
#define wrn_stat 1
#else
#define wrn_stat 0
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,14,0)
# define TIMESPEC     timespec64
# define TIMESPEC_ADD timespec64_add
#else
# define TIMESPEC     timespec
# define TIMESPEC_ADD timespec_add
#endif

/*
 * FIXME (for the whole file: we use readl/writel, not fmc_read/fmc_writel)
 */

/* We need a clear mapping for the registers of the various bits */
struct regmap {
	int trig_l;
	int trig_h;
	int cycle;
	int pulse;
	int fifo_tai_l;
	int fifo_tai_h;
	int fifo_cycle;
	int fifo_status;
};

#define R(x) (offsetof(struct DIO_WB, x))
static struct regmap regmap[] = {
	{
		.trig_l = R(TRIG0),
		.trig_h = R(TRIGH0),
		.cycle = R(CYC0),
		.pulse = R(PROG0_PULSE),
		.fifo_tai_l = R(TSF0_R0),
		.fifo_tai_h = R(TSF0_R1),
		.fifo_cycle = R(TSF0_R2),
		.fifo_status = R(TSF0_CSR),
	}, {
		.trig_l = R(TRIG1),
		.trig_h = R(TRIGH1),
		.cycle = R(CYC1),
		.pulse = R(PROG1_PULSE),
		.fifo_tai_l = R(TSF1_R0),
		.fifo_tai_h = R(TSF1_R1),
		.fifo_cycle = R(TSF1_R2),
		.fifo_status = R(TSF1_CSR),
	}, {
		.trig_l = R(TRIG2),
		.trig_h = R(TRIGH2),
		.cycle = R(CYC2),
		.pulse = R(PROG2_PULSE),
		.fifo_tai_l = R(TSF2_R0),
		.fifo_tai_h = R(TSF2_R1),
		.fifo_cycle = R(TSF2_R2),
		.fifo_status = R(TSF2_CSR),
	}, {
		.trig_l = R(TRIG3),
		.trig_h = R(TRIGH3),
		.cycle = R(CYC3),
		.pulse = R(PROG3_PULSE),
		.fifo_tai_l = R(TSF3_R0),
		.fifo_tai_h = R(TSF3_R1),
		.fifo_cycle = R(TSF3_R2),
		.fifo_status = R(TSF3_CSR),
	}, {
		.trig_l = R(TRIG4),
		.trig_h = R(TRIGH4),
		.cycle = R(CYC4),
		.pulse = R(PROG4_PULSE),
		.fifo_tai_l = R(TSF4_R0),
		.fifo_tai_h = R(TSF4_R1),
		.fifo_cycle = R(TSF4_R2),
		.fifo_status = R(TSF4_CSR),
	}
};

#define WRN_DIO_IRQ_MASK \
	(DIO_EIC_ISR_NEMPTY_0 \
	| DIO_EIC_ISR_NEMPTY_1 \
	| DIO_EIC_ISR_NEMPTY_2 \
	| DIO_EIC_ISR_NEMPTY_3\
	| DIO_EIC_ISR_NEMPTY_4)

/* This is the structure we need to manage interrupts and loop internally */
#define WRN_DIO_BUFFER_LEN  512
struct dio_channel {
	struct TIMESPEC tsbuf[WRN_DIO_BUFFER_LEN];
	int bhead, btail;
	wait_queue_head_t q;

	/* The input event may fire a new pulse on this or another channel */
	struct TIMESPEC prevts, delay;
	atomic_t count;
	int target_channel;
};

struct dio_device {
	struct dio_channel ch[5];
};

/* Instead of timespec_sub, just subtract the nanos */
static inline void wrn_ts_sub(struct TIMESPEC *ts, int nano)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,14,0)
	set_normalized_timespec64(ts, ts->tv_sec, ts->tv_nsec - nano);
#else
	set_normalized_timespec(ts, ts->tv_sec, ts->tv_nsec - nano);
#endif
}

/* This programs a new pulse without changing the width */
static void __wrn_new_pulse(struct wrn_drvdata *drvdata, int ch,
			    struct TIMESPEC *ts)
{
	struct DIO_WB __iomem *dio = drvdata->wrdio_base;
	void __iomem *base = dio;
	struct regmap *map;
	TRACE(TLVL_DEBUG+1,"(drvdata,%d,%lld.%09ld) START",ch,ts->tv_sec,ts->tv_nsec);

	map = regmap + ch;

	wrn_ts_sub(ts, 8); /* 1 cycle, to account for output latencies */
	writel(ts->tv_nsec / 8, base + map->cycle);
	ndelay(200);
	TRACE(TLVL_DEBUG+2,"TRACE after writel(ts->tv_nsec / 8 = %ld, base + map->cycle)", ts->tv_nsec/8);
	writel(GET_HI32(ts->tv_sec), base + map->trig_h);
	ndelay(200);
	TRACE(TLVL_DEBUG+3,"TRACE after writel(GET_HI32(ts->tv_sec=%lld), base + map->trig_h)", ts->tv_sec);
	writel(ts->tv_sec, base + map->trig_l);
	ndelay(200);
	TRACE(TLVL_DEBUG+4,"TRACE after writel(ts->tv_sec=%lld, base + map->trig_l)", ts->tv_sec);

	writel(1 << ch, &dio->R_LATCH);
	writel(1 << ch, &dio->R_LATCH);
	TRACE(TLVL_DEBUG+5,"(drvdata,%d,%lld.%09ld) DONE/RETURN",ch,ts->tv_sec,ts->tv_nsec);
}	// __wrn_new_pulse(struct wrn_drvdata *drvdata, int ch, struct TIMESPEC *ts)

static int wrn_dio_cmd_pulse(struct wrn_drvdata *drvdata,
			   struct wr_dio_cmd *cmd)
{
	struct DIO_WB __iomem *dio = drvdata->wrdio_base;
	void __iomem *base = dio;
	struct PPSG_WB __iomem *ppsg = drvdata->ppsg_base;
	struct dio_device *d = drvdata->mezzanine_data;
	struct dio_channel *c;
	struct regmap *map;
	struct TIMESPEC *ts;
	uint32_t regVal;
	int ch;

	ch = cmd->channel;
	TRACE(TLVL_DEBUG+10,"ch=%d flags=0x%x",ch,cmd->flags);
	if (ch > 4) {
		TRACE(TLVL_ERROR,"return -EINVAL - invalid channel");
		return -EINVAL; /* mask not supported */
	}
	c = d->ch + ch;
	map = regmap + ch;
	ts = cmd->t;

	/* First, configure this bit as DIO output */
	regVal = readl(&dio->IOMODE);
	TRACE(TLVL_DEBUG+11,"ch=%d IOMODE regVal read=0x%x write=0x%x",ch,regVal,regVal|(1<<4*ch));
	writel(regVal | (1 << 4*ch), &dio->IOMODE);
	ndelay(200);

	writel(ts[1].tv_nsec / 8, base + map->pulse); /* width */

	if (cmd->flags & WR_DIO_F_NOW) {
		/* if "now" we are done */
		TRACE(TLVL_DEBUG+12,"cmd->flags&WR_DIO_F_NOW == true -- return 0");
		writel(1 << ch, &dio->PULSE);
		return 0;
	}

	/* if relative, add current 40-bit second to TIMESPEC */
	if (cmd->flags & WR_DIO_F_REL) {
		uint32_t h1, l, h2;
		unsigned long now;

		h1 = readl(&ppsg->CNTR_UTCHI);
		l = readl(&ppsg->CNTR_UTCLO);
		h2 = readl(&ppsg->CNTR_UTCHI);
		if (h2 != h1)
			l = readl(&ppsg->CNTR_UTCLO);
		now = l;
		SET_HI32(now, h2);
		ts->tv_sec += now;
	}

	if (cmd->flags & WR_DIO_F_LOOP) {
		c->target_channel = ch;

		/* c->count is used after the pulse, so remove the first */
		if (cmd->value > 0)
			cmd->value--;
		atomic_set(&c->count, cmd->value);
		c->prevts = ts[0]; /* our current setpoint */
		c->delay = ts[2];
	}

	TRACE(TLVL_DEBUG+13,"calling __wrn_new_pulse(drvdata, %d, %lld.%09ld)",
	      ch, ts->tv_sec, ts->tv_nsec);
	__wrn_new_pulse(drvdata, ch, ts);
	TRACE(TLVL_DEBUG+14,"__wrn_new_pulse returned");
	TRACE(TLVL_DEBUG+15,"Done,return 0");
	return 0;
}	// wrn_dio_cmd_pulse(struct wrn_drvdata *drvdata, struct wr_dio_cmd *cmd)

static int wrn_dio_cmd_stamp(struct wrn_drvdata *drvdata,
			     struct wr_dio_cmd *cmd)
{
	struct dio_device *d = drvdata->mezzanine_data;
	struct dio_channel *dioChan_p = 0;
	struct TIMESPEC *ts = cmd->t;
	struct regmap *map;
	int mask, ch, last;
	int nstamp = 0;

	TRACE(TLVL_DEBUG+18,"START - WRN_DIO_BUFFER_LEN=%d WR_DIO_N_STAMP=%d",
	      WRN_DIO_BUFFER_LEN, WR_DIO_N_STAMP);
	if ((cmd->flags & (WR_DIO_F_MASK || WR_DIO_F_WAIT))
	    == (WR_DIO_F_MASK || WR_DIO_F_WAIT)) {
		TRACE(TLVL_ERROR,"Invalid cmd->flags - return -EINVAL");
		return -EINVAL; /* wait on several channels not supported */
	}

again:
	if (cmd->flags & WR_DIO_F_MASK) {
		ch = 0;
		last = 4;
		mask = cmd->channel;
		TRACE(TLVL_DEBUG+19,"START/again/DIO_F_MASK ch=%d last=%d mask=0x%x nstamp=%d",ch,last,mask,nstamp);
	} else {
		ch = cmd->channel;
		last = ch;
		mask = (1 << ch);
		TRACE(TLVL_DEBUG+19,"START/again/DIO_F_SINGLE ch=%d last=%d mask=0x%x nstamp=%d",ch,last,mask,nstamp);
	}
	/* handle the 1-channel and mask case in the same loop */
	dioChan_p = d->ch + ch;
	for (; ch <= last; ch++, dioChan_p++) {
		TRACE(TLVL_DEBUG+20,"Beginning of for loop - ch=%d, last=%d, mask=0x%x dioChan.bhead/tail=%d/%d .target_channel=%d"
		      , ch,last, mask, dioChan_p->bhead, dioChan_p->btail, dioChan_p->target_channel );
		if (((1 << ch) & mask) == 0) {
			TRACE(TLVL_DEBUG+21,"ch=%d, mask=0x%x - continue", ch, mask );
			continue;
		}
		map = regmap + ch;
		while (1) {
			int snap_head = dioChan_p->bhead;
			int snap_tail = dioChan_p->btail;
			TRACE(TLVL_DEBUG+22,"STAMP ch=%d bhead=%d btail=%d nstamp=%d",ch,snap_head,snap_tail,nstamp);
			if (nstamp == WR_DIO_N_STAMP) {
				TRACE(TLVL_DEBUG+23,"nstamp==WR_DIO_N_STAMP==%d - break",nstamp);
				break;
			}
			if (snap_head == snap_tail) {
				TRACE(TLVL_DEBUG+24,"STAMP ch=%d EMPTY bhead(%d)==btail(%d) nstamp=%d - break"
				      , ch, snap_head, snap_tail, nstamp);
				break;
			}
			*ts = dioChan_p->tsbuf[dioChan_p->btail];
			dioChan_p->btail = (dioChan_p->btail + 1) % WRN_DIO_BUFFER_LEN;
			nstamp++;
			ts++;
		}
		if (nstamp) {
			TRACE(TLVL_DEBUG+25,"setting cmd->channel=%d",ch);
			cmd->channel = ch;
			break;
		}
	}
	cmd->nstamp = nstamp;
	TRACE(TLVL_DEBUG+26,"ch=%d cmd->nstamp = nstamp(%d)",ch,nstamp);

	/* The user may ask to wait for timestamps, but for 1 channel only */
	if (!nstamp && cmd->flags & WR_DIO_F_WAIT) {
		ch--; dioChan_p--; /* The for above incremeted them */
		/*
		 * HACK: since 2.1.68 (Nov 1997) the ioctl is called locked.
		 * So we need to unlock, but that is dangerous for rmmod.
		 * Let's thus increase the module usage while sleeping
		 */
		TRACE(TLVL_DEBUG+27,"before wait_event_interruptible ch=%d", ch);
		try_module_get(THIS_MODULE);
		rtnl_unlock();
		wait_event_interruptible(dioChan_p->q, dioChan_p->bhead != dioChan_p->btail);
		rtnl_lock();
		module_put(THIS_MODULE);
		TRACE(TLVL_DEBUG+28,"after wait_event_interruptible");
		if (signal_pending(current)) {
			TRACE(TLVL_DEBUG+29,"signal_pending(current) - return -ERESTARTSYS");
			return -ERESTARTSYS;
		}
		TRACE(TLVL_DEBUG+30,"before goto again");
		goto again;
	}

	if (!nstamp) {
		TRACE(TLVL_DEBUG+31,"nstamp==0 return -EAGAIN");
		return -EAGAIN;
	}
	TRACE(TLVL_DEBUG+32,"return 0");
	return 0;
}	// wrn_dio_cmd_stamp(struct wrn_drvdata *drvdata, struct wr_dio_cmd *cmd)

static int wrn_dio_cmd_inout(struct wrn_drvdata *drvdata,
			     struct wr_dio_cmd *cmd)
{
	struct DIO_WB __iomem *dio = drvdata->wrdio_base;
	struct wrn_gpio_block __iomem *gpio = drvdata->gpio_base;
	int mask, ch, last, bits;
	uint32_t regVal, iomode;

	TRACE(TLVL_DEBUG+33,"START - flags=0x%x WR_DIO_F_MASK=0x%x value=0x%x"
	      , cmd->flags, WR_DIO_F_MASK, cmd->value);
	if (cmd->flags & WR_DIO_F_MASK) {
		ch = 0;
		last = 4;
		mask = cmd->channel;
	} else {
		ch = cmd->channel;
		last = ch;
		mask = (1 << ch);
	}
	TRACE(TLVL_DEBUG+34,"ch=%d last=%d mask=0x%x value=0x%x"
		      , ch, last, mask, cmd->value);	

	if (cmd->flags & WR_DIO_F_GET) {
		TRACE(TLVL_DEBUG+35,"WR_DIO_F_GET");
		cmd->value = 0;
		regVal = readl(&dio->IOMODE);

		for (; ch <= last; ch++) {
			uint32_t status;
			uint32_t src;

			if (((1 << ch) & mask) == 0)
				continue;

			iomode = WR_DIO_IOMODE_CH_DECODE(regVal, ch);
			src = iomode & WR_DIO_IOMODE_SRC_MASK;

			if (src != WR_DIO_IOMODE_SRC_GPIO)
				cmd->value |= WR_DIO_INOUT_DIO << ch;
			if (src == WR_DIO_IOMODE_SRC_WRPC)
				cmd->value |= WR_DIO_INOUT_VALUE << ch;
			if ((iomode & WR_DIO_IOMODE_OUTPUT_ENABLE_N) == 0)
				cmd->value |= WR_DIO_INOUT_OUTPUT << ch;
			if (iomode & WR_DIO_IOMODE_TERM_ENABLE)
				cmd->value |= WR_DIO_INOUT_TERM << ch;

			if (src == WR_DIO_IOMODE_SRC_GPIO) {
				status = readl(&gpio->status);
				if (status & WRN_GPIO_VALUE(ch))
					cmd->value |= WR_DIO_INOUT_VALUE << ch;
			}
		}
		TRACE(TLVL_DEBUG+36,"WR_DIO_F_GET return 0 - cmd->value=0x%x", cmd->value);
		return 0;
	}

	/* handle the 1-channel and mask case in the same loop */
	for (; ch <= last; ch++) {
		if (((1 << ch) & mask) == 0)
			continue;
		TRACE(TLVL_DEBUG+37,"in for, ch=%d mask=0x%x", ch, mask);
		/*
		 * In mask mode cmd->value carries channel bitmaps (0..4, 8..12,
		 * 16..20, 24..28), so align selected channel to bit 0.
		 * In single-channel mode cmd->value already describes one channel.
		 */
		bits = (cmd->flags & WR_DIO_F_MASK) ? (cmd->value >> ch) : cmd->value;

		/* Obtain the current value in iomode */
		
		regVal  = readl(&dio->IOMODE);
		TRACE(TLVL_DEBUG+38,"current IOMODE regVal=0x%x, ch=%d cleared => 0x%x"
		      , regVal, ch, regVal & ~(0xF << 4*ch) );
		regVal &= ~(0xF << 4*ch); /* clear this channel's nibble */

		/* Select IO mode */
		if (bits & WR_DIO_INOUT_DIO) {
			if(bits & WR_DIO_INOUT_VALUE) {
				iomode = 2; /* WRPC connection */
				TRACE(TLVL_DEBUG+39,"WRPC connection for ch=%d",ch);
			} else {
				iomode = 1; /* DIO connection */
				TRACE(TLVL_DEBUG+39,"DIO connection for ch=%d",ch);
			}
		} else {
			iomode = 0; /* GPIO  connection */
			TRACE(TLVL_DEBUG+39,"GPIO connection for ch=%d",ch);

			/* Output value is bit 0 (0x1) */
			if (bits & WR_DIO_INOUT_VALUE)
				writel(WRN_GPIO_VALUE(ch), &gpio->set);
			else
				writel(WRN_GPIO_VALUE(ch), &gpio->clear);
		}

		/* Appends to iomode TERM and OUTPUT_ENABLE_N bits */
		iomode |= (((bits & WR_DIO_INOUT_TERM) != 0) << 3)
			| (((bits & WR_DIO_INOUT_OUTPUT) == 0) << 2);
		TRACE(TLVL_DEBUG+40,"regVal=0x%x iomode=0x%x ch=%d dio->IOMODE=0x%x writel(0x%x,%p)"
		      , regVal, iomode, ch, dio->IOMODE, regVal|(iomode<<4*ch), (void*)&dio->IOMODE);
		writel(regVal | (iomode << 4*ch), &dio->IOMODE); // remember: writel(VAL,ADR)
	}
	TRACE(TLVL_DEBUG+41,"FINISH - return 0");
	return 0;
}	// wrn_dio_cmd_inout(struct wrn_drvdata *drvdata, struct wr_dio_cmd *cmd)


int wrn_mezzanine_ioctl(struct net_device *dev, struct ifreq *rq,
			       int ioctlcmd)
{
	struct wr_dio_cmd *cmd;
	struct wrn_drvdata *drvdata = dev->dev.parent->platform_data;
	ktime_t t, t0;
	int ret;

	TRACE(TLVL_DEBUG+42, "Ron - start strong wrn_mezzanine_ioctl wrn_stat=%d",wrn_stat);
	if (ioctlcmd == PRIV_MEZZANINE_ID) {
		TRACE(TLVL_DEBUG+43, "ioctlcmd==PRIV_MEZZANINE_ID - return -EAGAIN");
		return -EAGAIN; /* Special marker */
	}
	if (ioctlcmd != PRIV_MEZZANINE_CMD) {
		TRACE(TLVL_ERROR, "ioctlcmd!=PRIV_MEZZANINE_CMD - return -ENOIOCTLCMD");
		return -ENOIOCTLCMD;
	}

	if (wrn_stat) {
		t0 = ktime_get();
	}

	/* The cmd struct can't fit in the stack, so allocate it */
	cmd = kmalloc(sizeof(*cmd), GFP_KERNEL);
	if (!cmd) {
		TRACE(TLVL_ERROR, "return -ENOMEM");
		return -ENOMEM;
	}
	ret = -EFAULT;
	if (copy_from_user(cmd, rq->ifr_data, sizeof(*cmd))) {
		TRACE(TLVL_ERROR, "copy_from_user returned non-zero - goto out");
		goto out;
	}

	switch(cmd->command) {
	case WR_DIO_CMD_PULSE:
		TRACE(TLVL_DEBUG+44,"cmd->command case WR_DIO_CMD_PULSE");
		ret = wrn_dio_cmd_pulse(drvdata, cmd);
		break;
	case WR_DIO_CMD_STAMP:
		TRACE(TLVL_DEBUG+44,"cmd->command case WR_DIO_CMD_STAMP");
		ret = wrn_dio_cmd_stamp(drvdata, cmd);
		break;
	case WR_DIO_CMD_INOUT:
		TRACE(TLVL_DEBUG+44,"cmd->command case WR_DIO_CMD_INOUT");
		ret = wrn_dio_cmd_inout(drvdata, cmd);
		break;
	case WR_DIO_CMD_DAC:
		TRACE(TLVL_DEBUG+44,"cmd->command case WR_DIO_CMD_DAC");
		ret = -ENOTSUPP;
		goto out;
	default:
		TRACE(TLVL_DEBUG+44,"cmd->command case default/EINVAL");
		ret = -EINVAL;
		goto out;
	}

	TRACE(TLVL_DEBUG+45,"before copy_to_user() cmd->value=0x%x", cmd->value);
	if (copy_to_user(rq->ifr_data, cmd, sizeof(*cmd))) {
		TRACE(TLVL_ERROR, "copy_to_user error - return -ENOMEM");
		return -EFAULT;
	}
out:
	kfree(cmd);

	if (wrn_stat) {
		t = ktime_sub(ktime_get(), t0);
		dev_info(&dev->dev, "ioctl: %li ns\n", (long)ktime_to_ns(t));
	}
	TRACE(TLVL_DEBUG+46,"returning %d",ret);
	return ret;
}

/* This is called from the interrupt handler to program a new pulse */
static void wrn_trig_next_pulse(struct wrn_drvdata *drvdata,int ch,
				struct dio_channel *c, struct TIMESPEC *ts)
{
	struct TIMESPEC newts;

	if (c->target_channel == ch) {
		c->prevts =  TIMESPEC_ADD(c->prevts, c->delay); 
		newts = c->prevts;
	} else {
		newts = TIMESPEC_ADD(*ts, c->delay);
	}
	TRACE(TLVL_DEBUG+47,"calling __wrn_new_pulse(drvdata, %d, %lld.%09ld)", c->target_channel, newts.tv_sec, newts.tv_nsec);	
	__wrn_new_pulse(drvdata, c->target_channel, &newts);
	TRACE(TLVL_DEBUG+48,"__wrn_new_pulse returned");

	/* If the count is not-infinite, decrement it */
	if (atomic_read(&c->count) > 0)
		atomic_dec(&c->count);
}

irqreturn_t wrn_dio_interrupt(struct fmc_device *fmc)
{
	struct platform_device *pdev = fmc->mezzanine_data;
	struct wrn_drvdata *drvdata = pdev->dev.platform_data;
	struct VIC_WB __iomem *vic = drvdata->vic_base;
	struct DIO_WB __iomem *dio = drvdata->wrdio_base;
	void __iomem *base = drvdata->wrdio_base;
	struct dio_device *d = drvdata->mezzanine_data;
	static ktime_t t_ini, t_end;
	static int rate_avg;
	struct dio_channel *c;
	struct TIMESPEC *ts;
	struct regmap *map;
	uint32_t mask, regVal;
	int ch, chm;

	TRACE(TLVL_DEBUG+49,"START");
	if (unlikely(!fmc->eeprom)) {
		dev_err(fmc->hwdev, "WR-DIO: No mezzanine, disabling irqs\n");
		writel(~0, &dio->EIC_IDR);
		writel(~0, &dio->EIC_ISR);
		return IRQ_NONE;
	}

	/* Protect against interrupts taking 100% of cpu time */
	if (ktime_to_ns(t_end)) {
		int rate;
		u64 offtime, ontime;

		ontime = ktime_to_ns(t_end) - ktime_to_ns(t_ini);
		t_ini = ktime_get();
		offtime = ktime_to_ns(t_ini) - ktime_to_ns(t_end);

		/* avoid __udivdi3 */
		if (offtime > 100 * ontime)
			rate = 0;
		else
			rate = ((int)ontime * 100) / (int)offtime;

		rate_avg = (rate_avg * 1023 + rate) / 1024;
		if (rate_avg > 80) {
			dev_warn(fmc->hwdev, "DIO irq takes > 80%% CPU time: "
				 "disabling\n");
			TRACE(TLVL_WARNING,"DIO irq takes > 80%% CPU time: disabling");
			writel(WRN_VIC_MASK_DIO, &vic->IDR);
		}
	}

	mask = readl(&dio->EIC_ISR) & WRN_DIO_IRQ_MASK;

	/* Three indexes: channel, channel-mask, channel pointer */
	for (ch = 0, chm = 1, c = d->ch; mask; ch++, chm <<= 1, c++) {
		int h;

		if (!(mask & chm))
			continue;
		mask &= ~chm;

		/* Pull the FIFOs to the device structure */
		map = regmap + ch;
		ts = NULL;
		while (1) {
			regVal = readl(base + map->fifo_status);
			if (regVal & 0x20000) /* empty */
				break;
			h = c->bhead;
			ts = c->tsbuf + h;
			c->bhead = (h + 1) % WRN_DIO_BUFFER_LEN;
			TRACE(TLVL_DEBUG+51,"IRQ ch=%d bhead %d->%d btail=%d", ch, h, c->bhead, c->btail);
			if (c->bhead == c->btail)
				c->btail = (c->btail + 1) % WRN_DIO_BUFFER_LEN;
			/*
			 * fifo is not-empty, pick one sample. Read
			 * cycles last, as that operation pops the FIFO
			 */
			ts->tv_sec = 0;
			SET_HI32(ts->tv_sec, readl(base + map->fifo_tai_h));
			ts->tv_sec |= readl(base + map->fifo_tai_l);
			ts->tv_nsec = 8 * readl(base + map->fifo_cycle);
			/* subtract 5 cycles lost in input sync circuits */
			wrn_ts_sub(ts, 40);
		}
		writel(chm, &dio->EIC_ISR); /* ack */
		if (ts && atomic_read(&c->count) != 0) {
			wrn_trig_next_pulse(drvdata, ch, c, ts);
		}
		wake_up_interruptible(&c->q);
	}
	t_end = ktime_get();
	TRACE(TLVL_DEBUG+50,"IRQ_HANDLED");
	return IRQ_HANDLED;
}

/* Init and exit below are called when a netdevice is created/destroyed */
int wrn_mezzanine_init(struct net_device *dev)
{
	struct wrn_drvdata *drvdata = dev->dev.parent->platform_data;
	struct DIO_WB __iomem *dio = drvdata->wrdio_base;
	struct dio_device *d;
	int i;

	printk(KERN_INFO "Ron - strong %s\n",__func__);
	/* Allocate the data structure and enable interrupts for stamping */
	d = kzalloc(sizeof(*d), GFP_KERNEL);
	if (!d)
		return -ENOMEM;
	for (i = 0; i < ARRAY_SIZE(d->ch); i++)
		init_waitqueue_head(&d->ch[i].q);
	drvdata->mezzanine_data = d;

	/*
	 * Enable interrupts for FIFO, if there's no mezzanine the
	 * handler will notice and disable the interrupts
	 */
	writel(WRN_DIO_IRQ_MASK, &dio->EIC_IER);
	return 0;
}

void wrn_mezzanine_exit(struct net_device *dev)
{
	struct wrn_drvdata *drvdata = dev->dev.parent->platform_data;
	struct DIO_WB __iomem *dio = drvdata->wrdio_base;

	writel(~0, &dio->EIC_IDR);
	if (drvdata->mezzanine_data)
		kfree(drvdata->mezzanine_data);
}

