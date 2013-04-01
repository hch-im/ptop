/***************************************************************************
 * Name:database.h - Database access interfaces and data structures for
 *                   database records.
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
#ifndef DATABASE_H_
#define DATABASE_H_

#include <mysql/mysql.h>
#include "common.h"
/*
 * Data structures for database records. is that your structs are unnamed. You need to give them names so you can refer to them withi
 */

/*
 * System level energy consumption in a time interval.
 */
struct device_energy{
	long time;
	float ecpu;
	float emem;
	float enet;
	float edisk;
};

/*
 * Process level energy consumption in a time interval.
 */
struct process_energy{
	int pid;
	long time;
	float ecpu;
	float emem;
	float enet;
	float edisk;
	char* cmdline;
};

/*
 * Process info in a time interval.
 */
struct process_info {
	float 			read_bytes; 	// KBs read per second
	float 			write_bytes;	// KBs written per second
	float 			cancelled_write_bytes; // KBs cancelled per second
	u32 	vsz;	// total virtual memory
	u32 	rss;	// total resident memory
	float 			mem;		// % memory in average
	float			minflt; // minor faults
	float			majflt;	// major fault
	float 			utime; 	// % user mode cpu time
	float			stime; 	// % system mode cpu time
	float      		gtime; 	// % guest mode cpu time
	float			cpu; 	// % cpu on average
	float			cswch;	// no of voluntary context switch
	float			nvcswch;// involuntary
	u32	pid;	// process id
	u32 	time;	// time stamp, in seconds
	char* 			cmdline;

	/* network activities */
	float 			netsnd; //% network sending activities
	float 			netrcv; //% network receiving activies
	/* memory activities*/
	float ratio;
	u64 cpu_accesses;
};

struct sys_info{
	u32 time;
	u64 totalnetsnd;
	u64 totalnetrcv;
	u64 totaldiskread;
	u64 totaldiskwrite;
	float cpupower;
	u64 memaccess;
	u32 itv;
	u32 runtime;
	int active_process_num;
};

#define DEVICE_ENERGY_SIZE	(sizeof(struct device_energy))
#define PROCESS_ENERGY_SIZE	(sizeof(struct process_energy))
#define PROCESS_INFO_SIZE	(sizeof(struct process_info))
#define SYS_INFO_SIZE	(sizeof(struct sys_info))

/*
 * Database access interfaces.
 */
int insert_device_energy(struct device_energy * de);

int insert_process_energy(struct process_energy * pe);

int insert_ps(struct process_info * pst);

int insert_sys_info(struct sys_info * st);

int freeup_device_energy(u32 interval);

int freeup_process_energy(u32 interval);

int freeup_ps(u32 interval);

int freeup_sys_info(u32 interval);

int freeup_all(u32 interval);

int create_conn(MYSQL **conn);

int init_stat_data();

void clean_stat_data();

#endif  /* _DATABASE_H */
