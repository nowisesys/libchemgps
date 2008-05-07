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

#ifndef __CHEMGPS_H__
#define __CHEMGPS_H__

#define GNU_SOURCE_
#include <stdio.h>
#include <SQXCInterface.h>
#include <SQPCInterface.h>

/*
 * Options for cgps_xxx_option().
 */
#define CGPS_OPTION_FORMAT   1   /* read-write (int) */
#define CGPS_OPTION_SYSLOG   2   /* read-write (int) */
#define CGPS_OPTION_BATCH    3   /* read-write (int) */
#define CGPS_OPTION_DEBUG    4   /* read-write (int) */
#define CGPS_OPTION_VERBOSE  5   /* read-write (int) */
#define CGPS_OPTION_RESULT   6   /* read-write (int) */
#define CGPS_OPTION_PREFIX   7   /* read-write (const char *) */
#define CGPS_OPTION_LICENSE 12   /* write-only (const char *) */

#define CGPS_OUTPUT_FORMAT_PLAIN 1
#define CGPS_OUTPUT_FORMAT_XML   2
#define CGPS_OUTPUT_FORMAT_DEFAULT CGPS_OUTPUT_FORMAT_PLAIN

#define DEFAULT_NUMBER_OBSERVATIONS 1

/*
 * The data loading types for the datfunc() callback:
 */
#define CGPS_GET_QUANTITATIVE_DATA 1
#define CGPS_GET_QUALITATIVE_DATA  2
#define CGPS_GET_LAG_PARENTS_DATA  3
#define CGPS_GET_QUAL_LAGGED_DATA  4

/*
 * Threading policy for the threading member of cgps_options. Any
 * value > 0 enables threading
 */
#define CGPS_THREADING_UNSET    0  /* No specific threading policy */
#define CGPS_THREADING_OFF     -1  /* Turn threading off */
#define CGPS_THREADING_AUTO    -2  /* Detect number of CPU (cores) */
#define CGPS_THREADING_DEFAULT -3  /* Use library default */

/*
 * The various predicted results to output:
 */
enum PREDICTED_RESULTS {
	PREDICTED_RESULTS_NONE,           /* Dummy */
		
	PREDICTED_CONTRIB_SSW,            /* Contribution SSW */
	PREDICTED_CONTRIB_SSW_GROUP,      /* Contribution SSW Group */
	PREDICTED_CONTRIB_SMW,            /* Contribution SMW */
	PREDICTED_CONTRIB_SMW_GROUP,      /* Contribution SMW Group */
	PREDICTED_CONTRIB_DMOD_X,         /* DModX Contribution */
	PREDICTED_CONTRIB_DMOD_X_GROUP,   /* DModX Contribution Group */ 

	PREDICTED_DMOD_X_PS,              /* DModXPS */
	PREDICTED_DMOD_X_PS_COMB,         /* DModXCombinedPS */
	PREDICTED_PMOD_X_PS,              /* PModXPS */
	PREDICTED_PMOD_X_COMB_PS,         /* PModXCombinedPS */

	PREDICTED_TPS,                    /* TPS */
	PREDICTED_TCV_PS,                 /* TcvPS */
	PREDICTED_TCV_SEPS,               /* TcvSEPS */
	PREDICTED_TCV_SED_FPS,            /* TcvSEDFPS */

	PREDICTED_T2_RANGE_PS,            /* T2RangePS */
	PREDICTED_X_OBS_RES_PS,           /* XObsResPS */
	PREDICTED_X_OBS_PRED_PS,          /* XObsPredPS */
	PREDICTED_X_VAR_PS,               /* XVarPS */
	PREDICTED_X_VAR_RES_PS,           /* XVarResPS */
		
	PREDICTED_SERR_LPS,               /* SerrLPS */
	PREDICTED_SERR_UPS,               /* SerrUPS */	
		
	PREDICTED_Y_PRED_PS,              /* YPredPS */
	PREDICTED_Y_PRED_CV_CONF_INT_PS,  /* YPredCVConfIntPS */
	PREDICTED_Y_CV_PS,                /* YcvPS */
	PREDICTED_Y_CV_SEPS,              /* YcvSEPS */
	PREDICTED_Y_OBS_RES_PS,           /* YObsResPS */	
	PREDICTED_Y_VAR_PS,               /* YVarPS */
	PREDICTED_Y_VAR_RES_PS,           /* YVarResPS */	
	
	PREDICTED_RESULTS_ALL,            /* All results */	
	PREDICTED_RESULTS_LAST	          /* NaN entry */
};

