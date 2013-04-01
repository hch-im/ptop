/***************************************************************************
 * Name:energy.h - Energy interface definitions.
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
#ifndef ENERGY_H_
#define ENERGY_H_

float maxcpupower;	// in Watts
float mincpupower;	// in Watts
float avgcpupower;	// average cpu power, used in case cpu freq accounting is not supported

/***************************************************************************
 *	Energy primitive API prototype	*
 ***************************************************************************/
 extern float CPUEnergy(int PID, int length, time_t now);
 extern float DsplyEnergy(int PID, int length, time_t now);
 extern float NtwkEnergy(int PID, int length, time_t now);
 extern float DiskEnergy(int PID, int length, time_t now);
 extern float MemoryEnergy(int PID, int length, int interval, time_t now);
 extern unsigned long getEBattery();
 extern void systemEnergy(int length, time_t now, struct device_energy * pe);
#endif  /* _ENERGY_H */
