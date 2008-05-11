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

#ifdef HAVE_STRING_H
# include <string.h>
#endif
#include <SQXCInterface.h>
#include <SQPCInterface.h>

#include "chemgps.h"
#include "simcaqp.h"

/*
 * The lookup table of all prediction results.
 */
const struct cgps_result_entry cgps_result_entry_list[] = {
	{ PREDICTED_CONTRIB_SSW, "cssw", "Predicted Contribution SSW" },
	{ PREDICTED_CONTRIB_SSW_GROUP, "csswgrp", "Predicted Contribution SSW Group" },
	{ PREDICTED_CONTRIB_SMW, "csmw", "Predicted Contribution SMW" },
	{ PREDICTED_CONTRIB_SMW_GROUP, "csmwgrp", "Predicted Contribution SMW Group" },
	{ PREDICTED_CONTRIB_DMOD_X, "cdmodx", "Predicted DModX Contribution" },
	{ PREDICTED_CONTRIB_DMOD_X_GROUP, "cdmodxgrp", "Predicted DModX Contribution Group" },
	
	{ PREDICTED_DMOD_X_PS, "dmodxps", "Predicted DModXPS" },
	{ PREDICTED_DMOD_X_PS_COMB, "dmodxpscomb", "Predicted DModXCombinedPS" },
	{ PREDICTED_PMOD_X_PS, "pmodxps", "Predicted PModXPS" },
	{ PREDICTED_PMOD_X_COMB_PS, "pmodxcombps", "Predicted PModXCombinedPS" },
	
	{ PREDICTED_TPS, "tps", "Predicted TPS" },
	{ PREDICTED_TCV_PS, "tcvps", "Predicted TcvPS" },
	{ PREDICTED_TCV_SEPS, "tcvseps", "Predicted TcvSEPS" },
	{ PREDICTED_TCV_SED_FPS, "tcvsedfps", "Predicted TcvSEDFPS" },
	
	{ PREDICTED_T2_RANGE_PS, "t2rangeps", "Predicted T2RangePS" },
	{ PREDICTED_X_OBS_RES_PS, "xobsresps", "Predicted XObsResPS" },
	{ PREDICTED_X_OBS_PRED_PS, "xobspredps", "Predicted XObsPredPS" },
	{ PREDICTED_X_VAR_PS, "xvarps", "Predicted XVarPS" },
	{ PREDICTED_X_VAR_RES_PS, "xvarresps", "Predicted XVarResPS" },
	
	{ PREDICTED_SERR_LPS, "serrlps", "Predicted SerrLPS" },
	{ PREDICTED_SERR_UPS, "serrups", "Predicted SerrUPS" },
	
	{ PREDICTED_Y_PRED_PS, "ypredps", "Predicted YPredPS" },
	{ PREDICTED_Y_PRED_CV_CONF_INT_PS, "ypredcvconfintps", "Predicted YPredCVConfIntPS" },
	{ PREDICTED_Y_CV_PS, "ycvps", "Predicted YcvPS" },
	{ PREDICTED_Y_CV_SEPS, "ycvseps", "Predicted YcvSEPS" },
	{ PREDICTED_Y_OBS_RES_PS, "yobsresps", "Predicted YObsResPS" },
	{ PREDICTED_Y_VAR_PS, "yvarps", "Predicted YVarPS" },
	{ PREDICTED_Y_VAR_RES_PS, "yvarresps", "Predicted YVarResPS" },
	
	{ PREDICTED_RESULTS_ALL, "all", "Output All Predicted Results" },
	{ PREDICTED_RESULTS_NONE, NULL, NULL },	
	{ PREDICTED_RESULTS_LAST, NULL, NULL }
};

/*
 * Lookup predicted result entry by name.
 */
const struct cgps_result_entry * cgps_result_entry_value(const char *name)
{
	const struct cgps_result_entry *entry;
	for(entry = cgps_result_entry_list; entry->name; ++entry) {
		if(strcmp(entry->name, name) == 0) {
			return entry;
		}
	}
	return NULL;
}

/*
 * Lookup predicted result entry by value.
 */
const struct cgps_result_entry * cgps_result_entry_name(int value)
{
	const struct cgps_result_entry *entry;
	for(entry = cgps_result_entry_list; entry->name; ++entry) {
		if(entry->value == value) {
			return entry;
		}
	}
	return NULL;
}

