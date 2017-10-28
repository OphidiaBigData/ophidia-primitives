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

#include "oph_mul_scalar.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_mul_scalar_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if (args->arg_count < 3 || args->arg_count > 5) {
		strcpy(message, "ERROR: Wrong arguments! oph_mul_scalar(input_OPH_TYPE, output_OPH_TYPE, measure, [scalar, id_measure])");
		return 1;
	}

	for (i = 0; i < 3; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_mul_scalar function");
			return 1;
		}
	}

	if (args->arg_count > 3) {
		if (args->arg_type[3] == STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_mul_scalar function");
			return 1;
		}
		args->arg_type[3] = REAL_RESULT;
		if (args->arg_count > 4) {
			if (args->arg_type[4] == STRING_RESULT) {
				strcpy(message, "ERROR: Wrong arguments to oph_mul_scalar function");
				return 1;
			}
			args->arg_type[4] = INT_RESULT;
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;
	return 0;
}

void oph_mul_scalar_deinit(UDF_INIT * initid)
{
	//Free allocated space
	oph_multistring *multimeasure;
	if (initid->ptr) {
		multimeasure = (oph_multistring *) (initid->ptr);
		free(multimeasure->content);
		multimeasure->content = NULL;
		free_oph_multistring(multimeasure);
		multimeasure = NULL;
		initid->ptr = NULL;
	}
	if (initid->extension) {
		multimeasure = (oph_multistring *) (initid->extension);
		free_oph_multistring(multimeasure);
		multimeasure = NULL;
		initid->extension = NULL;
	}
}

char *oph_mul_scalar(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	oph_multistring *multim;
	oph_multistring *output;

	int id = 0;
	double scalar = 0;
	if (args->arg_count > 3)
		scalar = *((double *) args->args[3]);

	if (!initid->extension) {
		if (core_set_oph_multistring((oph_multistring **) (&(initid->extension)), args->args[0], &(args->lengths[0]))) {
			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	if (!initid->ptr) {
		if (core_set_oph_multistring(((oph_multistring **) (&(initid->ptr))), args->args[1], &(args->lengths[1]))) {
			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		//Check
		if (((oph_multistring *) (initid->ptr))->num_measure != ((oph_multistring *) (initid->extension))->num_measure) {
			pmesg(1, __FILE__, __LINE__, "Wrong input or output type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}
	multim = (oph_multistring *) (initid->extension);
	multim->content = args->args[2];
	multim->length = args->lengths[2];

	//Check
	if (multim->length % multim->blocksize) {
		pmesg(1, __FILE__, __LINE__, "Wrong input type or data corrupted\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	multim->numelem = multim->length / multim->blocksize;
	output = (oph_multistring *) (initid->ptr);
	output->numelem = multim->numelem;

	if (args->arg_count > 4) {
		id = *((long long *) args->args[4]);
		if ((id < 0) || (id > ((oph_multistring *) (initid->ptr))->num_measure)) {
			pmesg(1, __FILE__, __LINE__, "Wrong measure identifier. Correct values are integers in [1, %d].\n", ((oph_multistring *) (initid->ptr))->num_measure);
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	/* Allocate the right space for the result set */
	if (!output->content) {
		output->content = (char *) calloc(1, (output->numelem) * (output->blocksize));
		if (!output->content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating measures string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	if (core_oph_mul_scalar_multi(multim, scalar, output, id)) {
		pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
		*length = 0;
		*is_null = 1;
		*error = 1;
		return NULL;
	}
	*length = (output->numelem) * (output->blocksize);
	*error = 0;
	*is_null = 0;
	return (result = ((oph_multistring *) initid->ptr)->content);

}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
