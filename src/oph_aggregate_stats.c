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

#include "oph_aggregate_stats.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_aggregate_stats_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	if (args->arg_count < 3 || args->arg_count > 4) {
		strcpy(message, "ERROR: Wrong arguments! oph_aggregate_stats(input_OPH_TYPE, output_OPH_TYPE, measure, [MASK])");
		return 1;
	}

	int i;
	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_aggregate_stats function");
			return 1;
		}
	}

	oph_agg_stats_data *dat = (oph_agg_stats_data *) calloc(1, sizeof(oph_agg_stats_data));
	if (!dat) {
		strcpy(message, "ERROR: Unable to allocate result string");
		return 1;
	}

	dat->count = 0;
	dat->partials = NULL;
	dat->sum1 = 0;
	dat->sum2 = 0;
	dat->sum3 = 0;
	dat->sum4 = 0;
	dat->max = 0;
	dat->min = 0;

	dat->result.content = NULL;
	dat->result.length = NULL;
	dat->result.type = INVALID_TYPE;
	dat->result.elemsize = 0;
	dat->result.numelem = 0;

	dat->mask.content = NULL;
	dat->mask.length = NULL;
	dat->mask.type = INVALID_TYPE;
	dat->mask.elemsize = 0;
	dat->mask.numelem = 0;

	dat->measure.content = NULL;
	dat->measure.length = NULL;
	dat->measure.type = INVALID_TYPE;
	dat->measure.elemsize = 0;
	dat->measure.numelem = 0;

	initid->maybe_null = 1;
	initid->ptr = (char *) dat;

	return 0;
}

void oph_aggregate_stats_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		oph_agg_stats_data *dat = (oph_agg_stats_data *) initid->ptr;
		if (dat->result.content) {
			free(dat->result.content);
			dat->result.content = NULL;
		}
		if (dat->result.length) {
			free(dat->result.length);
			dat->result.length = NULL;
		}
		if (dat->partials) {
			int i = 0;
			if (dat->sum1) {
				if (dat->partials[i].content) {
					free(dat->partials[i].content);
					dat->partials[i].content = NULL;
				}
				i++;
			}
			if (dat->sum2) {
				if (dat->partials[i].content) {
					free(dat->partials[i].content);
					dat->partials[i].content = NULL;
				}
				i++;
			}
			if (dat->sum3) {
				if (dat->partials[i].content) {
					free(dat->partials[i].content);
					dat->partials[i].content = NULL;
				}
				i++;
			}
			if (dat->sum4) {
				if (dat->partials[i].content) {
					free(dat->partials[i].content);
					dat->partials[i].content = NULL;
				}
				i++;
			}
			if (dat->max) {
				if (dat->partials[i].content) {
					free(dat->partials[i].content);
					dat->partials[i].content = NULL;
				}
				i++;
			}
			if (dat->min) {
				if (dat->partials[i].content) {
					free(dat->partials[i].content);
					dat->partials[i].content = NULL;
				}
				i++;
			}
			free(dat->partials);
			dat->partials = NULL;
		}
		if (dat->mask.content) {
			free(dat->mask.content);
			dat->mask.content = NULL;
		}
		if (dat->measure.length) {
			free(dat->measure.length);
			dat->measure.length = NULL;
		}
		free(initid->ptr);
		initid->ptr = NULL;
	}
}

void oph_aggregate_stats_reset(UDF_INIT * initid, UDF_ARGS * args, char *is_null, char *error)
{
	oph_aggregate_stats_clear(initid, is_null, error);
	oph_aggregate_stats_add(initid, args, is_null, error);
}

void oph_aggregate_stats_clear(UDF_INIT * initid, char *is_null, char *error)
{
	if (initid->ptr) {
		oph_agg_stats_data *dat = (oph_agg_stats_data *) initid->ptr;
		dat->count = 0;
	}
	*is_null = 0;
	*error = 0;
}

