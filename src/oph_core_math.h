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

/*/ Standard C headers */
#ifndef __OPH_CORE_MATH__
#define __OPH_CORE_MATH__

#include <math.h>
#ifdef GSL_SUPPORTED
#include "oph_core_gsl.h"
#else
#include "oph_core.h"
#endif

#define OPER_LEN 40

typedef enum oph_math_oper { INVALID_MATH_OPER, OPH_MATH_ABS, OPH_MATH_ACOS, OPH_MATH_ASIN,
	OPH_MATH_ATAN, OPH_MATH_CEIL, OPH_MATH_COS, OPH_MATH_COT, OPH_MATH_DEGREES,
	OPH_MATH_EXP, OPH_MATH_FLOOR, OPH_MATH_LN, OPH_MATH_LOG10, OPH_MATH_LOG2,
	OPH_MATH_RADIANS, OPH_MATH_RAND, OPH_MATH_ROUND, OPH_MATH_SIGN, OPH_MATH_SIN, OPH_MATH_SQRT, OPH_MATH_TAN,
	OPH_MATH_GSL_COMPLEX_CONJUGATE, OPH_MATH_GSL_COMPLEX_INVERSE,
	OPH_MATH_GSL_COMPLEX_NEGATIVE, OPH_MATH_GSL_COMPLEX_SQRT,
	OPH_MATH_GSL_COMPLEX_EXP, OPH_MATH_GSL_COMPLEX_LOG,
	OPH_MATH_GSL_COMPLEX_LOG10, OPH_MATH_GSL_COMPLEX_SIN,
	OPH_MATH_GSL_COMPLEX_COS, OPH_MATH_GSL_COMPLEX_TAN,
	OPH_MATH_GSL_COMPLEX_SEC, OPH_MATH_GSL_COMPLEX_CSC,
	OPH_MATH_GSL_COMPLEX_COT, OPH_MATH_GSL_COMPLEX_ASIN,
	OPH_MATH_GSL_COMPLEX_ACOS, OPH_MATH_GSL_COMPLEX_ATAN,
	OPH_MATH_GSL_COMPLEX_ASEC, OPH_MATH_GSL_COMPLEX_ACSC,
	OPH_MATH_GSL_COMPLEX_ACOT, OPH_MATH_GSL_COMPLEX_SINH,
	OPH_MATH_GSL_COMPLEX_COSH, OPH_MATH_GSL_COMPLEX_TANH,
	OPH_MATH_GSL_COMPLEX_SECH, OPH_MATH_GSL_COMPLEX_CSCH,
	OPH_MATH_GSL_COMPLEX_COTH, OPH_MATH_GSL_COMPLEX_ASINH,
	OPH_MATH_GSL_COMPLEX_ACOSH, OPH_MATH_GSL_COMPLEX_ATANH,
	OPH_MATH_GSL_COMPLEX_ASECH, OPH_MATH_GSL_COMPLEX_ACSCH,
	OPH_MATH_GSL_COMPLEX_ACOTH
} oph_math_oper;

/*------------------------------------------------------------------|
 |               Functions declarations (BEGIN)                     |
 |------------------------------------------------------------------*/

int core_set_math_oper(oph_math_oper * operation, char *oper, unsigned long *len);

// Get the operator in enum oph_oper form
oph_math_oper core_get_math_oper(char *oper, unsigned long *len);


// Return the maximum value in an array (as result string)
int core_oph_math(oph_stringPtr byte_array, oph_math_oper * operation, oph_stringPtr output);

/*------------------------------------------------------------------|
 |               Functions declarations (END)                       |
 |------------------------------------------------------------------*/

#endif
