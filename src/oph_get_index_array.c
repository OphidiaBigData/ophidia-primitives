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

#include "oph_get_index_array.h"
#include "limits.h"
int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_get_index_array_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	int i = 0;
	if ((args->arg_count < 4) || (args->arg_count > 5)) {
		strcpy(message, "ERROR: Wrong arguments! oph_get_index_array(input_OPH_TYPE, output_OPH_TYPE, startValue, stopValue, [stride])");
		return 1;
	}

	for (i = 0; i < 2; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_get_index_array function");
			return 1;
		}
	}
	for (; i < args->arg_count; i++) {
		if (args->arg_type[i] == STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_get_index_array function");
			return 1;
		}
		args->arg_type[i] = REAL_RESULT;
	}

	initid->ptr = NULL;
	initid->extension = NULL;
	return 0;
}

void oph_get_index_array_deinit(UDF_INIT *initid)
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

char *oph_get_index_array(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
	int res = 0;
	double start, stop, stride = 1;
	start = *((double *) args->args[2]);
	stop = *((double *) args->args[3]);
	if (args->arg_count > 4)
		stride = *((double *) args->args[4]);

	if (*error || (start > stop) || (stride <= 0)) {
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
		core_set_type(measure, args->args[1], &(args->lengths[1]));
		if (core_set_elemsize(measure)) {
			pmesg(1, __FILE__, __LINE__, "Error on setting elements size\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		measure->numelem = 1 + (stop - start) / stride;
		if (!initid->extension) {
			initid->extension = (char *) malloc(measure->numelem * measure->elemsize);
			if (!initid->extension) {
				pmesg(1, __FILE__, __LINE__, "Error allocating measures string\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
		}
	}
	oph_string *measure = (oph_string *) initid->ptr;

	switch (measure->type) {
		case OPH_DOUBLE:{
				double tmp = start;
				for (res = 0; res < measure->numelem; res++) {
					memcpy((initid->extension) + (res * measure->elemsize), (void *) (&tmp), measure->elemsize);
					tmp += stride;
					if (tmp > stop)
						tmp = stop;
				}
				break;
			}
		case OPH_FLOAT:{
				float tmp = (float) start, tmp2 = (float) stride, tmp3 = (float) stop;
				for (res = 0; res < measure->numelem; res++) {
					memcpy((initid->extension) + (res * measure->elemsize), (void *) (&tmp), measure->elemsize);
					tmp += tmp2;
					if (tmp > tmp3)
						tmp = tmp3;
				}
				break;
			}
		case OPH_LONG:{
				long long tmp = (long long) start, tmp2 = (long long) stride, tmp3 = (long long) stop;
				for (res = 0; res < measure->numelem; res++) {
					memcpy((initid->extension) + (res * measure->elemsize), (void *) (&tmp), measure->elemsize);
					tmp += tmp2;
					if (tmp > tmp3)
						tmp = tmp3;
				}
				break;
			}
		case OPH_INT:{
				int tmp = (int) start, tmp2 = (int) stride, tmp3 = (int) stop;
				for (res = 0; res < measure->numelem; res++) {
					memcpy((initid->extension) + (res * measure->elemsize), (void *) (&tmp), measure->elemsize);
					tmp += tmp2;
					if (tmp > tmp3)
						tmp = tmp3;
				}
				break;
			}
		case OPH_SHORT:{
				short tmp = (short) start, tmp2 = (short) stride, tmp3 = (short) stop;
				for (res = 0; res < measure->numelem; res++) {
					memcpy((initid->extension) + (res * measure->elemsize), (void *) (&tmp), measure->elemsize);
					tmp += tmp2;
					if (tmp > tmp3)
						tmp = tmp3;
				}
				break;
			}
		case OPH_BYTE:{
				char tmp = (char) start, tmp2 = (char) stride, tmp3 = (char) stop;
				for (res = 0; res < measure->numelem; res++) {
					memcpy((initid->extension) + (res * measure->elemsize), (void *) (&tmp), measure->elemsize);
					tmp += tmp2;
					if (tmp > tmp3)
						tmp = tmp3;
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
