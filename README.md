ptop
====
For the using of pTop, the version of your Linux kernel must be higher than 2.6.31 and with an atop patch. 
PTop uses the system call "__NR_perf_counter_open"(in the latest version, the name is "__NR_perf_event_open") to stat 
the hardware events, such as L2 cache miss. This system call is supported from 2.6.31 version. To check whether your
kernel supply this system call, please see the file "/usr/include/asm/unistd_32.h" or "/usr/include/asm/unistd_64.h".
In addition, pTop uses the patch supplied by atop(www.atoptool.nl) to get the network usage of each process, so you
must patch your kernel with atop patches(http://www.atoptool.nl/downloadpatch.php), or the network energy consumption
for each process is not correct.

PTop greatly relies on "/proc", it uses the stat results of the system to compute the energy consumption for four main
devices, that is CPU, disk, memory and wireless network card. PTop uses a group of default parameters to compute the e-
nergy consumption, which is based on our experiment platform. To get a more accurate result on your test platform, you'd
better config these parameters based on your own platform. The following shows how to get these parameters:
1. CPU specifications
	For the max and min CPU power you can get it from the specification of your CPU.
2. memory specifications
	For memory power you can get it from your memory specification. For the L2 cache miss latency and L2 cache line si-
	ze you can use calibrator(http://homepages.cwi.nl/~manegold/Calibrator/calibrator.shtml) to do a test on your plat-
	form. 

    Get Frequency: sudo /usr/sbin/dmidecode | grep Speed
    L2 Cache Miss Latency: ./calibrator 2600MHz 10M result.txt

	caches:
	level  size    linesize   miss-latency        replace-time
  	1     32 KB   64 bytes   42.56 ns =  91 cy    3.61 ns =   8 cy
  	2      2 MB  128 bytes   59.80 ns = 128 cy  109.09 ns = 233 cy
3. disk specifications
	Your can get the disk's read power, write power, read bandwidth and write bandwidth from your disk specification.
	Then use these value to compute the EDISKRPERKB and EDISKRPERKB. 
4. wireless network specification
    The network specification is similar with the disk, you need to get the power from the specification file. And to
    get the real send and receive bandwidth you need do a test.
Finally, you can modify these specification based on your own platform by change the value from "src/energy.c" file.    

In the first edition of pTop, we use mysql database to record all the stat data. First, you need create an database sc-
hema named "ptop". Then use the scripts in "conf/db_schema.sql" to create the tables. The default database username and
password are both "root", you can changed it as your own by change "src/database.c".
After everything is done, you can use "make" to build pTop, then you can run it use the following commond, be sure to use
super use to run it, because the performance counter that stat each CPU's cache miss needs CAP_SYS_ADMIN privilege.
	sudo ./pTop 1   # 1 is the stat interval
	
Although the result of pTop is not very accurate now, it could tells which process consumes more energy. Also, the active
processes only consumes less than half amount of the total energy. 
Thanks for your interest to our work, if you have any problem during the use of pTop you can mail to ptop.mist@gmail.com! 	 