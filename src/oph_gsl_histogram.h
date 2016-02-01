/*
    Ophidia Primitives
    Copyright (C) 2012-2016 CMCC Foundation

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
#include <mysql.h> // It contains UDF-related symbols and data structures

/* GSL HISTOGRAMS headers  */
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_histogram.h>
#include <gsl/gsl_statistics.h>

/* Macro */
#define OPH_GSL_HISTOGRAM_RANGES "ranges"
#define OPH_GSL_HISTOGRAM_FREQS "freqs"
#define OPH_GSL_HISTOGRAM_ALL "all"
#define OPH_GSL_HISTOGRAM_ABS_FREQS "abs"
#define OPH_GSL_HISTOGRAM_REL_FREQS "pdf"
#define OPH_GSL_HISTOGRAM_CUM_FREQS "cdf"
/* Modes */
#define OPH_GSL_HISTOGRAM_RANGES_MODE        1
#define OPH_GSL_HISTOGRAM_FREQS_MODE         2
#define OPH_GSL_HISTOGRAM_ALL_MODE           3
#define OPH_GSL_HISTOGRAM_ABS_FREQS_FLAG     4
#define OPH_GSL_HISTOGRAM_REL_FREQS_FLAG     5
#define OPH_GSL_HISTOGRAM_CUM_FREQS_FLAG     6

/* Extraspace */
typedef struct oph_gsl_histogram_extraspace {
    int mode;
    int flag;
    oph_type intype;
    gsl_histogram *hist;
} oph_gsl_histogram_extraspace;

/*------------------------------------------------------------------|
|       Functions' declarations (BEGIN)             |
|------------------------------------------------------------------*/

/* These must be right or mysqld will not find the symbol! */
my_bool oph_gsl_histogram_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void oph_gsl_histogram_deinit(UDF_INIT *initid);
char* oph_gsl_histogram(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);

/*------------------------------------------------------------------|
|               Functions' declarations (END)                       |
|------------------------------------------------------------------*/

