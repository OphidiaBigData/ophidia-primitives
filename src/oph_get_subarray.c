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

#include "oph_get_subarray.h"

int msglevel = 1;

int core_oph_get_subarray_multi(oph_multistring *byte_array, oph_multistring *result, long long start)
{
	unsigned long start_ind;
	start_ind = start * byte_array->blocksize;
	(byte_array->content) += start_ind;
	if (core_oph_multistring_cast(byte_array, result, byte_array->missingvalue))
		return -1;
	return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_get_subarray_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	int i = 0;
	if (args->arg_count != 3 && args->arg_count != 4 && args->arg_count != 5) {
		strcpy(message, "ERROR: Wrong arguments! oph_get_subarray(input_OPH_TYPE, output_OPH_TYPE, measure, [start], [size])");
		return 1;
	}

	for (i = 0; (i < 3); i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_get_subarray function. Use strings");
			return 1;
		}
	}

	for (i = 3; i < args->arg_count; i++) {
		args->arg_type[i] = INT_RESULT;
	}

	initid->ptr = NULL;
	initid->extension = NULL;

	return 0;
}

void oph_get_subarray_deinit(UDF_INIT *initid)
{
	//Free allocated space
	oph_multistring *multimeasure;
	if (initid->ptr) {
		multimeasure = (oph_multistring *) (initid->ptr);
		free(multimeasure->content);
		multimeasure->content = NULL;
		free_oph_multistring(multimeasure);
		multimeasure = NULL;
		initid->ptr = NULL;
	}
	if (initid->extension) {
		multimeasure = (oph_multistring *) (initid->extension);
		free_oph_multistring(multimeasure);
		multimeasure = NULL;
		initid->extension = NULL;
	}
}

char *oph_get_subarray(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
	oph_multistring *multim;
	oph_multistring *output;

	long long size;
	long long start;
	unsigned long res_length;


	if (!initid->extension) {
		if (core_set_oph_multistring((oph_multistring **) (&(initid->extension)), args->args[0], &(args->lengths[0]))) {
			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	if (!initid->ptr) {
		if (core_set_oph_multistring(((oph_multistring **) (&(initid->ptr))), args->args[1], &(args->lengths[1]))) {
			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		//Check
		if (((oph_multistring *) (initid->ptr))->num_measure != ((oph_multistring *) (initid->extension))->num_measure) {
			pmesg(1, __FILE__, __LINE__, "Wrong input or output type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	multim = (oph_multistring *) (initid->extension);
	multim->content = args->args[2];
	multim->length = args->lengths[2];
	//Check
	if (multim->length % multim->blocksize) {
		pmesg(1, __FILE__, __LINE__, "Wrong input type or data corrupted\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	multim->numelem = multim->length / multim->blocksize;

	output = (oph_multistring *) (initid->ptr);

	switch (args->arg_count) {
		case 3:
			//No start provided
			//No size provided

			start = 0;
			size = (long long) (multim->numelem);
			output->numelem = (unsigned long) size;
			res_length = (output->numelem) * (output->blocksize);
			break;
		case 4:
			//No size provided
			start = *((long long *) args->args[3]) - 1;	// Non C-like indexing

			if (start < 0) {
				pmesg(1, __FILE__, __LINE__, "Wrong argument '%lld' to oph_get_subarray function\n", start);
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}

			if (multim->numelem <= start) {
				pmesg(1, __FILE__, __LINE__, "Start value '%lld' over limits\n", start);
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			size = (long long) (multim->numelem - start);
			output->numelem = (unsigned long) size;
			res_length = (output->numelem) * (output->blocksize);
			break;
		case 5:

			start = *((long long *) args->args[3]) - 1;	// Non C-like indexing
			size = *((long long *) args->args[4]);

			if ((start < 0) || (size <= 0)) {
				pmesg(1, __FILE__, __LINE__, "Wrong arguments '%lld' or '%lld' to oph_get_subarray function\n", start, size);
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}

			if (multim->numelem <= start) {
				pmesg(1, __FILE__, __LINE__, "Start value '%lld' over limits\n", start);
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}

			if (start + size > multim->numelem)
				size = (long long) multim->numelem - start;
			output->numelem = (unsigned long) size;
			res_length = (output->numelem) * (output->blocksize);
			break;
		default:
			pmesg(1, __FILE__, __LINE__, "Error on counting arguments\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
			break;
	}

	output->length = res_length;
	/* Allocate the right space for the result set */
	if (!output->content) {
		output->content = (char *) calloc(1, output->length);
		if (!output->content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating measures string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	if (core_oph_get_subarray_multi(multim, output, start)) {
		pmesg(1, __FILE__, __LINE__, "Unable to extract subarray\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	*length = output->length;
	*error = 0;
	*is_null = 0;
	return (result = ((oph_multistring *) (initid->ptr))->content);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
