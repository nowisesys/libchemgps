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

#define _GNU_SOURCE
#include <stdio.h>
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include "chemgps.h"
#include "simcaqp.h"

/*
 * Functions for loading data for prediction.
 */

/*
 * Print strings in vector names to buffer buff. The buffer is dynamic resized
 * and should be released by calling free(3) when done. 
 */
static int cgps_predict_vecstr(struct cgps_project *proj, char **buff, size_t *size, SQX_StringVector *names)
{
	FILE *fp;
	const char *del  = "";
	const char *str;
	int i, num;
	
	*buff = NULL;
	*size = 0;
			
	fp = open_memstream(buff, size);
	if(!fp) {
		logerr("failed open buffer writer stream");
		return -1;             /* not critical */
	}
	
	num = SQX_GetNumStringsInVector(names);
	for(i = 0; i < num; ++i) {
		if(!SQX_GetStringFromVector(names, i + 1, &str)) {
			logerr("failed get string from vector (%s)", cgps_simcaq_error());
			return -1;
		}
		if(str) {
			fprintf(fp, "%s[%s (%d/%d)]", del, str, i + 1, num);
		}
		del = ", ";
	}
	fclose(fp);
	return 0;
}

/*
 * Load quantitative (raw) data.
 */
int cgps_predict_get_raw_data(struct cgps_project *proj, int model, struct cgps_predict *pred, SQX_StringVector *names)
{
	int num;            /* number of elements in names */
	
	if(!SQP_GetQuantitativeNamesForPredict(proj->handle, model, 0, names)) {
		logerr("failed get quantitative variable names (%s)", cgps_simcaq_error());
		return -1;
	} else {
		debug("successful got quantitative variable names");
	}
	
	/*
	 * Set the data to use for the Prediction.
	 */
	if((num = SQX_GetNumStringsInVector(names)) > 0) {
		debug("there are %d \"ordinary\" variables in this model", num);

		if(pred->morawdata) {
			free(pred->morawdata);
		}
		pred->morawdata = malloc(sizeof(SQX_FloatMatrix));
		if(!pred->morawdata) {
			logerr("failed alloc memory");
			return -1;
		}
		
		if(proj->opts->indata(proj, pred->data, pred->morawdata, NULL, names, CGPS_GET_QUANTITATIVE_DATA) < 0) {
			logerr("failed load raw data (quantitative)");
			return -1;
		}
		
		if(proj->opts->debug) {
			char *buff = NULL;
			size_t size = 0;
			
			if(cgps_predict_vecstr(proj, &buff, &size, names) < 0) {
				logerr("failed get quanitative variable names");
				free(buff);
				return 0;        /* not critical */
			}
			
			debug("quantitative variable names: %s", buff);
			free(buff);
		}
	}
	
	return 0;
}

/*
 * Check if the project contains any lagged data. These are the names 
 * of the lagged variables whos data are needed as input for a predition.
 */
int cgps_predict_get_lag_parents(struct cgps_project *proj, int model, struct cgps_predict *pred, SQX_StringVector *parents)
{
	int num;       /* number of elements in parents */
	
	if(!SQP_GetLagParentNamesForPredict(proj->handle, model, 0, parents)) {
		logerr("failed get names of lagged variables (%s)", cgps_simcaq_error());
		return -1;
	}
	if((num = SQX_GetNumStringsInVector(parents)) > 0) {
		debug("there are %d lagged variables in this model", num);

		/*
		 * TODO: Check this code against SQPrepareForPredictionExample.c line 83.
		 */
		
		if(pred->molagdata) {
			free(pred->molagdata);
		}
		pred->molagdata = malloc(sizeof(SQX_FloatMatrix));
		if(!pred->molagdata) {
			logerr("failed alloc memory");
			return -1;
		}
		
		if(proj->opts->indata(proj, pred->data, pred->molagdata, NULL, parents, CGPS_GET_LAG_PARENTS_DATA) < 0) {
			logerr("failed load lagged variables (parents)");
			return -1;
		}
				
		if(proj->opts->debug) {
			int i;
			
			for(i = 0; i < num; ++i) {
				SQX_StringVector names;
				char *buff = NULL;
				size_t size = 0;
				
				if(!SQP_GetCompleteVariableLagNames(proj->handle, model, i + 1, 0, &names)) {
					logerr("failed get complete variable lag names (%s)", cgps_simcaq_error());
					return 0;     /* not critical */
				}
			
				if(cgps_predict_vecstr(proj, &buff, &size, &names) < 0) {
					logerr("failed get lag variable names");
					free(buff);
					return 0;     /* not critical */
				}
				debug("lagged variable names: %s", buff);
				free(buff);
				SQX_ClearStringVector(&names);
			}
		}
	}
	
	return 0;
}

 /*
  * Check if the project contains any qualitative data. These are the names 
  * of the qualitative variables whos data are needed as input for a prediction.
  */
