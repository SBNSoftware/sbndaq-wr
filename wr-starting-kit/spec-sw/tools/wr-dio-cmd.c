/*
 * Copyright (C) 2012 CERN (www.cern.ch)
 * Author: Alessandro Rubini <rubini@gnudd.com>
 *
 * Released to the public domain as sample code to be customized.
 *
 * This work is part of the White Rabbit project, a research effort led
 * by CERN, the European Institute for Nuclear Research.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <net/if.h>				/* struct ifreq (via nesting) */
#include <netpacket/packet.h>

#include "wr_nic/wr-nic.h"
#include "wr-dio.h"
#ifdef DO_TRACE
# include "TRACE/trace.h"
#else
# define TRACE(...)
#endif

char *prgname;
char c;
int sock;
char *ifname;
struct ifreq ifr;				/*  */

struct wr_dio_cmd _cmd;         /* defined in kernel/wr-dio.h */
struct wr_dio_cmd *cmd = &_cmd;

static int parse_ts(char *s, struct timespec *ts)
{
	int i, n;
	unsigned long nano;
	char c;

	/*
 	 * Hairy: if we scan "%ld%lf", the 0.009999 will become 9998 micro.
	 * Thus, scan as integer and string, so we can count leading zeros
	 */

	nano = 0;
	ts->tv_sec = 0;
	ts->tv_nsec = 0;

	if ( (i = sscanf(s, "%ld.%ld%c", &ts->tv_sec, &nano, &c)) == 1)
		return 0; /* seconds only */
	if (i == 3)
		return -1; /* trailing crap */
	if (i == 0)
		if (sscanf(s, ".%ld%c", &nano, &c) != 1)
			return -1; /* leading or trailing crap */

	s = strchr(s, '.') + 1;
	n = strlen(s);
	if (n > 9)
		return -1; /* too many decimals */
	while (n < 9) {
		nano *= 10;
		n++;
	}
	ts->tv_nsec = nano;
	return 0;
}

static int scan_pulse(int argc, char **argv)
{
	char c;

	if (argc != 3 && argc != 4 && argc != 6) {
		fprintf(stderr, "%s: %s: wrong number of arguments\n",
			prgname, argv[0]);
		fprintf(stderr, "  Use: %s <channel> <duration> <when> "
			"[<period> <count>]\n", argv[0]);
		return -1;
	}
	if (sscanf(argv[1], "%hi%c", &cmd->channel, &c) != 1
		|| cmd->channel < 0
		|| cmd->channel > 4) {
		fprintf(stderr, "%s: %s: not a channel number \"%s\"\n",
			prgname, argv[0], argv[1]);
		return -1;
	}

	if (argc == 3) {
		/* If only 3 args, we have the offset form: pulse <ch> .<offset> */
		*(int*)&cmd->value = atoi(argv[2]); /* Default to 0 if no offset provided */
		cmd->flags |= WR_DIO_F_PULSOFF;
		ifr.ifr_data = (void *)cmd;
		if (ioctl(sock, PRIV_MEZZANINE_CMD, &ifr) < 0) {
			fprintf(stderr, "%s: ioctl(PRIV_MEZZANINE_CMD(%s)): %s\n",
			prgname, ifname, strerror(errno));
			return -1;
		}
		return 0;
	}

	/* Duration is first time argument but position 1 for ioctl */
	if (parse_ts(argv[2], cmd->t + 1) < 0) {
		fprintf(stderr, "%s: %s: invalid time \"%s\"\n",
			prgname, argv[0], argv[2]);
		return -1;
	}
	if (cmd->t[1].tv_sec) {
		fprintf(stderr, "%s: %s: duration must be < 1s (got \"%s\")\n",
			prgname, argv[0], argv[2]);
		return -1;
	}

	/* Next argument is the "when", position 0 in ioctl timestamp array */
	if (!strcmp(argv[3], "now")) {
		cmd->flags |= WR_DIO_F_NOW;
	} else {
		char *s2 = argv[3];

		if (s2[0] == '+') {
			cmd->flags |= WR_DIO_F_REL;
			s2++;
		}

		if (parse_ts(s2, cmd->t) < 0) {
			fprintf(stderr, "%s: %s: invalid time \"%s\"\n",
				prgname, argv[0], argv[3]);
			return -1;
		}
	}

	/* If argc is 6, we have period and count */
	if (argc == 6) {
		cmd->flags |= WR_DIO_F_LOOP;

		if (parse_ts(argv[4], cmd->t + 2) < 0) {
			fprintf(stderr, "%s: %s: invalid time \"%s\"\n",
				prgname, argv[0], argv[4]);
			return -1;
		}
		if (sscanf(argv[5], "%i%c", &cmd->value, &c) != 1) {
			fprintf(stderr, "%s: %s: invalid count \"%s\"\n",
				prgname, argv[0], argv[5]);
			return -1;
		}
	}

	ifr.ifr_data = (void *)cmd;
	/*fprintf(stderr,"%s: in scan_pulse:  Ron - before ioctl PRIV_MEZZANINE_CMD\n", prgname );*/
	if (ioctl(sock, PRIV_MEZZANINE_CMD, &ifr) < 0) {
		fprintf(stderr, "%s: ioctl(PRIV_MEZZANINE_CMD(%s)): %s\n",
			prgname, ifname, strerror(errno));
			return -1;
	}
	return 0;
}

