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

#include "oph_bit_find.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_bit_find_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	if (args->arg_count != 4) {
		strcpy(message, "ERROR: Wrong arguments! oph_bit_find(input_OPH_TYPE, output_OPH_TYPE, bit_measure, bit_pattern)");
		return 1;
	}

	int i;
	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_bit_find function");
			return 1;
		}
	}

	initid->ptr = NULL;

	return 0;
}

void oph_bit_find_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		oph_bit_param2 *param = (oph_bit_param2 *) initid->ptr;
		if (param->measureA) {
			free(param->measureA);
			param->measureA = NULL;
		}
		if (param->measureB) {
			oph_bit_string *pattern = param->measureB;
			if (pattern->content) {
				free(pattern->content);
				pattern->content = NULL;
			}
			free(param->measureB);
			param->measureB = NULL;
		}
		if (param->result) {
			free(param->result);
			param->result = NULL;
		}
		free(initid->ptr);
		initid->ptr = NULL;
	}
}

char *oph_bit_find(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	int res = 0;
	oph_bit_param2 *param;
	oph_bit_string *measure;
	oph_bit_string *pattern;

	if (!initid->ptr) {
		initid->ptr = (char *) malloc(sizeof(oph_bit_param2));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_bit_param2\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		param = (oph_bit_param2 *) initid->ptr;
		param->measureA = NULL;
		param->measureB = NULL;
		param->result = NULL;
	} else
		param = (oph_bit_param2 *) initid->ptr;

	// It should be changed in case padding is considered - a bit inefficient
	if (!args->lengths[2] || !args->lengths[3]) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
	}

	if (!param->measureA || !param->measureB) {
		measure = param->measureA = (oph_bit_string *) malloc(sizeof(oph_bit_string));
		if (!param->measureA) {
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_bit_string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		pattern = param->measureB = (oph_bit_string *) malloc(sizeof(oph_bit_string));
		if (!param->measureB) {
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_bit_string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		param->length = args->lengths[2] * OPH_BITSBYTE * core_sizeof(OPH_LONG);	// bytes

		measure->numelem = core_oph_bit_numelem(args->lengths[2]);	// Const

		/* Initial implementation with pattern as bit measure
		   pattern->numelem = core_oph_bit_numelem(args->lengths[3]); // Const
		 */
		pattern->numelem = args->lengths[3];	// Const
		pattern->content = (char *) malloc(core_oph_bit_length(pattern->numelem));

		param->result = (char *) malloc(param->length);	// Output
		if (!param->result) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	} else {
		measure = param->measureA;
		pattern = param->measureB;
	}

	measure->content = args->args[2];	// Input A
	/* Initial implementation woth pattern as bit measure
	   pattern->content = args->args[3]; // Input B
	 */

	res = core_oph_bit_parse(args->args[3], pattern->content, pattern->numelem);	// Input B
	if (res) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
	}

	if (!measure->content || !pattern->content) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
	}

	res = core_oph_bit_find(param);
	if (res) {
		pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
		*length = 0;
		*is_null = 1;
		*error = 1;
		return NULL;
	}

	if (!param->length) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
	}

	*length = param->length;
	*error = 0;
	*is_null = 0;
	return (result = param->result);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
