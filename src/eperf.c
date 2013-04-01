#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <asm/unistd.h>
#include <dirent.h>
#include <string.h>

#include "eperf.h"
#include "avltree.h"

unsigned int verbose = 0;
//event to be countered
static struct perf_event_attr attrs[] = {
  { .type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_CACHE_MISSES	}
};

static u32	nr_cpus				=  0; // amount of cpus
static int			inherit				=  1;
static int			scale				=  1;
//used to save performance counter
static int			fd[MAX_NR_CPUS];

/*
 * Read out the results of a single counter:
 */
u64 read_counter(PCFile fd)
{
	u64 single_count[3];
	size_t res, nv;

	if (fd <= 0)
		return 0;

	nv = scale ? 3 : 1;
	res = read(fd, single_count, nv * sizeof(u64));

	if(res == nv * sizeof(u64)){
		return single_count[0];
	}else{
		return 0;
	}
}

u64 read_cpu_counter(){
	int cpu;
	u64 result = 0;
	for (cpu = 0; cpu < nr_cpus; cpu++){
		result += read_counter(fd[cpu]);
	}

	return result;
}

void close_all_counters(AvlTree tree){
	int counter;

	for (counter = 0; counter < nr_cpus; counter++){
		if (fd[counter] <= 0)
			continue;

		close(fd[counter]);
		fd[counter] = -1;
	}

	empty(tree);
}

int create_perf_stat_counter(int pid, int system_wide)
{
	struct perf_event_attr attr; //cache miss
	memcpy(&attr, attrs, sizeof(struct perf_event_attr));

	if (scale)
		attr.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED |
				    PERF_FORMAT_TOTAL_TIME_RUNNING;

	if (system_wide) {
		unsigned int cpu;

		for (cpu = 0; cpu < nr_cpus; cpu++) {
			fd[cpu] = sys_perf_event_open(&attr, -1, cpu, -1, 0);
		}

		return fd[cpu - 1];
	} else {
		attr.inherit	     = inherit;
		attr.disabled	     = 0;
		attr.enable_on_exec = 1;

		return sys_perf_event_open(&attr, pid, -1, -1, 0);
	}
}

void init_perf(AvlTree tree)
{
	nr_cpus = sysconf(_SC_NPROCESSORS_ONLN);//the the number of CPU
	//create counters for each CPU
	create_perf_stat_counter(1, TRUE);

}
