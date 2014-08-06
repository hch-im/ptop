/***************************************************************************
 * Name:energy.c - Energy interface.
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

/*
 ***************************************************************************
 *	Energy primitive API prototypes	*
 **************************************************************************
 */
#include <mysql/mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "database.h"
#include "energy.h"

/* Specifications */
/*
 **********************************************************************
 CPU: in my Vaio laptop
   - 800MHZ: 18W
   - 2.Ghz: 30W
 **********************************************************************
 */
float maxcpupower = 24.5;	// in Watts
float mincpupower = 6.0;	// in Watts

/*
 **********************************************************************
 Memory Specification
 **********************************************************************
 */
#define MEMORY_POWER 2.016 //in Watts, from my memory module specification
#define L2CACHE_MISS_LATENCY 59.80 //ns, get use calibrator
#define L2CACHE_LINE_SIZE 128 //byte get use calibrator

/*
 **********************************************************************
 Energy, in Joul, when read a bytes
   - Read: 0.00002 * 2.78
   - Write: 0.00002 * 2.19
 **********************************************************************
 */
#define EDISKRPERKB (0.00002 * 2.78)
#define EDISKWPERKB  (0.00002 * 2.19)

/*
 **********************************************************************
 My Laptop Intel 2200 BG wireless network card:
   - Transmit: 1.8 W
   - Receive: 1.4 W
   - Real upload bandwidth: 12.330M/s
   - Real download bandwidth 5.665M/s
 **********************************************************************
 */
#define ENETSNDPERKB (1.8 / (1024 * 12.330))
#define ENETRCVPERKB (1.4 / (1024 * 5.665))

/*
 *******************************************************************************
 * Get current battery capacity, in mVh
 * OUT:
 * -1: error
 * >=0: current battery energy
 *******************************************************************************
 */
unsigned long getEBattery() {
	FILE *fp;
	if ((fp = fopen("/sys/class/power_supply/BAT1/energy_now", "r"))==NULL) {
		return -1;
	}
	unsigned long e;
	fscanf(fp, "%ld",&e);

	fclose(fp);
	return e;
}

/*
 ***************************************************************************
 * Compute energy consumption of process per CPU in Joule
 *
 * IN:
 * @PID: process ID
 * @length: time interval
 *
 * RETURNS:
 * 	Amount of energy consumption by the process ID during length seconds
 * 	-1.0: database errors;
 * 	-2.0: PID doesn't exist
 ***************************************************************************
*/
float CPUEnergy(int PID, int length, time_t now) {
	char query [500];

	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	if (!create_conn(&conn)) {
			return 0;	// errors
	}
	else {
		// code to compute energy
		sprintf(query, "SELECT p.cpu, s.cpupower, s.itv, s.runtime, s.apnum FROM process_info p, sys_info s WHERE p.time = s.time and p.pid=%d and p.time>(%ld) and p.time <=%ld", PID, now-length,now);
		/* send query to database */
		if (mysql_real_query(conn, query, (unsigned int) strlen(query))) {
			mysql_close(conn);
			return 0;
		}
		res = mysql_use_result(conn);

		float power = 0.0;
		float cpu = 0.0;		// average cpu usage per second
		u32 itv, runtime;
		int apnum;

		float totalenergy = 0.0; // energy consumed
		float run_percent = 0;
		while ((row = mysql_fetch_row(res)) != NULL) {
			cpu = atof(row[0]);
			power = atof(row[1]);
			itv = atoi(row[2]);
			runtime = atoi(row[3]);
			apnum = atoi(row[4]);

			run_percent = runtime * 1.0f /itv;
			totalenergy += (cpu * power * run_percent) / 100;
		}
		/* close connection */
		mysql_free_result(res);
		mysql_close(conn);

		return totalenergy;
	}
}

/*
 ***************************************************************************
 * Compute energy consumption of process per display in Joule
 *
 * IN:
 * @PID: process ID
 * @length: time interval
 *
 * RETURNS:
 * Amount of energy consumption by the process ID on display
 * during length seconds
 ***************************************************************************
*/
float DsplyEnergy(int PID, int length, time_t now) {
	// dummy value, for interface
	return 0;
}


/*
 ***************************************************************************
 * Compute energy consumption of process per network, in Joule
 *
 * IN:
 * @PID: process ID
 * @length: time interval
 *
 * RETURNS:
 * Amount of energy consumption by the process ID on network
 * during length seconds
 ***************************************************************************
*/
float NtwkEnergy(int pid, int length, time_t now) {
	char query [500];
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	float netsnd = 0.0;		// average network sending activity
	float netrcv = 0.0;		// average network receiving activity
	float total = 0;
	/* connect to database */
	if (!create_conn(&conn)) {
		return 0;	// errors
	}
	else {
		sprintf(query, "SELECT netsnd, netrcv FROM process_info WHERE pid=%d and time>(%ld) and time <=%ld",pid, now - length, now);
		/* send query to database */
		if (mysql_real_query(conn, query, (unsigned int) strlen(query))) {
			mysql_close(conn);
			return 0;			/* error */
		}

		res = mysql_use_result(conn);
		while((row = mysql_fetch_row(res)) != NULL) {
			netsnd = atof(row[0]);
			netrcv = atof(row[1]);
			total += ((netsnd * ENETSNDPERKB /1024) + (netrcv * ENETRCVPERKB /1024));
		}
		/* close connection */
		mysql_free_result(res);
		mysql_close(conn);
	}

	return total;
}

