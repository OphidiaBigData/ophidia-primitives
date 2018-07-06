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

#include "oph_gsl_boxplot.h"

#include <gsl/gsl_sort.h>

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/

int core_oph_gsl_boxplot_multi(oph_multistring * byte_array, oph_multistring * result)
{
	int i, j, output_format = byte_array->num_measure == result->num_measure;
	char *in_string = byte_array->content, *out_string = result->content;
	size_t *blocksize = NULL;
	if (!output_format) {
		blocksize = (size_t *) malloc((BOXPLOT_VALUES - 1) * sizeof(size_t));
		for (i = 0; i < BOXPLOT_VALUES - 1; ++i) {
			if (i)
				blocksize[i] = blocksize[i - 1];
			else
				blocksize[i] = 0;
			for (j = 0; j < byte_array->num_measure; ++j)
				blocksize[i] += result->elemsize[byte_array->num_measure * i + j];
		}
	}
	for (j = 0; j < byte_array->num_measure; ++j) {
		switch (byte_array->type[j]) {
			case OPH_DOUBLE:{
					double *d, tmp;
					d = (double *) malloc(byte_array->numelem * byte_array->elemsize[j]);
					for (i = 0; i < byte_array->numelem; ++i)
						d[i] = *(double *) (in_string + i * byte_array->blocksize);

					gsl_sort(d, 1, (size_t) byte_array->numelem);

					if (core_oph_type_cast(d, out_string, byte_array->type[j], result->type[j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_quantile_from_sorted_data(d, 1, (size_t) byte_array->numelem, 0.25);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize : blocksize[0]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure + j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_median_from_sorted_data(d, 1, (size_t) byte_array->numelem);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize * 2 : blocksize[1]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 2 + j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_quantile_from_sorted_data(d, 1, (size_t) byte_array->numelem, 0.75);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize * 3 : blocksize[2]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 3 + j], byte_array->missingvalue))
						return -1;

					if (core_oph_type_cast
					    (d + byte_array->numelem - 1, out_string + (output_format ? result->blocksize * 4 : blocksize[3]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 4 + j], byte_array->missingvalue))
						return -1;

					free(d);
					break;
				}
			case OPH_FLOAT:{
					float *d, tmp;
					d = (float *) malloc(byte_array->numelem * byte_array->elemsize[j]);
					for (i = 0; i < byte_array->numelem; ++i)
						d[i] = *(float *) (in_string + i * byte_array->blocksize);

					gsl_sort_float(d, 1, (size_t) byte_array->numelem);

					if (core_oph_type_cast(d, out_string, byte_array->type[j], result->type[j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_float_quantile_from_sorted_data(d, 1, (size_t) byte_array->numelem, 0.25);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize : blocksize[0]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure + j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_float_median_from_sorted_data(d, 1, (size_t) byte_array->numelem);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize * 2 : blocksize[1]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 2 + j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_float_quantile_from_sorted_data(d, 1, (size_t) byte_array->numelem, 0.75);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize * 3 : blocksize[2]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 3 + j], byte_array->missingvalue))
						return -1;

					if (core_oph_type_cast
					    (d + byte_array->numelem - 1, out_string + (output_format ? result->blocksize * 4 : blocksize[3]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 4 + j], byte_array->missingvalue))
						return -1;

					free(d);
					break;
				}
			case OPH_INT:{
					int *d, tmp;
					d = (int *) malloc(byte_array->numelem * byte_array->elemsize[j]);
					for (i = 0; i < byte_array->numelem; ++i)
						d[i] = *(int *) (in_string + i * byte_array->blocksize);

					gsl_sort_int(d, 1, (size_t) byte_array->numelem);

					if (core_oph_type_cast(d, out_string, byte_array->type[j], result->type[j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_int_quantile_from_sorted_data(d, 1, (size_t) byte_array->numelem, 0.25);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize : blocksize[0]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure + j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_int_median_from_sorted_data(d, 1, (size_t) byte_array->numelem);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize * 2 : blocksize[1]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 2 + j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_int_quantile_from_sorted_data(d, 1, (size_t) byte_array->numelem, 0.75);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize * 3 : blocksize[2]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 3 + j], byte_array->missingvalue))
						return -1;

					if (core_oph_type_cast
					    (d + byte_array->numelem - 1, out_string + (output_format ? result->blocksize * 4 : blocksize[3]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 4 + j], byte_array->missingvalue))
						return -1;

					free(d);
					break;
				}
			case OPH_SHORT:{
					short *d, tmp;
					d = (short *) malloc(byte_array->numelem * byte_array->elemsize[j]);
					for (i = 0; i < byte_array->numelem; ++i)
						d[i] = *(short *) (in_string + i * byte_array->blocksize);

					gsl_sort_short(d, 1, (size_t) byte_array->numelem);

					if (core_oph_type_cast(d, out_string, byte_array->type[j], result->type[j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_short_quantile_from_sorted_data(d, 1, (size_t) byte_array->numelem, 0.25);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize : blocksize[0]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure + j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_short_median_from_sorted_data(d, 1, (size_t) byte_array->numelem);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize * 2 : blocksize[1]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 2 + j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_short_quantile_from_sorted_data(d, 1, (size_t) byte_array->numelem, 0.75);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize * 3 : blocksize[2]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 3 + j], byte_array->missingvalue))
						return -1;

					if (core_oph_type_cast
					    (d + byte_array->numelem - 1, out_string + (output_format ? result->blocksize * 4 : blocksize[3]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 4 + j], byte_array->missingvalue))
						return -1;

					free(d);
					break;
				}
			case OPH_BYTE:{
					char *d, tmp;
					d = (char *) malloc(byte_array->numelem * byte_array->elemsize[j]);
					for (i = 0; i < byte_array->numelem; ++i)
						d[i] = *(char *) (in_string + i * byte_array->blocksize);

					gsl_sort_char(d, 1, (size_t) byte_array->numelem);

					if (core_oph_type_cast(d, out_string, byte_array->type[j], result->type[j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_char_quantile_from_sorted_data(d, 1, (size_t) byte_array->numelem, 0.25);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize : blocksize[0]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure + j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_char_median_from_sorted_data(d, 1, (size_t) byte_array->numelem);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize * 2 : blocksize[1]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 2 + j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_char_quantile_from_sorted_data(d, 1, (size_t) byte_array->numelem, 0.75);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize * 3 : blocksize[2]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 3 + j], byte_array->missingvalue))
						return -1;

					if (core_oph_type_cast
					    (d + byte_array->numelem - 1, out_string + (output_format ? result->blocksize * 4 : blocksize[3]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 4 + j], byte_array->missingvalue))
						return -1;

					free(d);
					break;
				}
			case OPH_LONG:{
					long long *d, tmp;
					d = (long long *) malloc(byte_array->numelem * byte_array->elemsize[j]);
					for (i = 0; i < byte_array->numelem; ++i)
						d[i] = *(long long *) (in_string + i * byte_array->blocksize);

					gsl_sort_long((long int *) d, 1, (size_t) byte_array->numelem);

					if (core_oph_type_cast(d, out_string, byte_array->type[j], result->type[j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_long_quantile_from_sorted_data((const long int *) d, 1, (size_t) byte_array->numelem, 0.25);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize : blocksize[0]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure + j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_long_median_from_sorted_data((const long int *) d, 1, (size_t) byte_array->numelem);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize * 2 : blocksize[1]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 2 + j], byte_array->missingvalue))
						return -1;

					tmp = gsl_stats_long_quantile_from_sorted_data((const long int *) d, 1, (size_t) byte_array->numelem, 0.75);
					if (core_oph_type_cast
					    (&tmp, out_string + (output_format ? result->blocksize * 3 : blocksize[2]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 3 + j], byte_array->missingvalue))
						return -1;

					if (core_oph_type_cast
					    (d + byte_array->numelem - 1, out_string + (output_format ? result->blocksize * 4 : blocksize[3]), byte_array->type[j],
					     output_format ? result->type[j] : result->type[byte_array->num_measure * 4 + j], byte_array->missingvalue))
						return -1;

					free(d);
					break;
				}
			default:
				pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
				return -1;
		}
		in_string += byte_array->elemsize[j];
		out_string += result->elemsize[j];
	}
	if (blocksize)
		free(blocksize);
	return 0;
}

