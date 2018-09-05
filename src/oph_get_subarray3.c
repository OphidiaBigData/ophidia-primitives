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

#include "oph_get_subarray3.h"

int msglevel = 1;

int core_oph_get_subarray3_set_flags(oph_subset ** subset, char *flags, unsigned long size, unsigned long *total, oph_get_subarray3_block * blocks, int number)
{
	if (!subset || !flags || !size || !total || !blocks) {
		pmesg(1, __FILE__, __LINE__, "Null pointer\n");
		return OPH_SUBSET_NULL_POINTER_ERR;
	}

	unsigned long i, j, k;
	*total = 0;
	for (j = 0; j < size; ++j) {
		for (k = 0; k < number; ++k) {
			for (i = 0; i < subset[k]->number; ++i)
				if (oph_subset_is_in_subset(oph_subset_id_to_index2(j, blocks[k].block_size, blocks[k].max), subset[k]->start[i], subset[k]->stride[i], subset[k]->end[i]))
					break;
			if (!(flags[j] = i < subset[k]->number))
				break;	// Element skipped
		}
		if (flags[j])
			(*total)++;
	}
	return OPH_SUBSET_OK;
}

int core_oph_get_subarray3(oph_get_subarray3_param * param)
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
my_bool oph_get_subarray3_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	if (!args->arg_count || (args->arg_count % 3)) {
		strcpy(message, "ERROR: Wrong arguments! oph_get_subarray3(input_OPH_TYPE, output_OPH_TYPE, measure, [subset_string1, blocksize1, size1], [subset_string2, blocksize2, size2], ...)");
		return 1;
	}

	int i;
	for (i = 0; i < 3; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_subarray3 function");
			return 1;
		}
	}
	for (i = 3; i < args->arg_count;) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_get_subarray3 function");
			return 1;
		}
		i++;
		if (args->arg_type[i] != INT_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_get_subarray3 function");
			return 1;
		}
		i++;
		if (args->arg_type[i] != INT_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_get_subarray3 function");
			return 1;
		}
		i++;
	}

	initid->ptr = NULL;

	return 0;
}

void oph_get_subarray3_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		oph_get_subarray3_param *param = (oph_get_subarray3_param *) initid->ptr;
		if (param->measure) {
			free(param->measure);
			param->measure = NULL;
		}
		if (param->subset) {
			oph_subset_vector_free(param->subset, param->number);
			param->subset = NULL;
			param->number = 0;
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

char *oph_get_subarray3(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	int res = 0, i;
	oph_get_subarray3_param *param;
	oph_string *measure;
	unsigned long total;
	oph_get_subarray3_block *blocks = 0;

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
		initid->ptr = (char *) malloc(sizeof(oph_get_subarray3_param));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_get_subarray3_param\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		param = (oph_get_subarray3_param *) initid->ptr;
		param->measure = NULL;
		param->subset = NULL;
		param->number = 0;
		param->flags = NULL;
		param->result = NULL;
		param->error = 0;
	} else
		param = (oph_get_subarray3_param *) initid->ptr;

	if (param->error) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	if (!param->measure) {
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

		if (args->arg_count > 3) {
			param->number = (args->arg_count - 3) / 3;
			param->subset = (oph_subset **) malloc(param->number * sizeof(oph_subset *));
			blocks = (oph_get_subarray3_block *) malloc(param->number * sizeof(oph_get_subarray3_block));
			for (i = 0; i < param->number; ++i) {
				blocks[i].subset_string = args->args[i * 3 + 3];
				blocks[i].subset_strings_length = args->lengths[i * 3 + 3];
				blocks[i].block_size = *((long long *) args->args[i * 3 + 4]);
				blocks[i].max = *((long long *) args->args[i * 3 + 5]);
			}
		} else {
			param->number = 1;
			param->subset = (oph_subset **) malloc(sizeof(oph_subset *));
			blocks = (oph_get_subarray3_block *) malloc(sizeof(oph_get_subarray3_block));
			blocks[0].subset_string = 0;
			blocks[0].subset_strings_length = 0;
			blocks[0].block_size = 1;
			blocks[0].max = measure->numelem;
		}

		for (i = 0; i < param->number; ++i) {
			// Set subsetting structures
			if (oph_subset_init(&(param->subset[i]))) {
				pmesg(1, __FILE__, __LINE__, "Error in oph_subset initialization\n");
				param->error = 1;
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			if (oph_subset_parse
			    (blocks[i].subset_string ? blocks[i].subset_string : OPH_SUBSET_ALL, blocks[i].subset_strings_length ? blocks[i].subset_strings_length : strlen(OPH_SUBSET_ALL),
			     param->subset[i], blocks[i].max)) {
				pmesg(1, __FILE__, __LINE__, "Error in parsing subset_string\n");
				param->error = 1;
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
		}

		param->flags = malloc(measure->numelem * sizeof(char));
		if (!param->flags) {
			pmesg(1, __FILE__, __LINE__, "Error allocating flag array\n");
			param->error = 1;
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (core_oph_get_subarray3_set_flags(param->subset, param->flags, measure->numelem, &total, blocks, param->number)) {
			pmesg(1, __FILE__, __LINE__, "Error in setting flag array\n");
			param->error = 1;
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (blocks)
			free(blocks);

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

	res = core_oph_get_subarray3(param);
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
