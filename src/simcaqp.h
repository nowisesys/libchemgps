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

#ifndef __SIMCAQP_H__
#define __SIMCAQP_H__

#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif
#include <errno.h>

/*
 * This file defines the library private API.
 */

struct predicted_result_entry
{
	int value;
	const char *name;
	const char *desc;
};

/*
 * Lookup table for PREDICTED_RESULTS values.
 */
extern struct predicted_result_entry predicted_result_list[];

/*
 * Log functions that logs to stdout/stderr (in debug mode) or syslog.
 * 
 * These macros requires either GNU's C compiler GCC or an 
 * ISO C99 standard compliant compiler.
 */
#if defined(__GNUC__)
# define logerr(fmt, args...) do { \
	proj->opts->logger(proj->opts, errno ? errno : 0 , LOG_ERR , __FILE__ , __LINE__ , (fmt) , ## args); \
} while(0)

# define logwarn(fmt, args...) do { \
	proj->opts->logger(proj->opts, 0 , LOG_WARNING , __FILE__ , __LINE__ , (fmt) , ## args); \
} while(0)

# define loginfo(fmt, args...) do { \
	proj->opts->logger(proj->opts, 0 , LOG_INFO , __FILE__ , __LINE__ , (fmt) , ## args); \
} while(0)
# if ! defined(NDEBUG)

#  define debug(fmt, args...) do { \
	if(proj->opts->debug) { \
		proj->opts->logger(proj->opts, 0 , LOG_DEBUG , __FILE__ , __LINE__ , (fmt) , ## args); \
	} \
} while(0)
# else /* ! defined(NDEBUG) */
#  define debug(fmt, args...)
# endif

#else   /* ! defined(__GNUC__) */
# if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L  
#  define logerr(...) do { \
	proj->opts->logger(proj->opts, errno ? errno : 0 , LOG_ERR , __FILE__ , __LINE__ , __VA_ARGS__); \
} while(0)

#  define logwarn(...) do { \
	proj->opts->logger(proj->opts, 0 , LOG_WARNING , __FILE__ , __LINE__ , __VA_ARGS__); \
} while(0)

#  define loginfo(...) do { \
	proj->opts->logger(proj->opts, 0 , LOG_INFO , __FILE__ , __LINE__ , __VA_ARGS__); \
} while(0)

#  if ! defined(NDEBUG)
#   define debug(...) do { \
	if(proj->opts->debug) { \
		proj->opts->logger(proj->opts, 0 , LOG_DEBUG , __FILE__ , __LINE__ , __VA_ARGS__); \
	} \
} while(0)
#  else /* ! defined(NDEBUG) */
#   define debug(...)
#  endif

# else  /* ! __STDC_VERSION__ >= 199901L */
/*
 * No support for variadic macros at all. Should we provide non-macro functions?
 */
#  define CHEMGPS_LOGFUNC_NOT_VARIADIC 1
#  error "Use the GNU C compiler (gcc) or enable C99 standard when compiling this code."
# endif
#endif  /* defined(__GNUC__) */

/*
 * Default stderr logger:
 */
void cgps_stderr_logger(void *opts, int errcode, int level, const char *file, unsigned int line, const char *fmt, ...);

/*
 * Get Simca-QP error message:
 */
const char * cgps_simcaq_error(void);

/*
 * Loads client data:
 */
int cgps_predict_get_raw_data(struct cgps_project *proj, int model, struct cgps_predict *pred, SQX_StringVector *names);
int cgps_predict_get_lag_parents(struct cgps_project *proj, int model, struct cgps_predict *pred, SQX_StringVector *parents);
int cgps_predict_get_qual_data(struct cgps_project *proj, int model, struct cgps_predict *pred, SQX_StringVector *names);
int cgps_predict_get_qual_data_lagged(struct cgps_project *proj, int model, struct cgps_predict *pred, SQX_StringVector *names);

#endif /* __SIMCAQP_H__ */
