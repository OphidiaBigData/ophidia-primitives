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

#include "oph_to_bit.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_to_bit_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	if (args->arg_count != 3) {
		strcpy(message, "ERROR: Wrong arguments! oph_to_bit(input_OPH_TYPE, output_OPH_TYPE, bit_string)");
		return 1;
	}

	int i;
	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_to_bit function");
			return 1;
		}
	}

	initid->ptr = NULL;

	return 0;
}

void oph_to_bit_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		oph_bit_param *param = (oph_bit_param *) initid->ptr;
		if (param->measure) {
			free(param->measure);
			param->measure = NULL;
		}
		if (param->result) {
			free(param->result);
			param->result = NULL;
		}
		free(initid->ptr);
		initid->ptr = NULL;
	}
}

char *oph_to_bit(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	int res = 0;
	oph_bit_param *param;
	oph_bit_string *measure;

	if (!initid->ptr) {
		initid->ptr = (char *) malloc(sizeof(oph_bit_param));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_bit_param\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		param = (oph_bit_param *) initid->ptr;
		param->measure = NULL;
		param->result = NULL;
	} else
		param = (oph_bit_param *) initid->ptr;

	if (!param->measure) {
		measure = param->measure = (oph_bit_string *) malloc(sizeof(oph_bit_string));
		if (!param->measure) {
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_bit_string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		measure->numelem = args->lengths[2];	// Const
		param->length = core_oph_bit_length(args->lengths[2]);	// bytes
		if (!param->length) {
			*length = 0;
			*is_null = 1;
			*error = 0;
			return NULL;
		}

		param->result = (char *) malloc(param->length);	// Output
		if (!param->result) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	} else
		measure = (oph_bit_string *) param->measure;

	measure->content = args->args[2];	// Input
	if (!measure->content) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
	}

	res = core_oph_bit_parse(measure->content, param->result, measure->numelem);
	if (res) {
		pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
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
