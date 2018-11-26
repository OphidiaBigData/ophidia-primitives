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

#include "oph_choquet.h"

int msglevel = 1;

int core_oph_choquet_multi(oph_multistring * byte_array, oph_multistring * result, unsigned char size, int id)
{
	int i, j, js, je;
	char *in_string = byte_array->content, *out_string = result->content, *in_string_curr = NULL, *out_string_curr = NULL;

	if (id <= 0) {
		js = 0;
		je = byte_array->num_measure;
	} else if (id > byte_array->num_measure) {
		pmesg(1, __FILE__, __LINE__, "Index out of boundaries\n");
		return -1;
	} else {
		js = id - 1;
		je = id;
		while (--id > 0) {
			in_string += byte_array->elemsize[id];
			out_string += result->elemsize[id];
		}
	}

	double *weight = (double *) byte_array->extend;
	if (!weight) {
		pmesg(1, __FILE__, __LINE__, "Empty weight list\n");
		return -1;
	}

	unsigned long step = 0;
	char *in_string_start = in_string, *out_string_start = out_string;

	do {

		in_string = in_string_start + step * size * byte_array->blocksize;
		out_string = out_string_start + step * result->blocksize;

		for (j = js; j < je; ++j) {
			switch (byte_array->type[j]) {
				case OPH_DOUBLE:{
						double tmp = 0;
						for (i = 0, in_string_curr = in_string; i < size; i++, in_string_curr += byte_array->blocksize)
							tmp += *(double *) in_string_curr;
						if (core_oph_type_cast((void *) (&tmp), out_string_curr = out_string, byte_array->type[j], result->type[j], byte_array->missingvalue))
							return -1;
						out_string_curr += result->blocksize;
						break;
					}

				default:
					pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
					return -1;
			}
			in_string += byte_array->elemsize[j];
			out_string += result->elemsize[j];
		}

		step++;

	} while (step * size < byte_array->numelem);

	return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_choquet_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if (args->arg_count < 4 || args->arg_count > 6) {
		strcpy(message, "ERROR: Wrong arguments! oph_choquet(input_OPH_TYPE, output_OPH_TYPE, measure, weight, [size, id_measure])");
		return 1;
	}

	for (i = 0; i < 4; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_choquet function");
			return 1;
		}
	}

	if (args->arg_count > 4) {
		if (args->arg_type[4] == STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_choquet function");
			return 1;
		}
		args->arg_type[4] = INT_RESULT;
		if (args->arg_count > 5) {
			if (args->arg_type[5] == STRING_RESULT) {
				strcpy(message, "ERROR: Wrong arguments to oph_choquet function");
				return 1;
			}
			args->arg_type[5] = INT_RESULT;
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;
	return 0;
}

void oph_choquet_deinit(UDF_INIT * initid)
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
		if (multimeasure->extend) {
			free(multimeasure->extend);
			multimeasure->extend = NULL;
		}
		free_oph_multistring(multimeasure);
		multimeasure = NULL;
		initid->extension = NULL;
	}
}

char *oph_choquet(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	oph_multistring *multim;
	oph_multistring *output;

	int id = 0;

	if (!initid->extension) {
		if (core_set_oph_multistring((oph_multistring **) (&(initid->extension)), args->args[0], &(args->lengths[0]))) {
			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		((oph_multistring *) (initid->extension))->extend = NULL;
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

	unsigned char size = 0;
	if ((args->arg_count > 4) && args->args[4])
		size = *(long long *) args->args[4];
	if (!size)
		size = multim->numelem;

	output->numelem = multim->numelem / size;

	if (!multim->extend && args->args[3]) {
		unsigned char k, bsize = size * (size + 1) / 2;
		unsigned long kk;
		double tmp[bsize];
		char *pointer = (char *) args->args[3];
		switch (multim->type[0]) {
			case OPH_DOUBLE:{
					for (k = kk = 0; k < bsize; ++k, kk += sizeof(double), pointer += sizeof(double))
						if (kk < args->lengths[3])
							tmp[k] = *(double *) pointer;
						else
							tmp[k] = 0;
					break;
				}
			case OPH_FLOAT:{
					for (k = kk = 0; k < bsize; ++k, kk += sizeof(float), pointer += sizeof(float))
						if (kk < args->lengths[3])
							tmp[k] = *(float *) pointer;
						else
							tmp[k] = 0;
					break;
				}
			case OPH_INT:{
					for (k = kk = 0; k < bsize; ++k, kk += sizeof(int), pointer += sizeof(int))
						if (kk < args->lengths[3])
							tmp[k] = *(int *) pointer;
						else
							tmp[k] = 0;
					break;
				}
			case OPH_LONG:{
					for (k = kk = 0; k < bsize; ++k, kk += sizeof(long long), pointer += sizeof(long long))
						if (kk < args->lengths[3])
							tmp[k] = *(long long *) pointer;
						else
							tmp[k] = 0;
					break;
				}
			case OPH_SHORT:{
					for (k = kk = 0; k < bsize; ++k, kk += sizeof(short), pointer += sizeof(short))
						if (kk < args->lengths[3])
							tmp[k] = *(short *) pointer;
						else
							tmp[k] = 0;
					break;
				}
			case OPH_BYTE:{
					for (k = kk = 0; k < bsize; ++k, kk += sizeof(char), pointer += sizeof(char))
						if (kk < args->lengths[3])
							tmp[k] = *(char *) pointer;
						else
							tmp[k] = 0;
					break;
				}
			default:
				pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
		}
		double *weight = (double *) calloc(size * size, sizeof(double));
		if (!weight) {
			pmesg(1, __FILE__, __LINE__, "Memory error\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		k = 0;
		unsigned char i, j, m;
		for (i = 0, m = size; i < size; ++i, --m)
			for (j = 0; j < m; ++j)
				weight[i * size + j] = tmp[k++];
		multim->extend = (void *) weight;
	}

	if ((args->arg_count > 5) && args->args[5]) {
		id = *((long long *) args->args[5]);
		if ((id < 0) || (id > ((oph_multistring *) (initid->ptr))->num_measure)) {
			pmesg(1, __FILE__, __LINE__, "Wrong measure identifier. Correct values are integers in [1, %d].\n", ((oph_multistring *) (initid->ptr))->num_measure);
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	/* Allocate the right space for the result set */
	if (!output->content) {
		output->content = (char *) calloc(1, (output->numelem) * (output->blocksize));
		if (!output->content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating measures string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	if (core_oph_choquet_multi(multim, output, size, id)) {
		pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
		*length = 0;
		*is_null = 1;
		*error = 1;
		return NULL;
	}
	*length = (output->numelem) * (output->blocksize);
	*error = 0;
	*is_null = 0;
	return (result = ((oph_multistring *) initid->ptr)->content);

}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
