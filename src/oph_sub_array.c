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

#include "oph_sub_array.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_sub_array_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	if ((args->arg_count < 4) || (args->arg_count > 5)) {
		strcpy(message, "ERROR: Wrong arguments! oph_sub_array(input_OPH_TYPE, output_OPH_TYPE, measure_a, measure_b, [missingvalue])");
		return 1;
	}

	int i;
	for (i = 0; i < args->arg_count; i++) {
		if (i == 4) {
			if (args->args[i] && (args->arg_type[i] == STRING_RESULT)) {
				strcpy(message, "ERROR: Wrong argument 'missingvalue' to oph_abs_array function");
				return 1;
			}
			args->arg_type[i] = REAL_RESULT;
		} else if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_sub_array function");
			return 1;
		}
	}

	initid->ptr = NULL;

	return 0;
}

void oph_sub_array_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		free_oph_generic_param_multi((oph_generic_param_multi *) initid->ptr);
		initid->ptr = NULL;
	}
}

char *oph_sub_array(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	int i;

	if (*error) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	if (*is_null || !args->lengths[2] || !args->lengths[3]) {
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
		param->core_oph_oper = core_oph_sub_array_multi;

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
		if (core_set_oph_multistring2(&measure, args->args[0], &(args->lengths[0]), 2)) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		for (i = 0; i < 2; ++i) {
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
		if (measure[0].num_measure != measure[1].num_measure) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Number of input data types are different\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (measure[0].length != measure[1].length) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Lengths of input arrays are different\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		for (i = 0; i < measure->num_measure; ++i) {
			if (measure[0].type[i] != measure[1].type[i]) {
				param->error = 1;
				pmesg(1, __FILE__, __LINE__, "Data types of input arrays are different\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
		}

		param->measure = measure;
	} else
		measure = param->measure;

	for (i = 0; i < 2; ++i)
		measure[i].content = args->args[2 + i];

	double missingvalue;
	if ((args->arg_count > 4) && args->args[4]) {
		missingvalue = *((double *) (args->args[4]));
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

	if (!param->error && core_oph_oper_array_multi(param)) {
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
