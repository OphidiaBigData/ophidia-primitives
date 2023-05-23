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

#include "oph_concat2.h"

int msglevel = 1;

int core_oph_concat2_multi(oph_generic_param_multi *param)
{
	int i, j, k = 0;
	oph_multistring *measure, *result = param->result;
	char *ic, *oc = result->content;
	do			// Loop on input measures
	{
		measure = param->measure + k;
		ic = measure->content;
		for (i = 0; i < measure->num_measure; ++i)	// Loop on data types
		{
			for (j = 0; j < measure->numelem; ++j)	// Loop on elements
			{
				if (core_oph_type_cast(ic + j * measure->blocksize, oc + j * result->blocksize, measure->type[i], result->type[i], NULL)) {
					pmesg(1, __FILE__, __LINE__, "Error in compute array\n");
					return 1;
				}
			}
			ic += measure->elemsize[i];
			oc += result->elemsize[i];
		}
		oc += (measure->numelem - 1) * result->blocksize;

		if (k)
			break;
		k = (int) measure->param;
	}
	while (k);

	return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_concat2_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	if (args->arg_count < 3) {
		strcpy(message, "ERROR: Wrong arguments! oph_concat2(input_OPH_TYPE, output_OPH_TYPE, measure, ...)");
		return 1;
	}

	int i;
	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_concat2 function");
			return 1;
		}
	}

	initid->ptr = NULL;

	return 0;
}

void oph_concat2_deinit(UDF_INIT *initid)
{
	//Free allocated space
	if (initid->ptr) {
		free_oph_generic_param_multi((oph_generic_param_multi *) initid->ptr);
		initid->ptr = NULL;
	}
}

char *oph_concat2(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
	int i;

	if (*error) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	if (*is_null) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
	}
	for (i = 2; i < args->arg_count; ++i) {
		if (!args->lengths[i]) {
			*length = 0;
			*is_null = 1;
			*error = 0;
			return NULL;
		}
	}

	oph_generic_param_multi *param;
	if (!initid->ptr) {
		param = (oph_generic_param_multi *) malloc(sizeof(oph_generic_param_multi));
		if (!param) {
			pmesg(1, __FILE__, __LINE__, "Error in allocating parameters\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		param->measure = NULL;
		param->result = NULL;
		param->error = 0;
		param->core_oph_oper = NULL;

		initid->ptr = (char *) param;
	} else
		param = (oph_generic_param_multi *) initid->ptr;

	if (param->error) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	unsigned long numelem_total = 0;
	oph_multistring *measure;
	if (!param->error && !param->measure) {
		if (core_set_oph_multistring(&measure, args->args[0], &args->lengths[0])) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		int j;
		for (i = 0; i < args->arg_count - 2; ++i) {
			measure[i].length = args->lengths[2 + i];
			if ((measure[i].length % measure[i].blocksize) || (measure[i].islast != (i < args->arg_count - 3 ? 0 : 1))) {
				param->error = 1;
				pmesg(1, __FILE__, __LINE__, "Wrong input type or data corrupted\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			measure[i].numelem = measure[i].length / measure[i].blocksize;
			if (i <= 1)
				numelem_total += measure[i].numelem;	// Almost two measures are considered
			if (measure->num_measure != measure[i].num_measure) {
				param->error = 1;
				pmesg(1, __FILE__, __LINE__, "Wrong input data types\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			} else
				for (j = 0; j < measure->num_measure; ++j) {
					if (measure[i].type[j] != measure->type[j]) {
						param->error = 1;
						pmesg(1, __FILE__, __LINE__, "Data types of input arrays are different\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
		}
		measure->param = args->arg_count > 3 ? 1 : 0;

		param->measure = measure;
	} else {
		measure = param->measure;
		if (args->arg_count > 3) {
			measure->param++;
			if (measure->param > args->arg_count - 3)
				measure->param = 1;
		}
	}

	for (i = 0; i < args->arg_count - 2; ++i)
		measure[i].content = args->args[2 + i];

	oph_multistring *output;
	if (!param->error && !param->result) {
		if (core_set_oph_multistring(&output, args->args[1], &(args->lengths[1]))) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		output->numelem = numelem_total;
		output->length = output->numelem * output->blocksize;
		if (!output->length) {
			*length = 0;
			*is_null = 1;
			*error = 0;
			return NULL;
		}
		output->content = (char *) malloc(output->length);
		if (!output->content) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Error allocating measures string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		param->result = output;
	} else
		output = param->result;

	if (!param->error && core_oph_concat2_multi(param)) {
		param->error = 1;
		pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	*length = output->length;
	*error = 0;
	*is_null = 0;

	return (result = output->content);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
