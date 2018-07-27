/*
    Ophidia Primitives
    Copyright (C) 2012-2018 CMCC Foundation

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "oph_fdi_fwi.h"

#define OPH_FDI_FWI_NUM_MEASURE 4

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_fdi_fwi_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if (args->arg_count < 5 || args->arg_count > 9) {
		strcpy(message, "ERROR: Wrong arguments! oph_fdi_fwi(input_OPH_TYPE, output_OPH_TYPE, measure, date, lat, [INDEX], [prev_ffmc], [prev_dmc], [prev_dc])");
		return 1;
	}

	for (i = 0; i < 4; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_fdi_fwi function");
			return 1;
		}
	}
	args->arg_type[4] = REAL_RESULT;
	if (args->arg_count >= 6) {
		if (args->arg_type[5] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_fdi_fwi function");
			return 1;
		}
		if (args->arg_count >= 7) {
			args->arg_type[6] = REAL_RESULT;
			if (args->arg_count >= 8) {
				args->arg_type[7] = REAL_RESULT;
				if (args->arg_count == 9) {
					args->arg_type[8] = REAL_RESULT;
				}
			}
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;
	return 0;
}

void oph_fdi_fwi_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		oph_generic_param_multi *param = (oph_generic_param_multi *) initid->ptr;
		free_oph_generic_param_multi(param);
		if (param->extend) {
			free(param->extend);
			param->extend = NULL;
		}
		initid->ptr = NULL;
	}
}

char *oph_fdi_fwi(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	if (*error) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	if (*is_null || !args->lengths[2]) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
	}

	oph_generic_param_multi *param;
	if (!initid->ptr) {
		param = (oph_generic_param_multi *) malloc(sizeof(oph_generic_param_multi));
		if (!param) {
			pmesg(1, __FILE__, __LINE__, "Error in allocating parameters\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		param->measure = NULL;
		param->result = NULL;
		param->error = 0;
		param->extend = NULL;

		initid->ptr = (char *) param;
	} else
		param = (oph_generic_param_multi *) initid->ptr;

	if (param->error) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	oph_multistring *measure;
	if (!param->error && !param->measure) {
		if (core_set_oph_multistring(&measure, args->args[0], &(args->lengths[0]))) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (!measure->islast) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Wrong number of input measure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
//              if (measure->num_measure != OPH_FDI_FWI_NUM_MEASURE)
//              {
//                      param->error = 1;
//                      pmesg(1, __FILE__, __LINE__, "Wrong number of input measure\n");
//                      *length=0;
//                      *is_null=0;
//                      *error=1;
//                      return NULL;
//              }
//              if (measure->type[0]!=OPH_INT && measure->type[0]!=OPH_LONG && measure->type[0]!=OPH_FLOAT && measure->type[0]!=OPH_DOUBLE) {
//                  pmesg(1,  __FILE__, __LINE__, "Invalid type\n");
//                  *length=0;
//                  *is_null=0;
//                  *error=1;
//                  return NULL;
//              }
//              int j;
//              for (j=0;j<measure->num_measure;++j) if (measure->type[j] != measure->type[0])
//              {
//                      param->error = 1;
//                      pmesg(1, __FILE__, __LINE__, "Data type of input fields are different\n");
//                      *length=0;
//                      *is_null=0;
//                      *error=1;
//                      return NULL;
//              }
		if (measure->num_measure != 1) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Wrong number of input measure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (measure->type[0] != OPH_INT && measure->type[0] != OPH_LONG && measure->type[0] != OPH_FLOAT && measure->type[0] != OPH_DOUBLE) {
			pmesg(1, __FILE__, __LINE__, "Invalid type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		measure->length = args->lengths[2];
		measure->blocksize *= 4;	//TODO
		if (measure->length % measure->blocksize) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Wrong input type or data corrupted\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		measure->numelem = measure->length / measure->blocksize;

		param->measure = measure;
	} else
		measure = param->measure;

	measure->content = args->args[2];

	oph_multistring *output;
	if (!param->error && !param->result) {
		if (core_set_oph_multistring(&output, args->args[1], &(args->lengths[1]))) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
//              if (output->num_measure != 4)
//              {
//                      param->error = 1;
//                      pmesg(1, __FILE__, __LINE__, "Wrong number of output data type\n");
//                      *length=0;
//                      *is_null=0;
//                      *error=1;
//                      return NULL;
//              }
//              if (output->type[0]!=OPH_INT && output->type[0]!=OPH_LONG && output->type[0]!=OPH_FLOAT && output->type[0]!=OPH_DOUBLE) {
//                  pmesg(1,  __FILE__, __LINE__, "Invalid type\n");
//                  *length=0;
//                  *is_null=0;
//                  *error=1;
//                  return NULL;
//              }
//              int j;
//              for (j=0;j<output->num_measure;++j) if (output->type[j] != output->type[0])
//              {
//                      param->error = 1;
//                      pmesg(1, __FILE__, __LINE__, "Data type of output fields are different\n");
//                      *length=0;
//                      *is_null=0;
//                      *error=1;
//                      return NULL;
//              }
		if (output->num_measure != 1) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Wrong number of output data type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (output->type[0] != OPH_INT && output->type[0] != OPH_LONG && output->type[0] != OPH_FLOAT && output->type[0] != OPH_DOUBLE) {
			pmesg(1, __FILE__, __LINE__, "Invalid type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		output->numelem = measure->numelem;
		output->blocksize *= 4;	//TODO
		output->length = output->numelem * output->blocksize;
		if (!output->length) {
			*length = 0;
			*is_null = 1;
			*error = 0;
			return NULL;
		}
		output->content = (char *) malloc(output->length);
		if (!output->content) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Error allocating output string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		param->result = output;
	} else
		output = param->result;

	oph_fdi_fwi_conf *extra;
	if (!param->extend) {
		extra = (oph_fdi_fwi_conf *) malloc(sizeof(oph_fdi_fwi_conf));
		if (!extra) {
			pmesg(1, __FILE__, __LINE__, "Error allocating extra memory\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		snprintf(extra->index, 10, "%s", "FWI");	//default
		extra->dc0 = 0;	//default
		extra->dmc0 = 0;	//default
		extra->ffmc0 = 0;	//default

		extra->date0.tm_year = 0;
		extra->date0.tm_mon = 0;
		extra->date0.tm_mday = 1;
		extra->date0.tm_hour = 0;
		extra->date0.tm_min = 0;
		extra->date0.tm_sec = 0;
		extra->date0.tm_isdst = -1;

		extra->date.tm_year = 0;
		extra->date.tm_mon = 0;
		extra->date.tm_mday = 1;
		extra->date.tm_hour = 0;
		extra->date.tm_min = 0;
		extra->date.tm_sec = 0;
		extra->date.tm_isdst = -1;

		if (!strptime(args->args[3], "%d/%m/%Y", &(extra->date0))) {
			pmesg(1, __FILE__, __LINE__, "Invalid date\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (mktime(&(extra->date0)) == -1) {
			pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		extra->lat = *((double *) args->args[4]);
		if (args->arg_count >= 6) {
			if (!strcasecmp(args->args[5], "FFMC")) {
				memset(extra->index, 0, 10);
				snprintf(extra->index, 10, "%s", "FFMC");
			} else if (!strcasecmp(args->args[5], "DMC")) {
				memset(extra->index, 0, 10);
				snprintf(extra->index, 10, "%s", "DMC");
			} else if (!strcasecmp(args->args[5], "DC")) {
				memset(extra->index, 0, 10);
				snprintf(extra->index, 10, "%s", "DC");
			} else if (!strcasecmp(args->args[5], "ISI")) {
				memset(extra->index, 0, 10);
				snprintf(extra->index, 10, "%s", "ISI");
			} else if (!strcasecmp(args->args[5], "BUI")) {
				memset(extra->index, 0, 10);
				snprintf(extra->index, 10, "%s", "BUI");
			} else if (!strcasecmp(args->args[5], "DSR")) {
				memset(extra->index, 0, 10);
				snprintf(extra->index, 10, "%s", "DSR");
			} else if (!strcasecmp(args->args[5], "FWI")) {
				memset(extra->index, 0, 10);
				snprintf(extra->index, 10, "%s", "FWI");
			}
			if (args->arg_count >= 7) {
				extra->ffmc0 = *((double *) args->args[6]);
				if (args->arg_count >= 8) {
					extra->dmc0 = *((double *) args->args[7]);
					if (args->arg_count == 9) {
						extra->dc0 = *((double *) args->args[8]);
					}
				}
			}
		}
		if (oph_fdi_configuration_setup(&(extra->conf))) {
			pmesg(1, __FILE__, __LINE__, "Error initializing index configuration\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		param->extend = extra;
	} else
		extra = param->extend;

	double rain, temperature, humidity, wind;
	double res;
	int i;
	extra->prev_ffmc = extra->ffmc0;
	extra->prev_dmc = extra->dmc0;
	extra->prev_dc = extra->dc0;
	extra->date = extra->date0;

	switch (measure->type[0]) {
		case OPH_INT:
			if (!strcasecmp(extra->index, "FFMC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DMC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "ISI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_isi(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "BUI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_bui
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, extra->prev_dc,
					     &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DSR")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dsr
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, wind, extra->prev_ffmc, extra->prev_dmc,
					     extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "FWI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((int *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_main
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, wind, extra->prev_ffmc, extra->prev_dmc,
					     extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else {
				pmesg(1, __FILE__, __LINE__, "Invalid index\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			break;
		case OPH_SHORT:
			if (!strcasecmp(extra->index, "FFMC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DMC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "ISI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_isi(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "BUI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_bui
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, extra->prev_dc,
					     &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DSR")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dsr
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, wind, extra->prev_ffmc, extra->prev_dmc,
					     extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "FWI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((short *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_main
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, wind, extra->prev_ffmc, extra->prev_dmc,
					     extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else {
				pmesg(1, __FILE__, __LINE__, "Invalid index\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			break;
		case OPH_BYTE:
			if (!strcasecmp(extra->index, "FFMC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DMC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "ISI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_isi(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "BUI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_bui
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, extra->prev_dc,
					     &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DSR")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dsr
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, wind, extra->prev_ffmc, extra->prev_dmc,
					     extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "FWI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((char *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_main
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, wind, extra->prev_ffmc, extra->prev_dmc,
					     extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else {
				pmesg(1, __FILE__, __LINE__, "Invalid index\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			break;
		case OPH_LONG:
			if (!strcasecmp(extra->index, "FFMC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DMC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "ISI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_isi(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "BUI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_bui
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, extra->prev_dc,
					     &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DSR")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dsr
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, wind, extra->prev_ffmc, extra->prev_dmc,
					     extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "FWI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((long long *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_main
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, wind, extra->prev_ffmc, extra->prev_dmc,
					     extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else {
				pmesg(1, __FILE__, __LINE__, "Invalid index\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			break;
		case OPH_FLOAT:
			if (!strcasecmp(extra->index, "FFMC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DMC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "ISI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_isi(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "BUI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_bui
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, extra->prev_dc,
					     &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DSR")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dsr
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, wind, extra->prev_ffmc, extra->prev_dmc,
					     extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "FWI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (double) (((float *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_main
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, wind, extra->prev_ffmc, extra->prev_dmc,
					     extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else {
				pmesg(1, __FILE__, __LINE__, "Invalid index\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			break;
		case OPH_DOUBLE:
			if (!strcasecmp(extra->index, "FFMC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DMC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DC")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "ISI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_isi(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "BUI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_bui
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, extra->prev_dc,
					     &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "DSR")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_dsr
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, wind, extra->prev_ffmc, extra->prev_dmc,
					     extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "FWI")) {
				for (i = 0; i < measure->numelem; i++) {
					rain = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 0]);
					temperature = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 1]);
					humidity = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 2]);
					wind = (((double *) measure->content)[(i * OPH_FDI_FWI_NUM_MEASURE) + 3]);
					if (oph_fdi_fwi_main
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, wind, extra->prev_ffmc, extra->prev_dmc,
					     extra->prev_dc, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_ffmc(rain, temperature, humidity, wind, extra->prev_ffmc, &(extra->conf), &(extra->prev_ffmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute FFMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dmc
					    (extra->date.tm_mday, extra->date.tm_mon + 1, extra->date.tm_year + 1900, extra->lat, rain, temperature, humidity, extra->prev_dmc, &(extra->conf),
					     &(extra->prev_dmc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DMC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (oph_fdi_fwi_dc(extra->date.tm_mon + 1, rain, temperature, extra->prev_dc, &(extra->conf), &(extra->prev_dc))) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute DC\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_ffmc), output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dmc), output->content + i * output->blocksize + core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&(extra->prev_dc), output->content + i * output->blocksize + 2 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize + 3 * core_sizeof(output->type[0]), OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					extra->date.tm_mday++;
					if (mktime(&(extra->date)) == -1) {
						pmesg(1, __FILE__, __LINE__, "Error: unable to make time using mktime\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else {
				pmesg(1, __FILE__, __LINE__, "Invalid index\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			break;
		default:
			pmesg(1, __FILE__, __LINE__, "Invalid type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
	}

	*length = output->length;
	*error = 0;
	*is_null = 0;

	return (result = output->content);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
