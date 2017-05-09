/*
    Ophidia Primitives
    Copyright (C) 2012-2017 CMCC Foundation

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

#include "oph_reduce.h"

#ifdef GSL_SUPPORTED
#include <gsl/gsl_sort.h>
#endif

int msglevel = 1;

int core_oph_quantile_multi(oph_multistring * byte_array, oph_multistring * result)
{
#ifdef GSL_SUPPORTED
	if (!byte_array || (byte_array->param < 0.0) || (byte_array->param > 1.0)) {
		pmesg(1, __FILE__, __LINE__, "Wrong parameter\n");
		return -1;
	}
	int i, j, k;
	double sum, delta;
	char *in_string = byte_array->content, *current, *out_string = result->content;
	for (j = 0; j < byte_array->num_measure; j++) {
		current = in_string;
		switch (byte_array->type[j]) {
			case OPH_DOUBLE:
				{
					double *d = byte_array->extend;
					for (i = 0; i < byte_array->numelem; i++, current += byte_array->blocksize, d++)
						memcpy(d, current, sizeof(double));
					gsl_sort((double *) byte_array->extend, 1, byte_array->numelem);
					delta = (byte_array->numelem - 1) * byte_array->param;
					k = floor(delta);
					delta = delta - k;
					sum = (1 - delta) * (*(((double *) byte_array->extend) + k)) + delta * (*(((double *) byte_array->extend) + k + 1));
					if (core_oph_type_cast((void *) (&sum), out_string, OPH_DOUBLE, result->type[j], byte_array->missingvalue))
						return -1;
					break;
				}
			case OPH_FLOAT:{
					float *d = byte_array->extend;
					for (i = 0; i < byte_array->numelem; i++, current += byte_array->blocksize, d++)
						memcpy(d, current, sizeof(float));
					gsl_sort_float((float *) byte_array->extend, 1, byte_array->numelem);
					delta = (byte_array->numelem - 1) * byte_array->param;
					k = floor(delta);
					delta = delta - k;
					sum = (1 - delta) * (*(((float *) byte_array->extend) + k)) + delta * (*(((float *) byte_array->extend) + k + 1));
					if (core_oph_type_cast((void *) (&sum), out_string, OPH_DOUBLE, result->type[j], byte_array->missingvalue))
						return -1;
					break;
				}
			case OPH_INT:{
					int *d = byte_array->extend;
					for (i = 0; i < byte_array->numelem; i++, current += byte_array->blocksize, d++)
						memcpy(d, current, sizeof(int));
					gsl_sort_int((int *) byte_array->extend, 1, byte_array->numelem);
					delta = (byte_array->numelem - 1) * byte_array->param;
					k = floor(delta);
					delta = delta - k;
					sum = (1 - delta) * (*(((int *) byte_array->extend) + k)) + delta * (*(((int *) byte_array->extend) + k + 1));
					if (core_oph_type_cast((void *) (&sum), out_string, OPH_DOUBLE, result->type[j], byte_array->missingvalue))
						return -1;
					break;
				}
			case OPH_SHORT:{
					short *d = byte_array->extend;
					for (i = 0; i < byte_array->numelem; i++, current += byte_array->blocksize, d++)
						memcpy(d, current, sizeof(int));
					gsl_sort_short((short *) byte_array->extend, 1, byte_array->numelem);
					delta = (byte_array->numelem - 1) * byte_array->param;
					k = floor(delta);
					delta = delta - k;
					sum = (1 - delta) * (*(((short *) byte_array->extend) + k)) + delta * (*(((short *) byte_array->extend) + k + 1));
					if (core_oph_type_cast((void *) (&sum), out_string, OPH_DOUBLE, result->type[j], byte_array->missingvalue))
						return -1;
					break;
				}
			case OPH_BYTE:{
					char *d = byte_array->extend;
					for (i = 0; i < byte_array->numelem; i++, current += byte_array->blocksize, d++)
						memcpy(d, current, sizeof(int));
					gsl_sort_char((char *) byte_array->extend, 1, byte_array->numelem);
					delta = (byte_array->numelem - 1) * byte_array->param;
					k = floor(delta);
					delta = delta - k;
					sum = (1 - delta) * (*(((char *) byte_array->extend) + k)) + delta * (*(((char *) byte_array->extend) + k + 1));
					if (core_oph_type_cast((void *) (&sum), out_string, OPH_DOUBLE, result->type[j], byte_array->missingvalue))
						return -1;
					break;
				}
			case OPH_LONG:{
					long long *d = byte_array->extend;
					for (i = 0; i < byte_array->numelem; i++, current += byte_array->blocksize, d++)
						memcpy(d, current, sizeof(long long));
					gsl_sort_long((long *) byte_array->extend, 1, byte_array->numelem);
					delta = (byte_array->numelem - 1) * byte_array->param;
					k = floor(delta);
					delta = delta - k;
					sum = (1 - delta) * (*(((long long *) byte_array->extend) + k)) + delta * (*(((long long *) byte_array->extend) + k + 1));
					if (core_oph_type_cast((void *) (&sum), out_string, OPH_DOUBLE, result->type[j], byte_array->missingvalue))
						return -1;
					break;
				}
			default:
				pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
				return -1;
		}
		in_string += byte_array->elemsize[j];
		out_string += result->elemsize[j];
	}
	return 0;
#else
	pmesg(1, __FILE__, __LINE__, "Operation not allowed\n");
	return -1;
#endif
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_reduce_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if ((args->arg_count < 3) || (args->arg_count > 7)) {
		strcpy(message, "ERROR: Wrong arguments! oph_reduce(input_OPH_TYPE, output_OPH_TYPE, measure, [OPH_REDUCE_OPERATOR], [count], [order], [missingvalue])");
		return 1;
	}

	for (i = 0; i < args->arg_count; i++) {
		if (i == 4) {
			if (args->arg_type[i] != INT_RESULT) {
				strcpy(message, "ERROR: Wrong argument 'count' to oph_reduce function");
				return 1;
			}
		} else if (i == 5) {
			if (args->arg_type[i] == STRING_RESULT) {
				strcpy(message, "ERROR: Wrong argument 'order' to oph_reduce function");
				return 1;
			}
			args->arg_type[i] = REAL_RESULT;
		} else if (i == 6) {
			if (args->args[i] && (args->arg_type[i] == STRING_RESULT)) {
				strcpy(message, "ERROR: Wrong argument 'missingvalue' to oph_reduce function");
				return 1;
			}
			args->arg_type[i] = REAL_RESULT;
		} else {
			if (args->arg_type[i] != STRING_RESULT) {
				strcpy(message, "ERROR: Wrong arguments to oph_reduce function");
				return 1;
			}
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;
	return 0;
}

void oph_reduce_deinit(UDF_INIT * initid)
{
	oph_multistring *multimeasure;
	//Free allocated space
	if (initid->ptr) {
		multimeasure = (oph_multistring *) (initid->ptr);
		if (multimeasure->content)
			free(multimeasure->content);
		multimeasure->content = NULL;
		free_oph_multistring(multimeasure);
		multimeasure = NULL;
		initid->ptr = NULL;
	}
	if (initid->extension) {
		if (((oph_request_multi *) (initid->extension))->measure) {
			multimeasure = (oph_multistring *) (((oph_request_multi *) (initid->extension))->measure);
			if (multimeasure)
				free_oph_multistring(multimeasure);
			multimeasure = NULL;
			((oph_request_multi *) (initid->extension))->measure = NULL;
		}
		free(initid->extension);
		initid->extension = NULL;
	}
}

char *oph_reduce(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	oph_multistring *multim;
	oph_multistring *output;	//I can use a multistring to store information about the output types

	long long block_count = 0, offset = 0;
	int i = 0;
	oph_oper myoper = DEFAULT_OPER;

	if (!initid->extension) {
		initid->extension = (oph_request_multi *) calloc(1, sizeof(oph_request_multi));
		if (!initid->extension) {
			pmesg(1, __FILE__, __LINE__, "Memory allocation failed\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (core_set_oph_multistring(((oph_multistring **) (&(((oph_request_multi *) (initid->extension))->measure))), args->args[0], &(args->lengths[0]))) {
			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		((oph_request_multi *) (initid->extension))->core_oph_oper = NULL;
		if (args->arg_count < 4) {
			core_set_oper_multi(((oph_request_multi *) (initid->extension)), NULL, 0);
		} else {
			core_set_oper_multi(((oph_request_multi *) (initid->extension)), args->args[3], &(args->lengths[3]));
			myoper = core_get_oper(args->args[3], &(args->lengths[3]));
			if (myoper == OPH_QUANTILE)
				((oph_request_multi *) (initid->extension))->core_oph_oper = core_oph_quantile_multi;
		}

		if (!((oph_request_multi *) (initid->extension))->core_oph_oper) {
			pmesg(1, __FILE__, __LINE__, "Operator not recognized\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}
	//Single input/output measure array
	multim = (oph_multistring *) (&(((oph_request_multi *) (initid->extension))->measure[0]));
	multim->content = args->args[2];
	multim->length = args->lengths[2];
	//Check
	if (multim->length % multim->blocksize) {
		pmesg(1, __FILE__, __LINE__, "Wrong input type or data corrupted\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	multim->numelem = multim->length / multim->blocksize;
	if (!multim->extend && (myoper == OPH_QUANTILE)) {
		multim->extend = malloc(multim->length);
		if (!multim->extend) {
			pmesg(1, __FILE__, __LINE__, "Error allocating temporary string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	if (!initid->ptr) {
		if (core_set_oph_multistring(((oph_multistring **) (&(initid->ptr))), args->args[1], &(args->lengths[1]))) {
			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		//Check
		if (((oph_multistring *) (initid->ptr))->num_measure != (((oph_request_multi *) (initid->extension))->measure[0]).num_measure) {
			pmesg(1, __FILE__, __LINE__, "Wrong input or output type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}
	output = (oph_multistring *) ((initid->ptr));

	if (args->arg_count > 4)
		block_count = *((long long *) (args->args[4]));
	if (!block_count)
		block_count = multim->numelem;

	if (args->arg_count > 5) {
		multim->param = *((double *) (args->args[5]));
		if (multim->param < 0) {
			pmesg(1, __FILE__, __LINE__, "Wrong parameter value 'order' %f\n", multim->param);
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	} else
		multim->param = 2.0;

	double missingvalue;
	if ((args->arg_count > 6) && args->args[6]) {
		missingvalue = *((double *) (args->args[6]));
		multim->missingvalue = &missingvalue;
	} else
		multim->missingvalue = NULL;

	offset = multim->numelem % block_count;
	output->numelem = (multim->numelem) / block_count + (offset ? 1 : 0);
	output->length = (output->numelem) * (output->blocksize);

	/* Allocate the right space for the result set */
	if (!output->content) {
		output->content = (char *) calloc(1, output->length);
		if (!output->content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating measures string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	multim->numelem = block_count;
	char *res_content = output->content;
	for (i = 1; i <= output->numelem; i++) {
		if (offset && (i == output->numelem))
			multim->numelem = offset;
		if (((oph_request_multi *) (initid->extension))->core_oph_oper(multim, output)) {
			pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		multim->content = (multim->content) + ((multim->blocksize) * (block_count));
		output->content = (output->content) + (output->blocksize);
	}
	*length = output->length;
	*is_null = 0;
	*error = 0;

	output->content = res_content;
	return ((oph_multistring *) (initid->ptr))->content;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
