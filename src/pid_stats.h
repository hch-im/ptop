/***************************************************************************
 * Name:pid_stats.h - Process level stats functions.
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

#ifndef PID_STATS_H_
#define PID_STATS_H_

#include "common.h"
#include "avltree.h"

int read_proc_pid_stat(u32 pid, struct pid_stats *pst);

int read_proc_pid_status(u32 pid, struct pid_stats *pst);

int read_proc_pid_io(u32 pid, struct pid_stats *pst);

unsigned int count_pid(void);

int read_proc_pid_mem(u32, struct pid_stats *, struct AvlNode *, int);

#endif /* PID_STATS_H_ */
