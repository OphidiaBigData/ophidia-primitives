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

#include "oph_roll_up.h"

int msglevel = 1;

int core_oph_roll_up(oph_roll_up_param * param)
{
	if (param->rows < param->size) {
		oph_string *measure = (oph_string *) param->measure;
		unsigned long row_size = measure->numelem * param->result_elemsize;
		if (param->result_type != measure->type) {
			unsigned long i;
			char *input_ptr = measure->content, *output_ptr = param->result + param->rows * row_size;
			for (i = 0; i < measure->numelem; ++i, input_ptr += measure->elemsize, output_ptr += param->result_elemsize) {
				if (core_oph_type_cast(input_ptr, output_ptr, measure->type, param->result_type, NULL)) {
					pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
		} else
			memcpy(param->result + param->rows * row_size, measure->content, row_size);
		param->rows++;
	}
	return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_roll_up_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	if (args->arg_count != 4) {
		strcpy(message, "ERROR: Wrong arguments! oph_roll_up(input_OPH_TYPE, output_OPH_TYPE, measure, size)");
		return 1;
	}

	int i;
	for (i = 0; i < 3; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_shift function");
			return 1;
		}
	}

	if (args->arg_type[3] != INT_RESULT) {
		strcpy(message, "ERROR: Wrong arguments to oph_get_subarray function");
		return 1;
	}

	initid->ptr = NULL;

	return 0;
}

void oph_roll_up_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		oph_roll_up_param *param = (oph_roll_up_param *) initid->ptr;
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

void oph_roll_up_operator_reset(UDF_INIT * initid, UDF_ARGS * args, char *is_null, char *error)
{
	oph_roll_up_clear(initid, is_null, error);
	oph_roll_up_add(initid, args, is_null, error);
}

void oph_roll_up_clear(UDF_INIT * initid, char *is_null, char *error)
{
}

void oph_roll_up_add(UDF_INIT * initid, UDF_ARGS * args, char *is_null, char *error)
{
	int res = 0;
	oph_roll_up_param *param;
	oph_string *measure;
	long long size;

	if (!initid->ptr) {
		initid->ptr = (char *) malloc(sizeof(oph_roll_up_param));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_roll_up_param\n");
			*is_null = 0;
			*error = 1;
			return;
		}
		param = (oph_roll_up_param *) initid->ptr;
		param->measure = NULL;
		param->result = NULL;
		param->rows = 0;
	} else
		param = (oph_roll_up_param *) initid->ptr;

	if (!param->measure) {
		measure = param->measure = (oph_string *) malloc(sizeof(oph_string));
		if (!param->measure) {
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_string\n");
			*is_null = 0;
			*error = 1;
			return;
		}
		measure->length = &(args->lengths[2]);

		if (core_set_type(measure, args->args[0], &(args->lengths[0]))) {
			pmesg(1, __FILE__, __LINE__, "Error on setting the data type\n");
			*is_null = 0;
			*error = 1;
			return;
		}

		if (core_set_elemsize(measure)) {
			pmesg(1, __FILE__, __LINE__, "Error on setting data element size\n");
			*is_null = 0;
			*error = 1;
			return;
		}

		if (core_set_numelem(measure)) {
			pmesg(1, __FILE__, __LINE__, "Error on counting elements\n");
			*is_null = 0;
			*error = 1;
			return;
		}
	} else
		measure = param->measure;

	if (!param->result) {
		if (!args->args[2] || !args->lengths[2]) {
			*is_null = 1;
			*error = 0;
			return;
		}

		size = *((long long *) args->args[3]);
		if (size <= 0) {
			*is_null = 1;
			*error = 0;
			return;
		}

		oph_string output_array;
		core_set_type(&output_array, args->args[1], &(args->lengths[1]));
		if (!output_array.type) {
			pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
			*is_null = 0;
			*error = 1;
			return;
		}
		if (core_set_elemsize(&output_array)) {
			pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
			*is_null = 0;
			*error = 1;
			return;
		}
		param->result_type = output_array.type;
		param->result_elemsize = output_array.elemsize;

		param->length = size * measure->numelem * output_array.elemsize;	// bytes
		if (!param->length) {
			*is_null = 1;
			*error = 0;
			return;
		}
		param->result = (char *) malloc(param->length);	// Output
		if (!param->result) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result\n");
			*is_null = 0;
			*error = 1;
			return;
		}

		param->rows = 0;
		param->size = size;
	}

	measure->content = args->args[2];	// Input
	if (!measure->content) {
		*is_null = 1;
		*error = 0;
		return;
	}
	measure->length = &(args->lengths[2]);

	res = core_oph_roll_up(param);
	if (res) {
		pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
		*is_null = 1;
		*error = 1;
		return;
	}
	*error = 0;
	*is_null = 0;
}

char *oph_roll_up(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	oph_roll_up_param *param;

	if (!initid->ptr) {
		pmesg(1, __FILE__, __LINE__, "Error checking the result\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	} else
		param = (oph_roll_up_param *) initid->ptr;

	*length = param->length;
	*error = 0;
	*is_null = 0;
	return (result = param->result);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
