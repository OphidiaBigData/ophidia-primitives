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

#include "oph_id_to_index.h"

int msglevel = LOG_ERROR;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_id_to_index_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	unsigned int i;
	if (args->arg_count < 2) {
		strcpy(message, "ERROR! Wrong arguments! oph_id_to_index(id, size, ...)");
		return 1;
	}
	for (i = 0; i < args->arg_count; ++i)
		if (args->arg_type[i] != INT_RESULT) {
			strcpy(message, "Wrong type for arguments in oph_id_to_index() function");
			return 1;
		}
	return 0;
}

void oph_id_to_index_deinit(UDF_INIT *initid __attribute__((unused)))
{
}

long long oph_id_to_index(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
	long long size, id = *((long long *) args->args[0]) - 1, index = id;
	if (id < 0) {
		pmesg(LOG_ERROR, __FILE__, __LINE__, "Invalid value %d\n", id);
		*is_null = 0;
		*error = 1;
		return -1;
	}
	unsigned int counter = 1;
	while (counter < args->arg_count) {
		size = *((long long *) args->args[counter++]);
		index = id % size;
		id = (id - index) / size;
	}
	*is_null = 0;
	*error = 0;
	return index + 1;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
