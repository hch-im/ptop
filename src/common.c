/***************************************************************************
 * Name:common.c - Public functions.
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
#include <time.h>
#include <errno.h>
#include <unistd.h>	/* For STDOUT_FILENO, among others */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <dirent.h>

#include "common.h"
#include "cpu_stats.h"

/* Number of ticks per second */
unsigned int hz;
/* Number of bit shifts to convert pages to kB */
unsigned int kb_shift;

/*
 ***************************************************************************
 * Get local date and time
 *
 * OUT:
 * @rectime	Current local date and time.
 *
 * RETURNS:
 * Value of time in seconds since the Epoch.
 ***************************************************************************
 */
time_t get_localtime(struct tm *rectime)
{
	time_t timer;
	struct tm *ltm;

	time(&timer);
	ltm = localtime(&timer);

	*rectime = *ltm;
	return timer;
}

/*
 ***************************************************************************
 * Look for partitions of a given block device in /sys filesystem
 *
 * IN:
 * @dev_name	Name of the block device.
 *
 * RETURNS:
 * Number of partitions for the given block device.
 ***************************************************************************
 */
int get_dev_part_nr(char *dev_name)
{
	DIR *dir;
	struct dirent *drd;
	char dfile[MAX_PF_NAME], line[MAX_PF_NAME];
	int part = 0;

	snprintf(dfile, MAX_PF_NAME, "%s/%s", SYSFS_BLOCK, dev_name);
	dfile[MAX_PF_NAME - 1] = '\0';

	/* Open current device directory in /sys/block */
	if ((dir = opendir(dfile)) == NULL)
		return 0;

	/* Get current file entry */
	while ((drd = readdir(dir)) != NULL) {
		if (!strcmp(drd->d_name, ".") || !strcmp(drd->d_name, ".."))
			continue;
		snprintf(line, MAX_PF_NAME, "%s/%s/%s", dfile, drd->d_name, S_STAT);
		line[MAX_PF_NAME - 1] = '\0';

		/* Try to guess if current entry is a directory containing a stat file */
		if (!access(line, R_OK)) {
			/* Yep... */
			part++;
		}
	}

	/* Close directory */
	closedir(dir);

	return part;
}

#ifdef USE_NLS
/*
 ***************************************************************************
 * Init National Language Support
 ***************************************************************************
 */
void init_nls(void)
{
	setlocale(LC_MESSAGES, "");
	setlocale(LC_CTYPE, "");
	setlocale(LC_TIME, "");
	setlocale(LC_NUMERIC, "");

	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
}
#endif

/*
 ***************************************************************************
 * Get nb of rows of current window
 *
 * RETURNS:
 * Number of rows.
 ***************************************************************************
 */
int get_win_height(void)
{
	struct winsize win;
	/*
	 * This default value will be used whenever STDOUT
	 * is redirected to a pipe or a file
	 */
	int rows = 3600 * 24;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) != -1) {
		if (win.ws_row > 2) {
			rows = win.ws_row - 2;
		}
	}
	return rows;
}

/*
 ***************************************************************************
 * Get page shift in kB
 ***************************************************************************
 */
void get_kb_shift(void)
{
	int shift = 0;
	long size;

	/* One can also use getpagesize() to get the size of a page */
	if ((size = sysconf(_SC_PAGESIZE)) == -1) {
		perror("sysconf");
	}

	size >>= 10;	/* Assume that a page has a minimum size of 1 kB */

	while (size > 1) {
		shift++;
		size >>= 1;
	}

	kb_shift = (unsigned int) shift;
}

/*
 ***************************************************************************
 * Get number of clock ticks per second
 ***************************************************************************
 */
void get_HZ(void)
{
	long ticks;

	if ((ticks = sysconf(_SC_CLK_TCK)) == -1) {
		perror("sysconf");
	}

	hz = (unsigned int) ticks;
}

/*
 ***************************************************************************
 * Print debug info.
 *
 * IN:
 * @level 0:debug info, >1 error info.
 * @info  Information format.
 ***************************************************************************
 */
void debug(int level, char * info){
	#ifdef DEBUG
	if(level == 0)
		fprintf(stdout, info);
	else
		fprintf(stderr, info);
	#endif
}

/*
 ***************************************************************************
 * Generic function to subtract two counters taking into
 * account the possibility of overflow of a 32-bit kernel-counter.
 *
 * IN:
 * @newval	New value
 * @oldval  Old value
 ***************************************************************************
 */
u32 subcount(u32 newval, u32 oldval)
{
	if (newval >= oldval)
		return newval - oldval;
	else
		return (MAXU32VAL- oldval) + newval;
}

u64 u64_subcount(u64 newval, u64 oldval)
{
	if (newval >= oldval)
		return newval - oldval;
	else
		return (MAXU64VAL - oldval) + newval;
}
