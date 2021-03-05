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

/* GSL SORT headers  */
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_sort.h>

/* "order-safe" flag values */
#define ORDER_SAFE_FLAG_SET     1
#define ORDER_SAFE_FLAG_UNSET   0
#define DEFAULT_ORDER_SAFE_FLAG ORDER_SAFE_FLAG_SET


/*------------------------------------------------------------------|
|       Functions' declarations (BEGIN)             |
|------------------------------------------------------------------*/
/* Sort elements in ascending numerical order */
void oph_gsl_sort_produce(void *inout_data, const size_t data_len, oph_type data_type);

/* These must be right or mysqld will not find the symbol! */
my_bool oph_gsl_sort_init(UDF_INIT * initid, UDF_ARGS * args, char *message);
void oph_gsl_sort_deinit(UDF_INIT * initid);
char *oph_gsl_sort(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error);

/*------------------------------------------------------------------|
|               Functions' declarations (END)                       |
|------------------------------------------------------------------*/