static int scan_stamp(int argc, char **argv, int ismask)
{
	int i, ch;
	char c;

	cmd->flags = 0;
	if (argc == 3 && !strcmp(argv[2], "wait")) {
		cmd->flags = WR_DIO_F_WAIT;
		argc = 2;
	}
	if (argc == 1) {
		ismask = 1;
		ch = 0x1f;
	} else if (argc == 2) {
		if (sscanf(argv[1], "%i%c", &ch, &c) != 1) {
			fprintf(stderr, "%s: %s: not a channel \"%s\"\n",
				prgname, argv[0], argv[1]);
			exit(1);
		}
		if (ch < 0 || ch > 31 || (!ismask && ch > 4)) {
			fprintf(stderr, "%s: %s: out of range channel \"%s\"\n",
				prgname, argv[0], argv[1]);
			exit(1);
		}
	} else {
		fprintf(stderr, "%s: %s: wrong number of arguments\n",
			prgname, argv[0]);
		if (ismask)
			fprintf(stderr, "  Use: %s [<channel-mask>]\n",
				argv[0]);
		else
			fprintf(stderr, "  Use: %s [<channel>] [wait]\n",
				argv[0]);
		return -1;
	}
	if (ismask) {
		cmd->flags = WR_DIO_F_MASK;
	}

	while (1) {
		cmd->channel = ch;
		TRACE(TLVL_DEBUG+1,"%s: in scan_stamp: Ron - in while loop ch == %i \n", prgname, ch );
		errno = 0;
		ifr.ifr_data = (void *)cmd;
		TRACE(TLVL_DEBUG+1,
		      "%s: in scan_stamp: Ron - before ioctl(sock,PRIV_MEZZANINE_CMD,&ifr) w/ifr_data=cmd w/->command=%u"
		      , prgname, cmd->command );
		if (ioctl(sock, PRIV_MEZZANINE_CMD, &ifr) < 0 ) {
			if (errno == EAGAIN) {
				TRACE(TLVL_DEBUG+1,"EAGAIN - normal done/return");
				break;
			}
			fprintf(stderr, "%s: ioctl(PRIV_MEZZANINE_CMD(%s)): NOT EAGAIN status "
			        "%s\n", prgname, ifname, strerror(errno));
			return -1;
		}
		TRACE(TLVL_DEBUG+1,"%s: in scan_stamp: Ron - should loop over nstamp data %i \n", prgname, cmd->nstamp );
		for (i = 0; i < cmd->nstamp; i++) {
			TRACE(TLVL_DEBUG+9,"ch %i, %9li.%09li\n", cmd->channel,
			       (long)cmd->t[i].tv_sec, cmd->t[i].tv_nsec);
			printf("ch %i, %9li.%09li\n", cmd->channel,
			       (long)cmd->t[i].tv_sec, cmd->t[i].tv_nsec);
		}
	}
	TRACE(TLVL_DEBUG+1,"%s: in scan_stamp: Ron - exit and return 0 \n", prgname );
	return 0;
}	// scan_stamp(int argc, char **argv, int ismask)


