/***************************************************************************
 * Name:database.c - Database access functions.
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "database.h"
#include "common.h"
/*
 *********************************************
 * insert device energy record into database
 * IN:
 * @conn: MYSQL connection
 * @de: device energy record to be inserted.
 * RETURN:
 * 0: if success, 1 if failed
 ********************************************
 */
int insert_device_energy(struct device_energy * de){
	MYSQL *conn;
	int result;
	result = create_conn(&conn);
	if(result == 0) return 1;

	char query[1024];
	sprintf(query, "INSERT INTO device_energy(time, ecpu, emem, enet, edisk) VALUES(%ld, %9.2f, %9.2f, %9.2f, %9.2f)", de->time, de->ecpu, de->emem, de->enet, de->edisk);

    result = mysql_real_query(conn, query, (unsigned int) strlen(query));
    mysql_close(conn);
    return result;
}

/*
 *********************************************
 * insert process energy record into database
 * IN:
 * @conn: MYSQL connection
 * @de: process energy record to be inserted.
 * RETURN:
 * 0: if success, 1 if failed
 ********************************************
 */
int insert_process_energy(struct process_energy * pe){
	MYSQL *conn;
	int result;
	result = create_conn(&conn);
	if(result == 0) return 1;
	char query[1024];
	sprintf(query, "INSERT INTO process_energy(pid,time, ecpu, emem, enet, edisk) VALUES(%d,%ld, %9.2f, %9.2f, %9.2f, %9.2f)", pe->pid, pe->time, pe->ecpu, pe->emem, pe->enet, pe->edisk);

    result = mysql_real_query(conn, query, (unsigned int) strlen(query));
    mysql_close(conn);
    return result;
}

/*
 ********************************************
 * insert process record into database
 * IN:
 * @conn: MYSQL connection
 * @pst: process level record to be inserted
 * OUT:
 * 0: if success, 1 if failed
 ********************************************
 */
int insert_ps(struct process_info * pst) {
	MYSQL *conn;
	int result;
	result = create_conn(&conn);
	if(result == 0) return 1;

	char query[1024];
	if(pst->cmdline == NULL)
		pst->cmdline="";
	sprintf(query, "INSERT INTO process_info(pid, time, read_bytes, write_bytes, cancelled_write_bytes,"
			"total_vsz, total_rsz, mem, utime, stime, gtime, cpu, cswch, nvswch, minflt, majflt, netsnd, "
			"netrcv, ratio, cpu_accesses,cmdline) VALUES(%d, %u, %9.2f, %9.2f, %9.2f, %u, %u, %6.2f, %7.2f, %7.2f, %7.2f, "
			"%7.2f, %9.2f, %9.2f, %9.2f, %9.2f, %9.2f, %9.2f,%9.2f,%llu, \'%s\')", pst->pid, pst->time, pst->read_bytes,
			pst->write_bytes, pst->cancelled_write_bytes, pst->vsz, pst->rss, pst->mem, pst->utime, pst->stime,
			pst->gtime, pst->cpu, pst->cswch, pst->nvcswch, pst->minflt, pst->majflt, pst->netsnd, pst->netrcv,
			pst->ratio,pst->cpu_accesses, pst->cmdline);
	result = mysql_real_query(conn, query, (unsigned int) strlen(query));
    mysql_close(conn);
    return result;
}

/*
 ********************************************
 * insert system record into database
 * IN:
 * @conn: MYSQL connection
 * @pst: system level record to be inserted
 * OUT:
 * 0: if success, 1 if failed
 ********************************************
 */
int insert_sys_info(struct sys_info * st){
	MYSQL *conn;
	int result;
	result = create_conn(&conn);
	if(result == 0) return 1;
	char query[1024];

	sprintf(query, "INSERT INTO sys_info(time, netsend, netrcv, diskread, diskwrite,cpupower,memaccess, itv,runtime, apnum)"
			"VALUES(%u, %llu, %llu, %llu, %llu, %9.2f, %llu, %u, %u, %d)", st->time, st->totalnetsnd, st->totalnetrcv,
			st->totaldiskread, st->totaldiskwrite, st->cpupower, st->memaccess, st->itv, st->runtime, st->active_process_num);

	result = mysql_real_query(conn, query, (unsigned int) strlen(query));
    mysql_close(conn);
    return result;
}

