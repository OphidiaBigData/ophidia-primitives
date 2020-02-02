/*
    Ophidia Primitives
    Copyright (C) 2012-2020 CMCC Foundation

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

#include "oph_zonal_stats.h"

int msglevel = 1;

void zs_oper_double(oph_oper operator, double *tmp, double target)
{

	switch (operator) {

		case OPH_MAX:
			if (*tmp < target)
				*tmp = target;
			break;

		case OPH_MIN:
			if (*tmp > target)
				*tmp = target;
			break;

		case OPH_SUM:
		case OPH_AVG:
			*tmp += target;
			break;

		default:;
	}
}

void zs_oper_float(oph_oper operator, float *tmp, float target)
{

	switch (operator) {

		case OPH_MAX:
			if (*tmp < target)
				*tmp = target;
			break;

		case OPH_MIN:
			if (*tmp > target)
				*tmp = target;
			break;

		case OPH_SUM:
		case OPH_AVG:
			*tmp += target;
			break;

		default:;
	}
}

void zs_oper_int(oph_oper operator, int *tmp, int target)
{

	switch (operator) {

		case OPH_MAX:
			if (*tmp < target)
				*tmp = target;
			break;

		case OPH_MIN:
			if (*tmp > target)
				*tmp = target;
			break;

		case OPH_SUM:
		case OPH_AVG:
			*tmp += target;
			break;

		default:;
	}
}

void zs_oper_long(oph_oper operator, long long *tmp, long long target)
{

	switch (operator) {

		case OPH_MAX:
			if (*tmp < target)
				*tmp = target;
			break;

		case OPH_MIN:
			if (*tmp > target)
				*tmp = target;
			break;

		case OPH_SUM:
		case OPH_AVG:
			*tmp += target;
			break;

		default:;
	}
}

void zs_oper_short(oph_oper operator, short *tmp, short target)
{

	switch (operator) {

		case OPH_MAX:
			if (*tmp < target)
				*tmp = target;
			break;

		case OPH_MIN:
			if (*tmp > target)
				*tmp = target;
			break;

		case OPH_SUM:
		case OPH_AVG:
			*tmp += target;
			break;

		default:;
	}
}

void zs_oper_byte(oph_oper operator, char *tmp, char target)
{

	switch (operator) {

		case OPH_MAX:
			if (*tmp < target)
				*tmp = target;
			break;

		case OPH_MIN:
			if (*tmp > target)
				*tmp = target;
			break;

		case OPH_SUM:
		case OPH_AVG:
			*tmp += target;
			break;

		default:;
	}
}

int core_oph_zonal_stats_multi(oph_multistring * byte_array, oph_multistring * result, int id, int buffer, oph_oper operator, int blocksize)
{
	int i, j, js, je, cc, x, y, xi, yi, mx, my, mmx, mmy = byte_array->numelem / blocksize;	// Skip the third dimension
	char *in_string = byte_array->content, *out_string = result->content;

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

	for (j = js; j < je; ++j) {
		x = y = 0;
		switch (byte_array->type[j]) {
			case OPH_DOUBLE:{
					double mv = byte_array->missingvalue ? *byte_array->missingvalue : NAN, tmp, target;
					for (i = 0; i < byte_array->numelem; i++) {
						tmp = mv;
						cc = 0;

						mmx = x - buffer;
						if (mmx < 0)
							mmx = 0;
						mx = x + buffer;
						if (mx >= blocksize)
							mx = blocksize - 1;

						yi = y - buffer;
						if (yi < 0)
							yi = 0;
						my = y + buffer;
						if (my >= mmy)
							my = mmy - 1;

						for (; yi <= my; yi++) {
							for (xi = mmx; xi <= mx; xi++) {
								target = *(double *) (in_string + (xi + yi * blocksize) * byte_array->blocksize);
								if (!isnan(target) && (target != mv)) {
									if (!isnan(tmp) && (tmp != mv))
										zs_oper_double(operator, &tmp, target);
									else
										tmp = target;
									cc++;
								}
							}
						}
						if (operator == OPH_AVG)
							tmp /= cc;
						if (core_oph_type_cast((void *) (&tmp), out_string + (i * result->blocksize), byte_array->type[j], result->type[j], byte_array->missingvalue))
							return -1;
						x++;
						if (x >= blocksize) {
							x = 0;
							y++;	// Skip the third dimension
						}
					}
					break;
				}
			case OPH_FLOAT:{
					float mv = byte_array->missingvalue ? *byte_array->missingvalue : NAN, tmp, target;
					for (i = 0; i < byte_array->numelem; i++) {
						tmp = mv;
						cc = 0;

						mmx = x - buffer;
						if (mmx < 0)
							mmx = 0;
						mx = x + buffer;
						if (mx >= blocksize)
							mx = blocksize - 1;

						yi = y - buffer;
						if (yi < 0)
							yi = 0;
						my = y + buffer;
						if (my >= mmy)
							my = mmy - 1;

						for (; yi <= my; yi++) {
							for (xi = mmx; xi <= mx; xi++) {
								target = *(float *) (in_string + (xi + yi * blocksize) * byte_array->blocksize);
								if (!isnan(target) && (target != mv)) {
									if (!isnan(tmp) && (tmp != mv))
										zs_oper_float(operator, &tmp, target);
									else
										tmp = target;
									cc++;
								}
							}
						}
						if (operator == OPH_AVG)
							tmp /= cc;
						if (core_oph_type_cast((void *) (&tmp), out_string + (i * result->blocksize), byte_array->type[j], result->type[j], byte_array->missingvalue))
							return -1;
						x++;
						if (x >= blocksize) {
							x = 0;
							y++;	// Skip the third dimension
						}
					}
					break;
				}
			case OPH_INT:{
					char valid;
					int mv = byte_array->missingvalue ? *byte_array->missingvalue : 0, tmp, target;
					for (i = 0; i < byte_array->numelem; i++) {
						tmp = mv;
						valid = cc = 0;

						mmx = x - buffer;
						if (mmx < 0)
							mmx = 0;
						mx = x + buffer;
						if (mx >= blocksize)
							mx = blocksize - 1;

						yi = y - buffer;
						if (yi < 0)
							yi = 0;
						my = y + buffer;
						if (my >= mmy)
							my = mmy - 1;

						for (; yi <= my; yi++) {
							for (xi = mmx; xi <= mx; xi++) {
								target = *(int *) (in_string + (xi + yi * blocksize) * byte_array->blocksize);
								if (!byte_array->missingvalue || (target != mv)) {
									if (valid && (!byte_array->missingvalue || (tmp != mv)))
										zs_oper_int(operator, &tmp, target);
									else {
										tmp = target;
										valid = 1;
									}
									cc++;
								}
							}
						}
						if (operator == OPH_AVG) {
							float tmp2 = (float) tmp / cc;
							if (core_oph_type_cast((void *) (&tmp2), out_string + (i * result->blocksize), OPH_FLOAT, result->type[j], byte_array->missingvalue))
								return -1;
						} else if (core_oph_type_cast((void *) (&tmp), out_string + (i * result->blocksize), byte_array->type[j], result->type[j], byte_array->missingvalue))
							return -1;
						x++;
						if (x >= blocksize) {
							x = 0;
							y++;	// Skip the third dimension
						}
					}
					break;
				}
			case OPH_LONG:{
					char valid;
					long long mv = byte_array->missingvalue ? *byte_array->missingvalue : 0, tmp, target;
					for (i = 0; i < byte_array->numelem; i++) {
						tmp = mv;
						valid = cc = 0;

						mmx = x - buffer;
						if (mmx < 0)
							mmx = 0;
						mx = x + buffer;
						if (mx >= blocksize)
							mx = blocksize - 1;

						yi = y - buffer;
						if (yi < 0)
							yi = 0;
						my = y + buffer;
						if (my >= mmy)
							my = mmy - 1;

						for (; yi <= my; yi++) {
							for (xi = mmx; xi <= mx; xi++) {
								target = *(long long *) (in_string + (xi + yi * blocksize) * byte_array->blocksize);
								if (!byte_array->missingvalue || (target != mv)) {
									if (valid && (!byte_array->missingvalue || (tmp != mv)))
										zs_oper_long(operator, &tmp, target);
									else {
										tmp = target;
										valid = 1;
									}
									cc++;
								}
							}
						}
						if (operator == OPH_AVG) {
							double tmp2 = (double) tmp / cc;
							if (core_oph_type_cast((void *) (&tmp2), out_string + (i * result->blocksize), OPH_DOUBLE, result->type[j], byte_array->missingvalue))
								return -1;
						} else if (core_oph_type_cast((void *) (&tmp), out_string + (i * result->blocksize), byte_array->type[j], result->type[j], byte_array->missingvalue))
							return -1;
						x++;
						if (x >= blocksize) {
							x = 0;
							y++;	// Skip the third dimension
						}
					}
					break;
				}
			case OPH_SHORT:{
					char valid;
					short mv = byte_array->missingvalue ? *byte_array->missingvalue : 0, tmp, target;
					for (i = 0; i < byte_array->numelem; i++) {
						tmp = mv;
						tmp = mv;
						valid = cc = 0;

						mmx = x - buffer;
						if (mmx < 0)
							mmx = 0;
						mx = x + buffer;
						if (mx >= blocksize)
							mx = blocksize - 1;

						yi = y - buffer;
						if (yi < 0)
							yi = 0;
						my = y + buffer;
						if (my >= mmy)
							my = mmy - 1;

						for (; yi <= my; yi++) {
							for (xi = mmx; xi <= mx; xi++) {
								target = *(short *) (in_string + (xi + yi * blocksize) * byte_array->blocksize);
								if (!byte_array->missingvalue || (target != mv)) {
									if (valid && (!byte_array->missingvalue || (tmp != mv)))
										zs_oper_short(operator, &tmp, target);
									else {
										tmp = target;
										valid = 1;
									}
									cc++;
								}
							}
						}
						if (operator == OPH_AVG) {
							float tmp2 = (float) tmp / cc;
							if (core_oph_type_cast((void *) (&tmp2), out_string + (i * result->blocksize), OPH_FLOAT, result->type[j], byte_array->missingvalue))
								return -1;
						} else if (core_oph_type_cast((void *) (&tmp), out_string + (i * result->blocksize), byte_array->type[j], result->type[j], byte_array->missingvalue))
							return -1;
						x++;
						if (x >= blocksize) {
							x = 0;
							y++;	// Skip the third dimension
						}
					}
					break;
				}
			case OPH_BYTE:{
					char valid;
					char mv = byte_array->missingvalue ? *byte_array->missingvalue : 0, tmp, target;
					for (i = 0; i < byte_array->numelem; i++) {
						tmp = mv;
						tmp = mv;
						valid = cc = 0;

						mmx = x - buffer;
						if (mmx < 0)
							mmx = 0;
						mx = x + buffer;
						if (mx >= blocksize)
							mx = blocksize - 1;

						yi = y - buffer;
						if (yi < 0)
							yi = 0;
						my = y + buffer;
						if (my >= mmy)
							my = mmy - 1;

						for (; yi <= my; yi++) {
							for (xi = mmx; xi <= mx; xi++) {
								target = *(char *) (in_string + (xi + yi * blocksize) * byte_array->blocksize);
								if (!byte_array->missingvalue || (target != mv)) {
									if (valid && (!byte_array->missingvalue || (tmp != mv)))
										zs_oper_byte(operator, &tmp, target);
									else {
										tmp = target;
										valid = 1;
									}
									cc++;
								}
							}
						}
						if (operator == OPH_AVG) {
							float tmp2 = (float) tmp / cc;
							if (core_oph_type_cast((void *) (&tmp2), out_string + (i * result->blocksize), OPH_FLOAT, result->type[j], byte_array->missingvalue))
								return -1;
						} else if (core_oph_type_cast((void *) (&tmp), out_string + (i * result->blocksize), byte_array->type[j], result->type[j], byte_array->missingvalue))
							return -1;
						x++;
						if (x >= blocksize) {
							x = 0;
							y++;	// Skip the third dimension
						}
					}
					break;
				}
			default:
				pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
				return -1;
		}
		in_string += byte_array->elemsize[j];
		out_string += result->elemsize[j];
	}

	return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_zonal_stats_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	if (args->arg_count < 3 || args->arg_count > 8) {
		strcpy(message, "ERROR: Wrong arguments! oph_zonal_stats(input_OPH_TYPE, output_OPH_TYPE, measure, [buffer], [operator], [missing value], [blocksize], [id_measure])");
		return 1;
	}

	for (i = 0; i < 3; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_zonal_stats function");
			return 1;
		}
	}

	if (args->arg_count > 3) {
		if (args->arg_type[3] == STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_zonal_stats function");
			return 1;
		}
		args->arg_type[3] = INT_RESULT;
		if (args->arg_count > 4) {
			if (args->arg_type[4] != STRING_RESULT) {
				strcpy(message, "ERROR: Wrong arguments to oph_zonal_stats function");
				return 1;
			}
			if (args->arg_count > 5) {
				args->arg_type[5] = REAL_RESULT;
				if (args->arg_count > 6) {
					if (args->arg_type[6] == STRING_RESULT) {
						strcpy(message, "ERROR: Wrong arguments to oph_zonal_stats function");
						return 1;
					}
					args->arg_type[6] = INT_RESULT;
					if (args->arg_count > 7) {
						if (args->arg_type[7] == STRING_RESULT) {
							strcpy(message, "ERROR: Wrong arguments to oph_zonal_stats function");
							return 1;
						}
						args->arg_type[7] = INT_RESULT;
					}
				}
			}
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;
	return 0;
}

void oph_zonal_stats_deinit(UDF_INIT * initid)
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

char *oph_zonal_stats(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	oph_multistring *multim;
	oph_multistring *output;

	int id = 0;

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

	int buffer = 0, blocksize = 1;
	double missingvalue = NAN;
	oph_oper operator = DEFAULT_OPER;
	if ((args->arg_count > 3) && args->args[3]) {
		buffer = *((long long *) args->args[3]);
		if (buffer < 0) {
			pmesg(1, __FILE__, __LINE__, "Wrong buffer size.\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}
	if ((args->arg_count > 4) && args->args[4]) {
		operator = core_get_oper(args->args[4], &(args->lengths[4]));
		if ((operator != OPH_MAX) && (operator != OPH_MIN) && (operator != OPH_AVG) && (operator != OPH_SUM)) {
			pmesg(1, __FILE__, __LINE__, "Wrong operator.\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}
	if ((args->arg_count > 5) && (args->args[5])) {
		missingvalue = *((double *) args->args[5]);
		multim->missingvalue = &missingvalue;
	} else
		multim->missingvalue = NULL;
	if ((args->arg_count > 6) && args->args[6]) {
		blocksize = *((long long *) args->args[6]);
		if (blocksize < 1) {
			pmesg(1, __FILE__, __LINE__, "Wrong block size.\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}
	if ((args->arg_count > 7) && args->args[7]) {
		id = *((long long *) args->args[7]);
		if ((id < 0) || (id > ((oph_multistring *) (initid->ptr))->num_measure)) {
			pmesg(1, __FILE__, __LINE__, "Wrong measure identifier. Correct values are integers in [1, %d].\n", ((oph_multistring *) (initid->ptr))->num_measure);
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	output = (oph_multistring *) (initid->ptr);
	output->numelem = multim->numelem;

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

	if (core_oph_zonal_stats_multi(multim, output, id, buffer, operator, blocksize)) {
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
