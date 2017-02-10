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

/* Standard C headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <math.h>

/* Core function header */
#include "oph_core.h"

/* MySQL headers  */
#include <mysql.h>		// It contains UDF-related symbols and data structures

/* stats mask */
// 7-bit mask for different operators combinations
// 0000000 : invalid
// 1xxxxxx : mean
// x1xxxxx : variance
// xx1xxxx : std dev
// xxx1xxx : skew
// xxxx1xx : kurtosis (normalized to zero for a Gaussian distribution!)
// xxxxx1x : max
// xxxxxx1 : min
// 1111111 : all
// 101     : mean + std dev
#define MASK_LEN     7
#define DEFAULT_MASK "1111111"

typedef struct oph_agg_stats_partial_data {
	oph_string result;	// final result
	oph_string mask;
	oph_string measure;
	oph_stringPtr partials;	// array of partial results arrays
	int sum1;		// 1 if sum{x_i} needed
	int sum2;		// 1 if sum{(x_i)^2} needed
	int sum3;		// 1 if sum{(x_i)^3} needed
	int sum4;		// 1 if sum{(x_i)^4} needed
	int max;		// 1 if array of max values needed
	int min;		// 1 if array of min values needed
	int count;		// number of aggregated rows
} oph_agg_stats_partial_data;

/*------------------------------------------------------------------|
|       Functions' declarations (BEGIN)             |
|------------------------------------------------------------------*/

/* These must be right or mysqld will not find the symbol! */
my_bool oph_aggregate_stats_partial_init(UDF_INIT * initid, UDF_ARGS * args, char *message);
void oph_aggregate_stats_partial_deinit(UDF_INIT * initid);
char *oph_aggregate_stats_partial(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error);
void oph_aggregate_stats_partial_reset(UDF_INIT * initid, UDF_ARGS * args, char *is_null, char *error);
void oph_aggregate_stats_partial_clear(UDF_INIT * initid, char *is_null, char *error);
void oph_aggregate_stats_partial_add(UDF_INIT * initid, UDF_ARGS * args, char *is_null, char *error);

/*------------------------------------------------------------------|
|               Functions' declarations (END)                       |
|------------------------------------------------------------------*/
