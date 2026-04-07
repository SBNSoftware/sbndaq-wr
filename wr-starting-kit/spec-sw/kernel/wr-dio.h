/*
 * Copyright (C) 2010-2012 CERN (www.cern.ch)
 * Author: Alessandro Rubini <rubini@gnudd.com>
 *
 * Released according to the GNU GPL, version 2 or any later version.
 *
 * This work is part of the White Rabbit project, a research effort led
 * by CERN, the European Institute for Nuclear Research.
 */
#ifndef __WR_DIO_H__
#define __WR_DIO_H__
/* This should be included by both the kernel and the tools */

#ifdef __KERNEL__
#include <linux/version.h>
#include "wbgen-regs/wr-dio-regs.h"

/* For GPIO we have no wb-gen header */
struct wrn_gpio_block {
	uint32_t	clear;
	uint32_t	set;
	uint32_t	dir;
	uint32_t	status;
};

/* And this is our bit mapping */
#define WRN_GPIO_VALUE(bit)	(1 << ((4 * (bit)) + 0))

extern irqreturn_t wrn_dio_interrupt(struct fmc_device *fmc);

#endif /* __KERNEL__ */

enum wr_dio_cmd_name {
	WR_DIO_CMD_PULSE,
	WR_DIO_CMD_STAMP,
	WR_DIO_CMD_DAC,
	WR_DIO_CMD_INOUT,
};

/*
 * This is how parameters are used (K == reply from kernel):
 *
 *  CMD_PULSE:
 *     cmd->flags: F_NOW, F_REL, F_LOOP
 *     cmd->channel: the channel or the mask
 *     cmd->t[]: either 2 or 3 values (start, duration, loop)
 *     cmd->value: count of loops (0 to turn off)
 *
 *  CMD_STAMP:
 *     cmd->flags: F_MASK, F_WAIT
 *     cmd->channel: the channel or the mask
 *     K: cmd->channel: the channel where we had stamps
 *     K: cmd->nstamp: number of valid stamps
 *     K: cmd->t[]: the stamps
 *
 *  CMD_DAC:
 *     cmd->flags: none
 *     cmd->channel: which one
 *     cmd->value: the value
 *
 *  CMD_INOUT:
 *     cmd->flags: F_MASK
 *     cmd->channel: the channel or the mask
 *     cmd->value: bits 0..4: WR-DIO, 8..12 value, 16..20 OEN, 24..28 term
 *
 */

#define WR_DIO_INOUT_DIO	(1 << 0)
#define WR_DIO_INOUT_VALUE	(1 << 8)
#define WR_DIO_INOUT_OUTPUT	(1 << 16)
#define WR_DIO_INOUT_TERM	(1 << 24)

/* IOMODE register helpers: one 4-bit nibble per channel */
#define WR_DIO_IOMODE_CH_BITS		4
#define WR_DIO_IOMODE_CH_SHIFT(ch)	((ch) * WR_DIO_IOMODE_CH_BITS)
#define WR_DIO_IOMODE_CH_MASK(ch)	(0xFu << WR_DIO_IOMODE_CH_SHIFT(ch))

/* Per-channel nibble layout */
#define WR_DIO_IOMODE_SRC_MASK		0x3u
#define WR_DIO_IOMODE_OUTPUT_ENABLE_N	(1u << 2)
#define WR_DIO_IOMODE_TERM_ENABLE	(1u << 3)

/* Source select values for nibble bits [1:0] */
#define WR_DIO_IOMODE_SRC_GPIO		0u
#define WR_DIO_IOMODE_SRC_DIO		1u
#define WR_DIO_IOMODE_SRC_WRPC		2u

#define WR_DIO_IOMODE_CH_SRC_MASK(ch)		(WR_DIO_IOMODE_SRC_MASK << WR_DIO_IOMODE_CH_SHIFT(ch))
#define WR_DIO_IOMODE_CH_SOURCE(ch, src)	((((uint32_t)(src)) & WR_DIO_IOMODE_SRC_MASK) << WR_DIO_IOMODE_CH_SHIFT(ch))
#define WR_DIO_IOMODE_CH_ENCODE(ch, nibble)	((((uint32_t)(nibble)) & 0xFu) << WR_DIO_IOMODE_CH_SHIFT(ch))
#define WR_DIO_IOMODE_CH_DECODE(reg, ch)	((((uint32_t)(reg)) >> WR_DIO_IOMODE_CH_SHIFT(ch)) & 0xFu)

#define WR_DIO_N_STAMP  16 /* At least 5 * 3 */

struct wr_dio_cmd {
	uint16_t command;	/* from user */
	uint16_t channel;	/* 0..4 or mask from user */
	uint32_t value;		/* for DAC or I/O */
	uint32_t flags;
	uint32_t nstamp;	/* from kernel, if IN_STAMP */
#if defined(__KERNEL__)
# if LINUX_VERSION_CODE >= KERNEL_VERSION(5,14,0)
	struct timespec64 t[WR_DIO_N_STAMP];	/* may be from user */
# else
	struct timespec t[WR_DIO_N_STAMP];	/* may be from user */
# endif
#else
	struct timespec  t[WR_DIO_N_STAMP];	/* may be from user - COULD THIS BE A PROBLEM??? */
#endif
};

#define WR_DIO_F_NOW	0x01	/* Output is now, t[0] ignored */
#define WR_DIO_F_REL	0x02	/* t[0].tv_sec is relative */
#define WR_DIO_F_MASK	0x04	/* Channel is 0x00..0x1f */
#define WR_DIO_F_LOOP	0x08	/* Output should loop: t[2] is  looping*/
#define WR_DIO_F_WAIT	0x10	/* Wait for event */
#define WR_DIO_F_GET	0x20	/* Read mode/state instead of writing */
#define WR_DIO_F_PULSOFF	0x40	/* set the pulse offset to channel 4 pulses*/


#endif /* __WR_DIO_H__ */
