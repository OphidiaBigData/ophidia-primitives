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

#include "oph_sequence.h"

int msglevel = 1;

inline int oph_sequence_update(char type, char value, int index, int max, char *flag, long long *previous, long long *next)
{
	switch (type) {
		case 0:
			{
				if (value) {
					if (*flag)
						(*next)++;
					else {
						*flag = 1;
						*next = 1;
					}
					if (index == max - 1) {
						*previous = *next;
						return 1;
					}
				} else if (*flag) {
					*flag = 0;
					return 1;
				}
				break;
			}
		case 1:
			{
				if (value) {
					if (!*flag) {
						*flag = 1;
						*previous = index + 1;	// Non C-like indexing
						return 1;
					}
				} else if (*flag)
					*flag = 0;
				break;
			}
		case 2:
			{
				if (value) {
					*previous = index + 1;	// Non C-like indexing
					if (!*flag)
						*flag = 1;
					if (index == max - 1)
						return 1;
				} else if (*flag) {
					*flag = 0;
					return 1;
				}
				break;
			}
		default:;
	}
	return 0;
}

int core_oph_sequence_multi(oph_multistring * byte_array, oph_multistring * result, int id, char type, char padding, double filling)
{
	int i, j, js, je, k = 0;
	char *in_string = byte_array->content, *out_string = result->content, *current_in_string, *current_out_string, flag;
	long long previous, next;

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

	for (j = js; j < je; ++j) {
		current_in_string = in_string;
		current_out_string = out_string;
		flag = 0;
		switch (byte_array->type[j]) {
			case OPH_DOUBLE:{
					for (i = k = 0; i < byte_array->numelem; i++, current_in_string += byte_array->blocksize) {
						if (oph_sequence_update(type, *(double *) current_in_string, i, byte_array->numelem, &flag, &previous, &next)) {
							if (core_oph_type_cast((void *) &previous, current_out_string, OPH_LONG, result->type[j], byte_array->missingvalue))
								return -1;
							current_out_string += result->blocksize;
							k++;
						}
						if (!type)
							previous = next;
					}
					if (padding) {
						double tmp = (double) filling;
						for (; k < result->numelem; k++, current_out_string += result->blocksize) {
							if (core_oph_type_cast((void *) (&tmp), current_out_string, byte_array->type[j], result->type[j], byte_array->missingvalue))
								return -1;
						}
					}
					break;
				}
			case OPH_FLOAT:{
					for (i = k = 0; i < byte_array->numelem; i++, current_in_string += byte_array->blocksize) {
						if (oph_sequence_update(type, *(float *) current_in_string, i, byte_array->numelem, &flag, &previous, &next)) {
							if (core_oph_type_cast((void *) &previous, current_out_string, OPH_LONG, result->type[j], byte_array->missingvalue))
								return -1;
							current_out_string += result->blocksize;
							k++;
						}
						if (!type)
							previous = next;
					}
					if (padding) {
						float tmp = (float) filling;
						for (; k < result->numelem; k++, current_out_string += result->blocksize) {
							if (core_oph_type_cast((void *) (&tmp), current_out_string, byte_array->type[j], result->type[j], byte_array->missingvalue))
								return -1;
						}
					}
					break;
				}
			case OPH_INT:{
					for (i = k = 0; i < byte_array->numelem; i++, current_in_string += byte_array->blocksize) {
						if (oph_sequence_update(type, *(int *) current_in_string, i, byte_array->numelem, &flag, &previous, &next)) {
							if (core_oph_type_cast((void *) &previous, current_out_string, OPH_LONG, result->type[j], byte_array->missingvalue))
								return -1;
							current_out_string += result->blocksize;
							k++;
						}
						if (!type)
							previous = next;
					}
					if (padding) {
						int tmp = (int) filling;
						for (; k < result->numelem; k++, current_out_string += result->blocksize) {
							if (core_oph_type_cast((void *) (&tmp), current_out_string, byte_array->type[j], result->type[j], byte_array->missingvalue))
								return -1;
						}
					}
					break;
				}
			case OPH_LONG:{
					for (i = k = 0; i < byte_array->numelem; i++, current_in_string += byte_array->blocksize) {
						if (oph_sequence_update(type, *(long long *) current_in_string, i, byte_array->numelem, &flag, &previous, &next)) {
							if (core_oph_type_cast((void *) &previous, current_out_string, OPH_LONG, result->type[j], byte_array->missingvalue))
								return -1;
							current_out_string += result->blocksize;
							k++;
						}
						if (!type)
							previous = next;
					}
					if (padding) {
						long long tmp = (long long) filling;
						for (; k < result->numelem; k++, current_out_string += result->blocksize) {
							if (core_oph_type_cast((void *) (&tmp), current_out_string, byte_array->type[j], result->type[j], byte_array->missingvalue))
								return -1;
						}
					}
					break;
				}
			case OPH_SHORT:{
					for (i = k = 0; i < byte_array->numelem; i++, current_in_string += byte_array->blocksize) {
						if (oph_sequence_update(type, *(short *) current_in_string, i, byte_array->numelem, &flag, &previous, &next)) {
							if (core_oph_type_cast((void *) &previous, current_out_string, OPH_LONG, result->type[j], byte_array->missingvalue))
								return -1;
							current_out_string += result->blocksize;
							k++;
						}
						if (!type)
							previous = next;
					}
					if (padding) {
						short tmp = (short) filling;
						for (; k < result->numelem; k++, current_out_string += result->blocksize) {
							if (core_oph_type_cast((void *) (&tmp), current_out_string, byte_array->type[j], result->type[j], byte_array->missingvalue))
								return -1;
						}
					}
					break;
				}
			case OPH_BYTE:{
					for (i = k = 0; i < byte_array->numelem; i++, current_in_string += byte_array->blocksize) {
						if (oph_sequence_update(type, *(char *) current_in_string, i, byte_array->numelem, &flag, &previous, &next)) {
							if (core_oph_type_cast((void *) &previous, current_out_string, OPH_LONG, result->type[j], byte_array->missingvalue))
								return -1;
							current_out_string += result->blocksize;
							k++;
						}
						if (!type)
							previous = next;
					}
					if (padding) {
						char tmp = (char) filling;
						for (; k < result->numelem; k++, current_out_string += result->blocksize) {
							if (core_oph_type_cast((void *) (&tmp), current_out_string, byte_array->type[j], result->type[j], byte_array->missingvalue))
								return -1;
						}
					}
					break;
				}
			default:
				pmesg(1, __FILE__, __LINE__, "Not recognized type\n");
				return -1;
		}
		if (result->numelem > k)
			result->numelem = k;
		in_string += byte_array->elemsize[j];
		out_string += result->elemsize[j];
	}
	return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_sequence_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if (args->arg_count < 3 || args->arg_count > 5) {
		strcpy(message, "ERROR: Wrong arguments! oph_sequence(input_OPH_TYPE, output_OPH_TYPE, measure, [info], [padding])");
		return 1;
	}

	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_sequence function");
			return 1;
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;

	return 0;
}

