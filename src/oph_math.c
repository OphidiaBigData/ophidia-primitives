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

#include "oph_math.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_math_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	/* oph_math(measure, OPH_MATH_OPERATOR, OPH_TYPE) */
	int i = 0;
	if (args->arg_count != 4) {
		strcpy(message, "ERROR: Wrong arguments! oph_math(input_OPH_TYPE, output_OPH_TYPE, measure, OPH_MATH_OPERATOR)");
		return 1;
	}

	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_operator function");
			return 1;
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;
	return 0;
}

void oph_math_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		if (((oph_stringPtr) (initid->ptr))->content) {
			free(((oph_stringPtr) (initid->ptr))->content);
			((oph_stringPtr) (initid->ptr))->content = NULL;
		}
		if (((oph_stringPtr) (initid->ptr))->length) {
			free(((oph_stringPtr) (initid->ptr))->length);
			((oph_stringPtr) (initid->ptr))->length = NULL;
		}
		free(initid->ptr);
		initid->ptr = NULL;
	}
	if (initid->extension) {
		free(initid->extension);
		initid->extension = NULL;
	}
}

char *oph_math(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	oph_stringPtr measure, output;
	oph_math_oper *operation;
	int res = 0;

	if (!initid->ptr) {
		initid->ptr = (char *) calloc(1, sizeof(oph_string));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating output\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		initid->extension = (void *) calloc(1, sizeof(oph_string) + sizeof(oph_math_oper));
		if (!initid->extension) {
			pmesg(1, __FILE__, __LINE__, "Error allocating extension\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		measure = (oph_stringPtr) (initid->extension);
		operation = (oph_math_oper *) (initid->extension + sizeof(oph_string));
		output = (oph_stringPtr) (initid->ptr);

		core_set_type(measure, args->args[0], &(args->lengths[0]));
		if (!measure->type) {
			pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		measure->content = NULL;
		measure->length = &(args->lengths[2]);

		if (core_set_math_oper(operation, args->args[3], &(args->lengths[3]))) {
			pmesg(1, __FILE__, __LINE__, "Unable to recognize math operation\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		if (core_set_elemsize(measure)) {
			pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		if (core_set_numelem(measure)) {
			pmesg(1, __FILE__, __LINE__, "Error on counting elements\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		core_set_type(output, args->args[1], &(args->lengths[1]));
		if (!output->type) {
			pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (core_set_elemsize(output)) {
			pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		output->length = (unsigned long *) malloc(sizeof(unsigned long));
		if (!output->length) {
			*length = 0;
			*is_null = 1;
			*error = 0;
			return NULL;
		}
		*output->length = measure->numelem * output->elemsize;
		if (!(*output->length)) {
			*length = 0;
			*is_null = 1;
			*error = 0;
			return NULL;
		}

		output->content = (char *) malloc(*output->length);
		if (!output->content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating output string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	measure = (oph_stringPtr) (initid->extension);
	operation = (oph_math_oper *) (initid->extension + sizeof(oph_string));
	output = (oph_stringPtr) (initid->ptr);

	measure->content = args->args[2];
	measure->missingvalue = NULL;

	res = core_oph_math(measure, operation, output);
	if (res) {
		pmesg(1, __FILE__, __LINE__, "Unable to compute results\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	*length = *output->length;
	return (result = output->content);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
