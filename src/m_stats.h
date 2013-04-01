/***************************************************************************
 * Name:m_stats.h - Memory energy stats interfaces and data structures.
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

#ifndef M_STATS_H_
#define M_STATS_H_

/*
 * Structure for '/proc/<pid>/smaps' file
 */
struct stat_smaps{
	u32 size;
	u32 rss;
	u32 pss;
	u32 shared_clean;
	u32 shared_dirty;
	u32 private_clean;
	u32 private_dirty;
	u32 referenced;
	u32 swap;
};

void  stat_sysmem_stats(struct sys_info *info);
int total_active_memory();
void read_smaps(int pid, struct stat_smaps * stats);
void stat_sysnet_stats(struct sys_info *info);
#endif /* STATS_H_ */
