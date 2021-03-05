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

#include "oph_permute.h"

#define OPH_PERMUTE_SEPARATOR ","

int msglevel = 1;

int oph_permute_parse(const char *cond, unsigned long clen, unsigned int *permutation_indexes, unsigned int max)
{
	unsigned int i;
	char *result, temp[BUFF_LEN], flags[max], *savepointer;
	for (i = 0; i < max; ++i)
		flags[i] = 0;

	core_strncpy(temp, cond, &clen);
	result = strtok_r(temp, OPH_PERMUTE_SEPARATOR, &savepointer);
	i = 0;
	while (result && (i < max)) {
		permutation_indexes[i] = atol(result);
		if (!permutation_indexes[i] || (permutation_indexes[i] > max)) {
			pmesg(1, __FILE__, __LINE__, "Wrong index '%d' in permutation string\n", permutation_indexes[i]);
			return 1;
		}
		permutation_indexes[i]--;	// C-like indexing
		flags[permutation_indexes[i]] = 1;
		i++;
		result = strtok_r(NULL, OPH_PERMUTE_SEPARATOR, &savepointer);
	}
	if (result || (i < max)) {
		pmesg(1, __FILE__, __LINE__, "Wrong number of elements in permutation string or size parameters\n");
		return 2;
	}
	for (i = 0; i < max; ++i)
		if (!flags[i]) {
			pmesg(1, __FILE__, __LINE__, "Missed some dimensions in permutation string\n");
			return 3;
		}
	return 0;
}

int oph_permute_id_to_index(unsigned long id, unsigned long *index, unsigned long *sizes, int n)
{
	int i;
	for (i = 0; i < n; ++i) {
		index[i] = id % sizes[i];
		id = (id - index[i]) / sizes[i];
	}
	return 0;
}

int oph_permute_index_to_id(unsigned long *index, unsigned long *id, unsigned long *sizes, int n)
{
	int i;
	unsigned long last_size = 1;
	*id = 0;
	for (i = 0; i < n; ++i) {
		*id += index[i] * last_size;
		last_size *= sizes[i];
	}
	return 0;
}

// Permute
int core_oph_permute(oph_permute_param * param)
{
	unsigned long j;
	oph_string *measure = (oph_string *) param->measure;
	if (param->index) {
		for (j = 0; j < measure->numelem; ++j) {
			if (core_oph_type_cast(measure->content + j * measure->elemsize, param->result + param->index[j] * param->result_elemsize, measure->type, param->result_type, NULL)) {
				pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
				return 1;
			}
		}
	} else if (measure->type != param->result_type) {
		for (j = 0; j < measure->numelem; ++j) {
			if (core_oph_type_cast(measure->content + j * measure->elemsize, param->result + j * param->result_elemsize, measure->type, param->result_type, NULL)) {
				pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
				return 1;
			}
		}
	} else
		memcpy(param->result, measure->content, param->length);

	return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_permute_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i;
	if (args->arg_count < 4) {
		strcpy(message, "ERROR: Wrong arguments! oph_permute(input_OPH_TYPE, output_OPH_TYPE, measure, permutation_string, [size1], [size2], ...)");
		return 1;
	}

	for (i = 0; i < 4; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_permute function");
			return 1;
		}
	}

	if (args->arg_count > 4) {
		for (i = 4; i < args->arg_count; ++i) {
			if (args->arg_type[i] != INT_RESULT) {
				strcpy(message, "ERROR: Wrong arguments to oph_permute function");
				return 1;
			}
		}
	}

	initid->ptr = NULL;

	return 0;
}

void oph_permute_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		oph_permute_param *param = (oph_permute_param *) initid->ptr;
		if (param->measure) {
			free(param->measure);
			param->measure = NULL;
		}
		if (param->index) {
			free(param->index);
			param->index = NULL;
		}
		if (param->result) {
			free(param->result);
			param->result = NULL;
		}
		free(initid->ptr);
		initid->ptr = NULL;
	}
}

char *oph_permute(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	int res = 0, number = 1, i;
	oph_permute_param *param;
	oph_string *measure;

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
		initid->ptr = (char *) malloc(sizeof(oph_permute_param));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_permute_param\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		param = (oph_permute_param *) initid->ptr;
		param->measure = NULL;
		param->index = NULL;
		param->result = NULL;
		param->error = 0;

		initid->ptr = (char *) param;
	} else
		param = (oph_permute_param *) initid->ptr;

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

		if (args->arg_count > 5)	// At least 2 size should be given
		{
			number = args->arg_count - 4;

			unsigned long sizes[number];
			for (i = 0; i < number; ++i)
				sizes[i] = *((long long *) args->args[i + 4]);

			unsigned int permutation_indexes[number];

			// Read permutation indexes
			if (oph_permute_parse(args->args[3], args->lengths[3], permutation_indexes, number)) {
				pmesg(1, __FILE__, __LINE__, "Error in parsing permutation_string\n");
				param->error = 1;
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}

			int new_permutation = 0;
			for (i = 0; i < number; ++i)
				if (permutation_indexes[i] != i) {
					new_permutation = 1;
					break;
				}

			if (new_permutation) {
				param->index = (unsigned long *) malloc(measure->numelem * sizeof(unsigned long));
				if (!param->index) {
					pmesg(1, __FILE__, __LINE__, "Error in allocating index array\n");
					param->error = 1;
					*length = 0;
					*is_null = 0;
					*error = 1;
					return NULL;
				}

				unsigned long new_sizes[number];
				for (i = 0; i < number; ++i) {
					new_sizes[i] = sizes[number - 1 - permutation_indexes[number - 1 - i]];	// Order of sizes in input is reversed
				}

				// Setting new indexes
				unsigned long id;
				unsigned long index[number], new_index[number];
				for (id = 0; id < measure->numelem; ++id) {
					oph_permute_id_to_index(id, index, sizes, number);
					for (i = 0; i < number; ++i)
						new_index[i] = index[number - 1 - permutation_indexes[number - 1 - i]];	// Order of sizes in input is reversed
					oph_permute_index_to_id(new_index, &(param->index[id]), new_sizes, number);
					if (param->index[id] >= measure->numelem) {
						pmesg(1, __FILE__, __LINE__, "Error in evaluating new indexes\n");
						param->error = 1;
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
			} else
				pmesg(2, __FILE__, __LINE__, "Same permutation\n");
		} else
			pmesg(2, __FILE__, __LINE__, "No permutation\n");

		// Output
		param->length = measure->numelem * param->result_elemsize;	// bytes
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

	res = core_oph_permute(param);
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