/*
 ***************************************************************************
 * Compute energy consumption of process on disk, in Joule
 *
 * IN:
 * @PID: process ID
 * @length: time interval
 *
 * RETURNS:
 * Amount of energy consumption by the process ID on disk
 * during length seconds
 ***************************************************************************
*/
float DiskEnergy(int pid, int length, time_t now) {
	u64 rKB = 0, wKB = 0;
	float totale = 0;
	char query [500];
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	if(!create_conn(&conn)) {
		return 0;	// errors
	}
	else {
		sprintf(query, "SELECT read_bytes, write_bytes - cancelled_write_bytes FROM process_info WHERE pid=%d and time>(%ld) and time<= %ld", pid, now-length, now);
		/* send query to database */
		if (mysql_real_query(conn, query, (unsigned int) strlen(query))) {
			mysql_close(conn);
			return 0; /* error */
		}else{
			res = mysql_use_result(conn);
			while ((row = mysql_fetch_row(res)) != NULL) {
				rKB =atol(row[0]);
				wKB =atol(row[1]);
				totale += (rKB /1024 * EDISKRPERKB + wKB / 1024 * EDISKWPERKB);
			}
		}

		mysql_free_result(res);
		mysql_close(conn);
	}

	return totale;
}

/*
 ***************************************************************************
 * Compute energy consumption of process on memory, in Joule
 *
 * IN:
 * @PID: process ID
 * @length: time interval
 *
 * RETURNS:
 * Amount of energy consumption by the process ID on memory
 * during length seconds
 ***************************************************************************
*/
float MemoryEnergy(int PID, int length, int interval, time_t now){
	char query [500];

	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	if (!create_conn(&conn)) {
			return 0;	// errors
	}
	else {
		sprintf(query, "SELECT ratio, read_bytes,write_bytes - cancelled_write_bytes, cpu_accesses FROM process_info WHERE pid=%d and time>(%ld) and time <=%ld", PID, now-length, now);
		/* send query to database */
		if (mysql_real_query(conn, query, (unsigned int) strlen(query))) {
			mysql_close(conn);
			return 0;
		}
		res = mysql_use_result(conn);

		float ratio;
		float dma_rbytes; //in byte
		float dma_wbytes;
		long cpu_accesses;
		float totalenergy = 0.0; // energy consumed
		while ((row = mysql_fetch_row(res)) != NULL) {
			ratio = atof(row[0]);
			dma_rbytes = atof(row[1]);
			dma_wbytes = atof(row[2]);
			cpu_accesses = atol(row[3]);

			totalenergy += (((dma_rbytes  / L2CACHE_LINE_SIZE + dma_wbytes / L2CACHE_LINE_SIZE) * L2CACHE_MISS_LATENCY * MEMORY_POWER / 1000000000) //j
					       + (cpu_accesses * L2CACHE_MISS_LATENCY * MEMORY_POWER / 1000000000) //(times * ns * watt /10^6)j
					       /*+ (ratio * MEMORY_POWER * interval * 1000000000)*/ );//mj
		}
		/* close connection */
		mysql_free_result(res);
		mysql_close(conn);

		return totalenergy;
	}
}

void systemEnergy(int length, time_t now, struct device_energy * pe){
	char query [500];
	memset(pe, 0, sizeof(struct process_energy));
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	if (!create_conn(&conn)) {
			return;	// errors
	}
	else {
		// code to compute energy
		sprintf(query, "SELECT s.netsend, s.netrcv, s.diskread, s.diskwrite,"
				" s.cpupower, s.memaccess, s.itv FROM sys_info s WHERE s.time>(%ld) and s.time <=%ld ", now-length, now);
		/* send query to database */
		if (mysql_real_query(conn, query, (unsigned int) strlen(query))) {
			mysql_close(conn);
			return;
		}
		res = mysql_use_result(conn);

		u64 totalnetsnd = 0;
		u64 totalnetrcv = 0;
		u64 totaldiskread = 0;
		u64 totaldiskwrite = 0;
		float cpupower = 0;
		u64 memaccess = 0;
		u32 itv = 0;
		float etemp = 0;

		while ((row = mysql_fetch_row(res)) != NULL) {
			totalnetsnd = atol(row[0]);
			totalnetrcv = atol(row[1]);
			totaldiskread = atol(row[2]);
			totaldiskwrite = atol(row[3]);
			cpupower = atof(row[4]);
			memaccess = atol(row[5]);
			itv= atol(row[6]);

			pe->ecpu += cpupower;

			etemp = (((totaldiskread  / L2CACHE_LINE_SIZE + totaldiskwrite / L2CACHE_LINE_SIZE) * L2CACHE_MISS_LATENCY * MEMORY_POWER / 1000000000) //j
					       + (memaccess * L2CACHE_MISS_LATENCY * MEMORY_POWER / (1000000000 ))); //(times * ns * watt /10^9)
			pe->emem += etemp;

			etemp = totaldiskread /1024 * EDISKRPERKB + totaldiskwrite /1024 * EDISKWPERKB;
			pe->edisk += etemp;

			etemp = (totalnetsnd * ENETSNDPERKB / 1024)  + (totalnetrcv * ENETRCVPERKB / 1024);
			pe->enet += etemp;
		}
		/* close connection */
		mysql_free_result(res);
		mysql_close(conn);
	}
}
