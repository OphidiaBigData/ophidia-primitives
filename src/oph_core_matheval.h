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

/*/ Standard C headers */
#ifndef __OPH_CORE_MATHEVAL__
#define __OPH_CORE_MATHEVAL__

#include "oph_core.h"

/* libmatheval headers */
#include <matheval.h>

typedef enum { INVALID_COMP, OPH_GREATER_THAN_ZERO, OPH_LESS_THAN_ZERO, OPH_GREATER_OR_EQUAL_TO_ZERO, OPH_LESS_OR_EQUAL_TO_ZERO, OPH_EQUAL_TO_ZERO, OPH_NOT_EQUAL_TO_ZERO, OPH_NULL } oph_comp;

#define DEFAULT_COMP OPH_NOT_EQUAL_TO_ZERO

typedef struct {
	void *f[4];		// measure and expressions
	oph_comp op;		// comparison operator
	unsigned long length;	// size in bytes
	oph_type result_type;
	size_t result_elemsize;
} oph_predicate_param;


// Set/Get the operator in enum oph_comp form
int core_set_comp(oph_comp * op, char *oper, unsigned long *len);
oph_comp core_get_comp(char *type, unsigned long *len);

// oph_predicate: the result depends on the expressions stored in result
int core_oph_predicate(oph_stringPtr byte_array, char *result);

#endif
