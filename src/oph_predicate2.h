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

#include "oph_core_matheval.h"

/* MySQL headers  */
#include <mysql.h>		// It contains UDF-related symbols and data structures

#define OPH_PREDICATE2_ALL_OCCURRENCE "all"
#define OPH_PREDICATE2_FIRST_OCCURRENCE "first"
#define OPH_PREDICATE2_BEGIN_OCCURRENCE "begin"
#define OPH_PREDICATE2_LAST_OCCURRENCE "last"
#define OPH_PREDICATE2_END_OCCURRENCE "end"

typedef struct {
	void *f[4];		// measure and expressions
	char is_index[3];	// is_index
	oph_comp op;		// comparison operator
	unsigned long length;	// size in bytes
	oph_type result_type;
	size_t result_elemsize;
	long occurrence;
} oph_predicate2_param;

/*------------------------------------------------------------------|
|		Functions' declarations (BEGIN)			    |
|------------------------------------------------------------------*/

/* These must be right or mysqld will not find the symbol! */
my_bool oph_predicate2_init(UDF_INIT * initid, UDF_ARGS * args, char *message);
void oph_predicate2_deinit(UDF_INIT * initid);
char *oph_predicate2(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error);

/*------------------------------------------------------------------|
|               Functions' declarations (END)                       |
|------------------------------------------------------------------*/
