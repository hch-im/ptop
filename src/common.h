/***************************************************************************
 * Name:common.h - Public function definitions.
 **************************************************************************/

/*
 *   Copyright (C) 2009, Mobile and Internet Systems Laboratory.
 *   All rights reserved.
 *
 *   Authors: Suhib (suhibr@gmail.com), Chen hui (hchen229@gmail.com)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <time.h>

#ifdef USE_NLS
#include <locale.h>
#include <libintl.h>
#define _(string) gettext(string)
#else
#define _(string) (string)
#endif

/*
 ***************************************************************************
 * Various keywords and constants
 ***************************************************************************
 */

#define FALSE	0
#define TRUE	1

#define	MAXU32VAL	0x11111111
#define	MAXU64VAL	0x1111111111111111LL
/*
 * We define u64 as unsigned long long for every architecture
 * so that we can print it with %Lx without getting warnings.
 */
typedef unsigned long long u64;
typedef signed long long   s64;
typedef unsigned int	   u32;
typedef signed int	   s32;
typedef unsigned short	   u16;
typedef signed short	   s16;
typedef unsigned char	   u8;
typedef signed char	   s8;

/* Files */
#define STAT		 "/proc/stat"
#define UPTIME		 "/proc/uptime"
#define MEMINFO		 "/proc/meminfo"
#define SYSFS_BLOCK	 "/sys/block"
#define SYSFS_DEVCPU "/sys/devices/system/cpu"
#define S_STAT		 "stat"
#define DEVMAP_DIR	 "/dev/mapper"
#define NET_DEV	     "/proc/net/dev"
#define NET_WIRELESS "/proc/net/wireless"

#define MAX_FILE_LEN	256
#define MAX_PF_NAME	    1024
#define DEVMAP_MAJOR	253
#define MAX_NAME_LEN	72

/*
 ***************************************************************************
 * Macro functions definitions.
 ***************************************************************************
 */

/*
 * Macros used to display statistics values.
 *
 * NB: Define SP_VALUE() to normalize to %;
 * HZ is 1024 on IA64 and % should be normalized to 100.
 */
#define S_VALUE(m,n,p)	(((double) ((n) - (m))) / (p) * HZ)
#define SP_VALUE(m,n,p)	(((double) ((n) - (m))) / (p) * 100)

/*
 * Under very special circumstances, STDOUT may become unavailable.
 * This is what we try to guess here
 */
#define TEST_STDOUT(_fd_)	do {					\
					if (write(_fd_, "", 0) == -1) {	\
				        	perror("stdout");	\
				       		exit(6);		\
				 	}				\
				} while (0)


#define MINIMUM(a,b)	((a) < (b) ? (a) : (b))

/* Number of ticks per second */
#define HZ		hz
extern u32 hz;

/* Number of bit shifts to convert pages to kB */
extern u32 kb_shift;

/*
 * kB <-> number of pages.
 * Page size depends on machine architecture (4 kB, 8 kB, 16 kB, 64 kB...)
 */
#define KB_TO_PG(k)	((k) >> kb_shift)
#define PG_TO_KB(k)	((k) << kb_shift)

#define MAX_COMM_LEN	16
#define MAX_CMDLINE_LEN	128

#define PROC		"/proc"
#define PROC_PID	 "/proc/%u"
#define PID_STAT	 "/proc/%u/stat"
#define PID_STATUS	 "/proc/%u/status"
#define PID_IO		 "/proc/%u/io"
#define PID_CMDLINE	 "/proc/%u/cmdline"
/*
 * Datastructures
 */
/* process statistic structure */
struct pid_stats {
	u64 read_bytes;
	u64 write_bytes;
	u64 cancelled_write_bytes;
	u64 total_vsz;
	u64 total_rss;
	u32      minflt; // minor faults
	u32      cminflt; // cummulative minflt
	u32      majflt; // major fault
	u32      cmajflt;
	u64      utime; // user mode time
	u64      cutime; // cummulative usermode
	u64      stime; // system mode time
	u64      cstime;
	u64      gtime; // guest mode
	u64      cgtime;
	u32      vsz;
	u32      rss; // virtual size
	u32      nvcsw; // no of voluntary context switch
	u32      nivcsw; // involuntary
	u32       processor;
	u32       pid;
	char               comm[MAX_COMM_LEN];

	/* DISK STATISTICS						*/
	struct dsk {
		u64	rio;		/* number of read requests 	*/
		u64	rsz;		/* cumulative # sectors read	*/
		u64	wio;		/* number of write requests 	*/
		u64	wsz;		/* cumulative # sectors written	*/
		u64	cwsz;		/* cumulative # written sectors */
					/* being cancelled              */
		u64	cfuture[4];	/* reserved for future use	*/
	} dsk;


	/* NETWORK STATISTICS						*/
	struct net {
		u64 tcpsnd;		/* number of TCP-packets sent	*/	// number of packet
		u64 tcpssz;		/* cumulative size packets sent	*/	// number of bytes in total (cummulative)
		u64	tcprcv;		/* number of TCP-packets recved	*/
		u64 tcprsz;		/* cumulative size packets recvd	*/
		u64	udpsnd;		/* number of UDP-packets sent	*/
		u64 udpssz;		/* cumulative size packets sent	*/
		u64	udprcv;		/* number of UDP-packets recved	*/
		u64 udprsz;		/* cumulative size packets sent	*/
		u64	rawsnd;		/* number of raw packets sent	*/
		u64	rawrcv;		/* number of raw packets recved	*/
		u64	cfuture[4];	/* reserved for future use	*/
		u64 totalnetsnd;
		u64 totalnetrcv;
	} net;

	/* Memory Statistics*/
	struct memory {
		u64 memory_used;  /* Memory used by this process*/
		u64 total_active;/* total active memory*/
		u64 cpu_accesses;/* memory access times by cpu, AMD64 could get this.*/
	} mem;
};

#define PID_STATS_SIZE	(sizeof(struct pid_stats))

/*
 ***************************************************************************
 * Functions prototypes
 ***************************************************************************
 */
extern void get_HZ(void);
extern u64 get_interval(u64, u64);
extern void get_kb_shift(void);
extern time_t get_localtime(struct tm *);
extern int get_win_height(void);
extern void init_nls(void);
extern void debug(int level, char * info);
extern u32 subcount(u32 newval, u32 oldval);
extern u64 u64_subcount(u64 newval, u64 oldval);
#endif  /* _COMMON_H */
