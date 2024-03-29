/*
    Ophidia Primitives
    Copyright (C) 2012-2023 CMCC Foundation

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

#include "oph_gsl_fit_linear.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/

int core_oph_gsl_fit_linear_multi(oph_multistring * byte_array, oph_multistring * result, oph_gsl_fit_linear_param * fit)
{
	int j, k;
	double tmp, *pointer;
	double cov00, cov01, cov11, sumsq;
	char *in_string = byte_array->content, *out_string = result->content;
	char *in_pointer, *out_pointer;
	for (j = 0; j < byte_array->num_measure; ++j) {
		in_pointer = in_string;
		switch (byte_array->type[j]) {
			case OPH_DOUBLE:{
					pointer = (double *) in_string;
					break;
				}
			case OPH_FLOAT:{
					pointer = fit->tmp;
					for (k = 0; k < byte_array->numelem; ++k) {
						fit->tmp[k] = *(float *) in_pointer;
						in_pointer += byte_array->blocksize;
					}
					break;
				}
			case OPH_INT:{
					pointer = fit->tmp;
					for (k = 0; k < byte_array->numelem; ++k) {
						fit->tmp[k] = *(int *) in_pointer;
						in_pointer += byte_array->blocksize;
					}
					break;
				}
			case OPH_SHORT:{
					pointer = fit->tmp;
					for (k = 0; k < byte_array->numelem; ++k) {
						fit->tmp[k] = *(short *) in_pointer;
						in_pointer += byte_array->blocksize;
					}
					break;
				}
			case OPH_BYTE:{
					pointer = fit->tmp;
					for (k = 0; k < byte_array->numelem; ++k) {
						fit->tmp[k] = *(char *) in_pointer;
						in_pointer += byte_array->blocksize;
					}
					break;
				}
			case OPH_LONG:{
					pointer = fit->tmp;
					for (k = 0; k < byte_array->numelem; ++k) {
						fit->tmp[k] = *(long long *) in_pointer;
						in_pointer += byte_array->blocksize;
					}
					break;
				}
			default:
				pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
				return -1;
		}
		if (gsl_fit_linear(fit->old_x, 1, pointer, 1, fit->n, &fit->c[0], &fit->c[1], &cov00, &cov01, &cov11, &sumsq))
			return -1;
		out_pointer = out_string;
		for (k = 0; k < result->numelem; ++k) {
			tmp = fit->c[0] + fit->new_x[k] * fit->c[1];
			if (core_oph_type_cast(&tmp, out_pointer, OPH_DOUBLE, result->type[j], byte_array->missingvalue))
				return -1;
			out_pointer += result->blocksize;
		}
		in_string += byte_array->elemsize[j];
		out_string += result->elemsize[j];
	}
	return 0;
}

my_bool oph_gsl_fit_linear_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if (args->arg_count != 5) {
		strcpy(message, "ERROR: Wrong arguments! oph_gsl_fit_linear(input_OPH_TYPE, output_OPH_TYPE, measure, dimension, x)");
		return 1;
	}

	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_gsl_fit_linear function");
			return 1;
		}
	}

	initid->ptr = NULL;
	return 0;
}

void oph_gsl_fit_linear_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		oph_generic_param_multi *param = (oph_generic_param_multi *) initid->ptr;
		if (param->extend) {
			oph_gsl_fit_linear_param *fit = (oph_gsl_fit_linear_param *) param->extend;
			if (fit) {
				if (fit->tmp) {
					free(fit->tmp);
					fit->tmp = NULL;
				}
				free(fit);
			}
			param->extend = NULL;
		}
		free_oph_generic_param_multi(param);
		initid->ptr = NULL;
	}
}

char *oph_gsl_fit_linear(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	if (*error) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	if (*is_null || !args->lengths[2] || !args->lengths[3] || !args->lengths[4]) {
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
			pmesg(1, __FILE__, __LINE__, "Wrong number of input data type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		measure->length = args->lengths[2];
		if (measure->length % measure->blocksize) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Wrong input type or data corrupted\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		measure->numelem = measure->length / measure->blocksize;
		if (measure->numelem * sizeof(double) != args->lengths[3]) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Wrong input type or data corrupted\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

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
		if (output->num_measure == measure->num_measure) {
			output->length = args->lengths[4];
			if (output->length % sizeof(double)) {
				param->error = 1;
				pmesg(1, __FILE__, __LINE__, "Wrong input type or data corrupted\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			output->numelem = output->length / sizeof(double);
			output->length = output->numelem * output->blocksize;
		} else {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Wrong number of output data type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
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

	oph_gsl_fit_linear_param *fit;
	if (!param->extend) {
		fit = (oph_gsl_fit_linear_param *) malloc(sizeof(oph_gsl_fit_linear_param));
		if (!fit) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		fit->n = measure->numelem;
		fit->old_x = NULL;
		fit->tmp = (double *) malloc(measure->numelem * sizeof(double));	// Intermediate position of input arrays
		if (!fit->tmp) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		param->extend = (void *) fit;
	} else
		fit = (oph_gsl_fit_linear_param *) param->extend;

	fit->old_x = (double *) args->args[3];
	fit->new_x = (double *) args->args[4];

	if (!param->error && core_oph_gsl_fit_linear_multi(param->measure, param->result, fit)) {
		param->error = 1;
		pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
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