struct cgps_project
{
	struct cgps_options *opts;  /* common options */
	SQX_ProjectHandle handle;   /* project handle */
	int models;                 /* number of models */
	char *name;                 /* project name */
};

typedef void (*logfunc)(void *pref, int errcode, int level, const char *file, unsigned int line, const char *fmt, ...);
typedef int  (*datfunc)(struct cgps_project *proj, void *data, SQX_FloatMatrix *fmx, SQX_StringMatrix *smx, SQX_StringVector *names, int type);

struct cgps_options
{
	const char *prog;           /* log prefix */
	int syslog;                 /* use syslog */	
	int debug;                  /* enable debug output */
	int verbose;                /* be more verbose */
	int batch;                  /* enable batch job mode */
	int threading;              /* enable threading */
	
	char *logfile;              /* simca-qp log file */
	char *license;              /* simca-qp license path */
	int format;                 /* output format */
	int result;                 /* bitmask of results */	
	logfunc logger;             /* log function callback */
	datfunc indata;             /* external data loader */
};

struct cgps_predict
{
	SQX_StringMatrix *mqrawdata;         /* pQualData */
	SQX_StringMatrix *mqlagdata;         /* pQualLagData */
	SQX_FloatMatrix *morawdata;          /* pObsRawData */
	SQX_FloatMatrix *molagdata;          /* pObsLagData */
	SQP_ObservationRawData *porawdata;   /* pObservationRawData */
	SQP_QualitativeRawData *pqrawdata;   /* pQualitativeRawData */
	SQX_PredictionHandle handle;         /* pPredict */
	SQX_StringVector varnames;           /* variable names needed as input to Predict() */
	SQX_StringVector lagparents;         /* lag parent names for Predict() */
	SQX_StringVector qualnames;          /* qualitative names for Predict() */
	SQX_StringVector qlagnames;          /* qualitative lagged data names for Predict() */
	void *data;                          /* callback data for indata() */
};

struct cgps_result
{
	SQX_ModelType type;         /* eModelType */
	SQX_FloatMatrix matrix;     /* oFloatMatrix */
	SQX_IntVector index1;       /* oObsIndex1 */
	SQX_IntVector index2;       /* oObsIndex1 */
	FILE *out;                  /* result destination (socket or file) */
};

struct cgps_result_entry
{
	int value;
	const char *name;
	const char *desc;
};

const struct cgps_result_entry * cgps_result_entry_name(int value);
const struct cgps_result_entry * cgps_result_entry_value(const char *name);

extern const struct cgps_result_entry cgps_result_entry_list[];

/*
 * Macros for manipulating bitmasks:
 */
#define cgps_bitmask_fill(value)       ( value = ~0 )
#define cgps_bitmask_empty(value)      ( value =  0 )
#define cgps_bitmask_clear(value, pos) ( value &= ~(1 << (pos)) )
#define cgps_bitmask_set(value, pos)   ( value |=  (1 << (pos)) )
#define cgps_bitmask_get(value, pos)   ( value &   (1 << (pos)) ? 1 : 0 )

#define cgps_result_setall(res)      cgps_bitmask_fill((res))  
#define cgps_result_setopt(res, pos) cgps_bitmask_set((res), (pos))
#define cgps_result_isset(res, pos)  cgps_bitmask_get((res), (pos))

/*
 * Loads the project in path.
 */
int cgps_project_load(struct cgps_project *proj, const char *path, struct cgps_options *opts);

/*
 * Close the project and release allocated resources.
 */
void cgps_project_close(struct cgps_project *proj);

/*
 * Initilize for prediction.
 */
void cgps_predict_init(struct cgps_project *proj, struct cgps_predict *pred, void *data);

/*
 * Make prediction and returns model number or -1 on error.
 */
int cgps_predict(struct cgps_project *proj, int index, struct cgps_predict *pred);

/*
 * Cleanup after an prediction.
 */
void cgps_predict_cleanup(struct cgps_project *proj, struct cgps_predict *pred);

/*
 * Initilize result.
 */
int cgps_result_init(struct cgps_project *proj, struct cgps_result *res);

/*
 * Write result for model to output stream out (file or socket).
 */
int cgps_result(struct cgps_project *proj, int model, struct cgps_predict *pred, struct cgps_result *res, FILE *out);

/*
 * Cleanup result.
 */
void cgps_result_cleanup(struct cgps_project *proj, struct cgps_result *res);

/*
 * Sets an option value.
 */
int cgps_set_option(struct cgps_project *proj, int option, const void *value);

/*
 * Gets an option value.
 */
int cgps_get_option(struct cgps_project *proj, int option, void *value);

#endif /* __CHEMGPS_H__ */
