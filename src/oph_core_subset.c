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

#include "oph_core_subset.h"

// This internal function differs from core_strncpy only in terms of max buffer size dimension
int oph_subset_strncpy(char *buff, const char *str, unsigned long len)
{
	if (len > OPH_SUBSET_MAX_STRING_LENGTH) {
		pmesg(1, __FILE__, __LINE__, "Invalid string (exceedes buffer length)\n");
		return -1;
	}
	strncpy(buff, str, len);
	buff[len] = '\0';
	return 0;
}

void _oph_subset_free(oph_subset * subset)
{
	if (subset->type) {
		free(subset->type);
		subset->type = 0;
	}
	if (subset->start) {
		free(subset->start);
		subset->start = 0;
	}
	if (subset->end) {
		free(subset->end);
		subset->end = 0;
	}
	if (subset->stride) {
		free(subset->stride);
		subset->stride = 0;
	}
	if (subset->count) {
		free(subset->count);
		subset->count = 0;
	}
}

int oph_subset_init(oph_subset ** subset)
{
	if (!subset) {
		pmesg(1, __FILE__, __LINE__, "Null pointer\n");
		return OPH_SUBSET_NULL_POINTER_ERR;
	}
	*subset = (oph_subset *) malloc(sizeof(oph_subset));
	if (!(*subset)) {
		pmesg(1, __FILE__, __LINE__, "Error in allocating oph_subset\n");
		return OPH_SUBSET_SYSTEM_ERR;
	}
	return OPH_SUBSET_OK;
}

int oph_subset_validate(const char *cond, unsigned long len)
{
	pmesg(2, __FILE__, __LINE__, "Unsupported function 'oph_subset_validate'\n");
	return OPH_SUBSET_OK;
}

int oph_subset_parse(const char *cond, unsigned long len, oph_subset * subset, unsigned long max)
{
	char *result, *result2, temp0[OPH_SUBSET_MAX_STRING_LENGTH], temp1[OPH_SUBSET_MAX_STRING_LENGTH], temp2[OPH_SUBSET_MAX_STRING_LENGTH], *next, *temp, *savepointer;
	unsigned int number;

	if (!subset) {
		pmesg(1, __FILE__, __LINE__, "Null pointer\n");
		return OPH_SUBSET_NULL_POINTER_ERR;
	}

	subset->number = 0;
	oph_subset_strncpy(temp0, cond, len);
	strcpy(temp2, temp0);
	result = strtok_r(temp0, OPH_SUBSET_SUBSET_SEPARATOR, &savepointer);
	while (result) {
		subset->number++;
		result = strtok_r(NULL, OPH_SUBSET_SUBSET_SEPARATOR, &savepointer);
	}

	if (!subset->number)
		return OPH_SUBSET_DATA_ERR;

	int retval = OPH_SUBSET_OK, i = 0;

	subset->type = (oph_subset_type *) malloc(subset->number * sizeof(oph_subset_type));
	subset->start = (unsigned long *) malloc(subset->number * sizeof(unsigned long));
	subset->end = (unsigned long *) malloc(subset->number * sizeof(unsigned long));
	subset->stride = (unsigned long *) malloc(subset->number * sizeof(unsigned long));
	subset->count = (unsigned long *) malloc(subset->number * sizeof(unsigned long));
	subset->total = 0;

	next = temp2;
	result = strchr(temp2, OPH_SUBSET_SUBSET_SEPARATOR[0]);

	while (next && (retval == OPH_SUBSET_OK)) {
		if (result) {
			result[0] = '\0';
			temp = result + 1;
		} else
			temp = 0;
		result = next;
		next = temp;

		number = 0;
		strncpy(temp1, result, OPH_SUBSET_MAX_STRING_LENGTH);
		result2 = strtok_r(temp1, OPH_SUBSET_PARAM_SEPARATOR, &savepointer);
		while (result2 && (retval == OPH_SUBSET_OK)) {
			switch (number) {
				case 0:
					if (!strncasecmp(result2, OPH_SUBSET_PARAM_END, strlen(OPH_SUBSET_PARAM_END))) {
						if (max)
							subset->end[i] = max;
						else {
							pmesg(1, __FILE__, __LINE__, "Clause '%s' cannot be used in this context\n", OPH_SUBSET_PARAM_END);
							retval = OPH_SUBSET_DATA_ERR;
						}
					} else
						subset->end[i] = atol(result2);
					subset->start[i] = subset->end[i];
					subset->stride[i] = 1;
					subset->type[i] = OPH_SUBSET_SINGLE;
					break;
				case 1:
					if (!strncasecmp(result2, OPH_SUBSET_PARAM_END, strlen(OPH_SUBSET_PARAM_END))) {
						if (max)
							subset->end[i] = max;
						else {
							pmesg(1, __FILE__, __LINE__, "Clause '%s' cannot be used in this context\n", OPH_SUBSET_PARAM_END);
							retval = OPH_SUBSET_DATA_ERR;
						}
					} else
						subset->end[i] = atol(result2);
					subset->type[i] = OPH_SUBSET_INTERVAL;
					break;
				case 2:
					subset->stride[i] = subset->end[i];
					if (!strncasecmp(result2, OPH_SUBSET_PARAM_END, strlen(OPH_SUBSET_PARAM_END))) {
						if (max)
							subset->end[i] = max;
						else {
							pmesg(1, __FILE__, __LINE__, "Clause '%s' cannot be used in this context\n", OPH_SUBSET_PARAM_END);
							retval = OPH_SUBSET_DATA_ERR;
						}
					} else
						subset->end[i] = atol(result2);
					if (subset->stride[i] > 1)
						subset->type[i] = OPH_SUBSET_STRIDE;
					break;
				default:
					pmesg(1, __FILE__, __LINE__, "Wrong input data: too many '%s' in subset\n", OPH_SUBSET_PARAM_SEPARATOR);
					retval = OPH_SUBSET_DATA_ERR;
			}
			number++;
			result2 = strtok_r(NULL, OPH_SUBSET_PARAM_SEPARATOR, &savepointer);
		}
		if (retval != OPH_SUBSET_OK)
			break;

		if (!number) {
			subset->type[i] = OPH_SUBSET_INTERVAL;
			subset->start[i] = subset->stride[i] = 1;
			if (max)
				subset->end[i] = max;
			else
				subset->end[i] = subset->start[i];
		}

		if (!subset->stride[i] || (subset->start[i] <= 0) || (subset->start[i] > subset->end[i]) || (max && (subset->end[i] > max))) {
			pmesg(1, __FILE__, __LINE__, "Wrong input data: 'start', 'stop' or 'step' parameters are not correctly set\n");
			retval = OPH_SUBSET_DATA_ERR;
			break;
		}
		subset->start[i]--;
		subset->end[i]--;	// Translation to C-like indexing
		subset->count[i] = 1 + (subset->end[i] - subset->start[i]) / subset->stride[i];
		subset->total += subset->count[i];
		++i;
		if (next)
			result = strchr(next, OPH_SUBSET_SUBSET_SEPARATOR[0]);
	}

	if (retval != OPH_SUBSET_OK)
		_oph_subset_free(subset);

	return retval;
}

