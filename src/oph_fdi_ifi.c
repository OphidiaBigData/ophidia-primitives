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

#include "oph_fdi_ifi.h"

#define OPH_FDI_IFI_NUM_MEASURE 8

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_fdi_ifi_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if (args->arg_count < 3 || args->arg_count > 4) {
		strcpy(message, "ERROR: Wrong arguments! oph_fdi_ifi(input_OPH_TYPE, output_OPH_TYPE, measure, [INDEX])");
		return 1;
	}

	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_fdi_ifi function");
			return 1;
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;
	return 0;
}

void oph_fdi_ifi_deinit(UDF_INIT * initid)
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

char *oph_fdi_ifi(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
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

		initid->ptr = (char *)param;
	} else
		param = (oph_generic_param_multi *)initid->ptr;

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
//              if (measure->num_measure != OPH_FDI_IFI_NUM_MEASURE)
//              {
//                      param->error = 1;
//                      pmesg(1, __FILE__, __LINE__, "Wrong number of input measure\n");
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
		if (measure->type[0] != OPH_INT && measure->type[0] != OPH_LONG && measure->type[0] != OPH_FLOAT && measure->type[0] != OPH_DOUBLE) {
			pmesg(1, __FILE__, __LINE__, "Invalid type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		measure->length = args->lengths[2];
		measure->blocksize *= 8;	//TODO
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
		if (output->num_measure != 1) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Wrong number of output data type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		output->numelem = measure->numelem;
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
			pmesg(1, __FILE__, __LINE__, "Error allocating measures string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		param->result = output;
	} else
		output = param->result;

	oph_fdi_ifi_conf *extra;
	if (!param->extend) {
		extra = (oph_fdi_ifi_conf *) malloc(sizeof(oph_fdi_ifi_conf));
		snprintf(extra->index, 10, "%s", "IFI");	//default
		if (args->arg_count > 3) {
			if (!strcasecmp(args->args[3], "DC")) {
				memset(extra->index, 0, 10);
				snprintf(extra->index, 10, "%s", "DC");
			} else if (!strcasecmp(args->args[3], "MC")) {
				memset(extra->index, 0, 10);
				snprintf(extra->index, 10, "%s", "MC");
			} else if (!strcasecmp(args->args[3], "R")) {
				memset(extra->index, 0, 10);
				snprintf(extra->index, 10, "%s", "R");
			} else if (!strcasecmp(args->args[3], "FC")) {
				memset(extra->index, 0, 10);
				snprintf(extra->index, 10, "%s", "FC");
			} else if (!strcasecmp(args->args[3], "IFI")) {
				memset(extra->index, 0, 10);
				snprintf(extra->index, 10, "%s", "IFI");
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

	double radiation_mean, radiation_max;
	double temperature_mean, temperature_max;
	double total_precipitation, wind_speed_mean;
	double relative_humidity_mean, relative_humidity_min;
	double res;
	int i;

	switch (output->type[0]) {
		case OPH_INT:
			if (!strcasecmp(extra->index, "DC")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_mean = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 0]);
					temperature_mean = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					total_precipitation = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 4]);
					if (oph_fdi_ifi_dc(radiation_mean, temperature_mean, total_precipitation, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "MC")) {
				for (i = 0; i < measure->numelem; i++) {
					temperature_max = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 3]);
					wind_speed_mean = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 5]);
					relative_humidity_min = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 7]);
					if (oph_fdi_ifi_mc(temperature_max, wind_speed_mean, relative_humidity_min, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "R")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_max = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 1]);
					if (oph_fdi_ifi_r(radiation_max, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "FC")) {
				for (i = 0; i < measure->numelem; i++) {
					temperature_mean = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					relative_humidity_mean = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 6]);
					if (oph_fdi_ifi_fc(temperature_mean, relative_humidity_mean, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "IFI")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_mean = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 0]);
					radiation_max = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 1]);
					temperature_mean = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					temperature_max = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 3]);
					total_precipitation = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 4]);
					wind_speed_mean = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 5]);
					relative_humidity_mean = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 6]);
					relative_humidity_min = (double) (((int *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 7]);
					if (oph_fdi_ifi_main
					    (radiation_mean, radiation_max, temperature_mean, temperature_max, total_precipitation, wind_speed_mean, relative_humidity_mean, relative_humidity_min,
					     &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
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
			if (!strcasecmp(extra->index, "DC")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_mean = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 0]);
					temperature_mean = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					total_precipitation = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 4]);
					if (oph_fdi_ifi_dc(radiation_mean, temperature_mean, total_precipitation, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "MC")) {
				for (i = 0; i < measure->numelem; i++) {
					temperature_max = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 3]);
					wind_speed_mean = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 5]);
					relative_humidity_min = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 7]);
					if (oph_fdi_ifi_mc(temperature_max, wind_speed_mean, relative_humidity_min, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "R")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_max = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 1]);
					if (oph_fdi_ifi_r(radiation_max, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "FC")) {
				for (i = 0; i < measure->numelem; i++) {
					temperature_mean = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					relative_humidity_mean = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 6]);
					if (oph_fdi_ifi_fc(temperature_mean, relative_humidity_mean, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "IFI")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_mean = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 0]);
					radiation_max = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 1]);
					temperature_mean = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					temperature_max = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 3]);
					total_precipitation = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 4]);
					wind_speed_mean = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 5]);
					relative_humidity_mean = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 6]);
					relative_humidity_min = (double) (((short *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 7]);
					if (oph_fdi_ifi_main
					    (radiation_mean, radiation_max, temperature_mean, temperature_max, total_precipitation, wind_speed_mean, relative_humidity_mean, relative_humidity_min,
					     &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
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
			if (!strcasecmp(extra->index, "DC")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_mean = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 0]);
					temperature_mean = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					total_precipitation = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 4]);
					if (oph_fdi_ifi_dc(radiation_mean, temperature_mean, total_precipitation, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "MC")) {
				for (i = 0; i < measure->numelem; i++) {
					temperature_max = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 3]);
					wind_speed_mean = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 5]);
					relative_humidity_min = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 7]);
					if (oph_fdi_ifi_mc(temperature_max, wind_speed_mean, relative_humidity_min, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "R")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_max = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 1]);
					if (oph_fdi_ifi_r(radiation_max, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "FC")) {
				for (i = 0; i < measure->numelem; i++) {
					temperature_mean = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					relative_humidity_mean = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 6]);
					if (oph_fdi_ifi_fc(temperature_mean, relative_humidity_mean, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "IFI")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_mean = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 0]);
					radiation_max = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 1]);
					temperature_mean = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					temperature_max = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 3]);
					total_precipitation = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 4]);
					wind_speed_mean = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 5]);
					relative_humidity_mean = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 6]);
					relative_humidity_min = (double) (((char *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 7]);
					if (oph_fdi_ifi_main
					    (radiation_mean, radiation_max, temperature_mean, temperature_max, total_precipitation, wind_speed_mean, relative_humidity_mean, relative_humidity_min,
					     &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
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
			if (!strcasecmp(extra->index, "DC")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_mean = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 0]);
					temperature_mean = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					total_precipitation = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 4]);
					if (oph_fdi_ifi_dc(radiation_mean, temperature_mean, total_precipitation, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "MC")) {
				for (i = 0; i < measure->numelem; i++) {
					temperature_max = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 3]);
					wind_speed_mean = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 5]);
					relative_humidity_min = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 7]);
					if (oph_fdi_ifi_mc(temperature_max, wind_speed_mean, relative_humidity_min, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "R")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_max = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 1]);
					if (oph_fdi_ifi_r(radiation_max, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "FC")) {
				for (i = 0; i < measure->numelem; i++) {
					temperature_mean = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					relative_humidity_mean = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 6]);
					if (oph_fdi_ifi_fc(temperature_mean, relative_humidity_mean, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "IFI")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_mean = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 0]);
					radiation_max = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 1]);
					temperature_mean = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					temperature_max = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 3]);
					total_precipitation = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 4]);
					wind_speed_mean = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 5]);
					relative_humidity_mean = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 6]);
					relative_humidity_min = (double) (((long long *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 7]);
					if (oph_fdi_ifi_main
					    (radiation_mean, radiation_max, temperature_mean, temperature_max, total_precipitation, wind_speed_mean, relative_humidity_mean, relative_humidity_min,
					     &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
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
			if (!strcasecmp(extra->index, "DC")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_mean = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 0]);
					temperature_mean = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					total_precipitation = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 4]);
					if (oph_fdi_ifi_dc(radiation_mean, temperature_mean, total_precipitation, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "MC")) {
				for (i = 0; i < measure->numelem; i++) {
					temperature_max = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 3]);
					wind_speed_mean = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 5]);
					relative_humidity_min = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 7]);
					if (oph_fdi_ifi_mc(temperature_max, wind_speed_mean, relative_humidity_min, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "R")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_max = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 1]);
					if (oph_fdi_ifi_r(radiation_max, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "FC")) {
				for (i = 0; i < measure->numelem; i++) {
					temperature_mean = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					relative_humidity_mean = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 6]);
					if (oph_fdi_ifi_fc(temperature_mean, relative_humidity_mean, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "IFI")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_mean = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 0]);
					radiation_max = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 1]);
					temperature_mean = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					temperature_max = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 3]);
					total_precipitation = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 4]);
					wind_speed_mean = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 5]);
					relative_humidity_mean = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 6]);
					relative_humidity_min = (double) (((float *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 7]);
					if (oph_fdi_ifi_main
					    (radiation_mean, radiation_max, temperature_mean, temperature_max, total_precipitation, wind_speed_mean, relative_humidity_mean, relative_humidity_min,
					     &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
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
			if (!strcasecmp(extra->index, "DC")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_mean = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 0]);
					temperature_mean = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					total_precipitation = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 4]);
					if (oph_fdi_ifi_dc(radiation_mean, temperature_mean, total_precipitation, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "MC")) {
				for (i = 0; i < measure->numelem; i++) {
					temperature_max = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 3]);
					wind_speed_mean = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 5]);
					relative_humidity_min = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 7]);
					if (oph_fdi_ifi_mc(temperature_max, wind_speed_mean, relative_humidity_min, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "R")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_max = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 1]);
					if (oph_fdi_ifi_r(radiation_max, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "FC")) {
				for (i = 0; i < measure->numelem; i++) {
					temperature_mean = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					relative_humidity_mean = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 6]);
					if (oph_fdi_ifi_fc(temperature_mean, relative_humidity_mean, &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else if (!strcasecmp(extra->index, "IFI")) {
				for (i = 0; i < measure->numelem; i++) {
					radiation_mean = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 0]);
					radiation_max = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 1]);
					temperature_mean = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 2]);
					temperature_max = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 3]);
					total_precipitation = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 4]);
					wind_speed_mean = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 5]);
					relative_humidity_mean = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 6]);
					relative_humidity_min = (((double *) measure->content)[(i * OPH_FDI_IFI_NUM_MEASURE) + 7]);
					if (oph_fdi_ifi_main
					    (radiation_mean, radiation_max, temperature_mean, temperature_max, total_precipitation, wind_speed_mean, relative_humidity_mean, relative_humidity_min,
					     &(extra->conf), &res)) {
						pmesg(1, __FILE__, __LINE__, "Unable to compute index\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (core_oph_type_cast(&res, output->content + i * output->blocksize, OPH_DOUBLE, output->type[0], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Unable to covert data type\n");
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