/*
 * Print a single floating point number.
 */
static void cgps_result_print_single_value(struct cgps_project *proj, FILE *out, float f)
{
	if(proj->opts->format == CGPS_OUTPUT_FORMAT_PLAIN) {
		fprintf(out, "%f\t\n", f);
	}
	if(proj->opts->format == CGPS_OUTPUT_FORMAT_XML) {
		fprintf(out, "    <values num=\"1\">\n");
		fprintf(out, "      <value>%f</value>\n", f);
		fprintf(out, "    </values>\n");
	}
}

/*
 * Print a float point matrix as a tab separated table.
 */
static int cgps_result_print_matrix_plain(struct cgps_project *proj, FILE *out, SQX_FloatMatrix *matrix)
{
	float f;
	int i, j;
	
	for(i = 0; i < SQX_GetNumColumnsInFloatMatrix(matrix); ++i) {
		for(j = 0; j < SQX_GetNumRowsInFloatMatrix(matrix); ++j) {
			if(!SQX_GetDataFromFloatMatrix(matrix, j + 1, i + 1, &f)) {
				logerr("failed get float value from matrix (%s)", cgps_simcaq_error());
				SQX_ClearFloatMatrix(matrix);
				return -1;
			}
			fprintf(out, "%f\t", f);
		}
		fprintf(out, "\n");
	}

	SQX_ClearFloatMatrix(matrix);
	return 0;
}

/*
 * Print a float point matrix in XML output format.
 */
static int cgps_result_print_matrix_xml(struct cgps_project *proj, FILE *out, SQX_FloatMatrix *matrix)
{
	float f;
	int i, j, cols, rows;

	cols = SQX_GetNumColumnsInFloatMatrix(matrix);
	rows = SQX_GetNumRowsInFloatMatrix(matrix);
	
	for(i = 0; i < cols; ++i) {
		fprintf(out, "    <values num=\"%d\">\n      ", rows);
		for(j = 0; j < rows; ++j) {
			if(!SQX_GetDataFromFloatMatrix(matrix, j + 1, i + 1, &f)) {
				logerr("failed get float value from matrix (%s)", cgps_simcaq_error());
				SQX_ClearFloatMatrix(matrix);
				return -1;
			}
			fprintf(out, "<value>%f</value>", f);
		}
		fprintf(out, "\n    </values>\n");
	}

	SQX_ClearFloatMatrix(matrix);
	return 0;
}

/*
 * Print a float point matrix as a tab separated table.
 */
static int cgps_result_print_matrix(struct cgps_project *proj, FILE *out, SQX_FloatMatrix *matrix)
{
	if(proj->opts->format == CGPS_OUTPUT_FORMAT_PLAIN) {
		return cgps_result_print_matrix_plain(proj, out, matrix);
	}
	if(proj->opts->format == CGPS_OUTPUT_FORMAT_XML) {
		return cgps_result_print_matrix_xml(proj, out, matrix);
	}
	
	/*
	 * Impossible case, but keeps gcc happy :-)
	 */
	logerr("wrong output format for matrix output (expected xml or plain)");
	return -1;
}

/*
 * Print header for result dump.
 */
void cgps_result_print_header(struct cgps_project *proj, FILE *fs, const char *title, const char *name)
{
	if(proj->opts->format == CGPS_OUTPUT_FORMAT_PLAIN) {
		if(proj->opts->verbose) {
			fprintf(fs, "# ----------------------------------------------\n");
			fprintf(fs, "# %s (%s):\n", title, name);
			fprintf(fs, "# ----------------------------------------------\n");
		}
	}
	if(proj->opts->format == CGPS_OUTPUT_FORMAT_XML) {
		if(proj->opts->verbose) {
			fprintf(fs, "  <prediction name=\"%s\" desc=\"%s\">\n", name, title);
		} else {
			fprintf(fs, "  <prediction name=\"%s\">\n", name);
		}
	}
}

/*
 * Print footer for result dump.
 */
void cgps_result_print_footer(struct cgps_project *proj, FILE *fs)
{
	if(proj->opts->format == CGPS_OUTPUT_FORMAT_PLAIN) {		
		fprintf(fs, "\n");
	}
	if(proj->opts->format == CGPS_OUTPUT_FORMAT_XML) {
		fprintf(fs, "  </prediction>\n");
	}
}

