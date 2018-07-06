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

#include "oph_normalize.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_normalize_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if ((args->arg_count < 3) || (args->arg_count > 6)) {
		strcpy(message, "ERROR: Wrong arguments! oph_normalize(input_OPH_TYPE, output_OPH_TYPE, measure, [OPH_NORMALIZE_TYPE], [count], [missingvalue])");
		return 1;
	}

	for (i = 0; i < args->arg_count; i++) {
		if (i == 4) {
			if (args->arg_type[i] != INT_RESULT) {
				strcpy(message, "ERROR: Wrong argument 'count' to oph_normalize function");
				return 1;
			}
		} else if (i == 5) {
			if (args->args[i] && (args->arg_type[i] == STRING_RESULT)) {
				strcpy(message, "ERROR: Wrong argument 'missingvalue' to oph_normalize function");
				return 1;
			}
			args->arg_type[i] = REAL_RESULT;
		} else {
			if (args->arg_type[i] != STRING_RESULT) {
				strcpy(message, "ERROR: Wrong arguments to oph_normalize function");
				return 1;
			}
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;
	return 0;
}

void oph_normalize_deinit(UDF_INIT * initid)
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

char *oph_normalize(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	oph_multistring *multim;
	oph_multistring *output;

	long long block_count = 0, offset = 0;
	int i = 0, j;
	oph_oper myoper = DEFAULT_OPER;

	if (args->arg_count > 3)
		myoper = core_get_oper(args->args[3], &(args->lengths[3]));

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
		}

		if (!((oph_request_multi *) (initid->extension))->core_oph_oper) {
			pmesg(1, __FILE__, __LINE__, "Operator not recognized\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		if ((myoper != OPH_MAX) && (myoper != OPH_AVG) && (myoper != OPH_AVG_REL)) {
			pmesg(1, __FILE__, __LINE__, "Operator not supported\n");
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

	double missingvalue;
	if ((args->arg_count > 5) && args->args[5]) {
		missingvalue = *((double *) (args->args[5]));
		multim->missingvalue = &missingvalue;
	} else
		multim->missingvalue = NULL;

	offset = multim->numelem % block_count;
	unsigned long numelem = (multim->numelem) / block_count + (offset ? 1 : 0);
	output->numelem = multim->numelem;
	output->length = args->lengths[2];

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
	char *res_content = output->content, *pointer;
	double scalar;
	for (i = 1; i <= numelem; i++) {
		if (offset && (i == numelem))
			multim->numelem = offset;
		if (((oph_request_multi *) (initid->extension))->core_oph_oper(multim, output)) {
			pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		pointer = output->content;
		for (j = 0; j < output->num_measure; ++j) {
			switch (output->type[j]) {
				case OPH_DOUBLE:
					scalar = (*(double *) pointer);
					break;
				case OPH_FLOAT:
					scalar = (double) (*(float *) pointer);
					break;
				case OPH_INT:
					scalar = (double) (*(int *) pointer);
					break;
				case OPH_SHORT:
					scalar = (double) (*(short *) pointer);
					break;
				case OPH_BYTE:
					scalar = (double) (*(char *) pointer);
					break;
				case OPH_LONG:
					scalar = (double) (*(long long *) pointer);
					break;
				default:
					pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
					*length = 0;
					*is_null = 0;
					*error = 1;
					return NULL;
			}
			switch (myoper) {
				case OPH_MAX:
					core_oph_mul_scalar_multi(multim, 1.0 / scalar, output, j);
					break;
				case OPH_AVG:
					core_oph_sum_scalar_multi(multim, -scalar, output, j);
					break;
				case OPH_AVG_REL:
					core_oph_rel_scalar_multi(multim, scalar, output, j);
					break;
				default:
					pmesg(1, __FILE__, __LINE__, "Operator not supported\n");
					*length = 0;
					*is_null = 0;
					*error = 1;
					return NULL;
			}
			pointer += output->elemsize[j];
		}
		multim->content = (multim->content) + ((multim->blocksize) * (block_count));
		output->content = (output->content) + ((output->blocksize) * (block_count));
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
