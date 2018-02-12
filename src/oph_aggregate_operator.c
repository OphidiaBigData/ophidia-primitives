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

#include "oph_aggregate_operator.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_aggregate_operator_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if (args->arg_count < 3 || args->arg_count > 5) {
		strcpy(message, "ERROR: Wrong arguments! oph_aggregate_operator(input_OPH_TYPE, output_OPH_TYPE, measure, [OPERATOR], [missingvalue])");
		return 1;
	}

	for (i = 0; i < args->arg_count; i++) {
		if (i == 4) {
			if (args->args[i] && (args->arg_type[i] == STRING_RESULT)) {
				strcpy(message, "ERROR: Wrong argument 'missingvalue' to oph_operator function");
				return 1;
			}
			args->arg_type[i] = REAL_RESULT;
		} else if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_reverse function");
			return 1;
		}
	}

	oph_agg_oper_data *dat = (oph_agg_oper_data *) malloc(sizeof(oph_agg_oper_data));
	if (!dat) {
		strcpy(message, "ERROR: Unable to allocate result string");
		return 1;
	}
	dat->core_oph_oper = NULL;
	dat->count = NULL;
	dat->first = 1;
	dat->result.hier = INVALID_HIER;
	dat->result.oper = INVALID_OPER;

	dat->result.measure.content = NULL;
	dat->result.measure.length = NULL;
	dat->result.measure.type = INVALID_TYPE;
	dat->result.measure.elemsize = 0;
	dat->result.measure.numelem = 0;

	dat->result_data = NULL;
	dat->result_length = 0;
	dat->result_size = 0;
	dat->result_type = INVALID_TYPE;

	initid->maybe_null = 1;
	initid->ptr = (char *) dat;

	return 0;
}

void oph_aggregate_operator_deinit(UDF_INIT * initid)
{
	//Free allocated space 
	if (initid->ptr) {
		oph_agg_oper_data *dat = (oph_agg_oper_data *) initid->ptr;
		if (dat->count) {
			free(dat->count);
			dat->count = NULL;
		}
		oph_requestPtr res = (oph_requestPtr) & (dat->result);
		if (res->measure.content) {
			free(res->measure.content);
			res->measure.content = NULL;
		}
		if (res->measure.length) {
			free(res->measure.length);
			res->measure.length = NULL;
		}
		if (dat->result_data) {
			free(dat->result_data);
			dat->result_data = NULL;
		}
		free(initid->ptr);
		initid->ptr = NULL;
	}
}

void oph_aggregate_operator_reset(UDF_INIT * initid, UDF_ARGS * args, char *is_null, char *error)
{
	oph_aggregate_operator_clear(initid, is_null, error);
	oph_aggregate_operator_add(initid, args, is_null, error);
}

void oph_aggregate_operator_clear(UDF_INIT * initid, char *is_null, char *error)
{
	if (initid->ptr) {
		oph_agg_oper_data *dat = (oph_agg_oper_data *) initid->ptr;
		if (dat->count) {
			size_t i;
			for (i = 0; i < dat->result.measure.numelem; ++i)
				dat->count[i] = 0;
		}
		dat->first = 1;
	}
	*is_null = 0;
	*error = 0;
}

