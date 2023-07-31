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

#include "oph_predicate.h"

#include <pthread.h>

int msglevel = 1;

pthread_rwlock_t lock;

int core_oph_predicate(oph_stringPtr byte_array, char *result)
{
	unsigned long i;
	unsigned short r;
	double res, temporary;
	oph_predicate_param *_result = (oph_predicate_param *) result;
	switch (byte_array->type) {
		case OPH_DOUBLE:
			switch (_result->op) {
				case OPH_GREATER_THAN_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((double *) (byte_array->content + i * byte_array->elemsize)));
						r = res > 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((double *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_LESS_THAN_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((double *) (byte_array->content + i * byte_array->elemsize)));
						r = res < 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((double *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((double *) (byte_array->content + i * byte_array->elemsize)));
						r = res == 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((double *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_GREATER_OR_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((double *) (byte_array->content + i * byte_array->elemsize)));
						r = res >= 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((double *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_LESS_OR_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((double *) (byte_array->content + i * byte_array->elemsize)));
						r = res <= 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((double *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_NOT_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((double *) (byte_array->content + i * byte_array->elemsize)));
						r = res != 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((double *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_NULL:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((double *) (byte_array->content + i * byte_array->elemsize)));
						r = isnan(res);
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((double *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				default:
					pmesg(1, __FILE__, __LINE__, "Comparison value non recognized\n");
					return -1;
			}
			break;
		case OPH_FLOAT:
			switch (_result->op) {
				case OPH_GREATER_THAN_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((float *) (byte_array->content + i * byte_array->elemsize)));
						r = res > 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((float *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_LESS_THAN_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((float *) (byte_array->content + i * byte_array->elemsize)));
						r = res < 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((float *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((float *) (byte_array->content + i * byte_array->elemsize)));
						r = res == 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((float *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_GREATER_OR_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((float *) (byte_array->content + i * byte_array->elemsize)));
						r = res >= 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((float *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_LESS_OR_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((float *) (byte_array->content + i * byte_array->elemsize)));
						r = res <= 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((float *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_NOT_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((float *) (byte_array->content + i * byte_array->elemsize)));
						r = res != 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((float *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_NULL:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((float *) (byte_array->content + i * byte_array->elemsize)));
						r = isnan(res);
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((float *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				default:
					pmesg(1, __FILE__, __LINE__, "Comparison value non recognized\n");
					return -1;
			}
			break;
		case OPH_INT:
			switch (_result->op) {
				case OPH_GREATER_THAN_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((int *) (byte_array->content + i * byte_array->elemsize)));
						r = res > 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((int *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_LESS_THAN_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((int *) (byte_array->content + i * byte_array->elemsize)));
						r = res < 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((int *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((int *) (byte_array->content + i * byte_array->elemsize)));
						r = res == 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((int *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_GREATER_OR_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((int *) (byte_array->content + i * byte_array->elemsize)));
						r = res >= 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((int *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_LESS_OR_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((int *) (byte_array->content + i * byte_array->elemsize)));
						r = res <= 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((int *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_NOT_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((int *) (byte_array->content + i * byte_array->elemsize)));
						r = res != 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((int *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_NULL:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((int *) (byte_array->content + i * byte_array->elemsize)));
						r = isnan(res);
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((int *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				default:
					pmesg(1, __FILE__, __LINE__, "Comparison value non recognized\n");
					return -1;
			}
			break;
		case OPH_SHORT:
			switch (_result->op) {
				case OPH_GREATER_THAN_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((short *) (byte_array->content + i * byte_array->elemsize)));
						r = res > 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((short *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_LESS_THAN_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((short *) (byte_array->content + i * byte_array->elemsize)));
						r = res < 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((short *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((short *) (byte_array->content + i * byte_array->elemsize)));
						r = res == 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((short *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_GREATER_OR_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((short *) (byte_array->content + i * byte_array->elemsize)));
						r = res >= 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((short *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_LESS_OR_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((short *) (byte_array->content + i * byte_array->elemsize)));
						r = res <= 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((short *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_NOT_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((short *) (byte_array->content + i * byte_array->elemsize)));
						r = res != 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((short *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_NULL:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((short *) (byte_array->content + i * byte_array->elemsize)));
						r = isnan(res);
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((short *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				default:
					pmesg(1, __FILE__, __LINE__, "Comparison value non recognized\n");
					return -1;
			}
			break;
		case OPH_BYTE:
			switch (_result->op) {
				case OPH_GREATER_THAN_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((char *) (byte_array->content + i * byte_array->elemsize)));
						r = res > 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((char *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_LESS_THAN_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((char *) (byte_array->content + i * byte_array->elemsize)));
						r = res < 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((char *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((char *) (byte_array->content + i * byte_array->elemsize)));
						r = res == 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((char *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_GREATER_OR_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((char *) (byte_array->content + i * byte_array->elemsize)));
						r = res >= 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((char *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_LESS_OR_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((char *) (byte_array->content + i * byte_array->elemsize)));
						r = res <= 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((char *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_NOT_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((char *) (byte_array->content + i * byte_array->elemsize)));
						r = res != 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((char *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_NULL:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((char *) (byte_array->content + i * byte_array->elemsize)));
						r = isnan(res);
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3], *((char *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				default:
					pmesg(1, __FILE__, __LINE__, "Comparison value non recognized\n");
					return -1;

			}
			break;
		case OPH_LONG:
			switch (_result->op) {
				case OPH_GREATER_THAN_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((long long *) (byte_array->content + i * byte_array->elemsize)));
						r = res > 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3],
														*((long long *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_LESS_THAN_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((long long *) (byte_array->content + i * byte_array->elemsize)));
						r = res < 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3],
														*((long long *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((long long *) (byte_array->content + i * byte_array->elemsize)));
						r = res == 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3],
														*((long long *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_GREATER_OR_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((long long *) (byte_array->content + i * byte_array->elemsize)));
						r = res >= 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3],
														*((long long *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_LESS_OR_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((long long *) (byte_array->content + i * byte_array->elemsize)));
						r = res <= 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3],
														*((long long *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_NOT_EQUAL_TO_ZERO:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((long long *) (byte_array->content + i * byte_array->elemsize)));
						r = res != 0.0;
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3],
														*((long long *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				case OPH_NULL:
					for (i = 0; i < byte_array->numelem; ++i) {
						res = _result->is_index[0] ? i + 1 : evaluator_evaluate_x(_result->f[1], *((long long *) (byte_array->content + i * byte_array->elemsize)));
						r = isnan(res);
						temporary =
						    _result->is_index[r ? 1 : 2] ? i + 1 : evaluator_evaluate_x(_result->f[r ? 2 : 3],
														*((long long *) (byte_array->content + i * byte_array->elemsize)));
						if (core_oph_type_cast
						    (&temporary, (((char *) _result->f[0]) + i * _result->result_elemsize), OPH_DOUBLE, _result->result_type, byte_array->missingvalue)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}
					}
					break;
				default:
					pmesg(1, __FILE__, __LINE__, "Comparison value non recognized\n");
					return -1;
			}
			break;
		default:
			pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
			return -1;
	}
	return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_predicate_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	if (args->arg_count != 7) {
		strcpy(message, "ERROR: Wrong arguments! oph_predicate(input_OPH_TYPE, output_OPH_TYPE, measure, checking_expression, comparison_operator, output_expression1, output_expression2)");
		return 1;
	}

	int i;
	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_predicate function");
			return 1;
		}
	}

	initid->ptr = NULL;
	return 0;
}

void oph_predicate_deinit(UDF_INIT * initid)
{
	int i;
	//Free allocated space
	if (initid->ptr) {
		if (((oph_predicate_param *) initid->ptr)->f[0]) {
			free(((oph_predicate_param *) initid->ptr)->f[0]);
			((oph_predicate_param *) initid->ptr)->f[0] = NULL;	// binary_array free
		}
		for (i = 1; i < 4; ++i)
			if (((oph_predicate_param *) initid->ptr)->f[i]) {
				evaluator_destroy(((oph_predicate_param *) initid->ptr)->f[i]);
				((oph_predicate_param *) initid->ptr)->f[i] = NULL;	// expressions free
			}
		free(initid->ptr);
		initid->ptr = NULL;
	}
}

char *oph_predicate(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	oph_string measure;
	char *buffer;
	char **names;
	int count;
	oph_predicate_param *param;

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
	measure.missingvalue = NULL;

	core_set_elemsize(&(measure));

	if (core_set_numelem(&(measure))) {
		pmesg(1, __FILE__, __LINE__, "Error on counting elements\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	if (!initid->ptr) {
		initid->ptr = (char *) malloc(sizeof(oph_predicate_param));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		param = (oph_predicate_param *) initid->ptr;

		for (i = 0; i < 4; ++i)
			param->f[i] = NULL;
		for (i = 0; i < 3; ++i)
			param->is_index[i] = 0;

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
			param->is_index[0] = 1;
			free(buffer);
		}

		// Comparison operator
		if (core_set_comp(&param->op, args->args[4], &(args->lengths[4]))) {
			pmesg(1, __FILE__, __LINE__, "Comparison operator not recognized\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		if (args->args[5] && args->lengths[5]) {
			buffer = (char *) malloc(1 + args->lengths[5]);
			core_strncpy(buffer, args->args[5], &(args->lengths[5]));
			if (!strcasecmp(buffer, "nan"))
				sprintf(buffer, "0/0");
		} else
			buffer = strdup("0/0");

		if (strcasecmp(buffer, "INDEX")) {
			pthread_rwlock_wrlock(&lock);
			param->f[2] = evaluator_create(buffer);
			pthread_rwlock_unlock(&lock);
			free(buffer);
			if (!param->f[2]) {
				pmesg(1, __FILE__, __LINE__, "Error allocating evaluator\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			evaluator_get_variables(param->f[2], &names, &count);
			if (count > 1) {
				pmesg(1, __FILE__, __LINE__, "Too variables in expression\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
		} else {
			param->is_index[1] = 1;
			free(buffer);
		}

		if (args->args[6] && args->lengths[6]) {
			buffer = (char *) malloc(1 + args->lengths[6]);
			core_strncpy(buffer, args->args[6], &(args->lengths[6]));
			if (!strcasecmp(buffer, "nan"))
				sprintf(buffer, "0/0");
		} else
			buffer = strdup("0/0");

		if (strcasecmp(buffer, "INDEX")) {
			pthread_rwlock_wrlock(&lock);
			param->f[3] = evaluator_create(buffer);
			pthread_rwlock_unlock(&lock);
			free(buffer);
			if (!param->f[3]) {
				pmesg(1, __FILE__, __LINE__, "Error allocating evaluator\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			evaluator_get_variables(param->f[3], &names, &count);
			if (count > 1) {
				pmesg(1, __FILE__, __LINE__, "Too variables in expression\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
		} else {
			param->is_index[0] = 1;
			free(buffer);
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
		param = (oph_predicate_param *) initid->ptr;

	i = core_oph_predicate(&measure, initid->ptr);
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