void oph_aggregate_stats_add(UDF_INIT * initid, UDF_ARGS * args, char *is_null, char *error)
{
	if (*error != 0)
		return;
	if (args->args[2]) {
		oph_agg_stats_data *dat = (oph_agg_stats_data *) initid->ptr;

		/* Setting of the aggregate result */
		if (!dat->result.content) {
			//It's the first row

			// default values
			unsigned long def_mask_len = MASK_LEN;
			dat->mask.content = (char *) calloc(MASK_LEN + 1, sizeof(char));
			if (!dat->mask.content) {
				pmesg(1, __FILE__, __LINE__, "Error allocating mask content\n");
				*error = 1;
				return;
			}
			core_strncpy(dat->mask.content, DEFAULT_MASK, &def_mask_len);

			dat->mask.length = (unsigned long *) calloc(1, sizeof(unsigned long));
			if (!dat->mask.length) {
				pmesg(1, __FILE__, __LINE__, "Error allocating mask length\n");
				*error = 1;
				return;
			}
			*(dat->mask.length) = def_mask_len;

			core_set_type(&(dat->measure), NULL, 0);
			dat->measure.length = (unsigned long *) calloc(1, sizeof(unsigned long));
			if (!dat->measure.length) {
				pmesg(1, __FILE__, __LINE__, "Error allocating measure length\n");
				*error = 1;
				return;
			}
			*(dat->measure.length) = args->lengths[2];

			core_set_type(&(dat->measure), args->args[0], &(args->lengths[0]));
			if (dat->measure.type != OPH_SHORT && dat->measure.type != OPH_BYTE && dat->measure.type != OPH_INT && dat->measure.type != OPH_LONG && dat->measure.type != OPH_FLOAT
			    && dat->measure.type != OPH_DOUBLE) {
				pmesg(1, __FILE__, __LINE__, "Invalid type\n");
				*error = 1;
				return;
			}
			if (args->arg_count > 3) {
				core_strncpy(dat->mask.content, args->args[3], &(args->lengths[3]));
				*(dat->mask.length) = args->lengths[3];
			}

			if (core_set_elemsize(&(dat->measure))) {
				pmesg(1, __FILE__, __LINE__, "Error on setting measure elements size\n");
				*error = 1;
				return;
			}
			if (core_set_numelem(&(dat->measure))) {
				pmesg(1, __FILE__, __LINE__, "Error on counting measure elements\n");
				*error = 1;
				return;
			}

			core_set_type(&dat->result, args->args[1], &(args->lengths[1]));
			if (!dat->result.type) {
				pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
				*error = 1;
				return;
			}
			if (core_set_elemsize(&dat->result)) {
				pmesg(1, __FILE__, __LINE__, "Unable to recognize measures type\n");
				*error = 1;
				return;
			}
			// mask processing
			int i;
			for (i = 0; i < *(dat->mask.length); i++) {
				if (dat->mask.content[i] == '1') {
					(dat->mask.numelem)++;	// count 1s
					switch (i) {
						case 0:	// mean
							dat->sum1 = 1;	// sum{x_i} needed
							break;
						case 1:	// variance
							dat->sum1 = 1;
							dat->sum2 = 1;	// sum{(x_i)^2} needed
							break;
						case 2:	// std dev
							dat->sum1 = 1;
							dat->sum2 = 1;
							break;
						case 3:	// skew
							dat->sum1 = 1;
							dat->sum2 = 1;
							dat->sum3 = 1;	// sum{(x_i)^3} needed
							break;
						case 4:	// kurtosis
							dat->sum1 = 1;
							dat->sum2 = 1;
							dat->sum3 = 1;
							dat->sum4 = 1;	// sum{(x_i)^4} needed
							break;
						case 5:	// max
							dat->max = 1;	// array with max values needed
							break;
						case 6:	// min
							dat->min = 1;	// array with min values needed
					}
				}
			}
			if (dat->mask.numelem == 0) {
				pmesg(1, __FILE__, __LINE__, "Invalid mask\n");
				*error = 1;
				return;
			}
			// output array allocation
			dat->result.numelem = dat->mask.numelem * dat->measure.numelem;	// In future use a structed type of mask.numelem fields
			unsigned long outlen = dat->result.numelem * dat->result.elemsize;
			dat->result.length = (unsigned long *) calloc(1, sizeof(unsigned long));
			if (!dat->result.length) {
				pmesg(1, __FILE__, __LINE__, "Error allocating result length\n");
				*error = 1;
				return;
			}
			*(dat->result.length) = outlen;
			dat->result.content = (char *) calloc(1, *(dat->result.length));
			if (!dat->result.content) {
				pmesg(1, __FILE__, __LINE__, "Error allocating output array\n");
				*error = 1;
				return;
			}
			// partial results array allocation
			int size = 0;
			if (dat->sum1)
				size++;
			if (dat->sum2)
				size++;
			if (dat->sum3)
				size++;
			if (dat->sum4)
				size++;
			if (dat->max)
				size++;
			if (dat->min)
				size++;
			dat->partials = (oph_stringPtr) calloc(size, sizeof(oph_string));
			if (!dat->partials) {
				pmesg(1, __FILE__, __LINE__, "Error allocating intermediate arrays\n");
				*error = 1;
				return;
			}
			for (i = 0; i < size; i++) {
				dat->partials[i].type = dat->measure.type;
				dat->partials[i].elemsize = dat->measure.elemsize;
				dat->partials[i].numelem = dat->measure.numelem;
				dat->partials[i].length = dat->measure.length;
				dat->partials[i].content = (char *) calloc(1, *(dat->partials[i].length));
				if (!dat->partials[i].content) {
					pmesg(1, __FILE__, __LINE__, "Error allocating intermediate array\n");
					*error = 1;
					return;
				}
			}
		}

		if (!dat->count) {
			//It's the first row or the first row in the group of the GROUP BY clause

			int i = 0;
			int j;
			// copy input values
			if (dat->sum1) {
				memcpy((void *) (dat->partials[i].content), (void *) (args->args[2]), *(dat->partials[i].length));
				i++;
			}
			// copy input values squared
			if (dat->sum2) {
				switch (dat->measure.type) {
					case OPH_INT:
						{
							int val_i;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_i = ((int *) (args->args[2]))[j];
								((int *) (dat->partials[i].content))[j] = val_i * val_i;
							}
							break;
						}
					case OPH_LONG:
						{
							long long val_l;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_l = ((long long *) (args->args[2]))[j];
								((long long *) (dat->partials[i].content))[j] = val_l * val_l;
							}
							break;
						}
					case OPH_FLOAT:
						{
							float val_f;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_f = ((float *) (args->args[2]))[j];
								((float *) (dat->partials[i].content))[j] = val_f * val_f;
							}
							break;
						}
					case OPH_DOUBLE:
						{
							double val_d;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_d = ((double *) (args->args[2]))[j];
								((double *) (dat->partials[i].content))[j] = val_d * val_d;
							}
						}
					case OPH_SHORT:
						{
							short val_i;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_i = ((short *) (args->args[2]))[j];
								((short *) (dat->partials[i].content))[j] = val_i * val_i;
							}
							break;
						}
					case OPH_BYTE:
						{
							char val_i;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_i = ((char *) (args->args[2]))[j];
								((char *) (dat->partials[i].content))[j] = val_i * val_i;
							}
							break;
						}
					case OPH_COMPLEX_INT:
					case OPH_COMPLEX_LONG:
					case OPH_COMPLEX_FLOAT:
					case OPH_COMPLEX_DOUBLE:
					case INVALID_TYPE:
						break;
				}
				i++;
			}
			// copy input values raised to 3
			if (dat->sum3) {
				switch (dat->measure.type) {
					case OPH_INT:
						{
							int val_i;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_i = ((int *) (args->args[2]))[j];
								((int *) (dat->partials[i].content))[j] = val_i * val_i * val_i;
							}
							break;
						}
					case OPH_LONG:
						{
							long long val_l;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_l = ((long long *) (args->args[2]))[j];
								((long long *) (dat->partials[i].content))[j] = val_l * val_l * val_l;
							}
							break;
						}
					case OPH_FLOAT:
						{
							float val_f;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_f = ((float *) (args->args[2]))[j];
								((float *) (dat->partials[i].content))[j] = val_f * val_f * val_f;
							}
							break;
						}
					case OPH_DOUBLE:
						{
							double val_d;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_d = ((double *) (args->args[2]))[j];
								((double *) (dat->partials[i].content))[j] = val_d * val_d * val_d;
							}
						}
					case OPH_SHORT:
						{
							short val_i;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_i = ((short *) (args->args[2]))[j];
								((short *) (dat->partials[i].content))[j] = val_i * val_i * val_i;
							}
							break;
						}
					case OPH_BYTE:
						{
							char val_i;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_i = ((char *) (args->args[2]))[j];
								((char *) (dat->partials[i].content))[j] = val_i * val_i * val_i;
							}
							break;
						}
					case OPH_COMPLEX_INT:
					case OPH_COMPLEX_LONG:
					case OPH_COMPLEX_FLOAT:
					case OPH_COMPLEX_DOUBLE:
					case INVALID_TYPE:
						break;
				}
				i++;
			}
			// copy input values raised to 4
			if (dat->sum4) {
				switch (dat->measure.type) {
					case OPH_INT:
						{
							int val_i;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_i = ((int *) (args->args[2]))[j];
								((int *) (dat->partials[i].content))[j] = val_i * val_i * val_i * val_i;
							}
							break;
						}
					case OPH_LONG:
						{
							long long val_l;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_l = ((long long *) (args->args[2]))[j];
								((long long *) (dat->partials[i].content))[j] = val_l * val_l * val_l * val_l;
							}
							break;
						}
					case OPH_FLOAT:
						{
							float val_f;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_f = ((float *) (args->args[2]))[j];
								((float *) (dat->partials[i].content))[j] = val_f * val_f * val_f * val_f;
							}
							break;
						}
					case OPH_DOUBLE:
						{
							double val_d;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_d = ((double *) (args->args[2]))[j];
								((double *) (dat->partials[i].content))[j] = val_d * val_d * val_d * val_d;
							}
						}
					case OPH_SHORT:
						{
							short val_i;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_i = ((short *) (args->args[2]))[j];
								((short *) (dat->partials[i].content))[j] = val_i * val_i * val_i * val_i;
							}
							break;
						}
					case OPH_BYTE:
						{
							char val_i;
							for (j = 0; j < dat->measure.numelem; j++) {
								val_i = ((char *) (args->args[2]))[j];
								((char *) (dat->partials[i].content))[j] = val_i * val_i * val_i * val_i;
							}
							break;
						}
					case OPH_COMPLEX_INT:
					case OPH_COMPLEX_LONG:
					case OPH_COMPLEX_FLOAT:
					case OPH_COMPLEX_DOUBLE:
					case INVALID_TYPE:
						break;
				}
				i++;
			}
			// copy input values
			if (dat->max) {
				memcpy(dat->partials[i].content, (void *) (args->args[2]), *(dat->partials[i].length));
				i++;
			}
			// copy input values
			if (dat->min) {
				memcpy(dat->partials[i].content, (void *) (args->args[2]), *(dat->partials[i].length));
				i++;
			}
		} else {
			// Not the first row in the group => execute next aggregation step

			dat->measure.content = args->args[2];

			int i = 0;
			if (dat->sum1) {
				if (core_oph_sum_array(&(dat->measure), &(dat->partials[i]), dat->partials[i].content)) {
					pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
					*error = 1;
					return;
				}
				i++;
			}
			if (dat->sum2) {
				if (core_oph_sum2_array(&(dat->measure), &(dat->partials[i]), dat->partials[i].content)) {
					pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
					*error = 1;
					return;
				}
				i++;
			}
			if (dat->sum3) {
				if (core_oph_sum3_array(&(dat->measure), &(dat->partials[i]), dat->partials[i].content)) {
					pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
					*error = 1;
					return;
				}
				i++;
			}
			if (dat->sum4) {
				if (core_oph_sum4_array(&(dat->measure), &(dat->partials[i]), dat->partials[i].content)) {
					pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
					*error = 1;
					return;
				}
				i++;
			}
			if (dat->max) {
				if (core_oph_max_array(&(dat->measure), &(dat->partials[i]), dat->partials[i].content)) {
					pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
					*error = 1;
					return;
				}
				i++;
			}
			if (dat->min) {
				if (core_oph_min_array(&(dat->measure), &(dat->partials[i]), dat->partials[i].content)) {
					pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
					*error = 1;
					return;
				}
				i++;
			}
		}
		dat->count++;
	}
}

