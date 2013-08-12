/***************************************************************************
 * Name:display.c - Display functions.
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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <ncurses.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "display.h"
#include "database.h"
#include "ptop.h"
#include "energy.h"

char screen 	= 1;
static int	paused;
/*
 ***************************************************************************
 * Print banner
 *
 * IN:
 * @rectime	Date and time to display.
 * @sysname	System name to display.
 * @release	System release number to display.
 * @nodename	Hostname to display.
 * @machine	Machine architecture to display.
 * @cpu_nr	Number of CPU.
 *
 * RETURNS:
 * TRUE if S_TIME_FORMAT is set to ISO, or FALSE otherwise.
 ***************************************************************************
 */
int print_gal_header(struct tm *rectime, char *sysname, char *release,
		     char *nodename, char *machine, int cpu_nr)
{
	char cur_date[64];
	char *e;
	int rc = 0;

	if (((e = getenv(ENV_TIME_FMT)) != NULL) && !strcmp(e, K_ISO)) {
		strftime(cur_date, sizeof(cur_date), "%Y-%m-%d", rectime);
		rc = 1;
	}
	else {
		strftime(cur_date, sizeof(cur_date), "%x", rectime);
	}

	printf("%s %s (%s) \t%s \t_%s_\t(%d CPU)\n", sysname, release, nodename,
	       cur_date, machine, cpu_nr);

	return rc;
}

/*
 * Function val2elapstr() converts a value (number of seconds)
 * to an ascii-string of up to max 13 positions in NNNdNNhNNmNNs
 * stored in strvalue (at least 8 positions).
 * returnvalue: number of bytes stored
*/
int val2elapstr(int value, char *strvalue)
{
        char	*p=strvalue;

        if (value > DAYSECS)
        {
                p+=sprintf(p, "%dd", value/DAYSECS);
                value %= DAYSECS;
        }

        if (value > HOURSECS)
        {
                p+=sprintf(p, "%dh", value/HOURSECS);
                value %= HOURSECS;
        }

        if (value > MINSECS)
        {
                p+=sprintf(p, "%dm", value/MINSECS);
                value %= MINSECS;
        }

        if (value)
        {
                p+=sprintf(p, "%ds", value);
        }

        return p-strvalue;
}

/*
** Function numeric() checks if the ascii-string contains
** a numeric (positive) value.
** Returns 1 (true) if so, or 0 (false).
*/
int numeric(char *ns)
{
	register char *s = ns;

	while (*s)
		if (*s < '0' || *s > '9')
			return(0);		/* false */
		else
			s++;
	return(1);				/* true  */
}

/*
** get a numerical value from the user and verify
*/
static long getnumval(char *ask, long valuenow, int statline)
{
	char numval[16];
	long retval;

	echo();
	move(statline, 0);
	clrtoeol();
	printw(ask, valuenow);

	numval[0] = 0;
	scanw("%15s", numval);

	move(statline, 0);
	noecho();

	if (numval[0])  /* data entered ? */
	{
		if ( numeric(numval) )
		{
			retval = atol(numval);
		}
		else
		{
			beep();
			clrtoeol();
			printw("Value not numeric (current value kept)!");
			refresh();
			sleep(2);
			retval = valuenow;
		}
	}
	else
	{
		retval = valuenow;
	}

	return retval;
}
/*
** Function convtime() converts a value (number of seconds since
** 1-1-1970) to an ascii-string in the format hh:mm:ss, stored in
** chartim (9 bytes long).
*/
char * convtime(time_t utime, char *chartim)
{
	struct tm 	*tt;

	tt = localtime(&utime);

	sprintf(chartim, "%02d:%02d:%02d", tt->tm_hour, tt->tm_min, tt->tm_sec);

	return chartim;
}
/*
** Function convdate() converts a value (number of seconds since
** 1-1-1970) to an ascii-string in the format yyyy/mm/dd, stored in
** chardat (11 bytes long).
*/
char * convdate(time_t utime, char *chardat)
{
	struct tm 	*tt;

	tt = localtime(&utime);

	sprintf(chardat, "%04d/%02d/%02d",
		tt->tm_year+1900, tt->tm_mon+1, tt->tm_mday);

	return chardat;
}


/*
** function to be called when the program stops
*/
void generic_end(void)
{
	endwin();
}

/*
** signal catcher for cleanup before exit
*/
void cleanstop(exitcode)
{
	generic_end();
	system_clean();
	exit(exitcode);
}

/*
** check if the kernel is patched by ATOP patches
*/
int check_kernelpatch_status()
{
	FILE		*fp;
	/*
	** check if this kernel is patched for network activities accounting
	*/
	if ( (fp = fopen("/proc/1/stat", "r")) )
	{
		char	line[4096];

		/*
		** when the patch is installed, the output
		** of /proc/pid/stat contains two lines
		*/
		(void) fgets(line, sizeof line, fp);

		if ( fgets(line, sizeof line, fp) != NULL)
			return 1;

		fclose(fp);
	}
	return 0;
}

