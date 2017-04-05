/*
    Ophidia Primitives
    Copyright (C) 2012-2017 CMCC Foundation

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

#include "oph_bit_size.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_bit_size_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	if (args->arg_count != 3) {
		strcpy(message, "ERROR: Wrong arguments! oph_bit_size(input_OPH_TYPE, output_OPH_TYPE, bit_measure)");
		return 1;
	}

	int i;
	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_bit_size function");
			return 1;
		}
	}

	initid->ptr = NULL;

	return 0;
}

void oph_bit_size_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		oph_bit_param *param = (oph_bit_param *) initid->ptr;
		if (param->measure) {
			free(param->measure);
			param->measure = NULL;
		}
		if (param->result) {
			free(param->result);
			param->result = NULL;
		}
		free(initid->ptr);
		initid->ptr = NULL;
	}
}

// Current implementation does not consider any padding
long long oph_bit_size(UDF_INIT * initid, UDF_ARGS * args, char *is_null, char *error)
{
	if (!args->lengths[2]) {
		*is_null = 1;
		*error = 0;
		return 0;
	}

	*error = 0;
	*is_null = 0;
	return core_oph_bit_numelem(args->lengths[2]);;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
