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

#include "oph_gsl_quantile.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_gsl_quantile_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{

	int i = 0;

	if (args->arg_count < 4) {
		strcpy(message, "ERROR: Wrong arguments! oph_gsl_quantile(input_OPH_TYPE, output_OPH_TYPE, measure [,0.25] [,…] [,ORDER_FLAG])");
		return 1;
	}

	for (i = 0; i < 3; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_gsl_quantile function");
			return 1;
		}
	}

	if (args->arg_count == 4 && args->arg_type[3] != DECIMAL_RESULT) {
		strcpy(message, "ERROR: Wrong arguments to oph_gsl_quantile function");
		return 1;
	}

	initid->ptr = NULL;
	initid->extension = NULL;

	return 0;
}

void oph_gsl_quantile_deinit(UDF_INIT * initid)
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
		if ((&(((oph_stringPtr) initid->extension)[1]))->numelem == ORDER_FLAG_SET) {
			free((&(((oph_stringPtr) initid->extension)[0]))->content);
			(&(((oph_stringPtr) initid->extension)[0]))->content = NULL;
		}
		if ((&(((oph_stringPtr) initid->extension)[1]))->content) {
			free((&(((oph_stringPtr) initid->extension)[1]))->content);
			(&(((oph_stringPtr) initid->extension)[1]))->content = NULL;
		}
		free(initid->extension);
		initid->extension = NULL;
	}
}

char *oph_gsl_quantile(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
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

	oph_stringPtr measure, quantiles, output;

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

		measure = &(((oph_stringPtr) initid->extension)[0]);
		core_set_type(measure, args->args[0], &(args->lengths[0]));
		measure->length = &(args->lengths[2]);
		measure->content = NULL;

		quantiles = &(((oph_stringPtr) initid->extension)[1]);
		quantiles->numelem = DEFAULT_ORDER_FLAG;	//default

		output = (oph_stringPtr) initid->ptr;
		core_set_type(output, args->args[1], &(args->lengths[1]));
		output->numelem = 0;
		int i;
		for (i = 0; i < args->arg_count; i++) {	// count quantiles
			if (args->arg_type[i] == DECIMAL_RESULT) {
				output->numelem++;
			}
		}
		if (output->numelem == 0) {
			pmesg(1, __FILE__, __LINE__, "There must be at least 1 quantile\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		quantiles->content = (char *) calloc(output->numelem, sizeof(double));
		if (!quantiles->content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating quantiles array\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		// Detect quantiles
		int j = 0;
		for (i = 3; i < args->arg_count; i++) {
			if (args->arg_type[i] == DECIMAL_RESULT) {
				((double *) quantiles->content)[j] = strtod(args->args[i], NULL);
				j++;
			} else if (!strcasecmp(args->args[i], "ORDER_FLAG_SET")) {
				quantiles->numelem = ORDER_FLAG_SET;
			} else if (!strcasecmp(args->args[i], "ORDER_FLAG_UNSET")) {
				quantiles->numelem = ORDER_FLAG_UNSET;
			}
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

		output->content = (char *) calloc(output->numelem, core_sizeof(output->type));
		if (!output->content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating output array\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	measure = &(((oph_stringPtr) initid->extension)[0]);
	quantiles = &(((oph_stringPtr) initid->extension)[1]);
	output = (oph_stringPtr) initid->ptr;

	// sort input if necessary
	if (quantiles->numelem == ORDER_FLAG_SET) {
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

	// compute quantiles
	if (oph_gsl_quantile_produce(measure->content, measure->numelem, measure->type, (double *) quantiles->content, output->numelem, output->content, output->type)) {
		pmesg(1, __FILE__, __LINE__, "Error computing quantiles\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	*length = output->numelem * core_sizeof(output->type);
	*error = 0;
	*is_null = 0;

	return output->content;
}

int oph_gsl_quantile_produce(const void *sorted_data, const size_t data_len, oph_type data_type, const double quantiles[], const int qnum, char *output_data, oph_type out_data_type)
{

	int i;
	double tmp;

	switch (data_type) {
		case OPH_INT:
			for (i = 0; i < qnum; i++) {
				tmp = gsl_stats_int_quantile_from_sorted_data((int *) sorted_data, 1, data_len, quantiles[i]);
				if (core_oph_type_cast(&tmp, output_data + (i * core_sizeof(out_data_type)), OPH_DOUBLE, out_data_type, NULL)) {
					pmesg(1, __FILE__, __LINE__, "Error casting output\n");
					return 1;
				}
			}
			break;
		case OPH_SHORT:
			for (i = 0; i < qnum; i++) {
				tmp = gsl_stats_short_quantile_from_sorted_data((short *) sorted_data, 1, data_len, quantiles[i]);
				if (core_oph_type_cast(&tmp, output_data + (i * core_sizeof(out_data_type)), OPH_DOUBLE, out_data_type, NULL)) {
					pmesg(1, __FILE__, __LINE__, "Error casting output\n");
					return 1;
				}
			}
			break;
		case OPH_BYTE:
			for (i = 0; i < qnum; i++) {
				tmp = gsl_stats_char_quantile_from_sorted_data((char *) sorted_data, 1, data_len, quantiles[i]);
				if (core_oph_type_cast(&tmp, output_data + (i * core_sizeof(out_data_type)), OPH_DOUBLE, out_data_type, NULL)) {
					pmesg(1, __FILE__, __LINE__, "Error casting output\n");
					return 1;
				}
			}
			break;
		case OPH_LONG:
			for (i = 0; i < qnum; i++) {
				tmp = gsl_stats_long_quantile_from_sorted_data((long *) sorted_data, 1, data_len, quantiles[i]);
				if (core_oph_type_cast(&tmp, output_data + (i * core_sizeof(out_data_type)), OPH_DOUBLE, out_data_type, NULL)) {
					pmesg(1, __FILE__, __LINE__, "Error casting output\n");
					return 1;
				}
			}
			break;
		case OPH_FLOAT:
			for (i = 0; i < qnum; i++) {
				tmp = gsl_stats_float_quantile_from_sorted_data((float *) sorted_data, 1, data_len, quantiles[i]);
				if (core_oph_type_cast(&tmp, output_data + (i * core_sizeof(out_data_type)), OPH_DOUBLE, out_data_type, NULL)) {
					pmesg(1, __FILE__, __LINE__, "Error casting output\n");
					return 1;
				}
			}
			break;
		case OPH_DOUBLE:
			for (i = 0; i < qnum; i++) {
				tmp = gsl_stats_quantile_from_sorted_data((double *) sorted_data, 1, data_len, quantiles[i]);
				if (core_oph_type_cast(&tmp, output_data + (i * core_sizeof(out_data_type)), OPH_DOUBLE, out_data_type, NULL)) {
					pmesg(1, __FILE__, __LINE__, "Error casting output\n");
					return 1;
				}
			}
			break;
		default:;
	}

	return 0;
}
