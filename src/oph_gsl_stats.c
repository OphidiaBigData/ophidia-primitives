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

#include "oph_gsl_stats.h"

#include <gsl/gsl_sort.h>

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_gsl_stats_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	int i = 0;
	if (args->arg_count < 3 || args->arg_count > 4) {
		strcpy(message, "ERROR: Wrong arguments! oph_gsl_stats(input_OPH_TYPE, output_OPH_TYPE, measure [,STATS_MASK])");
		return 1;
	}

	for (i = 0; i < 3; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_gsl_stats function");
			return 1;
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;

	return 0;
}

void oph_gsl_stats_deinit(UDF_INIT *initid)
{
	//Free allocated space
	if (initid->ptr) {
		if (((oph_stringPtr) initid->ptr)->content) {
			free(((oph_stringPtr) initid->ptr)->content);
			((oph_stringPtr) initid->ptr)->content = NULL;
		}
		free(initid->ptr);
		initid->ptr = NULL;
	}
	if (initid->extension) {
		if ((&(((oph_stringPtr) initid->extension)[1]))->numelem == 1) {
			free((&(((oph_stringPtr) initid->extension)[0]))->content);
			(&(((oph_stringPtr) initid->extension)[0]))->content = NULL;
		}
		free(initid->extension);
		initid->extension = NULL;
	}
}

