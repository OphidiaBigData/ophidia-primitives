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

#include "oph_value_to_bin.h"
#include "limits.h"
int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_value_to_bin_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	if (args->arg_count != 3) {
		strcpy(message, "ERROR: Wrong arguments! oph_value_to_bin(input_OPH_TYPE, output_OPH_TYPE, value)");
		return 1;
	}

	int i;
	for (i = 0; i < 2; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_value_to_bin function");
			return 1;
		}
	}
	if ((args->arg_type[2] != INT_RESULT) && (args->arg_type[2] != REAL_RESULT) && (args->arg_type[2] != DECIMAL_RESULT)) {
		strcpy(message, "ERROR: Wrong arguments to oph_value_to_bin function");
		return 1;
	}
	args->arg_type[2] = DECIMAL_RESULT;

	initid->ptr = NULL;

	return 0;
}

void oph_value_to_bin_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		free(initid->ptr);
		initid->ptr = NULL;
	}
}

char *oph_value_to_bin(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
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

	oph_type out_type = core_get_type(args->args[1], &(args->lengths[1]));
	size_t size;

	if (!initid->ptr) {
		// Check provided output type
		if (!out_type || (out_type != OPH_BYTE && out_type != OPH_SHORT && out_type != OPH_INT && out_type != OPH_LONG && out_type != OPH_FLOAT && out_type != OPH_DOUBLE)) {
			pmesg(1, __FILE__, __LINE__, "Invalid type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		size = core_sizeof(out_type);
		initid->ptr = malloc(size);
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating output string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	} else
		size = core_sizeof(out_type);

	result = initid->ptr;

	//Decimal_result is passed as a string
	char buf[256];
	if ((args->lengths[2] > 256) || core_strncpy(buf, args->args[2], &(args->lengths[2]))) {
		pmesg(1, __FILE__, __LINE__, "Invalid string\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	if (args->arg_type[2] != DECIMAL_RESULT) {
		pmesg(1, __FILE__, __LINE__, "Invalid type\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	switch (out_type) {
		case OPH_INT:{
				int tmp = (int) strtol(buf, NULL, 10);
				memcpy(result, (void *) (&tmp), size);
				break;
			}
		case OPH_SHORT:{
				short tmp = (short) strtol(buf, NULL, 10);
				memcpy(result, (void *) (&tmp), size);
				break;
			}
		case OPH_BYTE:{
				char tmp = (char) strtol(buf, NULL, 10);
				memcpy(result, (void *) (&tmp), size);
				break;
			}
		case OPH_LONG:{
				long long tmp = strtol(buf, NULL, 10);
				memcpy(result, (void *) (&tmp), size);
				break;
			}
		case OPH_FLOAT:{
				float tmp = strtof(buf, NULL);
				memcpy(result, (void *) (&tmp), size);
				break;
			}
		case OPH_DOUBLE:{
				double tmp = strtod(buf, NULL);
				memcpy(result, (void *) (&tmp), size);
				break;
			}
		default:
			pmesg(1, __FILE__, __LINE__, "Invalid type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
	}

	*length = (unsigned long) size;
	*error = 0;
	*is_null = 0;

	return result;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