/*
 *******************************************************
 * Delete device energy records before time interval.
 * IN:
 * @conn: mysql connection
 * @interval: time to keep process profile
 * RETURN:
 * 0, success, 1 error
 *******************************************************
 */
int freeup_device_energy(u32 interval){
	MYSQL *conn;
	int result;
	result = create_conn(&conn);
	if(result == 0) return 1;
	long now = (long) time(NULL);

	char query[1024];
	sprintf(query, "DELETE FROM device_energy where time < %ld", now - interval);
	result = mysql_real_query(conn, query, (unsigned int) strlen(query));
    mysql_close(conn);
    return result;
}

/*
 *******************************************************
 * Delete process energy records before time interval.
 * IN:
 * @conn: mysql connection
 * @interval: time to keep process profile
 * RETURN:
 * 0, success, 1 error
 *******************************************************
 */
int freeup_process_energy(u32 interval){
	MYSQL *conn;
	int result;
	result = create_conn(&conn);
	if(result == 0) return 1;

	long now = (long) time(NULL);

	char query[1024];
	sprintf(query, "DELETE FROM process_energy where time < %ld", now - interval);
	result = mysql_real_query(conn, query, (unsigned int) strlen(query));
    mysql_close(conn);
    return result;
}

/*
 *******************************************************
 * Delete process info records before time interval.
 * IN:
 * @conn: mysql connection
 * @interval: time to keep process profile
 * RETURN:
 * 0, success, 1 error
 *******************************************************
 */
int freeup_ps(u32 interval){
	MYSQL *conn;
	int result;
	result = create_conn(&conn);
	if(result == 0) return 1;

	long now = (long) time(NULL);

	char query[1024];
	sprintf(query, "DELETE FROM process_info where time < %ld", now - interval);
	result = mysql_real_query(conn, query, (unsigned int) strlen(query));
    mysql_close(conn);
    return result;
}

/*
 *******************************************************
 * Delete system info records before time interval.
 * IN:
 * @conn: mysql connection
 * @interval: time to keep process profile
 * RETURN:
 * 0, success, 1 error
 *******************************************************
 */
int freeup_sys_info(u32 interval){
	MYSQL *conn;
	int result;
	result = create_conn(&conn);
	if(result == 0) return 1;
	long now = (long) time(NULL);

	char query[1024];
	sprintf(query, "DELETE FROM sys_info where time < %ld", now - interval);
	result = mysql_real_query(conn, query, (unsigned int) strlen(query));
    mysql_close(conn);
    return result;
}

/*
 *******************************************************
 * Delete records before time interval.
 * IN:
 * @conn: mysql connection
 * @interval: time to keep process profile
 * RETURN:
 * 0, success, 1 error
 *******************************************************
 */
int freeup_all(u32 interval){
	int result = 1;
//	result &= freeup_device_energy(interval);
//	result &= freeup_process_energy(interval);
	result &= freeup_ps(interval);
	result &= freeup_sys_info(interval);

	return result;
}

/*
 *******************************************************
 * Create db connection.
 * IN:
 * @conn: mysql connection
 * RETURNS:
 * 1, success, 0 error
 *******************************************************
 */
int create_conn(MYSQL **conn){
	char *server = "localhost";
	char *user = "root";
	char *password = "root";
	char *database = "ptop";

	*conn = mysql_init(NULL);
	/* connect to database */
	if (!mysql_real_connect(*conn, server, user, password, database, 0, NULL, 0)) {
			fprintf(stderr, "%s\n", mysql_error(*conn));
			return 0;	// errors
	}

	return 1;
}

/*
 *******************************************************
 * Initialize all the data.
 * OUT: 0: init failure, 1: init success.
 *******************************************************
 */
int init_stat_data(){
	MYSQL *conn;
	int result;
	result = create_conn(&conn);
	if(result == 0) return 1;

	/* delete all */
	freeup_all(0);
	mysql_close(conn);
	return 1;
}

/*
 *******************************************************
 * Do the clean work about data access before the
 * system exits.
 *******************************************************
 */
void clean_stat_data(){
	freeup_all(0);
}
