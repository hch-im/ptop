/***************************************************************************
 * Name:pid_stats.c - Implementation of process level stats functions.
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
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

#include "pid_stats.h"
#include "ptop.h"
#include "m_stats.h"
#include "eperf.h"
#include "avltree.h"
/*
 ***************************************************************************
 * Read stats from /proc/#[/task/##]/stat.
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
int read_proc_pid_stat(u32 pid, struct pid_stats *pst)
{
	FILE *fp;
	char filename[128], format[1024], comm[MAX_COMM_LEN + 1];
	size_t len;
	u32 thread_nr;
	sprintf(filename, PID_STAT, pid);

	if ((fp = fopen(filename, "r")) == NULL){
		/* No such process */
		return 1;
	}

	sprintf(format, "%%*d (%%%ds %%*s %%*d %%*d %%*d %%*d %%*d %%*u %%lu %%lu"
		" %%lu %%lu %%lu %%lu %%lu %%lu %%*d %%*d %%u %%*u %%*d %%lu %%lu"
		" %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u %%*u"
		" %%*u %%u %%*u %%*u %%*u %%lu %%lu\\n", MAX_COMM_LEN);

	fscanf(fp, format, comm,
	       &pst->minflt, &pst->cminflt, &pst->majflt, &pst->cmajflt,
	       &pst->utime,  &pst->stime, &pst->cutime, &pst->cstime,
	       &thread_nr, &pst->vsz, &pst->rss, &pst->processor,
	       &pst->gtime, &pst->cgtime);

	fclose(fp);

	/* Convert to kB */
	pst->vsz >>= 10;	// virtual memory size
	pst->rss = PG_TO_KB(pst->rss);	// resident set size: nb of pages has in mem

	strncpy(pst->comm, comm, MAX_COMM_LEN);
	pst->comm[MAX_COMM_LEN - 1] = '\0';

	/* Remove trailing ')' */
	len = strlen(pst->comm);
	if (len && (pst->comm[len - 1] == ')')) {
		pst->comm[len - 1] = '\0';
	}

	pst->pid = pid;

	/* code for reading network activities is here */

	char line[4096];

	/* open again to read --> borrow from atop */
	if ((fp = fopen(filename, "r")) == NULL)
		/* No such process */
		return 1;

	if (fgets(line, sizeof line, fp) == NULL)
	{
		fclose(fp);
		return 0;
	}

	if ( fgets(line, sizeof line, fp) != NULL)
	{
		sscanf(line, "%lld %llu %lld %llu %lld %llu %lld %llu %lld %llu %lld %llu %lld %lld",
			&pst->dsk.rio, &pst->dsk.rsz, &pst->dsk.wio, &pst->dsk.wsz,
			&pst->net.tcpsnd, &pst->net.tcpssz, &pst->net.tcprcv, &pst->net.tcprsz,
			&pst->net.udpsnd, &pst->net.udpssz, &pst->net.udprcv, &pst->net.udprsz,
			&pst->net.rawsnd, &pst->net.rawrcv);

		pst->net.totalnetsnd = pst->net.tcpsnd + pst->net.udpsnd + pst->net.rawsnd;
		pst->net.totalnetrcv = pst->net.tcprcv + pst->net.udprcv + pst->net.rawrcv;
	}
	fclose(fp);
	return 0;
}

/*
 *****************************************************************************
 * Read stats from /proc/#[/task/##]/status.
 *
 * IN:
 * @pid		Process whose stats are to be read.
 * @pst		Pointer on structure where stats will be saved.
 * @tgid	If !=0, thread whose stats are to be read.
 *
 * OUT:
 * @pst		Pointer on structure where stats have been saved.
 *
 * RETURNS:
 * 0 if stats have been successfully read, and 1 otherwise.
 *****************************************************************************
 */
int read_proc_pid_status(u32 pid, struct pid_stats *pst)
{
	FILE *fp;
	char filename[128], line[256];
	sprintf(filename, PID_STATUS, pid);

	if ((fp = fopen(filename, "r")) == NULL)
		/* No such process */
		return 1;

	while (fgets(line, 256, fp) != NULL) {

		if (!strncmp(line, "voluntary_ctxt_switches:", 24)) {
			sscanf(line + 25, "%u", &(pst->nvcsw));
		}
		else if (!strncmp(line, "nonvoluntary_ctxt_switches:", 27)) {
			sscanf(line + 28, "%u", &(pst->nivcsw));
		}
	}

	fclose(fp);

	pst->pid = pid;
	return 0;
}


/*
 ***************************************************************************
 * Read stats from /proc/#[/task/##]/io.
 *
 * IN:
 * @pid		Process whose stats are to be read.
 * @pst		Pointer on structure where stats will be saved.
 *
 * OUT:
 * @pst		Pointer on structure where stats have been saved.
 *
 * RETURNS:
 * 0 if stats have been successfully read.
 * Also returns 0 if current process has terminated or if its io file
 * doesn't exist, but in this case, set process' F_NO_PID_IO flag to
 * indicate that I/O stats should no longer be read for it.
 ***************************************************************************
 */
int read_proc_pid_io(u32 pid, struct pid_stats *pst)
{
	FILE *fp;
	char filename[128], line[256];

	sprintf(filename, PID_IO, pid);
	if ((fp = fopen(filename, "r")) == NULL) {
		return 1;
	}

	while (fgets(line, 256, fp) != NULL) {

		if (!strncmp(line, "read_bytes:", 11)) {
			sscanf(line + 12, "%llu", &pst->read_bytes);
		}
		else if (!strncmp(line, "write_bytes:", 12)) {
			sscanf(line + 13, "%llu", &pst->write_bytes);
		}
		else if (!strncmp(line, "cancelled_write_bytes:", 22)) {
			sscanf(line + 23, "%llu", &pst->cancelled_write_bytes);
		}
	}

	fclose(fp);
	pst->pid = pid;

	return 0;
}

int read_proc_pid_mem(u32 pid, struct pid_stats *pst, struct AvlNode* p, int active){
	struct stat_smaps smaps;

	read_smaps(pid, &smaps);
	pst->mem.memory_used = smaps.pss + smaps.private_clean + smaps.private_dirty;
	pst->mem.total_active = total_active_memory();

	if(active && p->fd<=0){
		p->fd = create_perf_stat_counter(pid, FALSE);
	}
	pst->mem.cpu_accesses = read_counter(p->fd);

	return 0;
}

/*
 ***************************************************************************
 * Count number of processes.
 *
 * RETURNS:
 * Number of processes.
 ***************************************************************************
 */
unsigned int count_pid(void)
{
	DIR *dir;
	struct dirent *drp;
	u32 pid = 0;

	/* Open /proc directory */
	if ((dir = opendir(PROC)) == NULL) {
		perror("opendir");
		exit(4);
	}

	/* Get directory entries */
	while ((drp = readdir(dir)) != NULL) {
		if (isdigit(drp->d_name[0])) {
			/* There is at least the TGID */
			pid++;
		}
	}

	/* Close /proc directory */
	closedir(dir);

	return pid;
}