my_bool oph_gsl_boxplot_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if (args->arg_count != 3) {
		strcpy(message, "ERROR: Wrong arguments! oph_gsl_boxplot(input_OPH_TYPE, output_OPH_TYPE, measure)");
		return 1;
	}

	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_gsl_boxplot function");
			return 1;
		}
	}

	initid->ptr = NULL;
	return 0;
}

void oph_gsl_boxplot_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		free_oph_generic_param_multi((oph_generic_param_multi *) initid->ptr);
		initid->ptr = NULL;
	}
}

char *oph_gsl_boxplot(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	if (*error) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	if (*is_null || !args->lengths[2]) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
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
		param->extend = NULL;

		initid->ptr = (char *) param;
	} else
		param = (oph_generic_param_multi *) initid->ptr;

	if (param->error) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	oph_multistring *measure;
	if (!param->error && !param->measure) {
		if (core_set_oph_multistring(&measure, args->args[0], &(args->lengths[0]))) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (!measure->islast) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Wrong number of input data type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		measure->length = args->lengths[2];
		if (measure->length % measure->blocksize) {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Wrong input type or data corrupted\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		measure->numelem = measure->length / measure->blocksize;

		param->measure = measure;
	} else
		measure = param->measure;

	measure->content = args->args[2];

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
		if (output->num_measure == measure->num_measure) {
			output->numelem = BOXPLOT_VALUES;	// 5 elements with the same struct of input
			output->length = BOXPLOT_VALUES * output->blocksize;
		} else if (output->num_measure == BOXPLOT_VALUES * measure->num_measure) {
			output->numelem = 1;	// 1 structured element
			output->length = output->blocksize;
		} else {
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Wrong number of output data type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
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

	if (!param->error && core_oph_gsl_boxplot_multi(param->measure, param->result)) {
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
