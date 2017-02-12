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
#ifndef __OPH_CORE_SUBSET__
#define __OPH_CORE_SUBSET__

#include "oph_core.h"

// Subset library

#define OPH_SUBSET_MAX_STRING_LENGTH 1024
#define OPH_SUBSET_OK 0
#define OPH_SUBSET_DATA_ERR 1
#define OPH_SUBSET_NULL_POINTER_ERR 2
#define OPH_SUBSET_SYSTEM_ERR 3

#define OPH_SUBSET_PARAM_SEPARATOR ":"
#define OPH_SUBSET_SUBSET_SEPARATOR ","
#define OPH_SUBSET_PARAM_END "end"
#define OPH_SUBSET_ALL "1:end"

typedef enum { OPH_SUBSET_SINGLE, OPH_SUBSET_INTERVAL, OPH_SUBSET_STRIDE } oph_subset_type;

typedef struct {
	oph_subset_type *type;
	unsigned long *start;
	unsigned long *end;
	unsigned long *stride;
	unsigned long *count;
	unsigned long total;
	unsigned int number;	// Number of intervals
} oph_subset;			// List of subsets in the form <start>:<stride>:<max>

// Initialization of struct oph_subset
int oph_subset_init(oph_subset ** subset);

// Not implemented yet
int oph_subset_validate(const char *cond, unsigned long len);

// Translate non-null-terminated string into an oph_subset struct. Set 'max' to 0 to avoid truncation to 'max' elements
int oph_subset_parse(const char *cond, unsigned long len, oph_subset * subset, unsigned long max);

// Freeing the struct oph_subset
int oph_subset_free(oph_subset * subset);

// Freeing the struct oph_subset
int oph_subset_vector_free(oph_subset ** subset, int number);

// Return 1 if an index is a subset
int oph_subset_is_in_subset(unsigned long index, unsigned long start, unsigned long stride, unsigned long end);

// Translate id to index given the sizes of quicker dimensions
unsigned long oph_subset_id_to_index(unsigned long id, unsigned long *sizes, int n);
// Translate id to index given the sizes of quicker dimensions
unsigned long oph_subset_id_to_index2(unsigned long id, unsigned long block_size, unsigned long size);

// Translate an oph_subset struct into a flag array
int oph_subset_set_flags(oph_subset * subset, char *flags, unsigned long size, unsigned long *total, unsigned long *sizes, int size_number);

#endif
