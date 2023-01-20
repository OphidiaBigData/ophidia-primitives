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

#include "oph_find.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_find_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if (args->arg_count < 4 || args->arg_count > 5) {
		strcpy(message, "ERROR: Wrong arguments! oph_find(input_OPH_TYPE, output_OPH_TYPE, measure, value, [distance])");
		return 1;
	}

	for (i = 0; i < 3; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_find function");
			return 1;
		}
	}

	// Check only if passed values are string or null
	if (args->arg_type[3] == STRING_RESULT || !(args->args[3])) {
		strcpy(message, "ERROR: Wrong arguments to oph_find function");
		return 1;
	}
	//Coerce numeric value to be a REAL_RESULT
	args->arg_type[3] = REAL_RESULT;

	if (args->arg_count > 4) {
		if (args->arg_type[4] == STRING_RESULT || !(args->args[4])) {
			strcpy(message, "ERROR: Wrong arguments to oph_find function");
			return 1;
		}
		//Coerce numeric value to be a REAL_RESULT
		args->arg_type[4] = REAL_RESULT;
	}

	return 0;
}

void oph_find_deinit(UDF_INIT * initid __attribute__ ((unused)))
{
}

long long oph_find(UDF_INIT * initid, UDF_ARGS * args, char *is_null, char *error)
{
	if (*error) {
		*is_null = 0;
		*error = 1;
		return -1;
	}
	if (*is_null || !args->lengths[2]) {
		*is_null = 1;
		*error = 0;
		return -1;
	}

	oph_string measure;

	int res = 0;
	long count = 0;
	double distance = 0;
	double value;

	core_set_type(&(measure), args->args[0], &(args->lengths[0]));

	if (core_set_elemsize(&(measure))) {
		pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
		*error = 1;
		return -1;
	}

	value = *((double *) args->args[3]);
	if (args->arg_count > 4) {
		distance = *((double *) args->args[4]);
	}

	measure.content = args->args[2];
	measure.length = &(args->lengths[2]);

	if (core_set_numelem(&(measure))) {
		pmesg(1, __FILE__, __LINE__, "Error on counting elements\n");
		*is_null = 0;
		*error = 1;
		return -1;
	}

	res = core_oph_find(&measure, value, distance, &count);
	if (res) {
		pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
		*is_null = 0;
		*error = 1;
		return -1;
	}

	*is_null = 0;
	*error = 0;

	return (long long) count;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
