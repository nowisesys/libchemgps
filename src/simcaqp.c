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
 *  Contact: Anders Lövgren <anders.lovgren@bmc.uu.se>
 * ----------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#ifdef HAVE_STRING_H
# include <string.h>
#endif
#include <stdarg.h>

#include "chemgps.h"
#include "simcaqp.h"

/*
 * Get latest error string from Simca-Q.
 */
const char * cgps_simcaq_error(void)
{
	static char *sqperr;
	if(!SQX_GetLatestError(&sqperr)) {
		sqperr = "unknown Simca-Q error (get error string failed)";
	}
	return sqperr;
}

/*
 * Custom callback function for logging messages from Simca-Q. Enable this
 * function whould force use to use global pointer to the project or
 * options struct, a no-no for libraries.
 */
/*
void cgps_logsimca(const char *msg, int status)
{
	switch(status) {
	case 0:
		if(!opts->batch) {
			loginfo("simca lib: %s", msg);
		}
		break;
	case 1:
		logerr("simca lib: %s", msg);
		break;
	default:
		logerr("simca lib: unknown status %d (msg=%s)", status, msg);
		break;
	}
}
*/

/*
 * The default stderr logger.
 */
void cgps_stderr_logger(void *pref, int status, int code, int level, const char *file, unsigned int line, const char *fmt, ...)
{
	struct cgps_options *opts = (struct cgps_options *)pref;
	
	switch(level) {
	case LOG_ERR:
	case LOG_CRIT:
		fprintf(stderr, "%s: error: ", opts->prog);
		break;
	case LOG_DEBUG:
		fprintf(stderr, "debug: ");
		break;
	case LOG_WARNING:
		fprintf(stderr, "%s: warning: ", opts->prog);
		break;
	}

        va_list ap;
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        va_end(ap);

        if(code) {
		fprintf(stderr, " (%s)", strerror(code));
	}
        if(level == LOG_DEBUG) {
		if(opts->debug > 1) {
			fprintf(stderr, "\t(%s:%d): ", file, line);
	        }
	}
	fprintf(stderr, "\n");
}
