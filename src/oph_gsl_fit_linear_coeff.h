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

/*/ Standard C headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* GSL Core function header */
//#include "oph_core.h"
#include "oph_core_gsl.h"

/* MySQL headers  */
#include <mysql.h> // It contains UDF-related symbols and data structures

/* GSL STATS headers  */
#include <gsl/gsl_fit.h>

#define OPH_GSL_FIT_LINEAR_COEFF_NUMBER 6

typedef struct {
	double c[OPH_GSL_FIT_LINEAR_COEFF_NUMBER];
	size_t n;
	double *old_x;
	double *tmp;
	short flag[OPH_GSL_FIT_LINEAR_COEFF_NUMBER];
} oph_gsl_fit_linear_coeff_param;

/*------------------------------------------------------------------|
|		Functions' declarations (BEGIN)			    |
|------------------------------------------------------------------*/

/* These must be right or mysqld will not find the symbol! */
my_bool oph_gsl_fit_linear_coeff_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void oph_gsl_fit_linear_coeff_deinit(UDF_INIT *initid);
char* oph_gsl_fit_linear_coeff(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);

/*------------------------------------------------------------------|
|               Functions' declarations (END)                       |
|------------------------------------------------------------------*/