// set cmd->channel (mask) and
//     cmd->value (4 byte fields with 5 channel mask bits each)
static int one_mode(int modeChar, int chanIdx)
{
	if (modeChar == '-')
		return 0;
	cmd->channel |= 1 << chanIdx;

	//Add error message for channel 0 
	if(chanIdx==0 && strchr("dD01",modeChar))
	{
		TRACE(TLVL_ERROR,
		      "Error: Only p/P modes are available as ouput mode for channel 0");
		return -1;
	}
	
	switch(modeChar) {
	case 'D':
		cmd->value |= WR_DIO_INOUT_TERM << chanIdx;
	case 'd':
		cmd->value |= WR_DIO_INOUT_DIO << chanIdx;
		cmd->value |= WR_DIO_INOUT_OUTPUT << chanIdx;
		break;

	case 'C':
		cmd->value |= WR_DIO_INOUT_TERM << chanIdx;
	case 'c':
		cmd->value |= WR_DIO_INOUT_DIO << chanIdx;
		cmd->value |= WR_DIO_INOUT_VALUE << chanIdx;
		if(chanIdx!=4)
			fprintf(stdout, "Warning: Clock mode is only available for last channel (ch4)\n,"
			 "(on other channel it corresponds to input mode without interruptions)\n");
		break;

	case 'P':
		cmd->value |= WR_DIO_INOUT_TERM << chanIdx;
	case 'p':
		cmd->value |= WR_DIO_INOUT_DIO << chanIdx;
		cmd->value |= WR_DIO_INOUT_VALUE << chanIdx;
		cmd->value |= WR_DIO_INOUT_OUTPUT << chanIdx;
		break;

	case 'I':
		cmd->value |= WR_DIO_INOUT_TERM << chanIdx;
	case 'i':
		break;

	case '1':
		cmd->value |= WR_DIO_INOUT_VALUE << chanIdx;
	case '0':
		cmd->value |= WR_DIO_INOUT_OUTPUT << chanIdx;
		break;

	default:
		TRACE(TLVL_ERROR, TSPRINTF("%s: mode: invalid mode '%c'\n",
		                           prgname, c));
		return -1;
	}
	TRACE(TLVL_DEBUG+1,"cmd->channel=0x%x value=0x%x",cmd->channel, cmd->value);
	return 0;
}	// one_mode(int modeChar, int chanIdx)

static const char *decode_mode_description(int bits)
{
	int is_dio = bits & WR_DIO_INOUT_DIO;
	int is_one = bits & WR_DIO_INOUT_VALUE;
	int is_out = bits & WR_DIO_INOUT_OUTPUT;
	int is_term = bits & WR_DIO_INOUT_TERM;

	if (is_dio) {
		if (is_out) {
			if (is_one) {
				return is_term ? "DIO pulse output with termination" : "DIO pulse output";
			} else {
				return is_term ? "DIO regular output with termination" : "DIO regular output";
			}
		}
		if (is_one)
			return is_term ? "DIO input active with termination" : "DIO input active";
		return "DIO unknown";
	}

	if (is_out)
		return is_one ? "Logic output high (1)" : "Logic output low (0)";

	if (is_one)
		return "Logic unknown";

	return is_term ? "Logic input with termination" : "Logic input";
}	// decode_mode_description(int bits)