/*
 * Print prediction results.
 */
int cgps_result_contrib_ssw(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_CONTRIB_SSW);	
	if(!SQP_GetPredictedContributionsSSW(pred->handle, 
					     -1,           /* model number, not used */
					     0,            /* iObs1Ix */ 
					     1,            /* iObs2Ix */ 
					     SQX_NoWeight, 
					     numcomp, 
					     1, 
					     0,            /* bReconstruct */ 
					     &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_contrib_ssw_group(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_CONTRIB_SSW_GROUP);	
	
	if(!SQP_GetPredictedContributionsSSWGroup(pred->handle, 
						  -1,      /* model number, not used */
						  &res->index1, 
						  &res->index2, 
						  SQX_NoWeight, 
						  numcomp, 
						  1, 
						  0,       /* bReconstruct */
						  &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
		
	return 0;
}

int cgps_result_contrib_smw(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_CONTRIB_SMW);
	if(!SQP_GetPredictedContributionsSMW(pred->handle, 
					     -1,           /* model number, not used */
					     0,            /* iObs1Ix */
					     1,            /* iObs2Ix */
					     SQX_P, 
					     NULL,         /* pComponents */
					     0,            /* bReconstruct */
					     &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_contrib_smw_group(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_CONTRIB_SMW_GROUP);
	if(!SQP_GetPredictedContributionsSMWGroup(pred->handle, 
						  -1,      /* model number, not used */
						  &res->index1, 
						  &res->index2, 
						  SQX_P, 
						  NULL,    /* pComponents */
						  0,       /* bReconstruct */
						  &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_contrib_dmod_x(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_CONTRIB_DMOD_X);
	if(!SQP_GetPredictedContributionsDModX(pred->handle, 
					       -1,         /* model number, not used */ 
					       1,          /* iObsIx */
					       SQX_RX, 
					       numcomp, 
					       1, 
					       0,          /* bReconstruct */
					       &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_contrib_dmod_x_group(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_CONTRIB_DMOD_X_GROUP);
	if(!SQP_GetPredictedContributionsDModXGroup(pred->handle, 
						    -1,    /* model number, not used */
						    &res->index1, 
						    SQX_RX, 
						    numcomp, 
						    1, 
						    0,     /* bReconstruct */
						    &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	SQX_ClearIntVector(&res->index1);
	SQX_ClearIntVector(&res->index2);
	
	return 0;
}

int cgps_result_dmod_x_ps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_DMOD_X_PS);
	if(!SQP_GetPredictedDModX(pred->handle, 
				  -1,                      /* model number, not used */
				  NULL,                    /* pnComponentList */
				  1,                       /* bNormalized */
				  0,                       /* bModelingPowerWeighted */
				  &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_dmod_x_ps_comb(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_DMOD_X_PS_COMB);
	if(!SQP_GetPredictedDModXCombined(pred->handle, 
					  -1,              /* model number, not used */
					  NULL,            /* pnComponentList */
					  1,               /* bNormalized */
					  0,               /* bModelingPowerWeighted */
					  &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_pmod_x_ps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_PMOD_X_PS);
	if(!SQP_GetPredictedPModX(pred->handle, 
				  -1,                      /* model number, not used */
				  NULL,                    /* pnComponentList */
				  &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_pmod_x_comb_ps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_PMOD_X_COMB_PS);
	if(!SQP_GetPredictedPModXCombined(pred->handle, 
					  -1,              /* model number, not used */
					  NULL,            /* pnComponentList */
					  &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}

	return 0;
}

int cgps_result_tps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;

	entry = cgps_result_entry_name(PREDICTED_TPS);
	if(numcomp <= 0) {
		logwarn("%s is not valid for a zero component model (skipped)", entry->desc);
		return -1;
	}
	if(!SQP_GetPredictedT(pred->handle, 
			      -1,                          /* model number, not used */
			      NULL,                        /* pnComponentList */
			      &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_tcv_ps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_TCV_PS);
	if(numcomp <= 0) {
		logwarn("%s is not valid for a zero component model (skipped)", entry->desc);
		return -1;
	}
	if(!SQP_GetPredictedTcv(pred->handle, 
				-1,                        /* model number, not used */
				numcomp, 
				&res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_tcv_seps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_TCV_SEPS);
	if(numcomp <= 0) {
		logwarn("%s is not valid for a zero component model (skipped)", entry->desc);
		return -1;
	}
	if(!SQP_GetPredictedTcvSE(pred->handle, 
				  -1,                      /* model number, not used */
				  numcomp, 
				  &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_tcv_sed_fps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	float f;
	
	entry = cgps_result_entry_name(PREDICTED_TCV_SED_FPS);
	if(numcomp <= 0) {
		logwarn("%s is not valid for a zero component model (skipped)", entry->desc);
		return -1;
	}
	if(!SQP_GetPredictedTcvSEDF(pred->handle, 
				    -1,                    /* model number, not used */
				    numcomp, 
				    &f)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_single_value(proj, res->out, f);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}
	
int cgps_result_t2_range_ps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_T2_RANGE_PS);
	if(!SQP_GetPredictedT2Range(pred->handle, 
				    -1,                    /* model number, not used */
				    1,    
				    numcomp, 
				    &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_x_obs_res_ps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_X_OBS_RES_PS);
	if(!SQP_GetPredictedXObsRes(pred->handle, 
				    -1,                   /* model number, not used */
				    numcomp, 
				    1,                    /* bUnscaled */
				    1,                    /* bBackTransformed */
				    NULL,                 /* pnObservationList */
				    1,                    /* bReconstruct */
				    &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_x_obs_pred_ps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{       
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_X_OBS_PRED_PS);
	if(!SQP_GetPredictedXObsPred(pred->handle, 
				     -1,                  /* model number, not used */
				     numcomp, 
				     1,                   /* bUnscaled */
				     1,                   /* bBackTransformed */
				     NULL,                /* pnObservationList */
				     1,                   /* bReconstruct */
				     &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_x_var_ps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_X_VAR_PS);
	if(!SQP_GetPredictedXVar(pred->handle, 
				 -1,                      /* model number, not used */
				 1,                       /* bUnscaled */
				 1,                       /* bBackTransformed */
				 NULL,                    /* pnColumnXIndices */
				 &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_x_var_res_ps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_X_VAR_RES_PS);
	if(!SQP_GetPredictedXVarRes(pred->handle, 
				    -1,                   /* model number, not used */
				    numcomp, 
				    NULL,                 /* pnColumnXIndices */
				    1,                    /* bUnscaled */
				    1,                    /* bBackTransformed */
				    0,                    /* bStandardized */
				    &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_serr_lps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp, int isresuinc)
{	
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_SERR_LPS);
	if(!(res->type == SQX_PLS_Class) || 
	    (res->type == SQX_PLS) || 
	    (res->type == SQX_PLS_DA)) {
		logwarn("%s is only valid for a PLS model (skipped)", entry->desc);
		return -1;
	}
	if(isresuinc != 1) {
		logwarn("%s is only valid if model residuals is still in the project (skipped)", 
			entry->desc);
		return -1;
	}
	if(!SQP_GetPredictedSerrL(pred->handle, 
				  -1,                      /* model number, not used */
				  numcomp, 
				  NULL,                    /* pnColumnYIndices */
				  &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_serr_ups(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp, int isresuinc)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_SERR_UPS);
	if(!(res->type == SQX_PLS_Class) || 
	    (res->type == SQX_PLS) || 
	    (res->type == SQX_PLS_DA)) {
		logwarn("%s is only valid for a PLS model (skipped)", entry->desc);
		return -1;
	}
	if(isresuinc != 1) {
		logwarn("%s is only valid if model residuals is still in the project (skipped)", 
			entry->desc);
		return -1;
	}
	if(!SQP_GetPredictedSerrU(pred->handle, 
				  -1,                      /* model number, not used */
				  numcomp, 
				  NULL,                    /* pnColumnYIndices */
				  &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_y_pred_ps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_Y_PRED_PS);
	if(!((numcomp > 0) && 
	    ((res->type == SQX_PLS_Class) || 
	     (res->type == SQX_PLS) || 
	     (res->type == SQX_PLS_DA)))) {
		logwarn("%s is only valid for a PLS model (skipped)", entry->desc);
		return -1;
	}
	if(!SQP_GetPredictedY(pred->handle, 
			      -1,                          /* model number, not used */
			      numcomp, 
			      1,                           /* bUnscaled */
			      0,                           /* bBackTransformed */
			      NULL,                        /* pnColumnYIndexes */
			      &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_y_pred_cv_conf_int_ps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_Y_PRED_CV_CONF_INT_PS);
	if(!((numcomp > 0) && 
	    ((res->type == SQX_PLS_Class) || 
	     (res->type == SQX_PLS) || 
	     (res->type == SQX_PLS_DA)))) {
		logwarn("%s is only valid for a PLS model (skipped)", entry->desc);
		return -1;
	}
	if(!SQP_GetPredictedYPredCVConfInt(pred->handle, 
					   -1,             /* model number, not used */
					   numcomp, 
					   1,              /* bUnscaled */
					   0,              /* bBackTransformed */
					   NULL,           /* pnColumnYIndexes */
					   &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_y_cv_ps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_Y_CV_PS);
	if(!((numcomp > 0) && 
	    ((res->type == SQX_PLS_Class) || 
	     (res->type == SQX_PLS) || 
	     (res->type == SQX_PLS_DA)))) {
		logwarn("%s is only valid for a PLS model (skipped)", entry->desc);
		return -1;
	}
	if(!SQP_GetPredictedYCV(pred->handle, 
				-1,                        /* model number, not used */
				numcomp, 
				1,                         /* bUnscaled */
				0,                         /* bBackTransformed */
				1,                         /* nColumnYIndex */
				&res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_y_cv_seps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_Y_CV_SEPS);
	if(!((numcomp > 0) && 
	    ((res->type == SQX_PLS_Class) || 
	     (res->type == SQX_PLS) || 
	     (res->type == SQX_PLS_DA)))) {
		logwarn("%s is only valid for a PLS model (skipped)", entry->desc);
		return -1;
	}
	if(!SQP_GetPredictedYCVSE(pred->handle, 
				  -1,                      /* model number, not used */
				  numcomp, 
				  NULL,                    /* pnColumnYIndexes */
				  &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_y_obs_res_ps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_Y_OBS_RES_PS);
	if(!((numcomp > 0) && 
	    ((res->type == SQX_PLS_Class) || 
	     (res->type == SQX_PLS) || 
	     (res->type == SQX_PLS_DA)))) {
		logwarn("%s is only valid for a PLS model (skipped)", entry->desc);
		return -1;
	}
	if(!SQP_GetPredictedYObsRes(pred->handle, 
				    -1,                    /* model number, not used */
				    numcomp, 
				    NULL,                  /* pnObservationList */
				    1,                     /* bUnscaled */
				    0,                     /* bBackTransformed */
				    &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_y_var_ps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_Y_VAR_PS);
	if(!((numcomp > 0) && 
	    ((res->type == SQX_PLS_Class) || 
	     (res->type == SQX_PLS) || 
	     (res->type == SQX_PLS_DA)))) {
		logwarn("%s is only valid for a PLS model (skipped)", entry->desc);
		return -1;
	}
	if(!SQP_GetPredictedYVar(pred->handle, 
				 -1,                       /* model number, not used */
				 1,                        /* bUnscaled */
				 0,                        /* bBackTransformed */
				 NULL,                     /* pnColumnYIndexes */
				 &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

int cgps_result_y_var_res_ps(struct cgps_project *proj, struct cgps_predict *pred, struct cgps_result *res, int numcomp)
{
	const struct cgps_result_entry *entry;
	
	entry = cgps_result_entry_name(PREDICTED_Y_VAR_RES_PS);
	if(!((numcomp > 0) && 
	    ((res->type == SQX_PLS_Class) || 
	     (res->type == SQX_PLS) || 
	     (res->type == SQX_PLS_DA)))) {
		logwarn("%s is only valid for a PLS model (skipped)", entry->desc);
		return -1;
	}
	if(!SQP_GetPredictedYVarRes(pred->handle, 
				    -1,                    /* model number, not used */
				    numcomp, 
				    1,                     /* bUnscaled */
				    0,                     /* bBackTransformed */
				    1,                     /* bStandardized */
				    NULL,                  /* pnColumnYIndexes */
				    &res->matrix)) {
		logerr("failed get result of %s", entry->desc);
		return -1;
	} else {
		cgps_result_print_header(proj, res->out, entry->desc, entry->name);
		cgps_result_print_matrix(proj, res->out, &res->matrix);
		cgps_result_print_footer(proj, res->out);
	}
	
	return 0;
}

/*
 * Initilize result, the enforcement of a valid project handle is not strictly
 * required, but follow our documented and suggested program flow.
 */
int cgps_result_init(struct cgps_project *proj, struct cgps_result *res)
{	
	if(proj->handle) {
		debug("initilizing result object");
		
		SQX_InitIntVector(&res->index1, 2);
		SQX_SetDataInIntVector(&res->index1, 1, 1);
		SQX_SetDataInIntVector(&res->index1, 2, 2);
		
		SQX_InitIntVector(&res->index2, 2);
		SQX_SetDataInIntVector(&res->index2, 1, 3);
		SQX_SetDataInIntVector(&res->index2, 2, 4);
		
		return 0;
	}
	
	logerr("no valid project handle");
	return -1;		
}

/*
 * Get result.
 */
int cgps_result(struct cgps_project *proj, int model, struct cgps_predict *pred, struct cgps_result *res, FILE *out)
{
	int numcomp = 0;
	int isreduced = 0;    /* Contains 1 if the project is a Reduced SIMCA-P project. */
	int isdatainc = 1;    /* Contains 1 if the primary data set is included in the project. */
	int isresuinc = 1;    /* Contains 1 if the model residuals is included in the project. */

	res->out = out;
	
	/* 
	 * Check if the project is a Reduced SIMCA-P Project (.rusp) 
	 */
	if(!SQX_GetIsReducedUSP(proj->handle, &isreduced)) {
		logerr("failed check if project is a reduced Simca-Q project (*.rusp) (%s)",
		       cgps_simcaq_error());
		return -1;
	}
	if(isreduced) {
		debug("project is a reduced Simca-Q project (*.rusp)");
		
		/* 
		 * Check if the primary data set has been excluded from the project.
		 */
		if(!SQX_GetIsPrimaryDataSetIncluded(proj->handle, &isdatainc)) {
			logerr("failed check if primary data set has been excluded from the project (%s)",
			       cgps_simcaq_error());
			return -1;
		} else {
			debug("primary data set has been excluded from the project");
		}
		
		/* 
		 * Check if the model residuals has been excluded from the project.
		 */
		if(!SQX_GetIsModelResidualsIncluded(proj->handle, &isresuinc)) {
			logerr("failed check if the model residuals has been excluded from the project (%s)",
			       cgps_simcaq_error());
			return -1;
		} else {
			debug("model residuals has been excluded from the project");
		}
	}
	
	/*
	 * Get the number of components for this model.
	 */
	if(!SQX_GetModelNumberOfComponents(proj->handle, model, &numcomp)) {
		logerr("failed get number of components for this model (%s)", cgps_simcaq_error());
		return -1;
	} else {
		debug("number of components for this model is %d", numcomp);
	}
	
	/*
	 * Get the model type for this model.
	 */
	if(!SQX_GetModelType(proj->handle, model, &res->type)) {
		logerr("failed get model type for this model (%s)", cgps_simcaq_error());
		return -1;
	} else {
		if(res->type == SQX_UnDefined) {
			debug("the model type can not be determined");
		} else {
#ifndef NDEBUG
			const char *types[] = {
				"undefined", "PCA_X", "PCA_Y", "PCA_All", 
				"PCA_Class", "PSL_Class", "PLS", "PLS_DA"
			};
			debug("the model is a %s model", types[res->type]);
#endif /* NDEBUG */
		}
	}
	
	/*
	 * Output XML header.
	 */
	if(proj->opts->format == CGPS_OUTPUT_FORMAT_XML) {
		fprintf(res->out, "<?xml version=\"1.0\"?>\n");
		if(proj->opts->verbose) {
			fprintf(res->out, "<result generator=\"%s\" version=\"%s\">\n", 
				proj->opts->prog, PACKAGE_VERSION);
		} else {
			fprintf(res->out, "<result>\n");
		}
	}
	
	/*
	 * See which result to present.
	 */
	if(cgps_result_isset(proj->opts->result, PREDICTED_CONTRIB_SSW)) {
		cgps_result_contrib_ssw(proj, pred, res, numcomp);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_CONTRIB_SSW_GROUP)) {
		cgps_result_contrib_ssw_group(proj, pred, res, numcomp);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_CONTRIB_SMW)) {
		cgps_result_contrib_smw(proj, pred, res);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_CONTRIB_SMW_GROUP)) {
		cgps_result_contrib_smw_group(proj, pred, res);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_CONTRIB_DMOD_X)) {
		cgps_result_contrib_dmod_x(proj, pred, res, numcomp);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_CONTRIB_DMOD_X_GROUP)) {
		cgps_result_contrib_dmod_x_group(proj, pred, res, numcomp);
	}
	
	if(cgps_result_isset(proj->opts->result, PREDICTED_DMOD_X_PS)) {
		cgps_result_dmod_x_ps(proj, pred, res);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_DMOD_X_PS_COMB)) {
		cgps_result_dmod_x_ps_comb(proj, pred, res);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_PMOD_X_PS)) {
		cgps_result_pmod_x_ps(proj, pred, res);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_PMOD_X_COMB_PS)) {
		cgps_result_pmod_x_comb_ps(proj, pred, res);
	}

	/*
	 * These four function requires that numcomp > 0
	 */
	if(cgps_result_isset(proj->opts->result, PREDICTED_TPS)) {
		cgps_result_tps(proj, pred, res, numcomp);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_TCV_PS)) {
		cgps_result_tcv_ps(proj, pred, res, numcomp);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_TCV_SEPS)) {
		cgps_result_tcv_seps(proj, pred, res, numcomp);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_TCV_SED_FPS)) {
		cgps_result_tcv_sed_fps(proj, pred, res, numcomp);
	}
	
	if(cgps_result_isset(proj->opts->result, PREDICTED_T2_RANGE_PS)) {
		cgps_result_t2_range_ps(proj, pred, res, numcomp);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_X_OBS_RES_PS)) {
		cgps_result_x_obs_res_ps(proj, pred, res, numcomp);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_X_OBS_PRED_PS)) {
		cgps_result_x_obs_pred_ps(proj, pred, res, numcomp);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_X_VAR_PS)) {
		cgps_result_x_var_ps(proj, pred, res);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_X_VAR_RES_PS)) {
		cgps_result_x_var_res_ps(proj, pred, res, numcomp);
	}

	if(cgps_result_isset(proj->opts->result, PREDICTED_SERR_LPS)) {
		cgps_result_serr_lps(proj, pred, res, numcomp, isresuinc);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_SERR_UPS)) {
		cgps_result_serr_ups(proj, pred, res, numcomp, isresuinc);
	}

	if(cgps_result_isset(proj->opts->result, PREDICTED_Y_PRED_PS)) {
		cgps_result_y_pred_ps(proj, pred, res, numcomp);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_Y_PRED_CV_CONF_INT_PS)) {
		cgps_result_y_pred_cv_conf_int_ps(proj, pred, res, numcomp);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_Y_CV_PS)) {
		cgps_result_y_cv_ps(proj, pred, res, numcomp);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_Y_CV_SEPS)) {
		cgps_result_y_cv_seps(proj, pred, res, numcomp);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_Y_OBS_RES_PS)) {
		cgps_result_y_obs_res_ps(proj, pred, res, numcomp);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_Y_VAR_PS)) {
		cgps_result_y_var_ps(proj, pred, res, numcomp);
	}
	if(cgps_result_isset(proj->opts->result, PREDICTED_Y_VAR_RES_PS)) {
		cgps_result_y_var_res_ps(proj, pred, res, numcomp);
	}

	if(proj->opts->format == CGPS_OUTPUT_FORMAT_XML) {
		fprintf(res->out, "</result>\n");
	}
	
	return 0;
}

/*
 * Cleanup result.
 */
void cgps_result_cleanup(struct cgps_project *proj, struct cgps_result *res)
{
	if(proj->handle) {
		debug("cleaning up result object");
	} else {
		logwarn("no valid project handle");
	}
	
	SQX_ClearFloatMatrix(&res->matrix);
	SQX_ClearIntVector(&res->index1);
	SQX_ClearIntVector(&res->index2);
}
