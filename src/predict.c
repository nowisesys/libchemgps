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

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#endif

#include "chemgps.h"
#include "simcaqp.h"

/*
 * Initilize for prediction. The data argument are data for indata callback.
 */
void cgps_predict_init(struct cgps_project *proj, struct cgps_predict *pred, void *data)
{
	debug("cgps_predict_init: initilizing predict structure");
	memset(pred, 0, sizeof(struct cgps_predict));
	pred->data = data;
}

/*
 * Make prediction.
 */
int cgps_predict(struct cgps_project *proj, int index, struct cgps_predict *pred)
{
	int fitted;
	int model;
	
	if(!SQX_GetModelNumber(proj->handle, index, &model)) {
		logerr("failed get model number for model index %d", index);
		return -1;
	} else {
		debug("got model number %d for model index %d", model, index);
	}
	
	if(!SQX_IsModelFitted(proj->handle, model, &fitted)) {
		fitted = 0;
		logerr("failed check if model is fitted");
	} else {
		debug("model number %d is fitted", model);
	}
	if(!fitted) {
		logerr("model number %d is not fitted (skipped model)", model);
		return -1;
	}

	if(cgps_predict_get_raw_data(proj, model, pred, &pred->varnames) < 0) {
		logerr("failed call cgps_predict_get_raw_data()");
		return -1;
	}

	if(cgps_predict_get_lag_parents(proj, model, pred, &pred->lagparents) < 0) {
		logerr("failed call cgps_predict_get_lag_parents()");
		return -1;
	}
	
	if(cgps_predict_get_qual_data(proj, model, pred, &pred->qualnames) < 0) {
		logerr("failed call cgps_predict_get_qual_data()");
		return -1;
	}
	
	if(cgps_predict_get_qual_data_lagged(proj, model, pred, &pred->qlagnames) < 0) {
		logerr("failed call cgps_predict_get_qual_data_lagged()");
		return -1;
	}

	/*
	 * Store the "ordinary" observation data in the container that 
	 * is sent to Simca-QP.
	 */
	if(pred->morawdata || pred->molagdata) {
		pred->porawdata = malloc(sizeof(SQP_ObservationRawData));
		if(!pred->porawdata) {
			logerr("failed alloc memory");
			return -1;
		}
		pred->porawdata->pObsRawData  = pred->morawdata;
		pred->porawdata->pObsNames    = NULL;
		pred->porawdata->pObsLagData  = pred->molagdata;
		pred->porawdata->pObsLagNames = NULL;
	}
	
	/*
	 * Store the qualitative observation data in the container that 
	 * is sent to Simca-QP.
	 */
	if(pred->mqrawdata || pred->mqlagdata) {
		pred->pqrawdata = malloc(sizeof(SQP_QualitativeRawData));
		if(!pred->pqrawdata) {
			logerr("failed alloc memory");
			return -1;
		}
		pred->pqrawdata->pQualRawData  = pred->mqrawdata;
		pred->pqrawdata->pQualNames    = NULL;
		pred->pqrawdata->pQualLagData  = pred->mqlagdata;
		pred->pqrawdata->pQualLagNames = NULL;
	}

	/*
	 * Make the prediction.
	 */
	if(!SQP_Predict(proj->handle, 
			model, 
			pred->porawdata, 
			pred->pqrawdata, 
			0, 
			&pred->handle)) {
		logerr("failed call predict (%s)", cgps_simcaq_error());
		return -1;
	}
	
	return model;
}

/*
 * Cleanup after an prediction.
 */
void cgps_predict_cleanup(struct cgps_project *proj, struct cgps_predict *pred)
{
	debug("cleaning up after prediction");
	
	/*
	 * Cleanup string vectors.
	 */
	if(SQX_GetNumStringsInVector(&pred->varnames)) {
		SQX_ClearStringVector(&pred->varnames);
	}
	if(SQX_GetNumStringsInVector(&pred->lagparents)) {
		SQX_ClearStringVector(&pred->lagparents);
	}
	if(SQX_GetNumStringsInVector(&pred->qualnames)) {
		SQX_ClearStringVector(&pred->qualnames);
	}
	if(SQX_GetNumStringsInVector(&pred->qlagnames)) {
		SQX_ClearStringVector(&pred->qlagnames);
	}
	
	/*
	 * Cleanup observation data.
	 */
	if(pred->porawdata) {
		if(pred->porawdata->pObsRawData) {
			SQX_ClearFloatMatrix(pred->porawdata->pObsRawData);
			pred->porawdata->pObsRawData = NULL;
		}
	
		if(pred->porawdata->pObsLagData) {
			SQX_ClearFloatMatrix(pred->porawdata->pObsLagData);
			pred->porawdata->pObsLagData = NULL;
		}
		free(pred->porawdata);
		pred->porawdata = NULL;
	}
	if(pred->morawdata) {
		free(pred->morawdata);
		pred->morawdata = NULL;
	}
	if(pred->molagdata) {
		free(pred->molagdata);
		pred->molagdata = NULL;
	}
	
	/*
	 * Cleanup qualitative data.
	 */
	if(pred->pqrawdata) {
		if(pred->pqrawdata->pQualRawData) {
			SQX_ClearStringMatrix(pred->pqrawdata->pQualRawData);
			pred->pqrawdata->pQualRawData = NULL;
		}
		if(pred->pqrawdata->pQualLagData) {
			SQX_ClearStringMatrix(pred->pqrawdata->pQualLagData);
			pred->pqrawdata->pQualLagData = NULL;
		}
		free(pred->pqrawdata);
		pred->pqrawdata = NULL;
	}
	if(pred->mqrawdata) {
		free(pred->mqrawdata);
		pred->mqrawdata = NULL;
	}
	if(pred->mqlagdata) {
		free(pred->mqlagdata);
		pred->mqlagdata = NULL;
	}

	if(pred->handle) {
		if(!SQP_ReleaseHandle(pred->handle)) {
			logerr("failed release handle for predict");
		}
		pred->handle = 0;
	}
}
