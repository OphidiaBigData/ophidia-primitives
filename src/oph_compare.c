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

#include "oph_compare.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_compare_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if (args->arg_count != 4) {
		strcpy(message, "ERROR: Wrong arguments! oph_compare(input_OPH_TYPE, output_OPH_TYPE, measure_a, measure_b)");
		return 1;
	}

	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_compare function");
			return 1;
		}
	}

	initid->ptr = NULL;

	return 0;
}

void oph_compare_deinit(UDF_INIT * initid)
{
}

long long oph_compare(UDF_INIT * initid, UDF_ARGS * args, char *is_null, char *error)
{
	if (*error || *is_null)
		return -1;
	*error = 0;
	*is_null = 0;

	unsigned long i, la = args->lengths[2], lb = args->lengths[3];

	if (la != lb)
		return 1;

	char *A = args->args[2];
	char *B = args->args[3];

	if (!A && !B)
		return 0;
	if (!A || !B)
		return 1;

	for (i = 0; i < la; ++i, ++A, ++B)
		if (*A != *B)
			return 1;

	return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
