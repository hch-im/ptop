/***************************************************************************
 * Name:ptop.h - pTop.
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
#ifndef PTOP_H_
#define PTOP_H_
#include "common.h"
#include "avltree.h"
#include "database.h"

extern int cpufreq_stats;		// not support, by default;
extern AvlTree tree;
extern struct process_info pst_list[];
extern int interval;

#define SHOW_NUM 40
#define NR_PID_PREALLOC	10
#define F_NO_PID_IO	0x01
#define NO_PID_IO(m)		(((m) & F_NO_PID_IO) == F_NO_PID_IO)

void system_clean();

#endif  /* _PTOP_H */