static void print_mode_reply(int mask)
{
	int ch;

	TRACE(TLVL_DEBUG+1,"START channel mask=0x%x cmd->value=0x%x",mask,cmd->value);
	for (ch = 0; ch < 5; ch++) {
		if (mask & (1 << ch)) {
			int bits = 0;

			bits |= (cmd->value >> ch) & WR_DIO_INOUT_DIO;
			bits |= (cmd->value >> ch) & WR_DIO_INOUT_VALUE;
			bits |= (cmd->value >> ch) & WR_DIO_INOUT_OUTPUT;
			bits |= (cmd->value >> ch) & WR_DIO_INOUT_TERM;
			TRACE(TLVL_DEBUG+1,"calling decode_mode_description(0x%x)",bits);
			printf("ch %d: %s\n", ch, decode_mode_description(bits));
		}
	}
	TRACE(TLVL_DEBUG+1,"DONE");
}	// print_mode_reply

// argv[0] == "mode"
static int scan_inout(int argc, char **argv)
{
	int i, ch;
	char c;
	int do_get = 0;
	int get_mask = 0;

	cmd->flags = WR_DIO_F_MASK;
	cmd->channel = 0;
	cmd->value = 0;

	TRACE(TLVL_DEBUG+1, "argc=%d", argc ); /* example: "mode" "1" "D" */
	if (argc == 1) {
		do_get = 1;
		get_mask = 0x1f;
		cmd->channel = get_mask;
		cmd->flags |= WR_DIO_F_GET;
		TRACE(TLVL_DEBUG+1,TSPRINTF("argc=1 argv[0]=%s - _GET all get_mask=0x%x"
		                            , argv[0], get_mask) );
	} else if (argc == 2) {
		TRACE(TLVL_DEBUG+1, TSPRINTF("argc=2 argv[0]=%s argv[1]=%s", argv[0], argv[1]));
		if (strlen(argv[1]) == 1 && sscanf(argv[1], "%i%c", &ch, &c) == 1) {
			if (ch < 0 || ch > 4) {
				TRACE(TLVL_ERROR, TSPRINTF("%s: mode: invalid channel \"%s\"\n",
				                         prgname, argv[1]));
				return -1;
			}
			do_get = 1;
			get_mask = 1 << ch;
			cmd->channel = get_mask;
			cmd->flags |= WR_DIO_F_GET;
			TRACE(TLVL_DEBUG+1,"_GET ch=%d",ch);
		} else {
			if (strlen(argv[1]) != 5) {
				TRACE(TLVL_ERROR, TSPRINTF("%s: %s: wrong argument \"%s\"",
				                           prgname, argv[0], argv[1]));
				exit(1);
			}
			for (i = 0; i < 5; i++) {
				TRACE(TLVL_DEBUG+1,
				      "attempt program/set ch=%d to mode %x", i, argv[1][i] );
				if (one_mode(argv[1][i], i) < 0) {
					return -1;
				}
			}
		}
	} else {
		if (argc < 3 || argc > 11 || ((argc & 1) == 0)) {
			fprintf(stderr, "%s: %s: wrong number of arguments\n",
				prgname, argv[0]);
			return -1;
		}
		TRACE(TLVL_DEBUG+1, TSPRINTF("argc=2 argv[0]=%s argv[1]=%s", argv[0], argv[1]));
		while (argc >= 3) {
			TRACE(TLVL_DEBUG+1, TSPRINTF("argc=%d argv[0]=%s [1]=%s [2]=%s",
			                             argc, argv[0],argv[1],argv[2]));
			if (sscanf(argv[1], "%i%c", &ch, &c) != 1
			    || ch < 0 || ch > 4) {
				fprintf(stderr, "%s: mode: invalid channel "
					"\"%s\"\n", prgname,  argv[1]);
				return -1;
			}
			if (strlen(argv[2]) != 1) {
				fprintf(stderr, "%s: mode: invalid mode "
					"\"%s\"\n", prgname,  argv[2]);
				return -1;
			}
			TRACE(TLVL_DEBUG+1, "before one_mode(%c,%d)",argv[2][0],ch);
			if (one_mode(argv[2][0], ch) < 0)
				return -1;
			TRACE(TLVL_DEBUG+1,"cmd->command=%u channel(mask)=0x%x value=0x%x flags=0x%x nstamp=%u"
			      " t[0].tv_sec=%ld t[0].tv_nsec=%ld",
			      cmd->command, cmd->channel, cmd->value, cmd->flags, cmd->nstamp,
			      cmd->t[0].tv_sec, cmd->t[0].tv_nsec);
			argv += 2;
			argc -= 2;
		}
	}
	TRACE(TLVL_DEBUG+1
	      , "before ioctl(PRIV_MEZZANINE_CMD) do_get=%d channel=0x%x value=0x%x"
	      , do_get, cmd->channel, cmd->value);
	ifr.ifr_data = (void *)cmd;
	if (ioctl(sock, PRIV_MEZZANINE_CMD, &ifr) < 0) { /* See spec-sw/kernel/wr_nic/nic-core.c:303 */
		fprintf(stderr, "%s: ioctl(PRIV_MEZZANINE_CMD(%s)): %s\n",
			prgname, ifname, strerror(errno));
			return -1;
	}

	if (do_get) {
		cmd->value = ((struct wr_dio_cmd *)ifr.ifr_data)->value;
		TRACE(TLVL_DEBUG+1,"calling print_mode_reply(get_mask=0x%x) cmd->value=0x%x"
		      , get_mask, cmd->value);
		print_mode_reply(get_mask);
	}

	TRACE(TLVL_DEBUG+1,"return 0");
	return 0;
}	// scan_inout(int argc, char **argv)

