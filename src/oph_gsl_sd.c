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

#include "oph_gsl_sd.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/

int core_oph_gsl_sd(oph_stringPtr byte_array, oph_gsl_sd_struct * result)
{
	switch (byte_array->type) {
		case OPH_DOUBLE:
			{
				double *d = (double *) byte_array->content;
				double res = gsl_stats_sd(d, 1, (size_t) byte_array->numelem);
				if (isnan(res)) {
					unsigned long i, max = 0;
					if (!result->wnan)
						result->wnan = (char *) malloc(*(byte_array->length));
					if (!result->wnan) {
						pmesg(1, __FILE__, __LINE__, "Error allocating result string\n");
						return -1;
					}
					double *tmp = (double *) result->wnan;
					for (i = 0; i < byte_array->numelem; i++, d++)
						if (!isnan(*d)) {
							tmp[max] = *d;
							max++;
						}
					if (max)
						res = gsl_stats_sd(tmp, 1, max);
				}
				if (core_oph_type_cast(&res, result->result, byte_array->type, result->result_type, NULL))
					return -1;
				break;
			}
		case OPH_FLOAT:
			{
				float *d = (float *) byte_array->content;
				float res = gsl_stats_float_sd(d, 1, (size_t) byte_array->numelem);
				if (isnan(res)) {
					unsigned long i, max = 0;
					if (!result->wnan)
						result->wnan = (char *) malloc(*(byte_array->length));
					if (!result->wnan) {
						pmesg(1, __FILE__, __LINE__, "Error allocating result string\n");
						return -1;
					}
					float *tmp = (float *) result->wnan;
					for (i = 0; i < byte_array->numelem; i++, d++)
						if (!isnan(*d)) {
							tmp[max] = *d;
							max++;
						}
					if (max)
						res = gsl_stats_float_sd(tmp, 1, max);
				}
				if (core_oph_type_cast(&res, result->result, byte_array->type, result->result_type, NULL))
					return -1;
				break;
			}
		case OPH_INT:
			{
				int *d = (int *) byte_array->content;
				int res = gsl_stats_int_sd(d, 1, (size_t) byte_array->numelem);
				if (core_oph_type_cast(&res, result->result, byte_array->type, result->result_type, NULL))
					return -1;
				break;
			}
		case OPH_SHORT:
			{
				short *d = (short *) byte_array->content;
				short res = gsl_stats_short_sd(d, 1, (size_t) byte_array->numelem);
				if (core_oph_type_cast(&res, result->result, byte_array->type, result->result_type, NULL))
					return -1;
				break;
			}
		case OPH_BYTE:
			{
				char *d = (char *) byte_array->content;
				char res = gsl_stats_char_sd(d, 1, (size_t) byte_array->numelem);
				if (core_oph_type_cast(&res, result->result, byte_array->type, result->result_type, NULL))
					return -1;
				break;
			}
		case OPH_LONG:
			{
				long long *d = (long long *) byte_array->content;
				long long res = gsl_stats_long_sd((const long int *) d, 1, (size_t) byte_array->numelem);
				if (core_oph_type_cast(&res, result->result, byte_array->type, result->result_type, NULL))
					return -1;
				break;
			}
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			return -1;
	}
	return 0;
}

my_bool oph_gsl_sd_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if (args->arg_count != 3) {
		strcpy(message, "ERROR: Wrong arguments! oph_gsl_sd(input_OPH_TYPE, output_OPH_TYPE, measure)");
		return 1;
	}

	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_gsl_sd function");
			return 1;
		}
	}

	initid->ptr = NULL;
	return 0;
}

void oph_gsl_sd_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		oph_gsl_sd_struct *gsl_sd_struct = (oph_gsl_sd_struct *) (initid->ptr);
		if (gsl_sd_struct->result) {
			free(gsl_sd_struct->result);
			gsl_sd_struct->result = 0;
		}
		if (gsl_sd_struct->wnan) {
			free(gsl_sd_struct->wnan);
			gsl_sd_struct->wnan = 0;
		}

		free(initid->ptr);
		initid->ptr = NULL;
	}
}

char *oph_gsl_sd(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	oph_string measure;

	int res = 0;

	core_set_type(&(measure), args->args[0], &(args->lengths[0]));

	measure.content = args->args[2];
	measure.length = &(args->lengths[2]);

	core_set_elemsize(&(measure));
	if (core_set_numelem(&(measure))) {
		pmesg(1, __FILE__, __LINE__, "Error on counting elements\n");
		*length = 0;
		*is_null = 1;
		*error = 1;
		return NULL;
	}

	oph_gsl_sd_struct *gsl_sd_struct;
	if (!initid->ptr) {
		initid->ptr = (char *) malloc(sizeof(oph_gsl_sd_struct));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

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

		gsl_sd_struct = (oph_gsl_sd_struct *) (initid->ptr);
		gsl_sd_struct->wnan = 0;
		gsl_sd_struct->result_type = output_array.type;
		gsl_sd_struct->result_elemsize = output_array.elemsize;
		gsl_sd_struct->result = (char *) malloc(gsl_sd_struct->result_elemsize);
		if (!gsl_sd_struct->result) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	} else
		gsl_sd_struct = (oph_gsl_sd_struct *) (initid->ptr);

	res = core_oph_gsl_sd(&measure, gsl_sd_struct);
	if (res) {
		pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
		*length = 0;
		*is_null = 1;
		*error = 1;
		return NULL;
	}
	*length = gsl_sd_struct->result_elemsize;
	*error = 0;
	*is_null = 0;

	return ((oph_gsl_sd_struct *) initid->ptr)->result;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
