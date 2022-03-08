/*
    Ophidia Primitives
    Copyright (C) 2012-2020 CMCC Foundation

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

#include "oph_operation_array.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_operation_array_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{

	if (args->arg_count < 6) {
		strcpy(message, "ERROR: Wrong arguments! oph_operation_array(input_OPH_TYPE, output_OPH_TYPE, measure..., oph_operation, missingvalue)");
		return 1;
	}

	if (args->args[args->arg_count - 1] && (args->arg_type[args->arg_count - 1] == STRING_RESULT)) {
		strcpy(message, "ERROR: Wrong argument 'missingvalue' to oph_operation_array function");
		return 1;
	}
	args->arg_type[args->arg_count - 1] = REAL_RESULT;

	int i;
	for (i = 0; i < args->arg_count - 1; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_operation_array function");
			return 1;
		}
	}

	initid->ptr = NULL;

	return 0;
}

void oph_operation_array_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		free_oph_generic_param_multi((oph_generic_param_multi *) initid->ptr);
		initid->ptr = NULL;
	}
}

char *oph_operation_array(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	int i, j, n = args->arg_count - 2;

	if (*error) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	for (i = 2; i < n; ++i) {
		if (*is_null || !args->lengths[i]) {
			*length = 0;
			*is_null = 1;
			*error = 0;
			return NULL;
		}
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
		param->n_measure = args->arg_count - 4;
		param->core_oph_oper = NULL;
		if (!strcasecmp(args->args[n], "oph_avg"))
			param->core_oph_oper2 = core_oph_avg_array_multicube;
		else if (!strcasecmp(args->args[n], "oph_sum"))
			param->core_oph_oper2 = core_oph_sum_array_multicube;
		else if (!strcasecmp(args->args[n], "oph_mul"))
			param->core_oph_oper2 = core_oph_mul_array_multicube;
		else if (!strcasecmp(args->args[n], "oph_max"))
			param->core_oph_oper2 = core_oph_max_array_multicube;
		else if (!strcasecmp(args->args[n], "oph_min"))
			param->core_oph_oper2 = core_oph_min_array_multicube;
		else if (!strcasecmp(args->args[n], "oph_arg_max"))
			param->core_oph_oper2 = core_oph_arg_max_array_multicube;
		else if (!strcasecmp(args->args[n], "oph_arg_min"))
			param->core_oph_oper2 = core_oph_arg_min_array_multicube;
		else {
			pmesg(1, __FILE__, __LINE__, "Unknown operation %s\n", args->args[n]);
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

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
		for (i = 0; i < param->n_measure; ++i) {
			measure[i].length = args->lengths[2 + i];
			if (!measure[i].blocksize || (measure[i].length % measure[i].blocksize) || (measure[i].islast != i)) {
				param->error = 1;
				pmesg(1, __FILE__, __LINE__, "Wrong input type or data corrupted\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			measure[i].numelem = measure[i].length / measure[i].blocksize;
		}

		for (i = 0; i < param->n_measure; ++i) {
			if (measure[0].num_measure != measure[i].num_measure) {
				param->error = 1;
				pmesg(1, __FILE__, __LINE__, "Number of input data types are different\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			if (measure[0].length != measure[i].length) {
				param->error = 1;
				pmesg(1, __FILE__, __LINE__, "Lengths of input arrays are different\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
		}
		for (i = 0; i < measure->num_measure; ++i) {
			for (j = 0; j < param->n_measure; ++j) {
				if (measure[0].type[i] != measure[j].type[i]) {
					param->error = 1;
					pmesg(1, __FILE__, __LINE__, "Data types of input arrays are different\n");
					*length = 0;
					*is_null = 0;
					*error = 1;
					return NULL;
				}
			}
		}
		param->measure = measure;
	} else
		measure = param->measure;

	for (i = 0; i < param->n_measure; ++i)
		measure[i].content = args->args[2 + i];


	double missingvalue;
	if (args->args[args->arg_count - 1]) {
		missingvalue = *((double *) (args->args[args->arg_count - 1]));
		measure->missingvalue = &missingvalue;
	} else
		measure->missingvalue = NULL;
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

	if (!param->error && core_oph_oper_array_multi2(param)) {
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
