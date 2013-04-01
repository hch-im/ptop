/***************************************************************************
 * Name:cpu_stats.c - CPU stats functions.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"
#include "cpu_stats.h"
#include "database.h"
#include "ptop.h"
#include "energy.h"

// for CPU frequency stuff
struct cpufreqdata freqs[16];
struct cpufreqdata oldfreqs[16];
struct cpufreqdata delta[16];

int topfreq = -1;

/*
 ***************************************************************************
 * Read CPU statistics and machine uptime.
 *
 * IN:
 * @st_cpu	Structure where stats will be saved.
 * @nbr		Total number of CPU (including cpu "all").
 *
 * OUT:
 * @st_cpu	Structure with statistics.
 * @uptime	Machine uptime multiplied by the number of processors.
 * @uptime0	Machine uptime. Filled only if previously set to zero.
 ***************************************************************************
 */
void read_stat_cpu(struct sys_info * sy)
{
	FILE *fp;
	struct stats_cpu st_cpu;
	char line[1024];

	if ((fp = fopen(STAT, "r")) == NULL) {
		fprintf(stderr, _("Cannot open %s: %s\n"), STAT, strerror(errno));
		exit(2);
	}

	memset(&st_cpu, 0, STATS_CPU_SIZE);
	if (fgets(line, 1024, fp) != NULL) {
		sscanf(line + 5, "%llu %llu %llu %llu %llu %llu %llu %llu %llu",
			       &st_cpu.cpu_user,
			       &st_cpu.cpu_nice,
			       &st_cpu.cpu_sys,
			       &st_cpu.cpu_idle,
			       &st_cpu.cpu_iowait,
			       &st_cpu.cpu_hardirq,
			       &st_cpu.cpu_softirq,
			       &st_cpu.cpu_steal,
			       &st_cpu.cpu_guest);

			sy->itv = st_cpu.cpu_user + st_cpu.cpu_nice    +
				st_cpu.cpu_sys    + st_cpu.cpu_idle    +
				st_cpu.cpu_iowait + st_cpu.cpu_hardirq +
				st_cpu.cpu_steal  + st_cpu.cpu_softirq;

			sy->runtime = st_cpu.cpu_user + st_cpu.cpu_sys - st_cpu.cpu_guest;
	}
	fclose(fp);
}

/*
 ***************************************************************************
 * Read memory statistics from /proc/meminfo.
 *
 * IN:
 * @st_memory	Structure where stats will be saved.
 *
 * OUT:
 * @st_memory	Structure with statistics.
 ***************************************************************************
 */
void read_meminfo(struct stats_memory *st_memory)
{
	FILE *fp;
	char line[128];

	if ((fp = fopen(MEMINFO, "r")) == NULL)
		return;

	while (fgets(line, 128, fp) != NULL) {

		if (!strncmp(line, "MemTotal:", 9)) {
			/* Read the total amount of memory in kB */
			sscanf(line + 9, "%u", &st_memory->tlmkb);
		}
		else if (!strncmp(line, "MemFree:", 8)) {
			/* Read the amount of free memory in kB */
			sscanf(line + 8, "%u", &st_memory->frmkb);
		}
		else if (!strncmp(line, "Buffers:", 8)) {
			/* Read the amount of buffered memory in kB */
			sscanf(line + 8, "%u", &st_memory->bufkb);
		}
		else if (!strncmp(line, "Cached:", 7)) {
			/* Read the amount of cached memory in kB */
			sscanf(line + 7, "%u", &st_memory->camkb);
		}
		else if (!strncmp(line, "SwapCached:", 11)) {
			/* Read the amount of cached swap in kB */
			sscanf(line + 11, "%u", &st_memory->caskb);
		}
		else if (!strncmp(line, "SwapTotal:", 10)) {
			/* Read the total amount of swap memory in kB */
			sscanf(line + 10, "%u", &st_memory->tlskb);
		}
		else if (!strncmp(line, "SwapFree:", 9)) {
			/* Read the amount of free swap memory in kB */
			sscanf(line + 9, "%u", &st_memory->frskb);
		}
		else if (!strncmp(line, "Committed_AS:", 13)) {
			/* Read the amount of commited memory in kB */
			sscanf(line + 13, "%u", &st_memory->comkb);
		}
	}

	fclose(fp);
}

/*
 ***************************************************************************
 * Count number of processors in /sys.
 *
 * RETURNS:
 * Number of processors (online and offline).
 * A value of 0 means that /sys was not mounted.
 * A value of N (!=0) means N processor(s) (0 .. N-1).
 ***************************************************************************
 */
int get_sys_cpu_nr(void)
{
	DIR *dir;
	struct dirent *drd;
	struct stat buf;
	char line[MAX_PF_NAME];
	int proc_nr = 0;

	/* Open relevant /sys directory */
	if ((dir = opendir(SYSFS_DEVCPU)) == NULL)
		return 0;

	/* Get current file entry */
	while ((drd = readdir(dir)) != NULL) {

		if (!strncmp(drd->d_name, "cpu", 3) && isdigit(drd->d_name[3])) {
			snprintf(line, MAX_PF_NAME, "%s/%s", SYSFS_DEVCPU, drd->d_name);
			line[MAX_PF_NAME - 1] = '\0';
			if (stat(line, &buf) < 0)
				continue;
			if (S_ISDIR(buf.st_mode)) {
				proc_nr++;
			}
		}
	}

	/* Close directory */
	closedir(dir);

	return proc_nr;
}

