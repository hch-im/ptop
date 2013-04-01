/***************************************************************************
 * Name:ptop.c - ptop main functions.
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
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <stdint.h>
#include <mysql/mysql.h>
#include <pthread.h>

#include "ptop.h"
#include "common.h"
#include "cpu_stats.h"
#include "m_stats.h"
#include "database.h"
#include "pid_stats.h"
#include "display.h"
#include "eperf.h"
#include "energy.h"

#define CLEAN_INTERVAL 31  //clean interval
struct sys_info sys_stats[3];
struct process_info pst_list[41];
int cpu_nr = 0;			    /* Number of processors on the machine */
u32 tlmkb;		/* Total memory in kB */
int cpufreq_stats = 0;
int interval = -1;			/* sample interval */
AvlTree tree = NULL;				/* AvlTree used to store performance counter of each process*/
int running_display = 1;

void add_higher_process_info(struct process_info *, int);

/*
 ***************************************************************************
 * SIGALRM signal handler.
 *
 * IN:
 * @sig	Signal number. Set to 0 for the first time, then to SIGALRM.
 ***************************************************************************
 */
void alarm_handler(int sig)
{
	signal(SIGALRM, alarm_handler);
	alarm(interval);
}

/*
 ***************************************************************************
 * Get current PID to display.
 * First, check that PID exists. *Then* check that it's an active process
 * and/or that the string (entered on the command line with option -C) is
 * found in command name.
 *
 * IN:
 * @prev	Index in array where stats used as reference are.
 * @curr	Index in array for current sample statistics.
 * @p		Index in process list.
 *
 * OUT:
 * @psti	Structure with PID statistics for current sample.
 * @pstj	Structure with PID statistics for previous sample.
 *
 * RETURNS:
 *  0 if PID no longer exists.
 * -1 if PID exists but should not be displayed.
 *  1 if PID can be displayed.
 ***************************************************************************
 */
int is_active(struct pid_stats* pstj, struct pid_stats* psti)
{
	if (psti == NULL || psti->pid <= 0)
		return 0;

	int isActive = FALSE;

	if ((psti->utime != pstj->utime) ||
				(psti->stime != pstj->stime)) {
		return TRUE;
	}
	else {
		if ((psti->cutime != pstj->cutime) ||
			    (psti->cstime != pstj->cstime)) {
			return TRUE;
		}
	}

	if ((psti->minflt != pstj->minflt) ||
		    (psti->majflt != pstj->majflt)) {
		return TRUE;
	}
	else {
		if ((psti->vsz != pstj->vsz) ||
			    (psti->rss != pstj->rss)) {
			return TRUE;
		}

		if ((psti->cminflt != pstj->cminflt) ||
			    (psti->cmajflt != pstj->cmajflt)) {
			return TRUE;
		}
	}

	if ((psti->read_bytes  != pstj->read_bytes)  ||
			(psti->write_bytes != pstj->write_bytes) ||
			(psti->cancelled_write_bytes !=
			 pstj->cancelled_write_bytes)) {
		return TRUE;
	}

	if ((psti->nvcsw  != pstj->nvcsw) ||
			(psti->nivcsw != pstj->nivcsw)) {
		return TRUE;
	}

	if (!isActive){
		return -1;
	}

	return 1;
}

/*
 ***************************************************************************
 * Init system state.
 ***************************************************************************
 */
void system_init(void)
{
	/* Default cpu frequency account is not supported.*/
	cpufreq_stats = 0;
	/* delete all the data and Initializa data access module*/
	if(init_stat_data() == 0){
		exit(0);
	}
	// Update cpu max and min power consumption values
	//update_cpu_maxmin_values();init_perf

#ifdef USE_NLS
	/* Init National Language Support */
	init_nls();
#endif
	/* Get HZ */
	get_HZ();
	/* Compute page shift in kB */
	get_kb_shift();
	/* Set the cpufreq_stats flag.*/
	cpufreq_stats = check_cpufreq_stats();
	/* Count nb of proc */
	cpu_nr = get_cpu_nr(~0);
	memset(&sys_stats, 0, sizeof(struct sys_info) * 2);
	init_perf(tree);
	clean_stat_data();
}

/*
 ***************************************************************************
 * Free structures according to system state.
 ***************************************************************************
 */
