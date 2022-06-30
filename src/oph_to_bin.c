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

#include "oph_to_bin.h"
#include "limits.h"
int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_to_bin_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if (args->arg_count != 3) {
		strcpy(message, "ERROR: Wrong arguments! oph_to_bin(input_OPH_TYPE, output_OPH_TYPE, 'string')");
		return 1;
	}

	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_to_bin function");
			return 1;
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;
	return 0;
}

void oph_to_bin_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		free((oph_string *) initid->ptr);
		initid->ptr = NULL;
	}
	if (initid->extension) {
		free(initid->extension);
		initid->extension = NULL;
	}
}

char *oph_to_bin(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	int res = 0;

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

	if (!initid->ptr) {
		initid->ptr = malloc(sizeof(oph_string));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating measures string\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}
		oph_string *measure = (oph_string *) initid->ptr;

		core_set_type((measure), args->args[1], &(args->lengths[1]));

		measure->content = args->args[2];
		if (!measure->content) {
			*length = 0;
			*is_null = 1;
			*error = 0;
			return NULL;
		}
		measure->length = &(args->lengths[2]);

		measure->numelem = 1;
		for (res = 0; res < *(measure->length) / sizeof(char); res++) {
			if (measure->content[res] == ',')
				measure->numelem++;
		}

		if (core_set_elemsize((measure))) {
			pmesg(1, __FILE__, __LINE__, "Error on setting elements size\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		// Allocate space for num_elem number, each with max INT_DIGIT integer digits and exactly PRECISION decimal digits (and one char for comma) + (num_elem -1) commas and spaces as separator
		if (!initid->extension) {
			initid->extension = (char *) malloc(measure->numelem * measure->elemsize);
			if (!initid->extension) {
				pmesg(1, __FILE__, __LINE__, "Error allocating measures string\n");
				*length = 0;
				*is_null = 1;
				*error = 1;
				return NULL;
			}
		}
	}
	oph_string *measure = (oph_string *) initid->ptr;

	measure->content = args->args[2];
	if (!measure->content) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
	}
	measure->length = &(args->lengths[2]);

	char *ptr = measure->content;
	char *endptr;

	switch (measure->type) {
		case OPH_DOUBLE:{
				double tmp;
				for (res = 0; res < measure->numelem; res++) {
					tmp = strtod(ptr, &endptr);

					//If no conversion is performed, zero is returned and the value of ptr is stored in the location referenced by endptr
					if (ptr == endptr) {
						pmesg(1, __FILE__, __LINE__, "Unable to convert supplied measure\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					memcpy((initid->extension) + (res * measure->elemsize), (void *) (&tmp), measure->elemsize);
					ptr = endptr + 1 * (sizeof(char));
				}
				break;
			}
		case OPH_FLOAT:{
				float tmp;
				for (res = 0; res < measure->numelem; res++) {
					tmp = strtof(ptr, &endptr);

					//If no conversion is performed, zero is returned and the value of ptr is stored in the location referenced by endptr
					if (ptr == endptr) {
						pmesg(1, __FILE__, __LINE__, "Unable to convert supplied measure\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					memcpy((initid->extension) + (res * measure->elemsize), (void *) (&tmp), measure->elemsize);
					ptr = endptr + 1 * (sizeof(char));
				}
				break;
			}
		case OPH_LONG:{
				long long tmp;
				for (res = 0; res < measure->numelem; res++) {
					tmp = strtoll(ptr, &endptr, 10);

					//If there were no digits at all, strtol() stores the original value of nptr in *endptr (and returns 0)
					if (ptr == endptr) {
						pmesg(1, __FILE__, __LINE__, "Unable to convert supplied measure\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					memcpy((initid->extension) + (res * measure->elemsize), (void *) (&tmp), measure->elemsize);
					ptr = endptr + 1 * (sizeof(char));
				}
				break;
			}
		case OPH_INT:{
				long tmp;
				int tmpint;
				for (res = 0; res < measure->numelem; res++) {
					tmp = strtol(ptr, &endptr, 10);

					//If no conversion is performed, zero is returned and the value of ptr is stored in the location referenced by endptr
					if (ptr == endptr) {
						pmesg(1, __FILE__, __LINE__, "Unable to convert supplied measure\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (INT_MIN <= tmp && tmp <= INT_MAX) {
						tmpint = (int) tmp;
						memcpy((initid->extension) + (res * measure->elemsize), (void *) (&tmpint), measure->elemsize);
						ptr = endptr + 1 * (sizeof(char));
					} else {
						pmesg(1, __FILE__, __LINE__, "Value out of integer range\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
				break;
			}
		case OPH_SHORT:{
				long tmp;
				short tmpint;
				for (res = 0; res < measure->numelem; res++) {
					tmp = strtol(ptr, &endptr, 10);

					//If no conversion is performed, zero is returned and the value of ptr is stored in the location referenced by endptr
					if (ptr == endptr) {
						pmesg(1, __FILE__, __LINE__, "Unable to convert supplied measure\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (INT_MIN <= tmp && tmp <= INT_MAX) {
						tmpint = (short) tmp;
						memcpy((initid->extension) + (res * measure->elemsize), (void *) (&tmpint), measure->elemsize);
						ptr = endptr + 1 * (sizeof(char));
					} else {
						pmesg(1, __FILE__, __LINE__, "Value out of integer range\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
				break;
			}
		case OPH_BYTE:{
				long tmp;
				char tmpint;
				for (res = 0; res < measure->numelem; res++) {
					tmp = strtol(ptr, &endptr, 10);

					//If no conversion is performed, zero is returned and the value of ptr is stored in the location referenced by endptr
					if (ptr == endptr) {
						pmesg(1, __FILE__, __LINE__, "Unable to convert supplied measure\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					if (INT_MIN <= tmp && tmp <= INT_MAX) {
						tmpint = (char) tmp;
						memcpy((initid->extension) + (res * measure->elemsize), (void *) (&tmpint), measure->elemsize);
						ptr = endptr + 1 * (sizeof(char));
					} else {
						pmesg(1, __FILE__, __LINE__, "Value out of integer range\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
				break;
			}
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;

	}
	*length = measure->numelem * measure->elemsize;
	*error = 0;
	*is_null = 0;
	return initid->extension;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