void oph_sequence_deinit(UDF_INIT * initid)
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

char *oph_sequence(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	if (*error) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	oph_multistring *multim;
	oph_multistring *output;

	int id = 0;		// Only this value is supported
	char type = 0, padding = 0;

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

	if ((args->arg_count > 3) && args->args[3] && args->lengths[3]) {
		if (!strncmp(args->args[3], "first", args->lengths[3]))
			type = 1;
		else if (!strncmp(args->args[3], "last", args->lengths[3]))
			type = 2;
		else if (strncmp(args->args[3], "length", args->lengths[3])) {
			pmesg(1, __FILE__, __LINE__, "Wrong operation type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if ((args->arg_count > 4) && args->args[4] && args->lengths[4]) {
			if (!strncmp(args->args[4], "yes", args->lengths[4]))
				padding = 1;
			else if (strncmp(args->args[4], "no", args->lengths[4])) {
				pmesg(1, __FILE__, __LINE__, "Wrong padding option\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
		}
	}

	output = (oph_multistring *) (initid->ptr);
	output->numelem = multim->numelem;

	/* Allocate the right space for the result set */
	if (!output->content) {
		output->content = (char *) calloc(output->numelem, output->blocksize);
		if (!output->content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating measures string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	if (core_oph_sequence_multi(multim, output, id, type, padding, 0)) {
		pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	if (!output->numelem) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
	}

	*length = output->numelem * output->blocksize;
	*error = 0;
	*is_null = 0;

	return (result = ((oph_multistring *) initid->ptr)->content);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
