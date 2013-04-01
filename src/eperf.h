/*
 * eperf.h
 *
 *  Created on: Jan 28, 2010
 *      Author: hchen
 */

#ifndef EPERF_H_
#define EPERF_H_
#include <time.h>
#include <asm/unistd.h>
#include <unistd.h>

#include "perf_event.h"
#include "common.h"
#include "avltree.h"

#define MAX_COUNTERS		256
#define MAX_NR_CPUS			32
#define PROC		"/proc"

u64 read_counter(PCFile fd);

static inline int
sys_perf_event_open(struct perf_event_attr *attr,
		      pid_t pid, int cpu, int group_fd,
		      unsigned long flags)
{
	attr->size = sizeof(*attr);
	//This system call is defined in asm/unistd.h, in the latest linux kernel
	//it's name has been changed to __NR_perf_event_open .
	return syscall(__NR_perf_counter_open, attr, pid, cpu, group_fd, flags);
}

int create_perf_stat_counter(int pid, int system_wide);
void init_perf(AvlTree tree);
u64 read_cpu_counter();
u64 read_counter(PCFile fd);
void close_all_counters(AvlTree tree);
#endif /* EPERF_H_ */
