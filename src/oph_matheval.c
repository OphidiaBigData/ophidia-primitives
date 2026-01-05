/*
    Ophidia Primitives
    Copyright (C) 2012-2025 CMCC Foundation

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

#include "oph_matheval.h"

#include <pthread.h>

int msglevel = 1;

pthread_rwlock_t lock;

int core_oph_matheval(oph_stringPtr byte_array, char *result)
{
	unsigned long i;
	oph_matheval_param *_result = (oph_matheval_param *) result;
	switch (_result->is_index) {
		case 0:{
				double temporary;
				switch (byte_array->type) {
					case OPH_DOUBLE:
						for (i = 0; i < byte_array->numelem; ++i) {
							temporary = *((double *) (byte_array->content + i * byte_array->elemsize));
							if (!isnan(temporary) && (!byte_array->missingvalue || (temporary != _result->missingvalue)))
								temporary = evaluator_evaluate_x(_result->f[1], temporary);
							if (core_oph_type_cast
							    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
								pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
								return 1;
							}
						}
						break;
					case OPH_FLOAT:{
							float input, ms = byte_array->missingvalue ? (float) _result->missingvalue : 0;
							for (i = 0; i < byte_array->numelem; ++i) {
								input = *((float *) (byte_array->content + i * byte_array->elemsize));
								if (!isnan(input) && (!byte_array->missingvalue || (input != ms)))
									temporary = evaluator_evaluate_x(_result->f[1], input);
								else
									temporary = input;
								if (core_oph_type_cast
								    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type,
								     byte_array->missingvalue)) {
									pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
									return 1;
								}
							}
							break;
						}
					case OPH_INT:{
							int input, ms = byte_array->missingvalue ? (int) _result->missingvalue : 0;
							for (i = 0; i < byte_array->numelem; ++i) {
								input = *((int *) (byte_array->content + i * byte_array->elemsize));
								if (!byte_array->missingvalue || (input != ms))
									temporary = evaluator_evaluate_x(_result->f[1], input);
								else
									temporary = input;
								if (core_oph_type_cast
								    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type,
								     byte_array->missingvalue)) {
									pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
									return 1;
								}
							}
							break;
						}
					case OPH_SHORT:{
							short input, ms = byte_array->missingvalue ? (short) _result->missingvalue : 0;
							for (i = 0; i < byte_array->numelem; ++i) {
								input = *((short *) (byte_array->content + i * byte_array->elemsize));
								if (!byte_array->missingvalue || (input != ms))
									temporary = evaluator_evaluate_x(_result->f[1], input);
								else
									temporary = input;
								if (core_oph_type_cast
								    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type,
								     byte_array->missingvalue)) {
									pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
									return 1;
								}
							}
							break;
						}
					case OPH_BYTE:{
							char input, ms = byte_array->missingvalue ? (char) _result->missingvalue : 0;
							for (i = 0; i < byte_array->numelem; ++i) {
								input = *((char *) (byte_array->content + i * byte_array->elemsize));
								if (!byte_array->missingvalue || (input != ms))
									temporary = evaluator_evaluate_x(_result->f[1], input);
								else
									temporary = input;
								if (core_oph_type_cast
								    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type,
								     byte_array->missingvalue)) {
									pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
									return 1;
								}
							}
							break;
						}
					case OPH_LONG:{
							long long input, ms = byte_array->missingvalue ? (long long) _result->missingvalue : 0;
							for (i = 0; i < byte_array->numelem; ++i) {
								input = *((long long *) (byte_array->content + i * byte_array->elemsize));
								if (!byte_array->missingvalue || (input != ms))
									temporary = evaluator_evaluate_x(_result->f[1], input);
								else
									temporary = input;
								if (core_oph_type_cast
								    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type,
								     byte_array->missingvalue)) {
									pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
									return 1;
								}
							}
							break;
						}
					default:
						pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
						return -1;
				}
				break;
			}
		case 1:{
				long long temporary = 1L;
				for (i = 0; i < byte_array->numelem; ++i, ++temporary) {
					if (core_oph_type_cast(&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_LONG, _result->result_type, byte_array->missingvalue)) {
						pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
		default:
			for (i = 0; i < byte_array->numelem; ++i) {
				if (core_oph_type_cast
				    (byte_array->content + i * byte_array->elemsize, (((char *) _result->f[0]) + i * _result->result_elemsize), byte_array->type, _result->result_type,
				     byte_array->missingvalue)) {
					pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
	}
	return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_matheval_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	if ((args->arg_count < 3) || (args->arg_count > 5)) {
		strcpy(message, "ERROR: Wrong arguments! oph_matheval(input_OPH_TYPE, output_OPH_TYPE, measure, expression, [missingvalue])");
		return 1;
	}

	int i;
	for (i = 0; i < 4; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_matheval function");
			return 1;
		}
	}
	if (args->arg_count > 4) {
		if (args->arg_type[4] == STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_matheval function");
			return 1;
		}
		args->arg_type[4] = REAL_RESULT;
	}

	initid->ptr = NULL;
	return 0;
}

void oph_matheval_deinit(UDF_INIT *initid)
{
	int i;
	//Free allocated space
	if (initid->ptr) {
		if (((oph_matheval_param *) initid->ptr)->f[0]) {
			free(((oph_matheval_param *) initid->ptr)->f[0]);
			((oph_matheval_param *) initid->ptr)->f[0] = NULL;	// binary_array free
		}
		for (i = 1; i < 2; ++i)
			if (((oph_matheval_param *) initid->ptr)->f[i]) {
				evaluator_destroy(((oph_matheval_param *) initid->ptr)->f[i]);
				((oph_matheval_param *) initid->ptr)->f[i] = NULL;	// expressions free
			}
		free(initid->ptr);
		initid->ptr = NULL;
	}
}

char *oph_matheval(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
	oph_string measure;
	char *buffer;
	char **names;
	int count;
	oph_matheval_param *param;

	int i = 0;

	if (core_set_type(&(measure), args->args[0], &(args->lengths[0]))) {
		pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	measure.content = args->args[2];
	measure.length = &(args->lengths[2]);

	core_set_elemsize(&(measure));

	if (core_set_numelem(&(measure))) {
		pmesg(1, __FILE__, __LINE__, "Error on counting elements\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	if (!initid->ptr) {
		initid->ptr = (char *) malloc(sizeof(oph_matheval_param));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		param = (oph_matheval_param *) initid->ptr;

		for (i = 0; i < 2; ++i)
			param->f[i] = NULL;
		param->is_index = 0;
		param->missingvalue = NAN;
		if ((args->arg_count > 4) && (args->args[4]))
			param->missingvalue = *((double *) args->args[4]);

		if (args->arg_count >= 3) {
			buffer = (char *) malloc(1 + args->lengths[3]);
			strncpy(buffer, args->args[3], args->lengths[3]);
			buffer[args->lengths[3]] = '\0';
			if (strcasecmp(buffer, "INDEX")) {
				pthread_rwlock_wrlock(&lock);
				param->f[1] = evaluator_create(buffer);
				pthread_rwlock_unlock(&lock);
				free(buffer);
				if (!param->f[1]) {
					pmesg(1, __FILE__, __LINE__, "Error allocating evaluator\n");
					*length = 0;
					*is_null = 0;
					*error = 1;
					return NULL;
				}
				evaluator_get_variables(param->f[1], &names, &count);
				if (count > 1) {
					pmesg(1, __FILE__, __LINE__, "Too variables in expression\n");
					*length = 0;
					*is_null = 0;
					*error = 1;
					return NULL;
				}
			} else {
				param->is_index = 1;
				free(buffer);
			}
		} else
			param->is_index = -1;

		oph_string output_array;
		core_set_type(&output_array, args->args[1], &(args->lengths[1]));
		if (!output_array.type) {
			pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (core_set_elemsize(&output_array)) {
			pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		param->result_type = output_array.type;
		param->result_elemsize = output_array.elemsize;

		param->length = output_array.elemsize * measure.numelem;
		param->f[0] = malloc(param->length);
		if (!param->f[0]) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	} else
		param = (oph_matheval_param *) initid->ptr;

	measure.missingvalue = (args->arg_count > 4) && (args->args[4]) ? &param->missingvalue : NULL;

	i = core_oph_matheval(&measure, initid->ptr);
	if (i) {
		pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	*length = param->length;
	*error = 0;
	*is_null = 0;
	return (char *) param->f[0];

}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
