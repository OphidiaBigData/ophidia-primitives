/*
    Ophidia Primitives
    Copyright (C) 2012-2018 CMCC Foundation

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

/* Core function header */
#include "oph_core_gsl.h"

/* MySQL headers  */
#include <mysql.h>		// It contains UDF-related symbols and data structures

/* GSL STATS headers  */
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_statistics.h>

/* stats mask */
// 14-bit mask for different operators combinations
// 00000000000000 : invalid
// 1xxxxxxxxxxxxx : mean
// x1xxxxxxxxxxxx : variance
// xx1xxxxxxxxxxx : std dev
// xxx1xxxxxxxxxx : abs dev
// xxxx1xxxxxxxxx : skew
// xxxxx1xxxxxxxx : kurtosis
// xxxxxx1xxxxxxx : autocorrelation
// xxxxxxx1xxxxxx : max
// xxxxxxxx1xxxxx : min
// xxxxxxxxx1xxxx : 0.05 quantile
// xxxxxxxxxx1xxx : 0.25    "     ->Q1
// xxxxxxxxxxx1xx : 0.5     "     ->Q2 (median)
// xxxxxxxxxxxx1x : 0.75    "     ->Q3
// xxxxxxxxxxxxx1 : 0.95    "
// 11111111111111 : all
// 101            : mean + std dev
#define MASK_LEN     14
#define DEFAULT_MASK "11111111111111"

/*------------------------------------------------------------------|
|       Functions' declarations (BEGIN)             |
|------------------------------------------------------------------*/
/* Compute requested statistics */
int oph_gsl_stats_produce(void *in_data, const size_t data_len, oph_type data_type, const char *mask, char *out_data, const size_t out_len, size_t * out_data_len, oph_type out_data_type);

/* These must be right or mysqld will not find the symbol! */
my_bool oph_gsl_stats_init(UDF_INIT * initid, UDF_ARGS * args, char *message);
void oph_gsl_stats_deinit(UDF_INIT * initid);
char *oph_gsl_stats(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error);

/*------------------------------------------------------------------|
|               Functions' declarations (END)                       |
|------------------------------------------------------------------*/
