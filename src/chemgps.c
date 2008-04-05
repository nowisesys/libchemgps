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

#include "chemgps.h"
#include "simcaqp.h"

/*
 * Sets an option value.
 */
int cgps_set_option(struct cgps_project *proj, int option, const void *value)
{
	debug("setting library option %d", option);
	
	switch(option) {
	case CGPS_OPTION_FORMAT:
		proj->opts->format = *(int *)value;
		break;
	case CGPS_OPTION_SYSLOG:
		proj->opts->syslog = *(int *)value;
		break;
	case CGPS_OPTION_BATCH:
		proj->opts->batch = *(int *)value;
		break;
	case CGPS_OPTION_DEBUG:
		proj->opts->debug = *(int *)value;
		break;
	case CGPS_OPTION_VERBOSE:
		proj->opts->verbose = *(int *)value;
		break;
	case CGPS_OPTION_RESULT:
		proj->opts->result = *(int *)value;
		break;
	case CGPS_OPTION_PREFIX:
		proj->opts->prog = (const char *)value;
		break;
	case CGPS_OPTION_LICENSE:
		proj->opts->license = (const char *)value;
		break;
	default:
		logerr("unknown option %d for cgps_set_option", option);
		return -1;
	}
	return 0;
}

/*
 * Gets an option value.
 */
int cgps_get_option(struct cgps_project *proj, int option, void *value)
{
	debug("getting library option %d", option);
	
	switch(option) {
	case CGPS_OPTION_FORMAT:
		*(int *)value = proj->opts->format;
		break;
	case CGPS_OPTION_SYSLOG:
		*(int *)value = proj->opts->syslog;
		break;
	case CGPS_OPTION_BATCH:
		*(int *)value = proj->opts->batch;
		break;
	case CGPS_OPTION_DEBUG:
		*(int *)value = proj->opts->debug;
		break;
	case CGPS_OPTION_VERBOSE:
		*(int *)value = proj->opts->verbose;
		break;
	case CGPS_OPTION_RESULT:
		*(int *)value = proj->opts->result;
		break;
	case CGPS_OPTION_PREFIX:
		(const char *)value = proj->opts->prog;
		break;
	default:
		logerr("unknown option %d for cgps_set_option", option);
		return -1;
	}
	return 0;
}