int cgps_predict_get_qual_data(struct cgps_project *proj, int model, struct cgps_predict *pred, SQX_StringVector *names)
{
	int num;       /* number of elements in names */

	if(!SQP_GetQualitativeNamesForPredict(proj->handle, model, names)) {
		logerr("failed get names of the qualitative variables (%s)", cgps_simcaq_error());
		return -1;
	}
	if((num = SQX_GetNumStringsInVector(names)) > 0) {
		debug("there are %d qualitative variables in this model", num);
	
		/*
		 * TODO: Check this code against SQPrepareForPredictionExample.c line 156.
		 */
		
		if(pred->mqrawdata) {
			free(pred->mqrawdata);
		}
		pred->mqrawdata = malloc(sizeof(SQX_StringMatrix));
		if(!pred->mqrawdata) {
			logerr("failed alloc memory");
			return -1;
		}
		
		if(proj->opts->indata(proj, pred->data, NULL, pred->mqrawdata, names, CGPS_GET_QUALITATIVE_DATA) < 0) {
			logerr("failed load lagged variables (parents)");
			return -1;
		}
		
		if(proj->opts->debug) {
			char *buff = NULL;
			size_t size = 0;

			if(cgps_predict_vecstr(proj, &buff, &size, names) < 0) {
				logerr("failed get qualitative variable names");
				free(buff);
				return 0;        /* not critical */
			}			
			debug("qualitative variable names: %s", buff);
			free(buff);
		}
	}
	
	return 0;
}

/*
 * Check if the project contains any qualitative data that are lagged. These 
 * are the names of the lagged qualitative variables whos data are needed as 
 * input for a prediction.
 */
int cgps_predict_get_qual_data_lagged(struct cgps_project *proj, int model, struct cgps_predict *pred, SQX_StringVector *names)
{
	int num;
	
	if(!SQP_GetLagParentNamesForPredict(proj->handle, model, 1, names)) {
		logerr("failed get names of lagged qualitative variables (%s)", cgps_simcaq_error());
		return -1;
	}
	if((num = SQX_GetNumStringsInVector(names)) > 0) {
		debug("there are %d lagged qualitative variables", num);
		
		/*
		 * TODO: Check this code against SQPrepareForPredictionExample.c line 209.
		 */
		
		if(pred->mqrawdata) {
			free(pred->mqrawdata);
		}
		pred->mqrawdata = malloc(sizeof(SQX_StringMatrix));
		if(!pred->mqrawdata) {
			logerr("failed alloc memory");
			return -1;
		}
		
		if(proj->opts->indata(proj, pred->data, NULL, pred->mqlagdata, names, CGPS_GET_QUAL_LAGGED_DATA) < 0) {
			logerr("failed load lagged variables (parents)");
			return -1;
		}

		if(proj->opts->debug) {
			int i;
			
			for(i = 0; i < num; ++i) {
				SQX_StringVector names;
				char *buff = NULL;
				size_t size = 0;
				
				if(!SQP_GetCompleteVariableLagNames(proj->handle, model, i + 1, 1, &names)) {
					logerr("failed get qualitative lag variable names (%s)", cgps_simcaq_error());
					return 0;     /* not critical */
				}
				
				if(cgps_predict_vecstr(proj, &buff, &size, &names) < 0) {
					logerr("failed get qualitative lag variable names");
					free(buff);
					return 0;     /* not critical */
				}
				debug("qualitative lag variable names: %s", buff);
				free(buff);
				SQX_ClearStringVector(&names);
			}
		}
	}
	
	return 0;
}
