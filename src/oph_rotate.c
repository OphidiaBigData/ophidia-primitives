/*
    Ophidia Primitives
    Copyright (C) 2012-2022 CMCC Foundation

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

#include "oph_rotate.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_rotate_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if (args->arg_count < 3 || args->arg_count > 4) {
		strcpy(message, "ERROR: Wrong arguments! oph_rotate(input_OPH_TYPE, output_OPH_TYPE, measure, [offset])");
		return 1;
	}

	for (i = 0; i < 3; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_rotate function");
			return 1;
		}
	}

	if (args->arg_count > 3) {
		if (args->arg_type[3] != INT_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_rotate function");
			return 1;
		}
	}

	initid->ptr = NULL;

	return 0;
}

void oph_rotate_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		oph_generic_param *param = (oph_generic_param *) initid->ptr;
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

char *oph_rotate(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	int res = 0;
	long long offset = 0;
	oph_generic_param *param;
	oph_string *measure;

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

	if (!initid->ptr) {
		initid->ptr = (char *) malloc(sizeof(oph_generic_param));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_generic_param\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		param = (oph_generic_param *) initid->ptr;
		param->measure = NULL;
		param->result = NULL;
		param->error = 0;
	} else
		param = (oph_generic_param *) initid->ptr;

	if (!param->error && !param->measure) {
		// Measure
		param->measure = malloc(sizeof(oph_string));
		if (!param->measure) {
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_string\n");
			param->error = 1;
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		measure = (oph_string *) param->measure;
		measure->length = &(args->lengths[2]);

		if (core_set_type(measure, args->args[0], &(args->lengths[0]))) {
			pmesg(1, __FILE__, __LINE__, "Error on setting the data type\n");
			param->error = 1;
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		if (core_set_elemsize(measure)) {
			pmesg(1, __FILE__, __LINE__, "Error on setting data element size\n");
			param->error = 1;
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		if (core_set_numelem(measure)) {
			pmesg(1, __FILE__, __LINE__, "Error on counting elements\n");
			param->error = 1;
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		oph_string output_array;
		core_set_type(&output_array, args->args[1], &(args->lengths[1]));
		if (!output_array.type) {
			pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
			param->error = 1;
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (core_set_elemsize(&output_array)) {
			pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
			param->error = 1;
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		param->result_type = output_array.type;
		param->result_elemsize = output_array.elemsize;

		param->length = output_array.elemsize * measure->numelem;	// bytes
		if (!param->length) {
			*length = 0;
			*is_null = 1;
			*error = 0;
			return NULL;
		}
		param->result = (char *) malloc(param->length);
		if (!param->result) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result\n");
			param->error = 1;
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	} else
		measure = (oph_string *) param->measure;

	// Set measure->content
	measure->content = args->args[2];	// Input
	if (!measure->content) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
	}

	if (args->arg_count > 3) {
		long long tmp = (long long) measure->numelem;
		offset = *((long long *) args->args[3]);
		if (offset >= tmp)
			offset %= tmp;
		else if (-offset >= tmp)
			offset = -((-offset) % tmp);
	}

	res = core_oph_shift(param, offset, 0, 1);
	if (res) {
		pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
		*length = 0;
		*is_null = 1;
		*error = 1;
		return NULL;
	}

	*length = param->length;
	*is_null = 0;
	*error = 0;
	return (result = param->result);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
