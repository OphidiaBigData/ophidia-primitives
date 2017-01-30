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

#include "oph_id3.h"

int msglevel=LOG_ERROR;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_id3_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	unsigned int i;
	if(args->arg_count!=3)
	{
		strcpy(message, "ERROR! Wrong arguments! oph_id3(id, list, block_size)");
		return 1;
	}
	for (i=0; i<args->arg_count; ++i) if(((args->arg_type[i] != INT_RESULT) && (i!=1)) || ((args->arg_type[i] != STRING_RESULT) && (i==1)))
	{
		strcpy(message, "Wrong type for arguments in oph_id3() function");
		return 1;
	}
	return 0;
}

void oph_id3_deinit(UDF_INIT *initid __attribute__((unused))) { }

long long oph_id3(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
	long long* list = (long long*)(args->args[1]);
	if (!list) return 1;

	long long k = *((long long*)(args->args[0])) - 1; // 'C'-like indexing
	long long new_size = args->lengths[1] / sizeof(long long);
	long long block_size = *((long long*)(args->args[2]));

	long long jj, reduced_size = 0;
	for (jj=0;jj<new_size;++jj) reduced_size += list[jj];

	long long row_index = k/block_size, i, start=0, stop=0, relative_row_index = row_index%reduced_size;

	for (i=0;i<new_size-1;++i)
	{
		stop += list[i];
		if ((relative_row_index >= start) && (relative_row_index < stop)) break;
		start = stop;
	}

	return k%block_size + (i+row_index/reduced_size*new_size)*block_size + 1; // Non 'C'-like indexing
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/