void system_clean()
{
	running_display = 0;
	/* Clean data access module*/
	clean_stat_data();
	/*close performance counter*/
	close_all_counters(tree);
}


/*
 ***************************************************************************
 * Read various stats for given PID.
 *
 * IN:
 * @pid		Process whose stats are to be read.
 * @pst		Pointer on structure where stats will be saved.
 *
 * OUT:
 * @pst		Pointer on structure where stats have been saved.

 *
 * RETURNS:
 * 0 if stats have been successfully read, and 1 otherwise.
 ***************************************************************************
 */
int read_pid_stats(u32 pid, struct pid_stats *pst, Position p, int curr)
{
	int active = FALSE;
	memset(pst, 0, sizeof(struct pid_stats));

	if (read_proc_pid_stat(pid, pst))
		return 1;

	if (read_proc_pid_status(pid, pst))
		return 1;

	if (read_proc_pid_io(pid, pst))
		return 1;

	active = is_active(p->pst_list[curr^1], pst);
	if (read_proc_pid_mem(pid,pst, p, active))
		return 1;

	return 0;
}

/*
 ***************************************************************************
 * Read various stats the the currently active process.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 ***************************************************************************
 */
void read_process_stats(int curr)
{
	DIR *dir;
	struct dirent *drp;
	u32 pid;
	struct pid_stats *psti;
	Position position;

	read_stat_cpu(&sys_stats[curr]);

	/* Open /proc directory */
	if ((dir = opendir(PROC)) == NULL) {
		perror("opendir /proc error!");
		exit(4);
	}

	while (1) {
		/* Get directory entries */
		while ((drp = readdir(dir)) != NULL) {
			if (isdigit(drp->d_name[0]))
				break;
		}

		if (drp) {
			pid = atoi(drp->d_name);
			position = find( pid, tree);
			if(!position){
				insert( pid, -1, &tree, &position);
			}

			psti = position->pst_list[curr];
			if (read_pid_stats(pid, psti, position, curr)) {
				/* Process has terminated */
				psti->pid = 0;
			}
		}
		else {
			break;
		}
	}

	/* Close /proc directory */
	closedir(dir);
}
/*
 ***************************************************************************
 * Read system level stats.
 */
void read_system_stats(int curr)
{
	read_cpufreq_stats(&sys_stats[curr]);
	stat_sysmem_stats(&sys_stats[curr]);
	stat_sysnet_stats(&sys_stats[curr]);
}

void write_system_stats(int curr, long time, int amount)
{
	int prev = !curr;

	sys_stats[curr].time = time;
	memcpy(&sys_stats[2], &sys_stats[curr], sizeof(struct sys_info));
	sys_stats[2].totalnetrcv = u64_subcount(sys_stats[curr].totalnetrcv, sys_stats[prev].totalnetrcv);
	sys_stats[2].totalnetsnd = u64_subcount(sys_stats[curr].totalnetsnd, sys_stats[prev].totalnetsnd);
	sys_stats[2].itv = u64_subcount(sys_stats[curr].itv, sys_stats[prev].itv);
	sys_stats[2].runtime = u64_subcount(sys_stats[curr].runtime, sys_stats[prev].runtime);
	sys_stats[2].memaccess = u64_subcount(sys_stats[curr].memaccess, sys_stats[prev].memaccess);
	sys_stats[2].active_process_num = amount;

	insert_sys_info(&sys_stats[2]);
}

