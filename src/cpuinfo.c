/* SIMCA-QP predictions for the ChemGPS project.
 * 
 * Copyright (C) 2007-2008 Anders Lövgren and the Computing Department, 
 * Uppsala Biomedical Centre, Uppsala University.
 * 
 * ----------------------------------------------------------------------
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * ----------------------------------------------------------------------
 *  Contact: Anders Lövgren <andlov@nowise.se>
 * ----------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_STRING_H
# include <string.h>
#endif

#include "chemgps.h"
#include "simcaqp.h"

#define PROC_CPUINFO "/proc/cpuinfo"
#define PROC_MAXLINE 256
#define PROC_CPUHEAD "processor\t:"

/*
 * Detect number of CPU's (cores) in system. Returns -1 on failure.
 */
int cgps_detect_cpus(struct cgps_project *proj)
{
	FILE *proc;
	char buff[PROC_MAXLINE];
	int cpus = 0;
	
	proc = fopen(PROC_CPUINFO, "r");
	if(!proc) {
		logerr("failed open %s", PROC_CPUINFO);
		return -1;
	} else {
		while(fgets(buff, PROC_MAXLINE - 1, proc)) {
			if(strstr(buff, PROC_CPUHEAD)) {
				cpus++;
			}
		}
		fclose(proc);
	}
	if(!cpus) {
		logerr("failed parse %s", PROC_CPUINFO);
		return -1;
	}
	return cpus;
}
