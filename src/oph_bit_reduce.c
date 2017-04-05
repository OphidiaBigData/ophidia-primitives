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

#include "oph_bit_reduce.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_bit_reduce_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	if (args->arg_count < 3 || args->arg_count > 4) {
		strcpy(message, "ERROR: Wrong arguments! oph_bit_reduce(input_OPH_TYPE, output_OPH_TYPE, bit_measure, [OPH_BIT_OP], [count])");
		return 1;
	}

	int i;
	for (i = 0; i < 4; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_bit_reduce function");
			return 1;
		}
	}

	if (args->arg_count > 4) {
		if (args->arg_type[4] != INT_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_bit_reduce function");
			return 1;
		}
	}

	initid->ptr = NULL;

	return 0;
}

void oph_bit_reduce_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		oph_bit_param_reduce *param = (oph_bit_param_reduce *) initid->ptr;
		if (param->measure) {
			free(param->measure);
			param->measure = NULL;
		}
		if (param->result) {
			free(param->result);
			param->result = NULL;
		}
		free(initid->ptr);
		initid->ptr = NULL;
	}
}

char *oph_bit_reduce(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	int res = 0;
	oph_bit_param_reduce *param;
	oph_bit_string *measure;

	if (!initid->ptr) {
		initid->ptr = (char *) malloc(sizeof(oph_bit_param_reduce));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_bit_param_reduce\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		param = (oph_bit_param_reduce *) initid->ptr;
		param->measure = NULL;
		param->result = NULL;
	} else
		param = (oph_bit_param_reduce *) initid->ptr;

	if (!param->measure) {
		measure = param->measure = (oph_bit_string *) malloc(sizeof(oph_bit_string));
		if (!param->measure) {
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_bit_string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		if (!args->lengths[2]) {
			*length = 0;
			*is_null = 1;
			*error = 0;
			return NULL;
		}
		measure->numelem = core_oph_bit_numelem(args->lengths[2]);	// Const

		if (args->arg_count < 4)
			res = core_oph_bit_set_op(&param->op, NULL, 0);	// Const
		else
			res = core_oph_bit_set_op(&param->op, args->args[3], &(args->lengths[3]));	// Const
		if (res) {
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		if (args->arg_count < 5)
			param->count = measure->numelem;	// Const
		else {
			param->count = *((long long *) args->args[4]);	// Const
			if ((param->count < 1) || (param->count > measure->numelem)) {
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
		}

		unsigned long numelem = measure->numelem / param->count;
		if (measure->numelem % param->count)
			numelem++;
		param->length = core_oph_bit_length(numelem);	// bytes

		param->result = (char *) malloc(param->length);	// Output
		if (!param->result) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	} else {
		measure = param->measure;
	}

	measure->content = args->args[2];	// Input
	if (!measure->content) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
	}

	res = core_oph_bit_reduce(param);
	if (res) {
		pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
		*length = 0;
		*is_null = 1;
		*error = 1;
		return NULL;
	}
	*length = param->length;
	*error = 0;
	*is_null = 0;
	return (result = param->result);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