void write_stat(int curr, Position position, long time, u64 itv, int * idx){
	struct process_info pst_db;		// process stats per second
	struct pid_stats *psti, *pstj;
	int prev = !curr;

	if(!position || !position->pst_list[0]){
	}else{
		psti = position->pst_list[curr];
		pstj = position->pst_list[prev];

		if (is_active(pstj, psti) <= 0){
			clear_position(position);
		}else{
			sys_stats[curr].totaldiskread += subcount(psti->read_bytes, pstj->read_bytes);
			sys_stats[curr].totaldiskwrite += (subcount(psti->write_bytes, pstj->write_bytes) - subcount(psti->cancelled_write_bytes, pstj->cancelled_write_bytes));

			memset(&pst_db, 0, PROCESS_INFO_SIZE);

			pst_db.time	= time;
			pst_db.pid = psti->pid;
			if(psti->utime < pstj->utime && (pstj->utime - psti->utime) < 100) psti->utime = pstj->utime;
			if(psti->stime < pstj->stime && (pstj->stime - psti->stime) < 100) psti->stime = pstj->stime;
			if(psti->gtime < pstj->gtime && (pstj->gtime - psti->gtime) < 100) psti->gtime = pstj->gtime;
			pst_db.utime = (u64_subcount(psti->utime, psti->utime)- u64_subcount(psti->gtime, pstj->gtime)) * (100.0f / itv);
			pst_db.stime = u64_subcount(psti->stime, pstj->stime) * (100.0f / itv);
			pst_db.gtime = u64_subcount(psti->gtime, pstj->gtime) * (100.0f / itv);
			pst_db.cpu	= (u64_subcount(psti->utime , pstj->utime) + u64_subcount( psti->stime , pstj->stime)) * (100.0f / itv);


			pst_db.minflt =  S_VALUE(pstj->minflt, psti->minflt, itv);
			pst_db.majflt = S_VALUE(pstj->majflt, psti->majflt, itv);
			pst_db.vsz = psti->vsz;
			pst_db.rss = psti->rss;
			pst_db.mem = tlmkb ? SP_VALUE(0, psti->rss, tlmkb) : 0.0;

			pst_db.read_bytes = u64_subcount(psti->read_bytes,  pstj->read_bytes);
			pst_db.write_bytes = u64_subcount(psti->write_bytes, pstj->write_bytes);
			pst_db.cancelled_write_bytes = u64_subcount(psti->cancelled_write_bytes, pstj->cancelled_write_bytes);
			if(pst_db.cancelled_write_bytes > pst_db.write_bytes) pst_db.cancelled_write_bytes = pst_db.write_bytes;

			pst_db.cswch = S_VALUE(pstj->nvcsw, psti->nvcsw, itv);
			pst_db.nvcswch = S_VALUE(pstj->nivcsw, psti->nivcsw, itv);

			pst_db.cmdline = psti->comm;

			// calculate % network send and receive activities
			pst_db.netsnd = u64_subcount(psti->net.totalnetsnd, pstj->net.totalnetsnd);
			pst_db.netrcv = u64_subcount(psti->net.totalnetrcv, pstj->net.totalnetrcv);

			pst_db.ratio = 0.0;
			if(psti->mem.total_active > 0)
					pst_db.ratio = (psti->mem.memory_used * 100.0f) / psti->mem.total_active;
			if(psti->mem.cpu_accesses>0)
				pst_db.cpu_accesses = u64_subcount(psti->mem.cpu_accesses, pstj->mem.cpu_accesses);

			if(screen && (pst_db.cpu  > 0 || pst_db.cpu_accesses > 0
					|| (pst_db.netrcv + pst_db.netsnd) > 0 || (pst_db.netrcv + pst_db.netsnd) > 0) ){
				add_higher_process_info(&pst_db, *idx >=SHOW_NUM ? SHOW_NUM : *idx);
				(*idx)++;
			}

			insert_ps(&pst_db);	// insert new record
		}
	}
	//write childs
	if(position->Left) write_stat(curr, position->Left, time, itv, idx);
	if(position->Right) write_stat(curr, position->Right, time, itv, idx);
}
/*
 ***************************************************************************
 * Get previous and current timestamps, then display statistics.
 *
 * IN:
 * @curr	Index in array for current sample statistics.
 * @dis		TRUE if a header line must be printed.
 *
 * RETURNS:
 * 0 if all the processes to display have terminated.
 * <> 0 if there are still some processes left to display.
 ***************************************************************************
 */
int write_process_stats(int curr, long time)
{
    u64 itv; //CPU usage time during the interval, jitter
    int prev = !curr;

    itv = u64_subcount(sys_stats[curr].runtime, sys_stats[prev].runtime);

	int idx = 0;
	write_stat(curr, tree, time, itv, &idx);

	return idx - 1;
}

void test(int amount, time_t now);
/*
 ***************************************************************************
 * Main loop: Read PID stats.
 ***************************************************************************
 */