// Internal utility functions
float internal_mean_i(int n, int sum1)
{
	return (float) sum1 / (float) n;
}

double internal_mean_l(int n, long long sum1)
{
	return (double) sum1 / (double) n;
}

float internal_mean_f(int n, float sum1)
{
	return sum1 / n;
}

double internal_mean_d(int n, double sum1)
{
	return sum1 / n;
}

float internal_mean_s(int n, short sum1)
{
	return (float) sum1 / (float) n;
}

float internal_mean_b(int n, char sum1)
{
	return (float) sum1 / (float) n;
}

float internal_variance_i(int n, int sum1, int sum2)
{
	float mu = (float) sum1 / n;
	return ((float) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
}

double internal_variance_l(int n, long long sum1, long long sum2)
{
	double mu = (double) sum1 / n;
	return ((double) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
}

float internal_variance_f(int n, float sum1, float sum2)
{
	float mu = (float) sum1 / n;
	return ((float) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
}

double internal_variance_d(int n, double sum1, double sum2)
{
	double mu = sum1 / n;
	return ((sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
}

float internal_variance_s(int n, short sum1, short sum2)
{
	float mu = (float) sum1 / n;
	return ((float) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
}

float internal_variance_b(int n, char sum1, char sum2)
{
	float mu = (float) sum1 / n;
	return ((float) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
}

float internal_stddev_i(int n, int sum1, int sum2)
{
	float mu = (float) sum1 / n;
	float var = (float) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1);
	return sqrt(var);
}

double internal_stddev_l(int n, long long sum1, long long sum2)
{
	double mu = (double) sum1 / n;
	double var = (double) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1);
	return sqrt(var);
}

float internal_stddev_f(int n, float sum1, float sum2)
{
	float mu = (float) sum1 / n;
	float var = (float) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1);
	return sqrt(var);
}

double internal_stddev_d(int n, double sum1, double sum2)
{
	double mu = sum1 / n;
	double var = (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1);
	return sqrt(var);
}

float internal_stddev_s(int n, short sum1, short sum2)
{
	float mu = (float) sum1 / n;
	float var = (float) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1);
	return sqrt(var);
}

float internal_stddev_b(int n, char sum1, char sum2)
{
	float mu = (float) sum1 / n;
	float var = (float) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1);
	return sqrt(var);
}

float internal_skew_i(int n, int sum1, int sum2, int sum3)
{
	float mu = (float) sum1 / n;
	float sigma = sqrt((float) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
	return (sum3 - 3 * mu * sum2 + 3 * mu * mu * sum1 - mu * mu * mu * n) / (n * sigma * sigma * sigma);
}

double internal_skew_l(int n, long long sum1, long long sum2, long long sum3)
{
	double mu = (double) sum1 / n;
	double sigma = sqrt((double) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
	return (sum3 - 3 * mu * sum2 + 3 * mu * mu * sum1 - mu * mu * mu * n) / (n * sigma * sigma * sigma);
}

float internal_skew_f(int n, float sum1, float sum2, float sum3)
{
	float mu = (float) sum1 / n;
	float sigma = sqrt((float) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
	return (sum3 - 3 * mu * sum2 + 3 * mu * mu * sum1 - mu * mu * mu * n) / (n * sigma * sigma * sigma);
}

double internal_skew_d(int n, double sum1, double sum2, double sum3)
{
	double mu = (double) sum1 / n;
	double sigma = sqrt((double) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
	return (sum3 - 3 * mu * sum2 + 3 * mu * mu * sum1 - mu * mu * mu * n) / (n * sigma * sigma * sigma);
}

float internal_skew_s(int n, short sum1, short sum2, short sum3)
{
	float mu = (float) sum1 / n;
	float sigma = sqrt((float) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
	return (sum3 - 3 * mu * sum2 + 3 * mu * mu * sum1 - mu * mu * mu * n) / (n * sigma * sigma * sigma);
}

float internal_skew_b(int n, char sum1, char sum2, char sum3)
{
	float mu = (float) sum1 / n;
	float sigma = sqrt((float) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
	return (sum3 - 3 * mu * sum2 + 3 * mu * mu * sum1 - mu * mu * mu * n) / (n * sigma * sigma * sigma);
}

float internal_kurtosis_i(int n, int sum1, int sum2, int sum3, int sum4)
{
	float mu = (float) sum1 / n;
	float sigma = sqrt((float) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
	return ((sum4 - 4 * mu * sum3 + 6 * mu * mu * sum2 - 4 * mu * mu * mu * sum1 + mu * mu * mu * mu * n) / (n * sigma * sigma * sigma * sigma)) - 3;
}

double internal_kurtosis_l(int n, long long sum1, long long sum2, long long sum3, long long sum4)
{
	double mu = (double) sum1 / n;
	double sigma = sqrt((double) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
	return ((sum4 - 4 * mu * sum3 + 6 * mu * mu * sum2 - 4 * mu * mu * mu * sum1 + mu * mu * mu * mu * n) / (n * sigma * sigma * sigma * sigma)) - 3;
}

float internal_kurtosis_f(int n, float sum1, float sum2, float sum3, float sum4)
{
	float mu = (float) sum1 / n;
	float sigma = sqrt((float) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
	return ((sum4 - 4 * mu * sum3 + 6 * mu * mu * sum2 - 4 * mu * mu * mu * sum1 + mu * mu * mu * mu * n) / (n * sigma * sigma * sigma * sigma)) - 3;
}

double internal_kurtosis_d(int n, double sum1, double sum2, double sum3, double sum4)
{
	double mu = (double) sum1 / n;
	double sigma = sqrt((double) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
	return ((sum4 - 4 * mu * sum3 + 6 * mu * mu * sum2 - 4 * mu * mu * mu * sum1 + mu * mu * mu * mu * n) / (n * sigma * sigma * sigma * sigma)) - 3;
}

float internal_kurtosis_s(int n, int sum1, short sum2, short sum3, short sum4)
{
	float mu = (float) sum1 / n;
	float sigma = sqrt((float) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
	return ((sum4 - 4 * mu * sum3 + 6 * mu * mu * sum2 - 4 * mu * mu * mu * sum1 + mu * mu * mu * mu * n) / (n * sigma * sigma * sigma * sigma)) - 3;
}

float internal_kurtosis_b(int n, char sum1, char sum2, char sum3, char sum4)
{
	float mu = (float) sum1 / n;
	float sigma = sqrt((float) (sum2 + n * mu * mu - 2 * mu * sum1) / (n - 1));
	return ((sum4 - 4 * mu * sum3 + 6 * mu * mu * sum2 - 4 * mu * mu * mu * sum1 + mu * mu * mu * mu * n) / (n * sigma * sigma * sigma * sigma)) - 3;
}

char *oph_aggregate_stats(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	if (*error != 0) {
		pmesg(1, __FILE__, __LINE__, "Error on adding elements\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	oph_agg_stats_data *dat = (oph_agg_stats_data *) initid->ptr;

	if (dat->count == 0) {
		*length = 0;
		*error = 0;
		*is_null = 1;
		return NULL;
	}
	if (!dat->result.content) {
		*length = 0;
		*error = 0;
		*is_null = 1;
		return NULL;
	}

	int i = 0, j, k, q = 0;
	for (j = 0; j < dat->mask.numelem; j++) {
		while (dat->mask.content[i] == '0')
			i++;
		if (dat->measure.type == OPH_INT) {
			switch (i) {
				case 0:	// mean
					{
						float d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_mean_i(dat->count, ((int *) (dat->partials[0].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_FLOAT, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 1:	// variance
					{
						float d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_variance_i(dat->count, ((int *) (dat->partials[0].content))[q], ((int *) (dat->partials[1].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_FLOAT, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 2:	// std dev
					{
						float d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_stddev_i(dat->count, ((int *) (dat->partials[0].content))[q], ((int *) (dat->partials[1].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_FLOAT, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 3:	// skew
					{
						float d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_skew_i(dat->count, ((int *) (dat->partials[0].content))[q], ((int *) (dat->partials[1].content))[q],
									    ((int *) (dat->partials[2].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_FLOAT, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 4:	// kurtosis
					{
						float d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_kurtosis_i(dat->count, ((int *) (dat->partials[0].content))[q], ((int *) (dat->partials[1].content))[q],
										((int *) (dat->partials[2].content))[q], ((int *) (dat->partials[3].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_FLOAT, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 5:	// max
					{
						int s;
						if (!dat->sum1) {
							s = 0;
						} else if (!dat->sum2) {
							s = 1;
						} else if (!dat->sum3) {
							s = 2;
						} else if (!dat->sum4) {
							s = 3;
						} else {
							s = 4;
						}
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							if (core_oph_type_cast
							    (dat->partials[s].content + q * dat->measure.elemsize, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type,
							     dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 6:	// min
					{
						int s;
						if (!dat->sum1) {
							s = 0;
						} else if (!dat->sum2) {
							s = 1;
						} else if (!dat->sum3) {
							s = 2;
						} else if (!dat->sum4) {
							s = 3;
						} else {
							s = 4;
						}
						if (dat->max) {
							s++;
						}
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							if (core_oph_type_cast
							    (dat->partials[s].content + q * dat->measure.elemsize, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type,
							     dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
					}
			}
		} else if (dat->measure.type == OPH_LONG) {
			switch (i) {
				case 0:	// mean
					{
						double d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_mean_l(dat->count, ((long long *) (dat->partials[0].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_DOUBLE, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 1:	// variance
					{
						double d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_variance_l(dat->count, ((long long *) (dat->partials[0].content))[q], ((long long *) (dat->partials[1].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_DOUBLE, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 2:	// std dev
					{
						double d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_stddev_l(dat->count, ((long long *) (dat->partials[0].content))[q], ((long long *) (dat->partials[1].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_DOUBLE, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 3:	// skew
					{
						double d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_skew_l(dat->count, ((long long *) (dat->partials[0].content))[q], ((long long *) (dat->partials[1].content))[q],
									    ((long long *) (dat->partials[2].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_DOUBLE, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 4:	// kurtosis
					{
						double d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_kurtosis_l(dat->count, ((long long *) (dat->partials[0].content))[q], ((long long *) (dat->partials[1].content))[q],
										((long long *) (dat->partials[2].content))[q], ((long long *) (dat->partials[3].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_DOUBLE, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 5:	// max
					{
						int s;
						if (!dat->sum1) {
							s = 0;
						} else if (!dat->sum2) {
							s = 1;
						} else if (!dat->sum3) {
							s = 2;
						} else if (!dat->sum4) {
							s = 3;
						} else {
							s = 4;
						}
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							if (core_oph_type_cast
							    (dat->partials[s].content + q * dat->measure.elemsize, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type,
							     dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 6:	// min
					{
						int s;
						if (!dat->sum1) {
							s = 0;
						} else if (!dat->sum2) {
							s = 1;
						} else if (!dat->sum3) {
							s = 2;
						} else if (!dat->sum4) {
							s = 3;
						} else {
							s = 4;
						}
						if (dat->max) {
							s++;
						}
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							if (core_oph_type_cast
							    (dat->partials[s].content + q * dat->measure.elemsize, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type,
							     dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
					}
			}
		} else if (dat->measure.type == OPH_FLOAT) {
			float d;
			switch (i) {
				case 0:	// mean
					for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
						d = internal_mean_f(dat->count, ((float *) (dat->partials[0].content))[q]);
						if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type, dat->result.type, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error in casting\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						q++;
					}
					break;
				case 1:	// variance
					for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
						d = internal_variance_f(dat->count, ((float *) (dat->partials[0].content))[q], ((float *) (dat->partials[1].content))[q]);
						if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type, dat->result.type, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error in casting\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						q++;
					}
					break;
				case 2:	// std dev
					for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
						d = internal_stddev_f(dat->count, ((float *) (dat->partials[0].content))[q], ((float *) (dat->partials[1].content))[q]);
						if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type, dat->result.type, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error in casting\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						q++;
					}
					break;
				case 3:	// skew
					for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
						d = internal_skew_f(dat->count, ((float *) (dat->partials[0].content))[q], ((float *) (dat->partials[1].content))[q],
								    ((float *) (dat->partials[2].content))[q]);
						if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type, dat->result.type, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error in casting\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						q++;
					}
					break;
				case 4:	// kurtosis
					for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
						d = internal_kurtosis_f(dat->count, ((float *) (dat->partials[0].content))[q], ((float *) (dat->partials[1].content))[q],
									((float *) (dat->partials[2].content))[q], ((float *) (dat->partials[3].content))[q]);
						if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type, dat->result.type, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error in casting\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						q++;
					}
					break;
				case 5:	// max
					{
						int s;
						if (!dat->sum1) {
							s = 0;
						} else if (!dat->sum2) {
							s = 1;
						} else if (!dat->sum3) {
							s = 2;
						} else if (!dat->sum4) {
							s = 3;
						} else {
							s = 4;
						}
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							if (core_oph_type_cast
							    (dat->partials[s].content + q * dat->measure.elemsize, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type,
							     dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 6:	// min
					{
						int s;
						if (!dat->sum1) {
							s = 0;
						} else if (!dat->sum2) {
							s = 1;
						} else if (!dat->sum3) {
							s = 2;
						} else if (!dat->sum4) {
							s = 3;
						} else {
							s = 4;
						}
						if (dat->max) {
							s++;
						}
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							if (core_oph_type_cast
							    (dat->partials[s].content + q * dat->measure.elemsize, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type,
							     dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
					}
			}
		} else if (dat->measure.type == OPH_DOUBLE) {
			double d;
			switch (i) {
				case 0:	// mean
					for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
						d = internal_mean_d(dat->count, ((double *) (dat->partials[0].content))[q]);
						if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type, dat->result.type, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error in casting\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						q++;
					}
					break;
				case 1:	// variance
					for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
						d = internal_variance_d(dat->count, ((double *) (dat->partials[0].content))[q], ((double *) (dat->partials[1].content))[q]);
						if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type, dat->result.type, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error in casting\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						q++;
					}
					break;
				case 2:	// std dev
					for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
						d = internal_stddev_d(dat->count, ((double *) (dat->partials[0].content))[q], ((double *) (dat->partials[1].content))[q]);
						if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type, dat->result.type, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error in casting\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						q++;
					}
					break;
				case 3:	// skew
					for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
						d = internal_skew_d(dat->count, ((double *) (dat->partials[0].content))[q], ((double *) (dat->partials[1].content))[q],
								    ((double *) (dat->partials[2].content))[q]);
						if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type, dat->result.type, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error in casting\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						q++;
					}
					break;
				case 4:	// kurtosis
					for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
						d = internal_kurtosis_d(dat->count, ((double *) (dat->partials[0].content))[q], ((double *) (dat->partials[1].content))[q],
									((double *) (dat->partials[2].content))[q], ((double *) (dat->partials[3].content))[q]);
						if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type, dat->result.type, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error in casting\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						q++;
					}
					break;
				case 5:	// max
					{
						int s;
						if (!dat->sum1) {
							s = 0;
						} else if (!dat->sum2) {
							s = 1;
						} else if (!dat->sum3) {
							s = 2;
						} else if (!dat->sum4) {
							s = 3;
						} else {
							s = 4;
						}
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							if (core_oph_type_cast
							    (dat->partials[s].content + q * dat->measure.elemsize, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type,
							     dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 6:	// min
					{
						int s;
						if (!dat->sum1) {
							s = 0;
						} else if (!dat->sum2) {
							s = 1;
						} else if (!dat->sum3) {
							s = 2;
						} else if (!dat->sum4) {
							s = 3;
						} else {
							s = 4;
						}
						if (dat->max) {
							s++;
						}
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							if (core_oph_type_cast
							    (dat->partials[s].content + q * dat->measure.elemsize, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type,
							     dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
					}
			}
		} else if (dat->measure.type == OPH_SHORT) {
			switch (i) {
				case 0:	// mean
					{
						float d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_mean_s(dat->count, ((short *) (dat->partials[0].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_FLOAT, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 1:	// variance
					{
						float d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_variance_s(dat->count, ((short *) (dat->partials[0].content))[q], ((short *) (dat->partials[1].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_FLOAT, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 2:	// std dev
					{
						float d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_stddev_s(dat->count, ((short *) (dat->partials[0].content))[q], ((short *) (dat->partials[1].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_FLOAT, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 3:	// skew
					{
						float d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_skew_s(dat->count, ((short *) (dat->partials[0].content))[q], ((short *) (dat->partials[1].content))[q],
									    ((short *) (dat->partials[2].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_FLOAT, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 4:	// kurtosis
					{
						float d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_kurtosis_s(dat->count, ((short *) (dat->partials[0].content))[q], ((short *) (dat->partials[1].content))[q],
										((short *) (dat->partials[2].content))[q], ((short *) (dat->partials[3].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_FLOAT, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 5:	// max
					{
						int s;
						if (!dat->sum1) {
							s = 0;
						} else if (!dat->sum2) {
							s = 1;
						} else if (!dat->sum3) {
							s = 2;
						} else if (!dat->sum4) {
							s = 3;
						} else {
							s = 4;
						}
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							if (core_oph_type_cast
							    (dat->partials[s].content + q * dat->measure.elemsize, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type,
							     dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 6:	// min
					{
						int s;
						if (!dat->sum1) {
							s = 0;
						} else if (!dat->sum2) {
							s = 1;
						} else if (!dat->sum3) {
							s = 2;
						} else if (!dat->sum4) {
							s = 3;
						} else {
							s = 4;
						}
						if (dat->max) {
							s++;
						}
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							if (core_oph_type_cast
							    (dat->partials[s].content + q * dat->measure.elemsize, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type,
							     dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
					}
			}
		} else if (dat->measure.type == OPH_BYTE) {
			switch (i) {
				case 0:	// mean
					{
						float d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_mean_b(dat->count, ((char *) (dat->partials[0].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_FLOAT, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 1:	// variance
					{
						float d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_variance_b(dat->count, ((char *) (dat->partials[0].content))[q], ((char *) (dat->partials[1].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_FLOAT, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 2:	// std dev
					{
						float d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_stddev_b(dat->count, ((char *) (dat->partials[0].content))[q], ((char *) (dat->partials[1].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_FLOAT, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 3:	// skew
					{
						float d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_skew_b(dat->count, ((char *) (dat->partials[0].content))[q], ((char *) (dat->partials[1].content))[q],
									    ((char *) (dat->partials[2].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_FLOAT, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 4:	// kurtosis
					{
						float d;
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							d = internal_kurtosis_b(dat->count, ((char *) (dat->partials[0].content))[q], ((char *) (dat->partials[1].content))[q],
										((char *) (dat->partials[2].content))[q], ((char *) (dat->partials[3].content))[q]);
							if (core_oph_type_cast(&d, dat->result.content + (j + k) * dat->result.elemsize, OPH_FLOAT, dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 5:	// max
					{
						int s;
						if (!dat->sum1) {
							s = 0;
						} else if (!dat->sum2) {
							s = 1;
						} else if (!dat->sum3) {
							s = 2;
						} else if (!dat->sum4) {
							s = 3;
						} else {
							s = 4;
						}
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							if (core_oph_type_cast
							    (dat->partials[s].content + q * dat->measure.elemsize, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type,
							     dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
						break;
					}
				case 6:	// min
					{
						int s;
						if (!dat->sum1) {
							s = 0;
						} else if (!dat->sum2) {
							s = 1;
						} else if (!dat->sum3) {
							s = 2;
						} else if (!dat->sum4) {
							s = 3;
						} else {
							s = 4;
						}
						if (dat->max) {
							s++;
						}
						for (k = 0; k < dat->result.numelem; k += dat->mask.numelem) {
							if (core_oph_type_cast
							    (dat->partials[s].content + q * dat->measure.elemsize, dat->result.content + (j + k) * dat->result.elemsize, dat->measure.type,
							     dat->result.type, NULL)) {
								pmesg(1, __FILE__, __LINE__, "Error in casting\n");
								*length = 0;
								*is_null = 0;
								*error = 1;
								return NULL;
							}
							q++;
						}
					}
			}
		} else {
			pmesg(1, __FILE__, __LINE__, "Unsupported data type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		q = 0;
		i++;
	}

	*is_null = 0;
	*error = 0;

	*length = *(dat->result.length);
	return (result = dat->result.content);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
