CREATE TABLE  `ptop`.`device_energy` (
  `id` int(11) NOT NULL auto_increment,
  `time` bigint(20) default NULL,
  `ecpu` float default NULL,
  `emem` float default NULL,
  `enet` float default NULL,
  `edisk` float default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=275 DEFAULT CHARSET=latin1;

CREATE TABLE  `ptop`.`process_energy` (
  `id` int(11) NOT NULL auto_increment,
  `pid` int(11) NOT NULL,
  `time` bigint(20) default NULL,
  `ecpu` float default NULL,
  `emem` float default NULL,
  `enet` float default NULL,
  `edisk` float default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE  `ptop`.`process_info` (
  `id` int(11) NOT NULL auto_increment,
  `pid` int(11) default NULL,
  `time` bigint(20) default NULL,
  `read_bytes` bigint(20) default NULL,
  `write_bytes` bigint(20) default NULL,
  `cancelled_write_bytes` bigint(20) default NULL,
  `total_vsz` bigint(20) default NULL,
  `total_rsz` bigint(20) default NULL,
  `mem` bigint(20) default NULL,
  `utime` float default NULL,
  `stime` float default NULL,
  `gtime` float default NULL,
  `cpu` float default NULL,
  `cswch` float default NULL,
  `nvswch` float default NULL,
  `minflt` float default NULL,
  `majflt` float default NULL,
  `netsnd` float default NULL,
  `netrcv` float default NULL,
  `ratio` float default NULL,
  `cpu_accesses` bigint(20) default NULL,
  `cmdline` char(40) default NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=26191 DEFAULT CHARSET=latin1;

CREATE TABLE  `ptop`.`sys_info` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `time` bigint(20) DEFAULT NULL,
  `netsend` bigint(20) DEFAULT NULL,
  `netrcv` bigint(20) DEFAULT NULL,
  `diskread` bigint(20) DEFAULT NULL,
  `diskwrite` bigint(20) DEFAULT NULL,
  `cpupower` float DEFAULT NULL,
  `memaccess` bigint(20) DEFAULT NULL,
  `itv` bigint(20) DEFAULT NULL,
  `runtime` bigint(20) DEFAULT NULL,
  `apnum` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=4339 DEFAULT CHARSET=latin1;