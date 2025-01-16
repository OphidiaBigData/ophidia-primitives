/*
    Ophidia Primitives
    Copyright (C) 2012-2023 CMCC Foundation

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

#include "oph_expand.h"

int msglevel = 1;

int core_oph_expand_multi(oph_multistring * byte_array, oph_multistring * result, int id)
{
	int i, j, k, js, je;
	double current;
	char nan[sizeof(double)];
	char *in_string = byte_array->content, *out_string = result->content;

	oph_expand_param *param = byte_array->extend;

	if (id <= 0) {
		js = 0;
		je = byte_array->num_measure;
	} else if (id > byte_array->num_measure) {
		pmesg(1, __FILE__, __LINE__, "Index out of boundaries\n");
		return -1;
	} else {
		js = id - 1;
		je = id;
		while (--id > 0) {
			in_string += byte_array->elemsize[id];
			out_string += result->elemsize[id];
		}
	}

	if (byte_array->type[js] == OPH_DOUBLE) {
		double d = param->filling;
		memcpy(nan, &d, sizeof(double));
	} else if (byte_array->type[js] == OPH_FLOAT) {
		float d = (float) param->filling;
		memcpy(nan, &d, sizeof(float));
	} else {
		pmesg(1, __FILE__, __LINE__, "Type not supported\n");
		return -1;
	}

	for (j = js; j < je; ++j) {
		if (!param) {
			for (i = 0; i < result->numelem; i++)
				if (core_oph_type_cast(in_string + i * byte_array->blocksize, out_string + i * result->blocksize, byte_array->type[j], result->type[j], byte_array->missingvalue))
					return -1;
		} else {
			k = 0;
			current = param->start;
			for (i = 0; i < result->numelem; i++) {
				while ((param->index[k] < current - param->step / 2.0) && (k < param->num))
					k++;
				if (k >= param->num) {
					pmesg(1, __FILE__, __LINE__, "Index %d out of boundaries\n", k);
					return -1;
				}
				if (param->index[k] < current + param->step / 2.0) {
					if (core_oph_type_cast
					    (in_string + k * byte_array->blocksize, out_string + i * result->blocksize, byte_array->type[j], result->type[j], byte_array->missingvalue))
						return -1;
					k++;
				} else {
					if (param->filling_type) {
						switch (byte_array->type[j]) {
							case OPH_DOUBLE:
								{
									if (core_oph_type_cast
									    (&current, out_string + i * result->blocksize, byte_array->type[j], result->type[j], byte_array->missingvalue))
										return -1;
									break;
								}
							case OPH_FLOAT:
								{
									float f = (float) current;
									if (core_oph_type_cast(&f, out_string + i * result->blocksize, byte_array->type[j], result->type[j], byte_array->missingvalue))
										return -1;
									break;
								}
							default:
								pmesg(1, __FILE__, __LINE__, "Type not supported\n");
								return -1;
						}
					} else if (core_oph_type_cast(nan, out_string + i * result->blocksize, byte_array->type[j], result->type[j], byte_array->missingvalue))
						return -1;
				}
				current += param->step;
			}
		}
		in_string += byte_array->elemsize[j];
		out_string += result->elemsize[j];
	}
	return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_expand_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if (args->arg_count < 3 || args->arg_count > 8) {
		strcpy(message, "ERROR: Wrong arguments! oph_expand(input_OPH_TYPE, output_OPH_TYPE, measure, [index], [start], [step], [max], [filling])");
		return 1;
	}

	for (i = 0; i < 3; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_expand function");
			return 1;
		}
	}

	if (args->arg_count > 3) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_expand function");
			return 1;
		}
		for (i++; i < args->arg_count && i < 7; i++)
			args->arg_type[i] = REAL_RESULT;
		if (args->arg_count == 8) {
			if (args->arg_type[i] != STRING_RESULT)
				args->arg_type[i] = REAL_RESULT;
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;
	return 0;
}

void oph_expand_deinit(UDF_INIT * initid)
{
	//Free allocated space
	oph_multistring *multimeasure;
	if (initid->ptr) {
		multimeasure = (oph_multistring *) (initid->ptr);
		if (multimeasure->content) {
			free(multimeasure->content);
			multimeasure->content = NULL;
		}
		if (multimeasure->extend) {
			free(multimeasure->extend);
			multimeasure->extend = NULL;
		}
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

char *oph_expand(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	oph_multistring *multim;
	oph_multistring *output;

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

	oph_expand_param *param = NULL;
	if (!multim->extend) {
		multim->extend = malloc(sizeof(oph_expand_param));
		if (!multim->extend) {
			pmesg(1, __FILE__, __LINE__, "Error allocating parameters structure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}
	param = (oph_expand_param *) multim->extend;

	if (args->arg_count > 3) {
		param->index = (double *) args->args[3];
		if (param->index)
			param->num = args->lengths[3] / sizeof(double);
		else
			param->num = 0;
		if (args->arg_count > 4) {
			if (!args->args[4])
				param->start = param->index[0];
			else {
				param->start = *((double *) args->args[4]);
				if (isnan(param->start))
					param->start = param->index[0];
			}
			if (args->arg_count > 5) {
				if (!args->args[5])
					param->step = 1;
				else {
					param->step = *((double *) args->args[5]);
					if (isnan(param->step))
						param->step = 1;
				}
				if (args->arg_count > 6) {
					if (!args->args[6])
						param->max = param->index[param->num - 1];
					else {
						param->max = *((double *) args->args[6]);
						if (isnan(param->max))
							param->max = param->index[param->num - 1];
					}
					if (args->arg_count > 7) {
						if (!args->args[7]) {
							param->filling = NAN;
							param->filling_type = 0;
						} else if (args->arg_type[7] == STRING_RESULT) {
							char tmp[1 + args->lengths[7]];
							snprintf(tmp, 1 + args->lengths[7], "%s", args->args[7]);
							if (!strcasecmp(tmp, "NAN")) {
								param->filling = NAN;
								param->filling_type = 0;
							} else if (!strcasecmp(tmp, "INTERP")) {
								param->filling = 0.0;
								param->filling_type = 1;
							} else {
								param->filling = *((double *) tmp);
								param->filling_type = 0;
							}
						} else {
							param->filling = *((double *) args->args[7]);
							param->filling_type = 0;
						}
					} else if (param->index && param->num) {
						param->filling = NAN;
						param->filling_type = 0;
					}
				} else if (param->index && param->num) {
					param->max = param->index[param->num - 1];
					param->filling = NAN;
					param->filling_type = 0;
				}
			} else if (param->index && param->num) {
				param->step = 1;
				param->max = param->index[param->num - 1];
				param->filling = NAN;
				param->filling_type = 0;
			}
		} else if (param->index && param->num) {
			param->start = param->index[0];
			param->step = 1;
			param->max = param->index[param->num - 1];
			param->filling = NAN;
			param->filling_type = 0;
		}
	} else
		param->index = NULL;

	output = (oph_multistring *) (initid->ptr);
	if (param->index && param->num)
		output->numelem = 1 + round((param->max - param->start) / param->step);
	else
		output->numelem = multim->numelem;

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

	if (core_oph_expand_multi(multim, output, 0)) {
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