int main(int argc, char **argv)
{

	prgname = argv[0];
	argv++, argc--;

	if (argc < 2) {
		fprintf(stderr, "%s: use \"%s <netdev> <cmd> [...]\"\n",
			prgname, prgname);
		exit(1);
	}
	ifname = argv[0];
	argv++, argc--;

	sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	//sock = socket(AF_INET, SOCK_DGRAM, 0 );
	if (sock < 0) {
		fprintf(stderr, "%s: socket(): %s\n",
			prgname, strerror(errno));
	exit(1);
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	TRACE(TLVL_DEBUG+1,
	      TSPRINTF("%s %s %s: in main: Ron - before ioctl PRIV_MEZZANINE_ID"
		           , prgname, ifname, argv[0]) );
	if (ioctl(sock, PRIV_MEZZANINE_ID, &ifr) < 0
	    /* EAGAIN is special: it means we have no ID to check yet */
		&& errno != EAGAIN) {
		fprintf(stderr, "%s: ioctl(PRIV_MEZZANINE_ID(%s)): %s\n",
			prgname, ifname, strerror(errno));
	}

	/*
	 * Parse the command line:
	 *
	 * pulse <ch> .<len> <seconds>.<fraction>  [<period> <count>]
	 * pulse <ch> .<len> now                   [<period> <count>]
	 * pulse <ch> .<len> +<seconds>.<fraction> [<period> <count>]
	 * pulse <ch> <offset>
	 *
	 * stamp [<channel>]
	 * stampm [<mask>]
	 *
	 * mode
	 * mode <ch>
	 * mode <01234>
	 * mode <ch> <mode> [...]
	 */
	if (!strcmp(argv[0], "pulse")) {
		cmd->command = WR_DIO_CMD_PULSE;
		if (scan_pulse(argc, argv) < 0)
			exit(1);
	} else if (!strcmp(argv[0], "stamp")) {
		cmd->command = WR_DIO_CMD_STAMP;
		if (scan_stamp(argc, argv, 0 /* no mask */) < 0)
			exit(1);
	} else if (!strcmp(argv[0], "stampm")) {
		cmd->command = WR_DIO_CMD_STAMP;
		if (scan_stamp(argc, argv, 1 /* mask */) < 0)
			exit(1);
	} else if (!strcmp(argv[0], "mode")) {
		cmd->command = WR_DIO_CMD_INOUT;
		if (scan_inout(argc, argv) < 0)
			exit(1);
	} else {
		fprintf(stderr, "%s: unknown command \"%s\"\n", prgname,
			argv[0]);
		exit(1);
	}

	ifr.ifr_data = (void *)cmd;
	exit(0);
}
