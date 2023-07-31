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

#include "oph_interlace2.h"

int msglevel = 1;

int core_oph_interlace2_multi(oph_generic_param_multi * param)
{
	int i, j, k, h;
	oph_multistring *measure = param->measure, *result = param->result;
	char *ic, *oc = result->content, output_format = measure->num_measure == result->num_measure;
	long long t, *list = (long long *) param->extend;
	int numelem = measure->numelem / list[0];
	for (j = 0; j < numelem; ++j)	// Loop on elements
	{
		k = 0;
		do		// Loop on input measures
		{
			measure = param->measure + k;
			ic = measure->content + j * measure->blocksize * list[k];
			for (t = 0; t < list[k]; ++t)	// Loop on ensables
				for (i = h = 0; i < measure->num_measure; ++i)	// Loop on data types
				{
					if (core_oph_type_cast(ic, oc, measure->type[i], result->type[output_format ? h % result->num_measure : h], NULL)) {
						pmesg(1, __FILE__, __LINE__, "Error in compute array\n");
						return 1;
					}
					ic += measure->elemsize[i];
					oc += result->elemsize[output_format ? h % result->num_measure : h];
					h++;
				}
			k++;
		}
		while (!measure->islast);
	}
	return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_interlace2_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	if (args->arg_count < 4) {
		strcpy(message, "ERROR: Wrong arguments! oph_interlace2(input_OPH_TYPE, output_OPH_TYPE, binary_count_list, measure, ...)");
		return 1;
	}

	int i;
	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			if ((i != 2) || (args->args[2] != NULL)) {
				strcpy(message, "ERROR: Wrong arguments to oph_interlace2 function");
				return 1;
			}
		}
	}

	initid->ptr = NULL;

	return 0;
}

void oph_interlace2_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		oph_generic_param_multi *param = (oph_generic_param_multi *) initid->ptr;
		if (param->extend)
			free(param->extend);
		free_oph_generic_param_multi(param);
		initid->ptr = NULL;
	}
}

char *oph_interlace2(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	int i;

	if (*error) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	if (*is_null) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
	}
	for (i = 3; i < args->arg_count; ++i) {
		if (!args->lengths[i]) {
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
		param->core_oph_oper = NULL;
		param->extend = NULL;

		initid->ptr = (char *) param;
	} else
		param = (oph_generic_param_multi *) initid->ptr;

	if (param->error) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	int num_measure = args->arg_count - 3;
	size_t num_measure_total = 0;
	unsigned long numelem = 0;
	long long *list = NULL;
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
		size_t max = num_measure * sizeof(long long);
		list = (long long *) calloc(num_measure, sizeof(long long));
		if (args->args[2]) {
			if (max > args->lengths[2])
				max = args->lengths[2];
			memcpy(list, args->args[2], max);
		}
		for (i = 0; i < num_measure; ++i)
			if (list[i] <= 0)
				list[i] = 1;
		param->extend = list;
		for (i = 0; i < num_measure; ++i) {
			measure[i].length = args->lengths[3 + i];
			if (measure[i].length % measure[i].blocksize) {
				param->error = 1;
				pmesg(1, __FILE__, __LINE__, "Wrong input type or data corrupted\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			measure[i].numelem = measure[i].length / measure[i].blocksize;
			num_measure_total += measure[i].num_measure;
			if (measure->numelem * list[i] != measure[i].numelem * list[0]) {
				param->error = 1;
				pmesg(1, __FILE__, __LINE__, "Measures have different number of elements\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			numelem += measure[i].numelem;
		}

		param->measure = measure;
	} else {
		measure = param->measure;
		list = (long long *) param->extend;
	}

	for (i = 0; i < num_measure; ++i)
		measure[i].content = args->args[3 + i];

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
		if (output->num_measure != num_measure_total) {
			param->error = output->num_measure != measure->num_measure;	// Simple case when all the measures are the same data type
			if (!param->error) {
				int j;
				for (i = 0; i < num_measure; ++i) {
					if (measure[i].num_measure != measure->num_measure) {
						param->error = 1;
						break;
					}
					for (j = 0; j < measure->num_measure; ++j)
						if (measure[i].type[j] != measure->type[j]) {
							param->error = 1;
							break;
						}
					if (param->error)
						break;
				}
				if (!param->error)
					output->blocksize *= num_measure;
			}
			if (param->error) {
				pmesg(1, __FILE__, __LINE__, "Output data type has a different number of fields from the set of input data types\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
		}
		output->numelem = numelem;
		output->length = output->numelem * output->blocksize / num_measure;
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

	if (!param->error && core_oph_interlace2_multi(param)) {
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
