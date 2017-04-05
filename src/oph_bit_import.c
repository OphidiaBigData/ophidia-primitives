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

#include "oph_bit_import.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_bit_import_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	if (args->arg_count != 3) {
		strcpy(message, "ERROR: Wrong arguments! oph_bit_import(input_OPH_TYPE, output_OPH_TYPE, measure)");
		return 1;
	}

	int i;
	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_bit_import function");
			return 1;
		}
	}

	initid->ptr = NULL;

	return 0;
}

void oph_bit_import_deinit(UDF_INIT * initid)
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

char *oph_bit_import(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	int res = 0;
	unsigned long bit_measure_length;
	oph_bit_param *param;
	oph_string *measure;

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
		measure = param->measure = (oph_string *) malloc(sizeof(oph_string));
		if (!param->measure) {
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		res = core_set_type(measure, args->args[0], &(args->lengths[0]));	// Const
		if (res) {
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		res = core_set_elemsize(measure);	// Const
		if (res) {
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		measure->length = &(args->lengths[2]);	// Variable, but used only to set numelem in the next sentence

		if (core_set_numelem(measure))	// Const
		{
			pmesg(1, __FILE__, __LINE__, "Error on counting elements\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		param->length = core_oph_bit_length(measure->numelem);	// bytes
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
		measure = (oph_string *) param->measure;

	measure->content = args->args[2];	// Input
	if (!measure->content) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
	}

	res = core_oph_bit_import(param);
	if (res) {
		pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
		*length = 0;
		*is_null = 1;
		*error = 1;
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
