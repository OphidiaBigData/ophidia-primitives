/*
    Ophidia Primitives
    Copyright (C) 2012-2020 CMCC Foundation

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

/*/ Standard C headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Core function header */
#include "oph_core.h"

/* MySQL headers  */
#include <mysql.h>		// It contains UDF-related symbols and data structures

/*Pthread and add-on header */
#include <pthread.h>
#include "barrier.h"

#define NTHREAD 4

typedef struct th_data {
	pthread_t thread[NTHREAD];
	barrier_t barr_start;
	barrier_t barr_end;
	void *curr_args;
	unsigned int exit_flag;
} th_data;

/*------------------------------------------------------------------|
|		Functions' declarations (BEGIN)			    |
|------------------------------------------------------------------*/

/* These must be right or mysqld will not find the symbol! */
my_bool oph_sum_array_init_r(UDF_INIT * initid, UDF_ARGS * args, char *message);
void oph_sum_array_deinit_r(UDF_INIT * initid);
char *oph_sum_array_r(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error);
void *sum_array_r(void *data_str);

/*------------------------------------------------------------------|
|               Functions' declarations (END)                       |
|------------------------------------------------------------------*/
