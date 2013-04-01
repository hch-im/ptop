/***************************************************************************
 * Name:display.h - Functions used to display data on the screen.
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
#include <time.h>
#include "database.h"

extern char screen;
/*
** color names
*/
#define	COLORLOW	2
#define	COLORMED	3
#define	COLORHIGH	4

// borrowed from ATOP
#define	MQUIT		'q'
#define	MKILLPROC	'k'
#define	MPAUSE		'z'
#define	MRESET		'r'
#define	MHELP		'h'

#define DAYSECS 	(24*60*60)
#define HOURSECS	(60*60)
#define MINSECS 	(60)

/* Keywords */
#define K_ISO	"ISO"
/* Environment variables */
#define ENV_TIME_FMT	"S_TIME_FORMAT"

int print_gal_header(struct tm *, char *, char *, char *, char *, int);
int show_proc_stat(struct process_info *pst_list, int amount, time_t now);
