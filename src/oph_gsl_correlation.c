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

#include "oph_gsl_correlation.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/

int oph_gsl_correlation_produce(const void *data, const void *data2, const size_t data_len, oph_type data_type, char *output_data, oph_type out_data_type)
{

	double tmp;
	switch (data_type) {
		case OPH_INT:
			tmp = gsl_stats_int_correlation((int *) data, 1, (int *) data2, 1, data_len);
			break;
		case OPH_SHORT:
			tmp = gsl_stats_short_correlation((short *) data, 1, (short *) data2, 1, data_len);
			break;
		case OPH_BYTE:
			tmp = gsl_stats_char_correlation((char *) data, 1, (char *) data2, 1, data_len);
			break;
		case OPH_LONG:
			tmp = gsl_stats_long_correlation((long int *) data, 1, (long int *) data2, 1, data_len);
			break;
		case OPH_FLOAT:
			tmp = gsl_stats_float_correlation((float *) data, 1, (float *) data2, 1, data_len);
			break;
		case OPH_DOUBLE:
			tmp = gsl_stats_correlation((double *) data, 1, (double *) data2, 1, data_len);
			break;
		default:;
	}
	if (core_oph_type_cast(&tmp, output_data, OPH_DOUBLE, out_data_type, NULL)) {
		pmesg(1, __FILE__, __LINE__, "Error casting output\n");
		return 1;
	}
	return 0;
}

my_bool oph_gsl_correlation_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{

	int i = 0;

	if (args->arg_count != 4) {
		strcpy(message, "ERROR: Wrong arguments! oph_gsl_correlation(input_OPH_TYPE, output_OPH_TYPE, measure1, measure2)");
		return 1;
	}

	for (i = 0; i < 4; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_gsl_correlation function");
			return 1;
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;

	return 0;
}

void oph_gsl_correlation_deinit(UDF_INIT * initid)
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
		if ((&(((oph_stringPtr) initid->extension)[0]))->content) {
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

char *oph_gsl_correlation(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
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

	oph_stringPtr measure, measure2, output;

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

		char *data_type1 = args->args[0], *data_type2 = NULL;
		unsigned long data_type_length1, data_type_length2 = 0;
		for (data_type_length1 = 0; data_type_length1 < args->lengths[0]; ++data_type_length1)
			if (data_type1[data_type_length1] == '|') {
				data_type_length2 = args->lengths[0] - data_type_length1 - 1;
				data_type2 = data_type1 + data_type_length1 + 1;
				break;
			}
		if (!data_type2 || !data_type_length2) {
			pmesg(1, __FILE__, __LINE__, "Input data type error\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		measure = &(((oph_stringPtr) initid->extension)[0]);
		core_set_type(measure, data_type1, &data_type_length1);
		measure->length = &(args->lengths[2]);
		measure->content = NULL;

		measure2 = &(((oph_stringPtr) initid->extension)[1]);
		core_set_type(measure2, data_type2, &data_type_length2);
		measure2->length = &(args->lengths[3]);
		measure2->content = NULL;

		output = (oph_stringPtr) initid->ptr;
		core_set_type(output, args->args[1], &(args->lengths[1]));
		output->numelem = 1;

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
		if (core_set_elemsize(measure2)) {
			pmesg(1, __FILE__, __LINE__, "Error on setting measure elements size\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (core_set_numelem(measure2)) {
			pmesg(1, __FILE__, __LINE__, "Error on counting measure elements\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}


		measure->content = calloc(measure->numelem, sizeof(double));
		if (!measure->content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating extension\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		measure2->content = calloc(measure2->numelem, sizeof(double));
		if (!measure2->content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating extension\n");
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
	measure2 = &(((oph_stringPtr) initid->extension)[1]);
	output = (oph_stringPtr) initid->ptr;

	int i;
	switch (measure->type) {
		case OPH_INT:
			for (i = 0; i < measure->numelem; i++)
				((double *) (measure->content))[i] = ((int *) (args->args[2]))[i];
			break;
		case OPH_SHORT:
			for (i = 0; i < measure->numelem; i++)
				((double *) (measure->content))[i] = ((short *) (args->args[2]))[i];
			break;
		case OPH_BYTE:
			for (i = 0; i < measure->numelem; i++)
				((double *) (measure->content))[i] = ((char *) (args->args[2]))[i];
			break;
		case OPH_LONG:
			for (i = 0; i < measure->numelem; i++)
				((double *) (measure->content))[i] = ((long long *) (args->args[2]))[i];
			break;
		case OPH_FLOAT:
			for (i = 0; i < measure->numelem; i++)
				((double *) (measure->content))[i] = ((float *) (args->args[2]))[i];
			break;
		case OPH_DOUBLE:
			for (i = 0; i < measure->numelem; i++)
				((double *) (measure->content))[i] = ((double *) (args->args[2]))[i];
			break;
		default:;
	}
	switch (measure2->type) {
		case OPH_INT:
			for (i = 0; i < measure2->numelem; i++)
				((double *) (measure2->content))[i] = ((int *) (args->args[3]))[i];
			break;
		case OPH_SHORT:
			for (i = 0; i < measure2->numelem; i++)
				((double *) (measure2->content))[i] = ((short *) (args->args[3]))[i];
			break;
		case OPH_BYTE:
			for (i = 0; i < measure2->numelem; i++)
				((double *) (measure2->content))[i] = ((char *) (args->args[3]))[i];
			break;
		case OPH_LONG:
			for (i = 0; i < measure2->numelem; i++)
				((double *) (measure2->content))[i] = ((long long *) (args->args[3]))[i];
			break;
		case OPH_FLOAT:
			for (i = 0; i < measure2->numelem; i++)
				((double *) (measure2->content))[i] = ((float *) (args->args[3]))[i];
			break;
		case OPH_DOUBLE:
			for (i = 0; i < measure2->numelem; i++)
				((double *) (measure2->content))[i] = ((double *) (args->args[3]))[i];
			break;
		default:;
	}

	// compute quantiles
	if (oph_gsl_correlation_produce(measure->content, measure2->content, measure->numelem, measure->type, output->content, output->type)) {
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
