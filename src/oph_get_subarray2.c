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

#include "oph_get_subarray2.h"

int msglevel = 1;

int core_oph_get_subarray2(oph_get_subarray2_param * param)
{
	oph_string *measure = (oph_string *) param->measure;
	unsigned long j, k = 0;

	for (j = 0; j < measure->numelem; ++j) {
		if (param->flags[j]) {
			if (core_oph_type_cast(measure->content + j * measure->elemsize, param->result + k * param->result_elemsize, measure->type, param->result_type, NULL)) {
				pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
				return OPH_SUBSET_SYSTEM_ERR;
			}
			k++;
		}
	}

	return OPH_SUBSET_OK;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_get_subarray2_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	if (args->arg_count < 4) {
		strcpy(message, "ERROR: Wrong arguments! oph_get_subarray2(input_OPH_TYPE, output_OPH_TYPE, measure, subset_string, [size1], [size2], ...)");
		return 1;
	}

	int i;
	for (i = 0; i < 4; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_subarray2 function");
			return 1;
		}
	}
	for (i = 4; i < args->arg_count; i++) {
		if (args->arg_type[i] != INT_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_subarray2 function");
			return 1;
		}
	}

	initid->ptr = NULL;

	return 0;
}

void oph_get_subarray2_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		oph_get_subarray2_param *param = (oph_get_subarray2_param *) initid->ptr;
		if (param->measure) {
			free(param->measure);
			param->measure = NULL;
		}
		if (param->subset) {
			oph_subset_free(param->subset);
			param->subset = NULL;
		}
		if (param->flags) {
			free(param->flags);
			param->flags = NULL;
		}
		if (param->result) {
			free(param->result);
			param->result = NULL;
		}
		free(initid->ptr);
		initid->ptr = NULL;
	}
}

char *oph_get_subarray2(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	int res = 0, number = 0, i;
	oph_get_subarray2_param *param;
	oph_string *measure;
	unsigned long total, max;
	unsigned long *sizes = 0;

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

	if (!initid->ptr) {
		initid->ptr = (char *) malloc(sizeof(oph_get_subarray2_param));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_get_subarray2_param\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		param = (oph_get_subarray2_param *) initid->ptr;
		param->measure = NULL;
		param->subset = NULL;
		param->flags = NULL;
		param->result = NULL;
		param->error = 0;
	} else
		param = (oph_get_subarray2_param *) initid->ptr;

	if (!param->error && !param->measure) {
		// Input
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

		// Set measure->length
		measure->length = &(args->lengths[2]);

		// Set measure->type
		res = core_set_type(measure, args->args[0], &(args->lengths[0]));
		if (res) {
			pmesg(1, __FILE__, __LINE__, "Error on setting the data type\n");
			param->error = 1;
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		// Set measure->elemsize
		if (core_set_elemsize(measure)) {
			pmesg(1, __FILE__, __LINE__, "Error on setting data element size\n");
			param->error = 1;
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		// Set measure->numelem
		if (core_set_numelem(measure)) {
			pmesg(1, __FILE__, __LINE__, "Error on counting elements\n");
			param->error = 1;
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		if (args->arg_count > 4) {
			max = *((long long *) args->args[args->arg_count - 1]);
			number = args->arg_count - 4;
			sizes = (unsigned long *) malloc(number * sizeof(unsigned long));
			for (i = 0; i < number; ++i)
				sizes[i] = *((long long *) args->args[i + 4]);
		} else
			max = measure->numelem;

		// Set subsetting structures
		if (oph_subset_init(&(param->subset))) {
			pmesg(1, __FILE__, __LINE__, "Error in oph_subset initialization\n");
			param->error = 1;
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (oph_subset_parse(args->args[3], args->lengths[3], param->subset, max)) {
			pmesg(1, __FILE__, __LINE__, "Error in parsing subset_string\n");
			param->error = 1;
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		// New implementation with flags
		param->flags = malloc(measure->numelem * sizeof(char));
		if (!param->flags) {
			pmesg(1, __FILE__, __LINE__, "Error allocating flag array\n");
			param->error = 1;
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (oph_subset_set_flags(param->subset, param->flags, measure->numelem, &total, sizes, number)) {
			pmesg(1, __FILE__, __LINE__, "Error in setting flag array\n");
			param->error = 1;
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (sizes)
			free(sizes);

		oph_string output_array;
		core_set_type(&output_array, args->args[1], &(args->lengths[1]));
		if (!output_array.type) {
			pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (core_set_elemsize(&output_array)) {
			pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		param->result_type = output_array.type;
		param->result_elemsize = output_array.elemsize;

		// Output
		param->length = total * param->result_elemsize;	// bytes
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

	res = core_oph_get_subarray2(param);
	if (res) {
		pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
		*length = 0;
		*is_null = 0;
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
