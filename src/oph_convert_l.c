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

#include "oph_convert_l.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_convert_l_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	/* oph_convert_l(single_binary_(long, int)_value, OPH_TYPE) */
	int i = 0;
	if (args->arg_count != 3) {
		strcpy(message, "ERROR: Wrong arguments! oph_convert_l(input_OPH_TYPE, output_OPH_TYPE, measure)");
		return 1;
	}

	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_convert_l function");
			return 1;
		}
	}
	initid->ptr = NULL;
	return 0;
}

void oph_convert_l_deinit(UDF_INIT * initid)
{
	if (initid->ptr) {
		free(initid->ptr);
		initid->ptr = NULL;
	}
}

long long oph_convert_l(UDF_INIT * initid, UDF_ARGS * args, char *is_null, char *error)
{
	oph_string *measure;

	if (*error) {
		*is_null = 0;
		*error = 1;
		return -1;
	}
	if (*is_null) {
		*is_null = 1;
		*error = 0;
		return -1;
	}


	long long result;

	if (!initid->ptr) {
		initid->ptr = (void *) malloc(sizeof(oph_string));
		measure = (oph_string *) initid->ptr;
		core_set_type(measure, args->args[0], &(args->lengths[0]));

		if (!measure->type || (measure->type != OPH_BYTE && measure->type != OPH_SHORT && measure->type != OPH_INT && measure->type != OPH_LONG)) {
			pmesg(1, __FILE__, __LINE__, "Invalid type\n");
			*is_null = 1;
			*error = 1;
			return -1;
		}

		if (core_set_elemsize(measure)) {
			pmesg(1, __FILE__, __LINE__, "Error on setting elements size\n");
			*is_null = 1;
			*error = 1;
			return -1;
		}
	} else
		measure = (oph_string *) initid->ptr;

	measure->content = args->args[2];
	measure->length = &(args->lengths[2]);
	if (core_set_numelem(measure)) {
		pmesg(1, __FILE__, __LINE__, "Error on counting elements\n");
		*is_null = 1;
		*error = 1;
		return -1;
	}


	if (measure->numelem > 1) {
		pmesg(1, __FILE__, __LINE__, "Allowed only one numeric value or wrong input type\n");
		*is_null = 1;
		*error = 1;
		return -1;
	}

	switch (measure->type) {
		case OPH_INT:{
				int tmp;
				core_oph_convert(measure, (void *) (&tmp));
				result = (long long) tmp;
				break;
			}
		case OPH_SHORT:{
				short tmp;
				core_oph_convert(measure, (void *) (&tmp));
				result = (long long) tmp;
				break;
			}
		case OPH_BYTE:{
				char tmp;
				core_oph_convert(measure, (void *) (&tmp));
				result = (long long) tmp;
				break;
			}
		case OPH_LONG:{
				long long tmp;
				core_oph_convert(measure, (void *) (&tmp));
				result = (long long) tmp;
				break;
			}
		default:
			pmesg(1, __FILE__, __LINE__, "Invalid type\n");
			*is_null = 1;
			*error = 1;
			return -1;
	}

	*error = 0;
	*is_null = 0;
	return result;

}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