void oph_aggregate_operator_add(UDF_INIT * initid, UDF_ARGS * args, char *is_null, char *error)
{
	if (*error || !args->args[2])
		return;

	//Assume that:
	//1. Type of each element is the same
	//2. Length of each array is the same
	oph_string measure;
	oph_agg_oper_data *dat = (oph_agg_oper_data *) initid->ptr;
	oph_requestPtr res = (oph_requestPtr) & (dat->result);

	/* Setting of the aggregate result */
	if (!res->measure.content) {
		//It's the first row 
		core_set_type(&(res->measure), args->args[0], &(args->lengths[0]));

		if (args->arg_count < 4) {
			core_set_oper(res, NULL, 0);
		} else {
			core_set_oper(res, args->args[3], &(args->lengths[3]));
		}

		if (!res->oper) {
			pmesg(1, __FILE__, __LINE__, "Operator not recognized\n");
			*is_null = 0;
			*error = 1;
			return;
		}
		core_set_hier(res, NULL, 0);
		res->measure.length = malloc(sizeof(unsigned long));
		if (!res->measure.length) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result string\n");
			*is_null = 0;
			*error = 1;
			return;
		}
		*(res->measure.length) = args->lengths[2];

		core_set_elemsize(&(res->measure));

		if (core_set_numelem(&(res->measure))) {
			pmesg(1, __FILE__, __LINE__, "Error on counting elements\n");
			*is_null = 0;
			*error = 1;
			return;
		}

		res->measure.content = (char *) malloc(*(res->measure.length));
		if (!res->measure.content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result string\n");
			*is_null = 0;
			*error = 1;
			return;
		}

		switch (res->oper) {
			case OPH_COUNT:
				dat->core_oph_oper = core_oph_count_array;
				break;
			case OPH_MAX:
				dat->core_oph_oper = core_oph_max_array;
				break;
			case OPH_MIN:
				dat->core_oph_oper = core_oph_min_array;
				break;
			case OPH_SUM:
				dat->core_oph_oper = core_oph_sum_array;
				break;
			case OPH_AVG:
				dat->core_oph_oper = NULL;
				break;
			default:
				pmesg(1, __FILE__, __LINE__, "Unable to recognize operator\n");
				*is_null = 0;
				*error = 1;
				return;
		}

		oph_string output_array;
		core_set_type(&output_array, args->args[1], &(args->lengths[1]));
		if (!output_array.type) {
			pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
			*is_null = 0;
			*error = 1;
			return;
		}
		if (core_set_elemsize(&output_array)) {
			pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
			*is_null = 0;
			*error = 1;
			return;
		}
		dat->result_size = output_array.elemsize;
		dat->result_type = output_array.type;
		dat->result_length = res->measure.numelem * dat->result_size;
		dat->result_data = (char *) malloc(dat->result_length);
		if (!dat->result_data) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result string\n");
			*is_null = 0;
			*error = 1;
			return;
		}
	}

	if (!dat->count && (res->oper == OPH_AVG)) {
		dat->count = (int *) malloc(res->measure.numelem * sizeof(int));
		if (!dat->count) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result string\n");
			*is_null = 0;
			*error = 1;
			return;
		}
		size_t i;
		for (i = 0; i < res->measure.numelem; ++i)
			dat->count[i] = 0;
	}

	measure.type = res->measure.type;
	measure.content = args->args[2];
	measure.length = res->measure.length;
	measure.elemsize = res->measure.elemsize;
	measure.numelem = res->measure.numelem;

	double missingvalue;
	if ((args->arg_count > 4) && args->args[4]) {
		missingvalue = *((double *) (args->args[4]));
		measure.missingvalue = &missingvalue;
	} else
		measure.missingvalue = NULL;

	if (res->oper == OPH_AVG) {
		if (core_oph_sum_array2(&measure, dat->first ? NULL : &(res->measure), res->measure.content, dat->count)) {
			pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
			*is_null = 0;
			*error = 1;
			return;
		}
	} else if (dat->first && (res->oper != OPH_COUNT)) {
		//It's the first row or the first row in the group of the GRUOP BY clause
		//Only perform the copy of the tuple
		memcpy(res->measure.content, args->args[2], *(res->measure.length));
	} else if (dat->core_oph_oper(&measure, dat->first ? NULL : &(res->measure), res->measure.content)) {
		pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
		*is_null = 0;
		*error = 1;
		return;
	}

	dat->first = 0;
}

