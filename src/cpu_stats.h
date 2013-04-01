/***************************************************************************
 * Name:cpu_stats.h - CPU stats interfaces and data structures.
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

#ifndef CPU_STATS_H_
#define CPU_STATS_H_
#include <stdint.h>
#include "database.h"

#define CPU_DIR "/sys/devices/system/cpu"
#define CPU_FREQUENCY_STAT "/sys/devices/system/cpu/%s/cpufreq/stats/time_in_state"
/*
 * Structure for CPU statistics.
 * In activity buffer: First structure is for global CPU utilisation ("all").
 * Following structures are for each individual CPU (0, 1, etc.)
 */
struct stats_cpu {
	u64 cpu_user;
	u64 cpu_nice;
	u64 cpu_sys;
	u64 cpu_idle;
	u64 cpu_iowait;
	u64 cpu_steal;
	u64 cpu_hardirq;
	u64 cpu_softirq;
	u64 cpu_guest;
};

#define STATS_CPU_SIZE	(sizeof(struct stats_cpu))

/* Structure for memory and swap space utilization statistics */
struct stats_memory {
	u32 frmkb;
	u32 bufkb;
	u32 camkb;
	u32 tlmkb;	// Total memory in KB
	u32 frskb;
	u32 tlskb;
	u32 caskb;
	u32 comkb;
};

#define STATS_MEMORY_SIZE	(sizeof(struct stats_memory))

struct cpufreqdata {
	u64	frequency;
	u64	count;
	float		power; // what power consumption
};

/*
 ***************************************************************************
 * Prototypes for functions used to read system statistics
 ***************************************************************************
 */

void read_stat_cpu(struct sys_info * sy);
void read_meminfo(struct stats_memory *);
void read_cpufreq_stats(struct sys_info *);
/*
 ***************************************************************************
 * Prototypes for functions used to count number of items
 ***************************************************************************
 */
int get_cpu_nr(unsigned int);

int check_cpufreq_stats(void);

#endif /* _RD_STATS_H */