/*
** show help information in interactive mode - borrowed from ATOP
*/
static struct helptext {
	char *helpline;
	char helparg;
} helptext[] = {
	{"General commands:\n", 				' '},
	{"\t^L  - redraw the screen                       \n",	' '},
	{"\t'%c' - pause-button to freeze current sample (toggle)\n",
								MPAUSE},
	{"\n",							' '},
	{"\t'%c' - kill a process (i.e. send a signal)\n",	MKILLPROC},
	{"\n",							' '},
	{"\t'%c' - help-information\n",				MHELP},
	{"\t'%c' - quit this program\n",			MQUIT},
};

static int helplines = sizeof(helptext)/sizeof(struct helptext);

static void showhelp(int helpline)
{
	int	winlines = LINES-helpline, lin;
	WINDOW	*helpwin;

	/*
	** create a new window for the help-info in which scrolling is
	** allowed
	*/
	helpwin = newwin(winlines, COLS, helpline, 0);
	scrollok(helpwin, 1);

	/*
	** show help-lines
	*/
	for (lin=0; lin < helplines; lin++)
	{
		wprintw(helpwin, helptext[lin].helpline, helptext[lin].helparg);

		/*
		** when the window is full, start paging interactively
		*/
		if (lin >= winlines-2)
		{
			wmove    (helpwin, winlines-1, 0);
			wclrtoeol(helpwin);
			wprintw  (helpwin, "Press any key for next line or "
			                   "'q' to leave help .......");

			if (wgetch(helpwin) == 'q')
			{
				delwin(helpwin);
				return;
			}

			wmove  (helpwin, winlines-1, 0);
		}
	}

	wmove    (helpwin, winlines-1, 0);
	wclrtoeol(helpwin);
	wprintw  (helpwin, "End of help - press any key to continue....");
	(void) wgetch(helpwin);
	delwin   (helpwin);
}

#define LEAST_COLUMS 30

void
persists_process_energy(time_t now, float ecpu, float emem, float entw, float edisk, struct process_info* pst)
{
  struct process_energy processEnergy;
  processEnergy.pid = pst->pid;
  processEnergy.time = now;
  processEnergy.ecpu = ecpu;
  processEnergy.emem = emem;
  processEnergy.enet = entw;
  processEnergy.edisk = edisk;
  processEnergy.cmdline = malloc(sizeof(char) * strlen(pst->cmdline));
  strcpy(processEnergy.cmdline, pst->cmdline);
  insert_process_energy(&processEnergy);
}

void
persist_device_energy(struct device_energy* deviceEnergy)
{
  insert_device_energy(&*deviceEnergy);
}