int oph_subset_free(oph_subset * subset)
{
	if (subset) {
		_oph_subset_free(subset);
		free(subset);
	}
	return OPH_SUBSET_OK;
}

int oph_subset_vector_free(oph_subset ** subset, int number)
{
	if (subset) {
		int n;
		for (n = 0; n < number; ++n)
			oph_subset_free(subset[n]);
		free(subset);
	}
	return OPH_SUBSET_OK;
}

int oph_subset_is_in_subset(unsigned long index, unsigned long start, unsigned long stride, unsigned long end)
{
	return (!((index - start) % stride) && (index >= start) && (index <= end));
}

unsigned long oph_subset_id_to_index(unsigned long id, unsigned long *sizes, int n)
{
	int i;
	unsigned long index = 0;
	for (i = 0; i < n; ++i) {
		index = id % sizes[i];
		id = (id - index) / sizes[i];
	}
	return index;
}

unsigned long oph_subset_id_to_index2(unsigned long id, unsigned long block_size, unsigned long size)
{
	return (id / block_size) % size;
}

int oph_subset_set_flags(oph_subset * subset, char *flags, unsigned long size, unsigned long *total, unsigned long *sizes, int size_number)
{
	if (!subset || !flags || !size || !total) {
		pmesg(1, __FILE__, __LINE__, "Null pointer\n");
		return OPH_SUBSET_NULL_POINTER_ERR;
	}
	if (!sizes || !size_number) {
		sizes = &size;
		size_number = 1;
	}

	unsigned long i, j;
	*total = 0;
	for (j = 0; j < size; ++j) {
		for (i = 0; i < subset->number; ++i)
			if (oph_subset_is_in_subset(oph_subset_id_to_index(j, sizes, size_number), subset->start[i], subset->stride[i], subset->end[i]))
				break;
		if ((flags[j] = i < subset->number))
			(*total)++;
	}
	return OPH_SUBSET_OK;
}