char *oph_gsl_stats(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
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

	oph_stringPtr measure, mask, output;

	if (!initid->ptr) {
		initid->ptr = calloc(1, sizeof(oph_string));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating output array\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		initid->extension = calloc(2, sizeof(oph_string));
		if (!initid->extension) {
			pmesg(1, __FILE__, __LINE__, "Error allocating extension\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		unsigned long dmasklen = strlen(DEFAULT_MASK);

		measure = &(((oph_stringPtr) initid->extension)[0]);
		core_set_type(measure, args->args[0], &(args->lengths[0]));
		if (measure->type != OPH_BYTE && measure->type != OPH_SHORT && measure->type != OPH_INT && measure->type != OPH_LONG && measure->type != OPH_FLOAT && measure->type != OPH_DOUBLE) {
			pmesg(1, __FILE__, __LINE__, "Invalid input type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		measure->length = &(args->lengths[2]);
		measure->content = NULL;

		mask = &(((oph_stringPtr) initid->extension)[1]);
		mask->content = DEFAULT_MASK;
		mask->length = &dmasklen;

		// Detect mask
		if (args->arg_count > 3) {
			mask->content = args->args[3];
			mask->length = &(args->lengths[3]);
		}

		if (core_set_elemsize(measure)) {
			pmesg(1, __FILE__, __LINE__, "Error on setting measure elements size\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (core_set_numelem(measure)) {
			pmesg(1, __FILE__, __LINE__, "Error on counting measure elements\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		output = (oph_stringPtr) initid->ptr;
		core_set_type(output, args->args[1], &(args->lengths[1]));
		if (output->type != OPH_BYTE && output->type != OPH_SHORT && output->type != OPH_INT && output->type != OPH_LONG && output->type != OPH_FLOAT && output->type != OPH_DOUBLE) {
			pmesg(1, __FILE__, __LINE__, "Invalid output type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (core_set_elemsize(output)) {
			pmesg(1, __FILE__, __LINE__, "Error on setting output elements size\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		mask->numelem = 0;
		// mask processing
		int i;
		for (i = 0; i < *(mask->length); i++) {
			if (mask->content[i] == '1') {
				(output->numelem)++;
				if (i > 8) {
					mask->numelem = 1;	// => order array to compute quantiles
				}
			}
		}
		if (output->numelem == 0) {
			pmesg(1, __FILE__, __LINE__, "Invalid mask\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		// output array allocation
		output->content = (char *) calloc(output->numelem, output->elemsize);
		if (!output->content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating output array\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	measure = &(((oph_stringPtr) initid->extension)[0]);
	mask = &(((oph_stringPtr) initid->extension)[1]);
	output = (oph_stringPtr) initid->ptr;

	// sort input if necessary
	if (mask->numelem == 1) {
		if (!measure->content) {
			measure->content = (char *) calloc(1, *(measure->length));
			if (!measure->content) {
				pmesg(1, __FILE__, __LINE__, "Error allocating duplicate array\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
		}
		memcpy(measure->content, args->args[2], *(measure->length));
		switch (measure->type) {
			case OPH_INT:
				gsl_sort_int((int *) measure->content, 1, measure->numelem);
				break;
			case OPH_SHORT:
				gsl_sort_short((short *) measure->content, 1, measure->numelem);
				break;
			case OPH_BYTE:
				gsl_sort_char((char *) measure->content, 1, measure->numelem);
				break;
			case OPH_LONG:
				gsl_sort_long((long *) measure->content, 1, measure->numelem);
				break;
			case OPH_FLOAT:
				gsl_sort_float((float *) measure->content, 1, measure->numelem);
				break;
			case OPH_DOUBLE:
				gsl_sort((double *) measure->content, 1, measure->numelem);
				break;
			default:;
		}
	} else {
		measure->content = args->args[2];
	}

	if (oph_gsl_stats_produce((void *) measure->content, measure->numelem, measure->type, mask->content, output->content, (size_t) output->numelem, (size_t *) length, output->type)) {
		pmesg(1, __FILE__, __LINE__, "Error computing stats\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	*error = 0;
	*is_null = 0;

	return output->content;
}

/* Compute requested statistics */
int oph_gsl_stats_produce(void *in_data, const size_t data_len, oph_type data_type, const char *mask, char *out_data, const size_t out_len, size_t *out_data_len, oph_type out_data_type)
{

	int i, j;

	gsl_set_error_handler_off();

	// compute stats according to data type
	i = 0;
	double tmp;
	for (j = 0; j < out_len; j++) {
		while (mask[i] == '0')
			i++;
		if (data_type == OPH_INT) {
			switch (i) {
				case 0:	// 1xxxxxxxxxxxxx : mean
					tmp = gsl_stats_int_mean((int *) in_data, 1, data_len);
					break;
				case 1:	// x1xxxxxxxxxxxx : variance
					tmp = gsl_stats_int_variance((int *) in_data, 1, data_len);
					break;
				case 2:	// xx1xxxxxxxxxxx : std dev
					tmp = gsl_stats_int_sd((int *) in_data, 1, data_len);
					break;
				case 3:	// xxx1xxxxxxxxxx : abs dev
					tmp = gsl_stats_int_absdev((int *) in_data, 1, data_len);
					break;
				case 4:	// xxxx1xxxxxxxxx : skew
					tmp = gsl_stats_int_skew((int *) in_data, 1, data_len);
					break;
				case 5:	// xxxxx1xxxxxxxx : kurtosis
					tmp = gsl_stats_int_kurtosis((int *) in_data, 1, data_len);
					break;
				case 6:	// xxxxxx1xxxxxxx : autocorrelation
					tmp = gsl_stats_int_lag1_autocorrelation((int *) in_data, 1, data_len);
					break;
				case 7:	// xxxxxxx1xxxxxx : max
					tmp = gsl_stats_int_max((int *) in_data, 1, data_len);
					break;
				case 8:	// xxxxxxxx1xxxxx : min
					tmp = gsl_stats_int_min((int *) in_data, 1, data_len);
					break;
				case 9:	// xxxxxxxxx1xxxx : 0.05 quantile
					tmp = gsl_stats_int_quantile_from_sorted_data((int *) in_data, 1, data_len, 0.05);
					break;
				case 10:	// xxxxxxxxxx1xxx : 0.25 quantile->Q1
					tmp = gsl_stats_int_quantile_from_sorted_data((int *) in_data, 1, data_len, 0.25);
					break;
				case 11:	// xxxxxxxxxxx1xx : 0.5 quantile->Q2 (median)
					tmp = gsl_stats_int_quantile_from_sorted_data((int *) in_data, 1, data_len, 0.5);
					break;
				case 12:	// xxxxxxxxxxxx1x : 0.75 quantile->Q3
					tmp = gsl_stats_int_quantile_from_sorted_data((int *) in_data, 1, data_len, 0.75);
					break;
				case 13:	// xxxxxxxxxxxxx1 : 0.95 quantile
					tmp = gsl_stats_int_quantile_from_sorted_data((int *) in_data, 1, data_len, 0.95);
			}
		} else if (data_type == OPH_SHORT) {
			switch (i) {
				case 0:	// 1xxxxxxxxxxxxx : mean
					tmp = gsl_stats_short_mean((short *) in_data, 1, data_len);
					break;
				case 1:	// x1xxxxxxxxxxxx : variance
					tmp = gsl_stats_short_variance((short *) in_data, 1, data_len);
					break;
				case 2:	// xx1xxxxxxxxxxx : std dev
					tmp = gsl_stats_short_sd((short *) in_data, 1, data_len);
					break;
				case 3:	// xxx1xxxxxxxxxx : abs dev
					tmp = gsl_stats_short_absdev((short *) in_data, 1, data_len);
					break;
				case 4:	// xxxx1xxxxxxxxx : skew
					tmp = gsl_stats_short_skew((short *) in_data, 1, data_len);
					break;
				case 5:	// xxxxx1xxxxxxxx : kurtosis
					tmp = gsl_stats_short_kurtosis((short *) in_data, 1, data_len);
					break;
				case 6:	// xxxxxx1xxxxxxx : autocorrelation
					tmp = gsl_stats_short_lag1_autocorrelation((short *) in_data, 1, data_len);
					break;
				case 7:	// xxxxxxx1xxxxxx : max
					tmp = gsl_stats_short_max((short *) in_data, 1, data_len);
					break;
				case 8:	// xxxxxxxx1xxxxx : min
					tmp = gsl_stats_short_min((short *) in_data, 1, data_len);
					break;
				case 9:	// xxxxxxxxx1xxxx : 0.05 quantile
					tmp = gsl_stats_short_quantile_from_sorted_data((short *) in_data, 1, data_len, 0.05);
					break;
				case 10:	// xxxxxxxxxx1xxx : 0.25 quantile->Q1
					tmp = gsl_stats_short_quantile_from_sorted_data((short *) in_data, 1, data_len, 0.25);
					break;
				case 11:	// xxxxxxxxxxx1xx : 0.5 quantile->Q2 (median)
					tmp = gsl_stats_short_quantile_from_sorted_data((short *) in_data, 1, data_len, 0.5);
					break;
				case 12:	// xxxxxxxxxxxx1x : 0.75 quantile->Q3
					tmp = gsl_stats_short_quantile_from_sorted_data((short *) in_data, 1, data_len, 0.75);
					break;
				case 13:	// xxxxxxxxxxxxx1 : 0.95 quantile
					tmp = gsl_stats_short_quantile_from_sorted_data((short *) in_data, 1, data_len, 0.95);
			}
		} else if (data_type == OPH_BYTE) {
			switch (i) {
				case 0:	// 1xxxxxxxxxxxxx : mean
					tmp = gsl_stats_char_mean((char *) in_data, 1, data_len);
					break;
				case 1:	// x1xxxxxxxxxxxx : variance
					tmp = gsl_stats_char_variance((char *) in_data, 1, data_len);
					break;
				case 2:	// xx1xxxxxxxxxxx : std dev
					tmp = gsl_stats_char_sd((char *) in_data, 1, data_len);
					break;
				case 3:	// xxx1xxxxxxxxxx : abs dev
					tmp = gsl_stats_char_absdev((char *) in_data, 1, data_len);
					break;
				case 4:	// xxxx1xxxxxxxxx : skew
					tmp = gsl_stats_char_skew((char *) in_data, 1, data_len);
					break;
				case 5:	// xxxxx1xxxxxxxx : kurtosis
					tmp = gsl_stats_char_kurtosis((char *) in_data, 1, data_len);
					break;
				case 6:	// xxxxxx1xxxxxxx : autocorrelation
					tmp = gsl_stats_char_lag1_autocorrelation((char *) in_data, 1, data_len);
					break;
				case 7:	// xxxxxxx1xxxxxx : max
					tmp = gsl_stats_char_max((char *) in_data, 1, data_len);
					break;
				case 8:	// xxxxxxxx1xxxxx : min
					tmp = gsl_stats_char_min((char *) in_data, 1, data_len);
					break;
				case 9:	// xxxxxxxxx1xxxx : 0.05 quantile
					tmp = gsl_stats_char_quantile_from_sorted_data((char *) in_data, 1, data_len, 0.05);
					break;
				case 10:	// xxxxxxxxxx1xxx : 0.25 quantile->Q1
					tmp = gsl_stats_char_quantile_from_sorted_data((char *) in_data, 1, data_len, 0.25);
					break;
				case 11:	// xxxxxxxxxxx1xx : 0.5 quantile->Q2 (median)
					tmp = gsl_stats_char_quantile_from_sorted_data((char *) in_data, 1, data_len, 0.5);
					break;
				case 12:	// xxxxxxxxxxxx1x : 0.75 quantile->Q3
					tmp = gsl_stats_char_quantile_from_sorted_data((char *) in_data, 1, data_len, 0.75);
					break;
				case 13:	// xxxxxxxxxxxxx1 : 0.95 quantile
					tmp = gsl_stats_char_quantile_from_sorted_data((char *) in_data, 1, data_len, 0.95);
			}
		} else if (data_type == OPH_LONG) {
			switch (i) {
				case 0:	// 1xxxxxxxxxxxxx : mean
					tmp = gsl_stats_long_mean((long *) in_data, 1, data_len);
					break;
				case 1:	// x1xxxxxxxxxxxx : variance
					tmp = gsl_stats_long_variance((long *) in_data, 1, data_len);
					break;
				case 2:	// xx1xxxxxxxxxxx : std dev
					tmp = gsl_stats_long_sd((long *) in_data, 1, data_len);
					break;
				case 3:	// xxx1xxxxxxxxxx : abs dev
					tmp = gsl_stats_long_absdev((long *) in_data, 1, data_len);
					break;
				case 4:	// xxxx1xxxxxxxxx : skew
					tmp = gsl_stats_long_skew((long *) in_data, 1, data_len);
					break;
				case 5:	// xxxxx1xxxxxxxx : kurtosis
					tmp = gsl_stats_long_kurtosis((long *) in_data, 1, data_len);
					break;
				case 6:	// xxxxxx1xxxxxxx : autocorrelation
					tmp = gsl_stats_long_lag1_autocorrelation((long *) in_data, 1, data_len);
					break;
				case 7:	// xxxxxxx1xxxxxx : max
					tmp = gsl_stats_long_max((long *) in_data, 1, data_len);
					break;
				case 8:	// xxxxxxxx1xxxxx : min
					tmp = gsl_stats_long_min((long *) in_data, 1, data_len);
					break;
				case 9:	// xxxxxxxxx1xxxx : 0.05 quantile
					tmp = gsl_stats_long_quantile_from_sorted_data((long *) in_data, 1, data_len, 0.05);
					break;
				case 10:	// xxxxxxxxxx1xxx : 0.25 quantile->Q1
					tmp = gsl_stats_long_quantile_from_sorted_data((long *) in_data, 1, data_len, 0.25);
					break;
				case 11:	// xxxxxxxxxxx1xx : 0.5 quantile->Q2 (median)
					tmp = gsl_stats_long_quantile_from_sorted_data((long *) in_data, 1, data_len, 0.5);
					break;
				case 12:	// xxxxxxxxxxxx1x : 0.75 quantile->Q3
					tmp = gsl_stats_long_quantile_from_sorted_data((long *) in_data, 1, data_len, 0.75);
					break;
				case 13:	// xxxxxxxxxxxxx1 : 0.95 quantile
					tmp = gsl_stats_long_quantile_from_sorted_data((long *) in_data, 1, data_len, 0.95);
			}
		} else if (data_type == OPH_FLOAT) {
			switch (i) {
				case 0:	// 1xxxxxxxxxxxxx : mean
					tmp = gsl_stats_float_mean((float *) in_data, 1, data_len);
					break;
				case 1:	// x1xxxxxxxxxxxx : variance
					tmp = gsl_stats_float_variance((float *) in_data, 1, data_len);
					break;
				case 2:	// xx1xxxxxxxxxxx : std dev
					tmp = gsl_stats_float_sd((float *) in_data, 1, data_len);
					break;
				case 3:	// xxx1xxxxxxxxxx : abs dev
					tmp = gsl_stats_float_absdev((float *) in_data, 1, data_len);
					break;
				case 4:	// xxxx1xxxxxxxxx : skew
					tmp = gsl_stats_float_skew((float *) in_data, 1, data_len);
					break;
				case 5:	// xxxxx1xxxxxxxx : kurtosis
					tmp = gsl_stats_float_kurtosis((float *) in_data, 1, data_len);
					break;
				case 6:	// xxxxxx1xxxxxxx : autocorrelation
					tmp = gsl_stats_float_lag1_autocorrelation((float *) in_data, 1, data_len);
					break;
				case 7:	// xxxxxxx1xxxxxx : max
					tmp = gsl_stats_float_max((float *) in_data, 1, data_len);
					break;
				case 8:	// xxxxxxxx1xxxxx : min
					tmp = gsl_stats_float_min((float *) in_data, 1, data_len);
					break;
				case 9:	// xxxxxxxxx1xxxx : 0.05 quantile
					tmp = gsl_stats_float_quantile_from_sorted_data((float *) in_data, 1, data_len, 0.05);
					break;
				case 10:	// xxxxxxxxxx1xxx : 0.25 quantile->Q1
					tmp = gsl_stats_float_quantile_from_sorted_data((float *) in_data, 1, data_len, 0.25);
					break;
				case 11:	// xxxxxxxxxxx1xx : 0.5 quantile->Q2 (median)
					tmp = gsl_stats_float_quantile_from_sorted_data((float *) in_data, 1, data_len, 0.5);
					break;
				case 12:	// xxxxxxxxxxxx1x : 0.75 quantile->Q3
					tmp = gsl_stats_float_quantile_from_sorted_data((float *) in_data, 1, data_len, 0.75);
					break;
				case 13:	// xxxxxxxxxxxxx1 : 0.95 quantile
					tmp = gsl_stats_float_quantile_from_sorted_data((float *) in_data, 1, data_len, 0.95);
			}
		} else {
			switch (i) {
				case 0:	// 1xxxxxxxxxxxxx : mean
					tmp = gsl_stats_mean((double *) in_data, 1, data_len);
					break;
				case 1:	// x1xxxxxxxxxxxx : variance
					tmp = gsl_stats_variance((double *) in_data, 1, data_len);
					break;
				case 2:	// xx1xxxxxxxxxxx : std dev
					tmp = gsl_stats_sd((double *) in_data, 1, data_len);
					break;
				case 3:	// xxx1xxxxxxxxxx : abs dev
					tmp = gsl_stats_absdev((double *) in_data, 1, data_len);
					break;
				case 4:	// xxxx1xxxxxxxxx : skew
					tmp = gsl_stats_skew((double *) in_data, 1, data_len);
					break;
				case 5:	// xxxxx1xxxxxxxx : kurtosis
					tmp = gsl_stats_kurtosis((double *) in_data, 1, data_len);
					break;
				case 6:	// xxxxxx1xxxxxxx : autocorrelation
					tmp = gsl_stats_lag1_autocorrelation((double *) in_data, 1, data_len);
					break;
				case 7:	// xxxxxxx1xxxxxx : max
					tmp = gsl_stats_max((double *) in_data, 1, data_len);
					break;
				case 8:	// xxxxxxxx1xxxxx : min
					tmp = gsl_stats_min((double *) in_data, 1, data_len);
					break;
				case 9:	// xxxxxxxxx1xxxx : 0.05 quantile
					tmp = gsl_stats_quantile_from_sorted_data((double *) in_data, 1, data_len, 0.05);
					break;
				case 10:	// xxxxxxxxxx1xxx : 0.25 quantile->Q1
					tmp = gsl_stats_quantile_from_sorted_data((double *) in_data, 1, data_len, 0.25);
					break;
				case 11:	// xxxxxxxxxxx1xx : 0.5 quantile->Q2 (median)
					tmp = gsl_stats_quantile_from_sorted_data((double *) in_data, 1, data_len, 0.5);
					break;
				case 12:	// xxxxxxxxxxxx1x : 0.75 quantile->Q3
					tmp = gsl_stats_quantile_from_sorted_data((double *) in_data, 1, data_len, 0.75);
					break;
				case 13:	// xxxxxxxxxxxxx1 : 0.95 quantile
					tmp = gsl_stats_quantile_from_sorted_data((double *) in_data, 1, data_len, 0.95);
			}
		}
		if (core_oph_type_cast(&tmp, out_data + (j * core_sizeof(out_data_type)), OPH_DOUBLE, out_data_type, NULL))
			return 1;

		i++;
	}

	*out_data_len = out_len * core_sizeof(out_data_type);

	return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