char *oph_aggregate_operator(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	int i;

	if (*error != 0) {
		pmesg(1, __FILE__, __LINE__, "Error on adding elements\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	oph_agg_oper_data *dat = (oph_agg_oper_data *) initid->ptr;
	oph_stringPtr res = (oph_stringPtr) & (dat->result.measure);
	if (!res->content) {
		*error = 0;
		*is_null = 0;
		return NULL;
	}

	double missingvalue = 0, *pointer = NULL;
	if ((args->arg_count > 4) && args->args[4]) {
		missingvalue = *((double *) (args->args[4]));
		pointer = &missingvalue;
	}

	switch (dat->result.oper) {
		case OPH_COUNT:
		case OPH_MAX:
		case OPH_MIN:
		case OPH_SUM:
			break;
		case OPH_AVG:
			{
				switch (res->type) {
					case OPH_DOUBLE:{
							double *position = (double *) res->content;
							for (i = 0; i < res->numelem; i++) {
								if (!dat->count[i])
									*position = pointer ? missingvalue : NAN;
								else
									*position /= dat->count[i];
								position++;
							}
							break;
						}
					case OPH_FLOAT:{
							float *position = (float *) res->content;
							for (i = 0; i < res->numelem; i++) {
								if (!dat->count[i])
									*position = pointer ? (float) missingvalue : NAN;
								else
									*position /= dat->count[i];
								position++;
							}
							break;
						}
					case OPH_INT:{
							float accumulator[res->numelem];
							int *position = (int *) res->content;
							for (i = 0; i < res->numelem; i++) {
								if (!dat->count[pointer ? i : 0])
									accumulator[i] = (float) missingvalue;
								else
									accumulator[i] = (float) (*position) / dat->count[pointer ? i : 0];	// Only count[0] is updated to improve performance
								position++;
							}
							free(res->content);
							unsigned long length = res->numelem * core_sizeof(OPH_FLOAT);
							res->content = malloc(length);
							memcpy(res->content, accumulator, length);
							break;
						}
					case OPH_LONG:{
							double accumulator[res->numelem];
							long long *position = (long long *) res->content;
							for (i = 0; i < res->numelem; i++) {
								if (!dat->count[pointer ? i : 0])
									accumulator[i] = missingvalue;
								else
									accumulator[i] = (double) (*position) / dat->count[pointer ? i : 0];	// Only count[0] is updated to improve performance
								position++;
							}
							free(res->content);
							unsigned long length = res->numelem * core_sizeof(OPH_DOUBLE);
							res->content = malloc(length);
							memcpy(res->content, accumulator, length);
							break;
						}
					case OPH_SHORT:{
							float accumulator[res->numelem];
							short *position = (short *) res->content;
							for (i = 0; i < res->numelem; i++) {
								if (!dat->count[pointer ? i : 0])
									accumulator[i] = (float) missingvalue;
								else
									accumulator[i] = (float) (*position) / dat->count[pointer ? i : 0];	// Only count[0] is updated to improve performance
								position++;
							}
							free(res->content);
							unsigned long length = res->numelem * core_sizeof(OPH_FLOAT);
							res->content = malloc(length);
							memcpy(res->content, accumulator, length);
							break;
						}
					case OPH_BYTE:{
							float accumulator[res->numelem];
							char *position = (char *) res->content;
							for (i = 0; i < res->numelem; i++) {
								if (!dat->count[pointer ? i : 0])
									accumulator[i] = (float) missingvalue;
								else
									accumulator[i] = (float) (*position) / dat->count[pointer ? i : 0];	// Only count[0] is updated to improve performance
								position++;
							}
							free(res->content);
							unsigned long length = res->numelem * core_sizeof(OPH_FLOAT);
							res->content = malloc(length);
							memcpy(res->content, accumulator, length);
							break;
						}
					default:
						pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
				}
				break;
			}
		default:
			pmesg(1, __FILE__, __LINE__, "Operation not recognized\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
	}

	for (i = 0; i < res->numelem; ++i) {
		if (core_oph_type_cast(res->content + i * res->elemsize, dat->result_data + i * dat->result_size, res->type, dat->result_type, pointer)) {
			pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	*length = dat->result_length;
	*error = 0;
	*is_null = 0;

	return (result = dat->result_data);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
