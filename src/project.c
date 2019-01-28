/* Simca-QP predictions for the ChemGPS project.
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

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#endif

#include "chemgps.h"
#include "simcaqp.h"

extern int cgps_detect_cpus(struct cgps_project *proj);

/*
 * Loads the project in path.
 */
int cgps_project_load(struct cgps_project *proj, const char *path, struct cgps_options *opts)
{
#if ! defined(__linux__)
	char *version;               /* Simca-Q dll-version */
#endif
	int i;

	memset(proj, 0, sizeof(struct cgps_project));	
	proj->opts = opts;
	
	/*
	 * Set default options:
	 */
	if(!opts->prog) {
		opts->prog = PACKAGE_NAME;
	}	
	if(!opts->logger) {
		opts->logger = cgps_stderr_logger;
	}
	if(!opts->format) {
		opts->format = CGPS_OUTPUT_FORMAT_DEFAULT;
	}
	if(!opts->indata) {
		logerr("data loader function is not set in library options");
		return -1;
	}
	
	if(!opts->license) {
		opts->license = getenv("SIMCAQLICENSE");
		if(!opts->license) {
			debug("no SIMCAQLICENSE environment variable defined");
		}
	}
	if(opts->license) {
		if(!SQX_SetLicensePath(opts->license)) {
			logerr("failed set license path (%s)", cgps_simcaq_error());
		}
		debug("licens path set to %s", opts->license);
	}
	
	if(opts->logfile) {
		if(!SQX_SetLogFile(opts->logfile)) {
			logerr("failed set logfile for Simca-Q to %s (%s)", opts->logfile, cgps_simcaq_error());
		} else {
			debug("successful set Simca-Q logfile to %s", opts->logfile);
		}
	}
	/*
	 * Log message callback is disabled because it requires global pointers.
	 */
	/*
	 else {
	        if(!SQX_SetLogFunction(cgps_logsimca)) {
			logerr("failed set custom log function: cgps_logsimca() (%s)", cgps_simcaq_error());
		} else {
			debug("successful set custom log function: cgps_logsimca()");
		}
	 }
	 */

	if(opts->threading != CGPS_THREADING_UNSET) {
		if(opts->threading > 0) {
			if(!SQX_UseMultiThreading(1, opts->threading)) {
				logerr("failed turn multithreading on (user defined: %d number of cpus)",
				       opts->threading);
				return -1;
			}
			debug("multithreading turned on (user defined: %d number of cpus)",
			      opts->threading);
		} else if(opts->threading == CGPS_THREADING_OFF) {
			if(!SQX_UseMultiThreading(0, 0)) {
				logerr("failed turn multithreading off");
				return -1;
			}
			debug("multithreading turned off");
		} else if(opts->threading == CGPS_THREADING_AUTO) {
			int cpus = cgps_detect_cpus(proj);
			if(!SQX_UseMultiThreading(1, cpus)) {
				logerr("failed turn multithreading on (auto: %d number of cpus)", cpus);
				return -1;
			}
			debug("multithreading turned on (auto: %d number of cpus)", cpus);
		} else if(opts->threading == CGPS_THREADING_DEFAULT) {
			if(!SQX_UseMultiThreading(1, -1)) {
				logerr("failed turn multithreading on (default number of cpus)");
				return -1;
			}
			debug("multithreading turned on (default number of cpus)");
		}
	}
	
	if(!SQX_AddProject(path, 1, NULL, &proj->handle)) {
		/*
		 * We better give up if loading project fails.
		 */
		logerr("failed load project (%s)", cgps_simcaq_error());
		return -1;
	}
	debug("successful loaded project %s", path);
	
	if(opts->debug) {
#if ! defined(__linux__)
		if(!SQX_GetVersionNumber(&version)) {
			logerr("failed get Simca-Q version (%s)", cgps_simcaq_error());
		} else {
			debug("initiated Simca-Q version %s with project %s", version, path);
		}
#endif  /* ! defined(__linux__) */
		
		if(!SQX_GetProjectName(proj->handle, &proj->name)) {
			logerr("failed get project name (%s)", cgps_simcaq_error());
		} else {
			debug("project name: %s", proj->name);
		}
		
		if(!SQX_GetNumberOfObservationIDs(proj->handle, &i)) {
			logerr("failed get number of observation ids in the project");
		} else {
			debug("the project contains %d observation ids", i);
		}
		
		if(!SQX_GetNumberOfVariableIDs(proj->handle, &i)) {
			logerr("failed get number of variable ids in the project");
		} else {
			debug("The project contains %d variable ids", i);
		}		
	}

	if(!SQX_GetNumberOfModels(proj->handle, &proj->models)) {
		logerr("failed get number of models");
	} else {
		debug("project contains %d number of models", proj->models);
	}
	return 0;
}

/*
 * Close the project and release allocated resources.
 */
void cgps_project_close(struct cgps_project *proj)
{
	if(proj->handle) {
		if(!SQX_RemoveProject(proj->handle)) {
			logerr("failed remove Simca-Q project");
		} else {
			debug("successful closed project");
		}
	}
}
