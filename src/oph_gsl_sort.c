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

#include "oph_gsl_sort.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_gsl_sort_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;

	if (args->arg_count != 3) {
		strcpy(message, "ERROR: Wrong arguments! oph_gsl_sort(input_OPH_TYPE, output_OPH_TYPE, measure)");
		return 1;
	}

	for (i = 0; i < 3; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_gsl_sort function");
			return 1;
		}
	}

	initid->ptr = NULL;

	return 0;
}

void oph_gsl_sort_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		if ((((oph_stringPtr) initid->ptr)[0]).content) {
			free((((oph_stringPtr) initid->ptr)[0]).content);
			(((oph_stringPtr) initid->ptr)[0]).content = NULL;
		}
		if ((((oph_stringPtr) initid->ptr)[0]).length) {
			free((((oph_stringPtr) initid->ptr)[0]).length);
			(((oph_stringPtr) initid->ptr)[0]).length = NULL;
		}
		if ((((oph_stringPtr) initid->ptr)[1]).length) {
			free((((oph_stringPtr) initid->ptr)[1]).length);
			(((oph_stringPtr) initid->ptr)[1]).length = NULL;
		}
		free(initid->ptr);
		initid->ptr = NULL;
	}
}

char *oph_gsl_sort(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
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

	oph_stringPtr output;
	oph_stringPtr measure;

	if (!initid->ptr) {
		initid->ptr = malloc(2 * sizeof(oph_string));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating ptr\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}
		output = &(((oph_stringPtr) initid->ptr)[0]);
		measure = &(((oph_stringPtr) initid->ptr)[1]);

		core_set_type(measure, args->args[0], &(args->lengths[0]));
		if (measure->type != OPH_BYTE && measure->type != OPH_SHORT && measure->type != OPH_INT && measure->type != OPH_LONG && measure->type != OPH_FLOAT && measure->type != OPH_DOUBLE) {
			pmesg(1, __FILE__, __LINE__, "Invalid input type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		core_set_type(output, args->args[1], &(args->lengths[1]));
		if (output->type != OPH_BYTE && output->type != OPH_SHORT && output->type != OPH_INT && output->type != OPH_LONG && output->type != OPH_FLOAT && output->type != OPH_DOUBLE) {
			pmesg(1, __FILE__, __LINE__, "Invalid output type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		measure->length = malloc(sizeof(unsigned long));
		if (!measure->length) {
			pmesg(1, __FILE__, __LINE__, "Error allocating length\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		output->length = malloc(sizeof(unsigned long));
		if (!output->length) {
			pmesg(1, __FILE__, __LINE__, "Error allocating length\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		*(measure->length) = args->lengths[2];

		if (core_set_elemsize(measure)) {
			pmesg(1, __FILE__, __LINE__, "Error on setting elements size\n");
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

		if (core_set_elemsize(output)) {
			pmesg(1, __FILE__, __LINE__, "Error on setting elements size\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		*(output->length) = measure->numelem * output->elemsize;
		if (core_set_numelem(output)) {
			pmesg(1, __FILE__, __LINE__, "Error on counting elements\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		output->content = (char *) calloc(1, *(output->length));	// output array
		if (!output->content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating output array\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	output = &(((oph_stringPtr) initid->ptr)[0]);
	measure = &(((oph_stringPtr) initid->ptr)[1]);

	measure->content = args->args[2];

	int i;
	for (i = 0; i < measure->numelem; i++) {
		if (core_oph_type_cast(measure->content + (i * measure->elemsize), output->content + (i * output->elemsize), measure->type, output->type, NULL)) {
			pmesg(1, __FILE__, __LINE__, "Error casting output\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	// Order data
	oph_gsl_sort_produce((void *) output->content, output->numelem, output->type);

	*length = *(output->length);
	*error = 0;
	*is_null = 0;

	return output->content;
}

void oph_gsl_sort_produce(void *inout_data, const size_t data_len, oph_type data_type)
{
	switch (data_type) {
		case OPH_INT:
			gsl_sort_int((int *) inout_data, 1, data_len);
			break;
		case OPH_SHORT:
			gsl_sort_short((short *) inout_data, 1, data_len);
			break;
		case OPH_BYTE:
			gsl_sort_char((char *) inout_data, 1, data_len);
			break;
		case OPH_LONG:
			gsl_sort_long((long *) inout_data, 1, data_len);
			break;
		case OPH_FLOAT:
			gsl_sort_float((float *) inout_data, 1, data_len);
			break;
		case OPH_DOUBLE:
			gsl_sort((double *) inout_data, 1, data_len);
			break;
		default:;
	}
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