void main_loop()
{
	int curr = 1;
	/* Don't buffer data if redirected to a pipe */
	setbuf(stdout, NULL);

	/* Set a handler for SIGALRM */
	alarm_handler(0);

	int cycle = 1;
	long ctime;
	int amount = 0;
	do {
		//Reset sys info
		memset(&sys_stats[curr], 0, sizeof(struct sys_info));

		ctime = time(NULL);
		//read the process level stats
		read_process_stats(curr);
		//read the system level stats
		read_system_stats(curr);

		/* Record process level stat results */
		amount = write_process_stats(curr, ctime);
		/* Record process level stat results */
		write_system_stats(curr, ctime, amount);
//		test(amount, ctime);
   	    if(screen && cycle%3 == 0)  show_proc_stat(pst_list, amount, ctime);
		if(cycle%CLEAN_INTERVAL == 0) freeup_all(CLEAN_INTERVAL);
		cycle++;
		//Change the array that record temp stat data.
		curr ^= 1;
		pause();
	}
	while (TRUE);
}

/* Customized main method */
int main(int argc, char **argv)
{
	struct utsname header;
	struct tm time;

	if(argc != 2) {
		printf("Usage: ./ptop [sample rate]\n");
		interval = 1;
	}else{
		interval = atoi(argv[1]);
	}

	if (interval <= 0) {
		/* Interval not set => display stats since boot time */
		interval = 1;
	}

	printf("Begin to Initialize...");
	system_init();

	/* Get system name, release number and hostname */
	get_localtime(&time);	/* Get time */
	uname(&header);
	print_gal_header(&time, header.sysname, header.release,
			 header.nodename, header.machine, cpu_nr);
	/* Main loop */
	printf("pTop running...\n");

	main_loop();
	/* Clean before exit.*/
	system_clean();
	return 0;
}

void add_higher_process_info(struct process_info * pi, int length){
	int i = length - 1;

	for(;i >= 0; i--){
		if(pst_list[i].cpu < pi->cpu && i != (SHOW_NUM - 1)){
			memcpy(&pst_list[i + 1], &pst_list[i], PROCESS_INFO_SIZE);
		}else{
			break;
		}
	}

	if((i+1) <= (SHOW_NUM  - 1)) memcpy(&pst_list[i + 1], pi, PROCESS_INFO_SIZE);
}

void test(int amount, time_t now){
	char format3[128];
	int tlength = interval;
	struct device_energy pe;
	int len2 = 10;
	sprintf(format3, "%%-%ds%%-%ds%%-%ds%%-%ds%%-%ds%%-%ds%%-%ds\n", len2, len2, len2, len2, len2, len2, len2);
	printf(format3,
			"PID", "ALL(Percent)", "ECPU", "EDISK", "ENET","EMEM", "CMD");

	struct process_info * pst;
	int j;
	float ecpu,edisk,entw,emem, eall, etotalsys, percent;

	amount = amount > SHOW_NUM ? SHOW_NUM:amount;
	sprintf(format3, "%%-%ds%%-%d.3f%%-%d.3f%%-%d.3f%%-%d.3f%%-%d.3f%%-%ds\n", len2, len2, len2, len2, len2, len2, len2);
	systemEnergy(tlength,now, &pe);
	etotalsys = pe.ecpu + pe.edisk + pe.emem + pe.enet;
	printf(format3,
				"", 100.00f, pe.ecpu, pe.edisk, pe.enet, pe.emem, "");
	sprintf(format3, "%%-%dd%%-%d.3f%%-%d.3f%%-%d.3f%%-%d.3f%%-%d.3f%%-%ds\n", len2, len2, len2, len2, len2, len2, len2);

	for(j=0;j<amount;j++)
	{
		pst = &pst_list[j];
		ecpu 	= CPUEnergy(pst->pid, tlength,now);
		edisk	= DiskEnergy(pst->pid, tlength,now);
		entw	= NtwkEnergy(pst->pid, tlength,now);
		emem    = MemoryEnergy(pst->pid, tlength, interval,now);
		eall = ecpu + edisk + emem;

		percent = (etotalsys> 0)?eall / etotalsys * 100 : 0;
		printf(format3,
					pst->pid, percent, ecpu, edisk, entw, emem, pst->cmdline);
	}
}