/*
 ***************************************************************************
 * Count number of processors in /proc/stat.
 *
 * RETURNS:
 * Number of processors. The returned value is greater than or equal to the
 * number of online processors.
 * A value of 0 means one processor and non SMP kernel.
 * A value of N (!=0) means N processor(s) (0 .. N-1) with SMP kernel.
 ***************************************************************************
 */
int get_proc_cpu_nr(void)
{
	FILE *fp;
	char line[16];
	int num_proc, proc_nr = -1;

	if ((fp = fopen(STAT, "r")) == NULL) {
		fprintf(stderr, _("Cannot open %s: %s\n"), STAT, strerror(errno));
		exit(1);
	}

	while (fgets(line, 16, fp) != NULL) {

		if (strncmp(line, "cpu ", 4) && !strncmp(line, "cpu", 3)) {
			sscanf(line + 3, "%d", &num_proc);
			if (num_proc > proc_nr) {
				proc_nr = num_proc;
			}
		}
	}

	fclose(fp);

	return (proc_nr + 1);
}

/*
 ***************************************************************************
 * Count the number of processors on the machine.
 * Try to use /sys for that, or /proc/stat if /sys doesn't exist.
 *
 * IN:
 * @max_nr_cpus	Maximum number of proc that sysstat can handle.
 *
 * RETURNS:
 * Number of processors.
 * 0: one proc and non SMP kernel.
 * 1: one proc and SMP kernel (NB: On SMP machines where all the CPUs but
 *    one have been disabled, we get the total number of proc since we use
 *    /sys to count them).
 * 2: two proc...
 ***************************************************************************
 */
int get_cpu_nr(unsigned int max_nr_cpus)
{
	int cpu_nr;
	cpu_nr = sysconf(_SC_NPROCESSORS_ONLN);//the the number of CPU
//	if ((cpu_nr = get_sys_cpu_nr()) == 0) {
//		/* /sys may be not mounted. Use /proc/stat instead */
//		cpu_nr = get_proc_cpu_nr();
//	}

	if (cpu_nr > max_nr_cpus) {
		fprintf(stderr, _("Cannot handle so many processors!\n"));
		exit(1);
	}

	return cpu_nr;
}

/*
 *****************************************************************
 * Check if the system supports cpu freq accounting
 * RETURNS:
 * 0: Not supported.
 * 1: Supported.
 *****************************************************************
 */
int check_cpufreq_stats(void) {
	FILE *file;
	int cpufreq_stats;
	file = fopen("/sys/devices/system/cpu/cpu0/cpufreq/stats/time_in_state", "r");
	if (!file)
		cpufreq_stats = 0;	// not support
	else{
		cpufreq_stats = 1;		// support
		fclose(file);
	}

	return cpufreq_stats;
}

/*
 ****************************************************************
	Calculate cpu freq stat, and relevant power consumption.
 ****************************************************************
 */
void  read_cpufreq_stats(struct sys_info *info)
{
	info->time = time(NULL);
	info->cpupower = 0.0;
	if (!cpufreq_stats) {	//Not supported
		info->cpupower = 0;
		return;
	}
	else {
		DIR *dir;
		struct dirent *dirent;
		FILE *file;
		char filename[PATH_MAX];
		char line[32];
		float rangecpupower = maxcpupower - mincpupower;
		float delta_power;
		int ret = 0;
		int maxfreq = 0;
		u64 total_time = 0;

		memcpy(&oldfreqs, &freqs, sizeof(freqs));

		for (ret = 0; ret<16; ret++)
			freqs[ret].count = 0;

		dir = opendir(CPU_DIR);
		if (!dir)
			return;

		int i;
		while ((dirent = readdir(dir))) {
			if (strncmp(dirent->d_name,"cpu", 3) != 0)
				continue;

			sprintf(filename, CPU_FREQUENCY_STAT, dirent->d_name);

			file = fopen(filename, "r");
			if (!file)
				continue;

			memset(line, 0, 32);
			i = 0;
			while (!feof(file)) {
				if(fgets(line, 32,file) == NULL)
					break;

				sscanf(line, "%llu %llu", &freqs[i].frequency, &freqs[i].count);

				i++;
				if (i>15)
					break;
			}

			maxfreq = i - 1;
			fclose(file);
		}

		closedir(dir);
		//count unit 10ms, usertime
		for (ret = 0; ret < 16; ret++) {
			delta[ret].count = freqs[ret].count - oldfreqs[ret].count;
			total_time += delta[ret].count;
			delta[ret].frequency = freqs[ret].frequency;
		}

		delta_power = 0.0;
		for (ret = 0; ret <= maxfreq; ret++) {
			if(total_time > 0 && maxfreq > 0)
				delta_power = (maxcpupower - rangecpupower * ret / maxfreq) * delta[ret].count / 100;//Watt * 10ms
			info->cpupower += delta_power;
		}
	}
}