int show_proc_stat(struct process_info *pst_list, int amount, time_t now)
{
	time_t		curtime;
	register int	curline, statline;
	int nsecs 	= 2;
	struct utsname	utsname;
	int		utsnodenamelen;
	char 		*p;
	int		lastchar = 0, killpid, killsig;
	char		format1[16], format2[16], format3[128];
	char		*statmsg = NULL;
	char		buf[13];
	static int	usecolors=1;

    if (isatty(1) ){
    	screen = 1;
    }else{
    	screen = 0;
    	return 0;
    }

	(void) uname(&utsname);
	if ( (p = strchr(utsname.nodename, '.')) ){
		*p = '\0';
	}

	utsnodenamelen = strlen(utsname.nodename);
    setvbuf(stdout, (char *)0, _IOLBF, BUFSIZ);

    //initialize screen-handling via curses
    initscr();
    cbreak();
    noecho();

    if (COLS  < LEAST_COLUMS)
    {
    	printw("Not enough columns (need at least %d columns)\n", LEAST_COLUMS);
    	refresh();
    	sleep(3);
    	cleanstop(1);
    }

    if (has_colors())
    {
    	use_default_colors();
    	start_color();

    	init_pair(COLORLOW,  COLOR_GREEN,  -1);
    	init_pair(COLORMED,  COLOR_CYAN,   -1);
    	init_pair(COLORHIGH, COLOR_RED,    -1);
    }
    else
    {
    	usecolors = 0;
    }

    signal(SIGINT,   cleanstop);
    signal(SIGTERM,  cleanstop);
    werase(stdscr);
	attron(A_REVERSE);

	curtime  = time(0);
    convdate(curtime, format1);
    convtime(curtime, format2);

    int seclen	= val2elapstr(nsecs, buf);
    int lenavail 	= COLS - 35 - seclen - utsnodenamelen;
    int len1	= lenavail / 2;

	printw("pTop - %s%*s%s  %s%*s%s elapsed", utsname.nodename, len1, "",
			format1, format2, lenavail-len1, "", buf);
	attroff(A_REVERSE);

	if (statmsg)
	{
		clrtoeol();
		if (usecolors)
			attron(COLOR_PAIR(COLORLOW));

		printw(statmsg);

		if (usecolors)
			attroff(COLOR_PAIR(COLORLOW));

		statmsg = NULL;
	}
	else
	{
		if (usecolors)
			attron(COLOR_PAIR(COLORLOW));
		attron(A_BLINK);
		printw("%*s", (COLS-38)/2, " ");
		printw("** All process activity since boot **\n");

		if (usecolors)
			attroff(COLOR_PAIR(COLORLOW));
		attroff(A_BLINK);
	}

	int tlength = interval;
	struct device_energy deviceEnergy;
	int lenavail2 	= COLS;
	int len2 = lenavail2/7;
	sprintf(format3, "%%-%ds%%-%ds%%-%ds%%-%ds%%-%ds%%-%ds%%-%ds", len2, len2, len2, len2, len2, len2, lenavail2 - 6 * len2);
	attron(A_REVERSE);
	printw(format3,
			"PID", "ALL(Percent)", "ECPU", "EDISK", "ENET","EMEM", "CMD");
	attroff(A_REVERSE);

	lenavail2 	= COLS;
	len2 = lenavail2/7;
	struct process_info * pst;
	int j;
	float ecpu,edisk,entw,emem, eall, etotalsys, percent;

	amount = amount > SHOW_NUM ? SHOW_NUM:amount;
	sprintf(format3, "%%-%ds%%-%d.3f%%-%d.3f%%-%d.3f%%-%d.3f%%-%d.3f%%-%ds\n", len2, len2, len2, len2, len2, len2, lenavail2 - 6 * len2);
	systemEnergy(tlength, now, &deviceEnergy);

    persist_device_energy(&deviceEnergy);

	etotalsys = deviceEnergy.ecpu + deviceEnergy.edisk + deviceEnergy.emem;
	printw(format3,
				"Total", 100.0f, deviceEnergy.ecpu, deviceEnergy.edisk, deviceEnergy.enet, deviceEnergy.emem, "");

	sprintf(format3, "%%-%dd%%-%d.3f%%-%d.3f%%-%d.3f%%-%d.3f%%-%d.3f%%-%ds", len2, len2, len2, len2, len2, len2, lenavail2 - 6 * len2);

	for(j=0;j<amount;j++)
	{
		pst = &pst_list[j];
		ecpu 	= CPUEnergy(pst->pid, tlength, now);
		edisk	= DiskEnergy(pst->pid, tlength, now);
		entw	= NtwkEnergy(pst->pid, tlength, now);
		emem    = MemoryEnergy(pst->pid, tlength, interval, now);
		eall = ecpu + edisk + emem;

		percent = (etotalsys> 0)?eall / etotalsys * 100 : 0;
		printw(format3,
					pst->pid, percent, ecpu, edisk, entw, emem, pst->cmdline);

        persists_process_energy(now, ecpu, emem, entw, edisk, pst);

		refresh();
	}

	curline = 1;
	statline = curline;
    move(curline, 0);

    fd_set rfds;
    int key;
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100;

	FD_ZERO(&rfds);
	FD_SET(0, &rfds);
	//wait for stdin for 100ms
	key = select(1, &rfds, NULL, NULL, &tv);
	if(key < 0)
	    perror("select");/* select error */
	else if(key == 0){
	   return key; /* overtime */
	}else { /* some changes happen on stdin */
		lastchar = mvgetch(statline, 0);
	}

	switch (lastchar)
	{
		case ERR:
		case 0:
			return lastchar;
		case MQUIT:
			move(LINES-1, 0);
			clrtoeol();
			refresh();
			cleanstop(0);
		case MHELP:
			alarm(0);	/* stop the clock         */
			move(1, 0);
			clrtobot();	/* blank the screen */
			refresh();
			showhelp(2);
			move(statline, 0);
			if (interval && !paused)
				alarm(3); /* force new sample     */
			break;
		case MKILLPROC:
			alarm(0);	/* stop the clock */
			killpid = getnumval("Pid of process: ", 0, statline);

			switch (killpid)
			{
				case 0:
				case -1:
					break;
				case 1:
					statmsg = "Sending signal to pid 1 not allowed!";
					beep();
					break;
				default:
					clrtoeol();
					killsig = getnumval("Signal [%d]: ", 15, statline);

					if ( kill(killpid, killsig) == -1)
					{
						statmsg = "Not possible to send signal to this pid!";
						beep();
					}
			}

			if (!paused)
				alarm(3); /* set short timer */
			break;
			case KEY_RESIZE: //handle screen resize
				statmsg = "Window has been resized....";
				break;
			default:
				beep();
		}

	return 1;
}
