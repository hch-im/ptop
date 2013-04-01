/***************************************************************************
 * Name:m_stats.c - Memory and network stats functions.
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
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include "common.h"
#include "eperf.h"
#include "database.h"
#include "m_stats.h"

#define BUFFER_SIZE 128
#define FORMAT "%lu"

/*
 * Functions used to stat static energy.
 */

/*
 *******************************************************************************
 * Get the memory usage info of process from file '/proc/<pid>/smaps'
 * IN:
 * @pid:   Process id.
 * @stats: Memory usage info.
 *******************************************************************************
 */
void read_smaps(int pid, struct stat_smaps * stats)
{
	char smaps_file[40];
	char buff[BUFFER_SIZE];
	struct stat_smaps stmaps;
	FILE * fp;

	memset(stats, 0, sizeof(struct stat_smaps));
	sprintf(smaps_file, "/proc/%d/smaps", pid);

	if ((fp = fopen(smaps_file, "r")) == NULL) {
		fprintf(stderr, "Cannot open %s: %s\n", smaps_file, strerror(errno));
		exit(2);
	}

	while(fgets(buff, BUFFER_SIZE, fp) != NULL)
	{
		if (!strncmp(buff, "Size:", 5)) {
			sscanf(buff + 5, "%d", &stmaps.size);
			stats->size += stmaps.size;
		} else if(!strncmp(buff, "Rss:", 4)){
			sscanf(buff + 4, "%d", &stmaps.rss);
			stats->rss += stmaps.rss;
		} else if(!strncmp(buff, "Pss:", 4)){
			sscanf(buff + 4, "%d", &stmaps.pss);
			stats->pss += stmaps.pss;
		} else if(!strncmp(buff, "Shared_Clean:", 13)){
			sscanf(buff + 13, "%d", &stmaps.shared_clean);
			stats->shared_clean += stmaps.shared_clean;
		} else if(!strncmp(buff, "Shared_Dirty:", 13)){
			sscanf(buff + 13, "%d", &stmaps.shared_dirty);
			stats->shared_dirty += stmaps.shared_dirty;
		} else if(!strncmp(buff, "Private_Clean:", 14)){
			sscanf(buff + 14, "%d", &stmaps.private_clean);
			stats->private_clean += stmaps.private_clean;
		}else if(!strncmp(buff, "Private_Dirty:", 14)){
			sscanf(buff + 14, "%d", &stmaps.private_dirty);
			stats->private_dirty += stmaps.private_dirty;
		}else if(!strncmp(buff, "Referenced:", 11)){
			sscanf(buff + 11, "%d", &stmaps.referenced);
			stats->referenced += stmaps.referenced;
		}else if(!strncmp(buff, "Swap:", 5)){
			sscanf(buff + 5, "%d", &stmaps.swap);
			stats->swap += stmaps.swap;
		}
	}

	fclose(fp);
}

/*
 *******************************************************************************
 * Get the active memory info from file '/proc/meminfo'
 * OUT:
 * Size of memory that are currently used by the system.
 *******************************************************************************
 */
int total_active_memory()
{
	char buff[BUFFER_SIZE];
	FILE * fp;
	int active;

	if ((fp = fopen(MEMINFO, "r")) == NULL) {
		exit(2);
	}

	while(fgets(buff, BUFFER_SIZE, fp) != NULL)
	{
		if (!strncmp(buff, "Active:", 7)) {
			sscanf(buff + 7, "%d", &active);
			break;
		}
	}

	fclose(fp);
	return active;
}

void  stat_sysmem_stats(struct sys_info *info){
	if(info == NULL)
		return;

	info->memaccess = read_cpu_counter();
}

void stat_sysnet_stats(struct sys_info *info){
	char buff[BUFFER_SIZE];
	FILE * fp;
	char wireless[10];
	int haswireless = 0;
	info->totalnetrcv = info->totalnetsnd = 0;
	if ((fp = fopen(NET_WIRELESS, "r")) == NULL) {
		return;
	}

	while(fgets(buff, BUFFER_SIZE, fp) != NULL)
	{
		sscanf(buff, "%s", wireless);

		if (!strncmp(wireless, "eth", 3)) {
			haswireless = 1;
			break;
		}
	}

	fclose(fp);
	if(haswireless == 0) return;

	if ((fp = fopen(NET_DEV, "r")) == NULL) {
		return;
	}

	u32 temp;
	char name[20];
	char format[10];
	int len = strlen(wireless);
	int i = 0;
	sprintf(format,"%%%ds", len);
	while(fgets(buff, BUFFER_SIZE, fp) != NULL)
	{
		sscanf(buff, format, name);
		if (!strncmp(name, wireless, len - 1)) {
			while(buff[i++] == ' ') len ++;
			sscanf(buff+ len, "%llu%u%u%u%u%u%u%u%llu%u", &info->totalnetrcv,
					&temp, &temp, &temp, &temp, &temp, &temp, &temp,&info->totalnetsnd, &i);
			break;
		}
	}

	fclose(fp);
}
