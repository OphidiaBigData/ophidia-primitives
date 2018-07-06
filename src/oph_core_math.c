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

#include "oph_core_math.h"

#include <time.h>

int core_set_math_oper(oph_math_oper * operation, char *oper, unsigned long *len)
{
	*operation = core_get_math_oper(oper, len);
	if (*operation == INVALID_MATH_OPER) {
		pmesg(1, __FILE__, __LINE__, "Operation not recognized\n");
		return -1;
	}
	return 0;
}

oph_math_oper core_get_math_oper(char *oper, unsigned long *len)
{
	char oper_buff[OPER_LEN];
	if (core_strncpy(oper_buff, oper, len)) {
		pmesg(1, __FILE__, __LINE__, "Invalid operation\n");
		return INVALID_OPER;
	}
	//Case insensitive comparison
	if (!strcasecmp(oper_buff, "OPH_MATH_ABS"))
		return OPH_MATH_ABS;
	if (!strcasecmp(oper_buff, "OPH_MATH_ACOS"))
		return OPH_MATH_ACOS;
	if (!strcasecmp(oper_buff, "OPH_MATH_ASIN"))
		return OPH_MATH_ASIN;
	if (!strcasecmp(oper_buff, "OPH_MATH_ATAN"))
		return OPH_MATH_ATAN;
	if (!strcasecmp(oper_buff, "OPH_MATH_CEIL"))
		return OPH_MATH_CEIL;
	if (!strcasecmp(oper_buff, "OPH_MATH_COS"))
		return OPH_MATH_COS;
	if (!strcasecmp(oper_buff, "OPH_MATH_COT"))
		return OPH_MATH_COT;
	if (!strcasecmp(oper_buff, "OPH_MATH_DEGREES"))
		return OPH_MATH_DEGREES;
	if (!strcasecmp(oper_buff, "OPH_MATH_EXP"))
		return OPH_MATH_EXP;
	if (!strcasecmp(oper_buff, "OPH_MATH_FLOOR"))
		return OPH_MATH_FLOOR;
	if (!strcasecmp(oper_buff, "OPH_MATH_LN"))
		return OPH_MATH_LN;
	if (!strcasecmp(oper_buff, "OPH_MATH_LOG10"))
		return OPH_MATH_LOG10;
	if (!strcasecmp(oper_buff, "OPH_MATH_LOG2"))
		return OPH_MATH_LOG2;
	if (!strcasecmp(oper_buff, "OPH_MATH_RADIANS"))
		return OPH_MATH_RADIANS;
	if (!strcasecmp(oper_buff, "OPH_MATH_RAND"))
		return OPH_MATH_RAND;
	if (!strcasecmp(oper_buff, "OPH_MATH_ROUND"))
		return OPH_MATH_ROUND;
	if (!strcasecmp(oper_buff, "OPH_MATH_SIGN"))
		return OPH_MATH_SIGN;
	if (!strcasecmp(oper_buff, "OPH_MATH_SIN"))
		return OPH_MATH_SIN;
	if (!strcasecmp(oper_buff, "OPH_MATH_SQRT"))
		return OPH_MATH_SQRT;
	if (!strcasecmp(oper_buff, "OPH_MATH_TAN"))
		return OPH_MATH_TAN;

	// complex operators
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_CONJUGATE"))
		return OPH_MATH_GSL_COMPLEX_CONJUGATE;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_INVERSE"))
		return OPH_MATH_GSL_COMPLEX_INVERSE;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_NEGATIVE"))
		return OPH_MATH_GSL_COMPLEX_NEGATIVE;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_SQRT"))
		return OPH_MATH_GSL_COMPLEX_SQRT;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_EXP"))
		return OPH_MATH_GSL_COMPLEX_EXP;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_LOG"))
		return OPH_MATH_GSL_COMPLEX_LOG;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_LOG10"))
		return OPH_MATH_GSL_COMPLEX_LOG10;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_SIN"))
		return OPH_MATH_GSL_COMPLEX_SIN;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_COS"))
		return OPH_MATH_GSL_COMPLEX_COS;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_TAN"))
		return OPH_MATH_GSL_COMPLEX_TAN;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_SEC"))
		return OPH_MATH_GSL_COMPLEX_SEC;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_CSC"))
		return OPH_MATH_GSL_COMPLEX_CSC;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_COT"))
		return OPH_MATH_GSL_COMPLEX_COT;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_ASIN"))
		return OPH_MATH_GSL_COMPLEX_ASIN;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_ACOS"))
		return OPH_MATH_GSL_COMPLEX_ACOS;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_ATAN"))
		return OPH_MATH_GSL_COMPLEX_ATAN;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_ASEC"))
		return OPH_MATH_GSL_COMPLEX_ASEC;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_ACSC"))
		return OPH_MATH_GSL_COMPLEX_ACSC;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_ACOT"))
		return OPH_MATH_GSL_COMPLEX_ACOT;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_SINH"))
		return OPH_MATH_GSL_COMPLEX_SINH;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_COSH"))
		return OPH_MATH_GSL_COMPLEX_COSH;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_TANH"))
		return OPH_MATH_GSL_COMPLEX_TANH;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_SECH"))
		return OPH_MATH_GSL_COMPLEX_SECH;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_CSCH"))
		return OPH_MATH_GSL_COMPLEX_CSCH;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_COTH"))
		return OPH_MATH_GSL_COMPLEX_COTH;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_ASINH"))
		return OPH_MATH_GSL_COMPLEX_ASINH;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_ACOSH"))
		return OPH_MATH_GSL_COMPLEX_ACOSH;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_ATANH"))
		return OPH_MATH_GSL_COMPLEX_ATANH;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_ASECH"))
		return OPH_MATH_GSL_COMPLEX_ASECH;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_ACSCH"))
		return OPH_MATH_GSL_COMPLEX_ACSCH;
	if (!strcasecmp(oper_buff, "OPH_MATH_GSL_COMPLEX_ACOTH"))
		return OPH_MATH_GSL_COMPLEX_ACOTH;
	pmesg(1, __FILE__, __LINE__, "Invalid operation\n");
	return INVALID_OPER;
}


double internal_cot_d(double value)
{
	return cos(value) / sin(value);
}

double internal_degrees_d(double value)
{
	return value * 180 / M_PI;
}

double internal_log2_d(double value)
{
	return log(value) / M_LN2;
}

double internal_rand_d(double value)
{
	return ((double) rand() / RAND_MAX);
}

double internal_radians_d(double value)
{
	return value * M_PI / 180;
}

double internal_sign_d(double value)
{
	return (value >= 0 ? (value == 0 ? 0 : 1) : -1);
}

float internal_cot_f(float value)
{
	return cosf(value) / sinf(value);
}

float internal_degrees_f(float value)
{
	return value * 180 / M_PI;
}

float internal_log2_f(float value)
{
	return logf(value) / M_LN2;
}

float internal_rand_f(float value)
{
	return ((float) rand() / RAND_MAX);
}

float internal_radians_f(float value)
{
	return value * M_PI / 180;
}

float internal_sign_f(float value)
{
	return (value >= 0 ? (value == 0 ? 0 : 1) : -1);
}

int internal_cot_i(int value)
{
	return cos(value) / sin(value);
}

int internal_log2_i(int value)
{
	return log(value) / M_LN2;
}

int internal_rand_i(int value)
{
	return rand();
}

int internal_sign_i(int value)
{
	return (value >= 0 ? (value == 0 ? 0 : 1) : -1);
}

int internal_round_i(int value)
{
	return value;
}

int internal_acos_i(int value)
{
	return acos(value);
}

int internal_asin_i(int value)
{
	return asin(value);
}

int internal_atan_i(int value)
{
	return atan(value);
}

int internal_cos_i(int value)
{
	return cos(value);
}

int internal_degrees_i(int value)
{
	return value * 180 / M_PI;
}

int internal_exp_i(int value)
{
	return exp(value);
}

int internal_log_i(int value)
{
	return log(value);
}

int internal_log10_i(int value)
{
	return log10(value);
}

int internal_radians_i(int value)
{
	return value * M_PI / 180;
}

int internal_sin_i(int value)
{
	return sin(value);
}

int internal_sqrt_i(int value)
{
	return sqrt(value);
}

int internal_tan_i(int value)
{
	return tan(value);
}





short internal_cot_s(short value)
{
	return cos(value) / sin(value);
}

short internal_log2_s(short value)
{
	return log(value) / M_LN2;
}

short internal_rand_s(short value)
{
	return rand();
}

short internal_sign_s(short value)
{
	return (value >= 0 ? (value == 0 ? 0 : 1) : -1);
}

short internal_round_s(short value)
{
	return value;
}

short internal_abs_s(short value)
{
	return abs(value);
}

short internal_acos_s(short value)
{
	return acos(value);
}

short internal_asin_s(short value)
{
	return asin(value);
}

short internal_atan_s(short value)
{
	return atan(value);
}

short internal_cos_s(short value)
{
	return cos(value);
}

short internal_degrees_s(short value)
{
	return value * 180 / M_PI;
}

short internal_exp_s(short value)
{
	return exp(value);
}

short internal_log_s(short value)
{
	return log(value);
}

short internal_log10_s(short value)
{
	return log10(value);
}

short internal_radians_s(short value)
{
	return value * M_PI / 180;
}

short internal_sin_s(short value)
{
	return sin(value);
}

short internal_sqrt_s(short value)
{
	return sqrt(value);
}

short internal_tan_s(short value)
{
	return tan(value);
}



char internal_cot_b(char value)
{
	return cos(value) / sin(value);
}

char internal_log2_b(char value)
{
	return log(value) / M_LN2;
}

char internal_rand_b(char value)
{
	return rand();
}

char internal_sign_b(char value)
{
	return (value >= 0 ? (value == 0 ? 0 : 1) : -1);
}

char internal_round_b(char value)
{
	return value;
}

char internal_abs_b(char value)
{
	return abs(value);
}

char internal_acos_b(char value)
{
	return acos(value);
}

char internal_asin_b(char value)
{
	return asin(value);
}

char internal_atan_b(char value)
{
	return atan(value);
}

char internal_cos_b(char value)
{
	return cos(value);
}

char internal_degrees_b(char value)
{
	return value * 180 / M_PI;
}

char internal_exp_b(char value)
{
	return exp(value);
}

char internal_log_b(char value)
{
	return log(value);
}

char internal_log10_b(char value)
{
	return log10(value);
}

char internal_radians_b(char value)
{
	return value * M_PI / 180;
}

char internal_sin_b(char value)
{
	return sin(value);
}

char internal_sqrt_b(char value)
{
	return sqrt(value);
}

char internal_tan_b(char value)
{
	return tan(value);
}







long long internal_round_l(long long value)
{
	return value;
}

long long internal_cot_l(long long value)
{
	return cos(value) / sin(value);
}

long long internal_log2_l(long long value)
{
	return log(value) / M_LN2;
}

long long internal_rand_l(long long value)
{
	return random();
}

long long internal_sign_l(long long value)
{
	return (value >= 0 ? (value == 0 ? 0 : 1) : -1);
}

long long internal_acos_l(long long value)
{
	return acos(value);
}

long long internal_asin_l(long long value)
{
	return asin(value);
}

long long internal_atan_l(long long value)
{
	return atan(value);
}

long long internal_cos_l(long long value)
{
	return cos(value);
}

long long internal_degrees_l(long long value)
{
	return value * 180 / M_PI;
}

long long internal_exp_l(long long value)
{
	return exp(value);
}

long long internal_log_l(long long value)
{
	return log(value);
}

long long internal_log10_l(long long value)
{
	return log10(value);
}

long long internal_radians_l(long long value)
{
	return value * M_PI / 180;
}

long long internal_sin_l(long long value)
{
	return sin(value);
}

long long internal_sqrt_l(long long value)
{
	return sqrt(value);
}

long long internal_tan_l(long long value)
{
	return tan(value);
}

long long internal_abs_l(long long value)
{
	return abs(value);
}


int core_oph_math(oph_stringPtr byte_array, oph_math_oper * operation, oph_stringPtr output)
{
	char *result = output->content;
	int i;
	srand(time(NULL));
	switch (byte_array->type) {
		case OPH_DOUBLE:{
				double (*fnptr_d) (double);
				switch (*operation) {
					case OPH_MATH_ABS:
						fnptr_d = &fabs;
						break;
					case OPH_MATH_ACOS:
						fnptr_d = &acos;
						break;	//Measure has to be between -1 and 1
					case OPH_MATH_ASIN:
						fnptr_d = &asin;
						break;	//Measure has to be between -1 and 1
					case OPH_MATH_ATAN:
						fnptr_d = &atan;
						break;
					case OPH_MATH_CEIL:
						fnptr_d = &ceil;
						break;
					case OPH_MATH_COS:
						fnptr_d = &cos;
						break;
					case OPH_MATH_COT:
						fnptr_d = &internal_cot_d;
						break;
					case OPH_MATH_DEGREES:
						fnptr_d = &internal_degrees_d;
						break;
					case OPH_MATH_EXP:
						fnptr_d = &exp;
						break;	//Measure is the exponent of e
					case OPH_MATH_FLOOR:
						fnptr_d = &floor;
						break;
					case OPH_MATH_LN:
						fnptr_d = &log;
						break;
					case OPH_MATH_LOG10:
						fnptr_d = &log10;
						break;
					case OPH_MATH_LOG2:
						fnptr_d = &internal_log2_d;
						break;
					case OPH_MATH_RADIANS:
						fnptr_d = &internal_radians_d;
						break;
					case OPH_MATH_RAND:
						fnptr_d = &internal_rand_d;
						break;
					case OPH_MATH_ROUND:
						fnptr_d = &round;
						break;
					case OPH_MATH_SIGN:
						fnptr_d = &internal_sign_d;
						break;
					case OPH_MATH_SIN:
						fnptr_d = &sin;
						break;
					case OPH_MATH_SQRT:
						fnptr_d = &sqrt;
						break;
					case OPH_MATH_TAN:
						fnptr_d = &tan;
						break;
					default:
						pmesg(1, __FILE__, __LINE__, "Operation not recognized\n");
						return -1;
				}
				double d;
				for (i = 0; i < byte_array->numelem; i++) {
					d = fnptr_d(*(double *) (byte_array->content + i * byte_array->elemsize));
					if (core_oph_type_cast(&d, result + i * output->elemsize, byte_array->type, output->type, byte_array->missingvalue)) {
						pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
		case OPH_FLOAT:{
				float (*fnptr_f) (float);
				switch (*operation) {
					case OPH_MATH_ABS:
						fnptr_f = &fabsf;
						break;
					case OPH_MATH_ACOS:
						fnptr_f = &acosf;
						break;	//Measure has to be between -1 and 1
					case OPH_MATH_ASIN:
						fnptr_f = &asinf;
						break;	//Measure has to be between -1 and 1
					case OPH_MATH_ATAN:
						fnptr_f = &atanf;
						break;
					case OPH_MATH_CEIL:
						fnptr_f = &ceilf;
						break;
					case OPH_MATH_COS:
						fnptr_f = &cosf;
						break;
					case OPH_MATH_COT:
						fnptr_f = &internal_cot_f;
						break;
					case OPH_MATH_DEGREES:
						fnptr_f = &internal_degrees_f;
						break;
					case OPH_MATH_EXP:
						fnptr_f = &expf;
						break;	//Measure is the exponent (it has to be )
					case OPH_MATH_FLOOR:
						fnptr_f = &floorf;
						break;
					case OPH_MATH_LN:
						fnptr_f = &logf;
						break;
					case OPH_MATH_LOG10:
						fnptr_f = &log10f;
						break;
					case OPH_MATH_LOG2:
						fnptr_f = &internal_log2_f;
						break;
					case OPH_MATH_RADIANS:
						fnptr_f = &internal_radians_f;
						break;
					case OPH_MATH_RAND:
						fnptr_f = &internal_rand_f;
						break;
					case OPH_MATH_ROUND:
						fnptr_f = &roundf;
						break;
					case OPH_MATH_SIGN:
						fnptr_f = &internal_sign_f;
						break;
					case OPH_MATH_SIN:
						fnptr_f = &sinf;
						break;
					case OPH_MATH_SQRT:
						fnptr_f = &sqrtf;
						break;
					case OPH_MATH_TAN:
						fnptr_f = &tanf;
						break;
					default:
						pmesg(1, __FILE__, __LINE__, "Operation not recognized\n");
						return -1;
				}
				float d;
				for (i = 0; i < byte_array->numelem; i++) {
					d = fnptr_f(*(float *) (byte_array->content + i * byte_array->elemsize));
					if (core_oph_type_cast(&d, result + i * output->elemsize, byte_array->type, output->type, byte_array->missingvalue)) {
						pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
		case OPH_INT:{
				int (*fnptr_i) (int);
				switch (*operation) {
					case OPH_MATH_ABS:
						fnptr_i = &abs;
						break;
					case OPH_MATH_ACOS:
						fnptr_i = &internal_acos_i;
						break;	//Measure has to be between -1 and 1
					case OPH_MATH_ASIN:
						fnptr_i = &internal_asin_i;
						break;	//Measure has to be between -1 and 1
					case OPH_MATH_ATAN:
						fnptr_i = &internal_atan_i;
						break;
					case OPH_MATH_CEIL:
						fnptr_i = &internal_round_i;
						break;
					case OPH_MATH_COS:
						fnptr_i = &internal_cos_i;
						break;
					case OPH_MATH_COT:
						fnptr_i = &internal_cot_i;
						break;
					case OPH_MATH_DEGREES:
						fnptr_i = &internal_degrees_i;
						break;
					case OPH_MATH_EXP:
						fnptr_i = &internal_exp_i;
						break;	//Measure is the exponent (it has to be )
					case OPH_MATH_FLOOR:
						fnptr_i = &internal_round_i;
						break;
					case OPH_MATH_LN:
						fnptr_i = &internal_log_i;
						break;
					case OPH_MATH_LOG10:
						fnptr_i = &internal_log10_i;
						break;
					case OPH_MATH_LOG2:
						fnptr_i = &internal_log2_i;
						break;
					case OPH_MATH_RADIANS:
						fnptr_i = &internal_radians_i;
						break;
					case OPH_MATH_RAND:
						fnptr_i = &internal_rand_i;
						break;
					case OPH_MATH_ROUND:
						fnptr_i = &internal_round_i;
						break;
					case OPH_MATH_SIGN:
						fnptr_i = &internal_sign_i;
						break;
					case OPH_MATH_SIN:
						fnptr_i = &internal_sin_i;
						break;
					case OPH_MATH_SQRT:
						fnptr_i = &internal_sqrt_i;
						break;
					case OPH_MATH_TAN:
						fnptr_i = &internal_tan_i;
						break;
					default:
						pmesg(1, __FILE__, __LINE__, "Operation not recognized\n");
						return -1;
				}
				int d;
				for (i = 0; i < byte_array->numelem; i++) {
					d = fnptr_i(*(int *) (byte_array->content + i * byte_array->elemsize));
					if (core_oph_type_cast(&d, result + i * output->elemsize, byte_array->type, output->type, byte_array->missingvalue)) {
						pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
		case OPH_SHORT:{
				short (*fnptr_s) (short);
				switch (*operation) {
					case OPH_MATH_ABS:
						fnptr_s = &internal_abs_s;
						break;
					case OPH_MATH_ACOS:
						fnptr_s = &internal_acos_s;
						break;	//Measure has to be between -1 and 1
					case OPH_MATH_ASIN:
						fnptr_s = &internal_asin_s;
						break;	//Measure has to be between -1 and 1
					case OPH_MATH_ATAN:
						fnptr_s = &internal_atan_s;
						break;
					case OPH_MATH_CEIL:
						fnptr_s = &internal_round_s;
						break;
					case OPH_MATH_COS:
						fnptr_s = &internal_cos_s;
						break;
					case OPH_MATH_COT:
						fnptr_s = &internal_cot_s;
						break;
					case OPH_MATH_DEGREES:
						fnptr_s = &internal_degrees_s;
						break;
					case OPH_MATH_EXP:
						fnptr_s = &internal_exp_s;
						break;	//Measure is the exponent (it has to be )
					case OPH_MATH_FLOOR:
						fnptr_s = &internal_round_s;
						break;
					case OPH_MATH_LN:
						fnptr_s = &internal_log_s;
						break;
					case OPH_MATH_LOG10:
						fnptr_s = &internal_log10_s;
						break;
					case OPH_MATH_LOG2:
						fnptr_s = &internal_log2_s;
						break;
					case OPH_MATH_RADIANS:
						fnptr_s = &internal_radians_s;
						break;
					case OPH_MATH_RAND:
						fnptr_s = &internal_rand_s;
						break;
					case OPH_MATH_ROUND:
						fnptr_s = &internal_round_s;
						break;
					case OPH_MATH_SIGN:
						fnptr_s = &internal_sign_s;
						break;
					case OPH_MATH_SIN:
						fnptr_s = &internal_sin_s;
						break;
					case OPH_MATH_SQRT:
						fnptr_s = &internal_sqrt_s;
						break;
					case OPH_MATH_TAN:
						fnptr_s = &internal_tan_s;
						break;
					default:
						pmesg(1, __FILE__, __LINE__, "Operation not recognized\n");
						return -1;
				}
				short d;
				for (i = 0; i < byte_array->numelem; i++) {
					d = fnptr_s(*(short *) (byte_array->content + i * byte_array->elemsize));
					if (core_oph_type_cast(&d, result + i * output->elemsize, byte_array->type, output->type, byte_array->missingvalue)) {
						pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
		case OPH_BYTE:{
				char (*fnptr_b) (char);
				switch (*operation) {
					case OPH_MATH_ABS:
						fnptr_b = &internal_abs_b;
						break;
					case OPH_MATH_ACOS:
						fnptr_b = &internal_acos_b;
						break;	//Measure has to be between -1 and 1
					case OPH_MATH_ASIN:
						fnptr_b = &internal_asin_b;
						break;	//Measure has to be between -1 and 1
					case OPH_MATH_ATAN:
						fnptr_b = &internal_atan_b;
						break;
					case OPH_MATH_CEIL:
						fnptr_b = &internal_round_b;
						break;
					case OPH_MATH_COS:
						fnptr_b = &internal_cos_b;
						break;
					case OPH_MATH_COT:
						fnptr_b = &internal_cot_b;
						break;
					case OPH_MATH_DEGREES:
						fnptr_b = &internal_degrees_b;
						break;
					case OPH_MATH_EXP:
						fnptr_b = &internal_exp_b;
						break;	//Measure is the exponent (it has to be )
					case OPH_MATH_FLOOR:
						fnptr_b = &internal_round_b;
						break;
					case OPH_MATH_LN:
						fnptr_b = &internal_log_b;
						break;
					case OPH_MATH_LOG10:
						fnptr_b = &internal_log10_b;
						break;
					case OPH_MATH_LOG2:
						fnptr_b = &internal_log2_b;
						break;
					case OPH_MATH_RADIANS:
						fnptr_b = &internal_radians_b;
						break;
					case OPH_MATH_RAND:
						fnptr_b = &internal_rand_b;
						break;
					case OPH_MATH_ROUND:
						fnptr_b = &internal_round_b;
						break;
					case OPH_MATH_SIGN:
						fnptr_b = &internal_sign_b;
						break;
					case OPH_MATH_SIN:
						fnptr_b = &internal_sin_b;
						break;
					case OPH_MATH_SQRT:
						fnptr_b = &internal_sqrt_b;
						break;
					case OPH_MATH_TAN:
						fnptr_b = &internal_tan_b;
						break;
					default:
						pmesg(1, __FILE__, __LINE__, "Operation not recognized\n");
						return -1;
				}
				char d;
				for (i = 0; i < byte_array->numelem; i++) {
					d = fnptr_b(*(char *) (byte_array->content + i * byte_array->elemsize));
					if (core_oph_type_cast(&d, result + i * output->elemsize, byte_array->type, output->type, byte_array->missingvalue)) {
						pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
		case OPH_LONG:{
				long long (*fnptr_l) (long long);
				switch (*operation) {
					case OPH_MATH_ABS:
						fnptr_l = &internal_abs_l;
						break;
					case OPH_MATH_ACOS:
						fnptr_l = &internal_acos_l;
						break;	//Measure has to be between -1 and 1
					case OPH_MATH_ASIN:
						fnptr_l = &internal_asin_l;
						break;	//Measure has to be between -1 and 1
					case OPH_MATH_ATAN:
						fnptr_l = &internal_atan_l;
						break;
					case OPH_MATH_CEIL:
						fnptr_l = &internal_round_l;
						break;
					case OPH_MATH_COS:
						fnptr_l = &internal_cos_l;
						break;
					case OPH_MATH_COT:
						fnptr_l = &internal_cot_l;
						break;
					case OPH_MATH_DEGREES:
						fnptr_l = &internal_degrees_l;
						break;
					case OPH_MATH_EXP:
						fnptr_l = &internal_exp_l;
						break;	//Measure is the exponent (it has to be )
					case OPH_MATH_FLOOR:
						fnptr_l = &internal_round_l;
						break;
					case OPH_MATH_LN:
						fnptr_l = &internal_log_l;
						break;
					case OPH_MATH_LOG10:
						fnptr_l = &internal_log10_l;
						break;
					case OPH_MATH_LOG2:
						fnptr_l = &internal_log2_l;
						break;
					case OPH_MATH_RADIANS:
						fnptr_l = &internal_radians_l;
						break;
					case OPH_MATH_RAND:
						fnptr_l = &internal_rand_l;
						break;
					case OPH_MATH_ROUND:
						fnptr_l = &internal_round_l;
						break;
					case OPH_MATH_SIGN:
						fnptr_l = &internal_sign_l;
						break;
					case OPH_MATH_SIN:
						fnptr_l = &internal_sin_l;
						break;
					case OPH_MATH_SQRT:
						fnptr_l = &internal_sqrt_l;
						break;
					case OPH_MATH_TAN:
						fnptr_l = &internal_tan_l;
						break;
					default:
						pmesg(1, __FILE__, __LINE__, "Operation not recognized\n");
						return -1;
				}
				long long d;
				for (i = 0; i < byte_array->numelem; i++) {
					d = fnptr_l(*(long long *) (byte_array->content + i * byte_array->elemsize));
					if (core_oph_type_cast(&d, result + i * output->elemsize, byte_array->type, output->type, byte_array->missingvalue)) {
						pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
		case OPH_COMPLEX_DOUBLE:{
#ifdef GSL_SUPPORTED
				gsl_complex(*fnptr_cd) (gsl_complex);
				switch (*operation) {
					case OPH_MATH_GSL_COMPLEX_CONJUGATE:
						fnptr_cd = &gsl_complex_conjugate;
						break;
					case OPH_MATH_GSL_COMPLEX_INVERSE:
						fnptr_cd = &gsl_complex_inverse;
						break;
					case OPH_MATH_GSL_COMPLEX_NEGATIVE:
						fnptr_cd = &gsl_complex_negative;
						break;
					case OPH_MATH_GSL_COMPLEX_SQRT:
						fnptr_cd = &gsl_complex_sqrt;
						break;
					case OPH_MATH_GSL_COMPLEX_EXP:
						fnptr_cd = &gsl_complex_exp;
						break;
					case OPH_MATH_GSL_COMPLEX_LOG:
						fnptr_cd = &gsl_complex_log;
						break;
					case OPH_MATH_GSL_COMPLEX_LOG10:
						fnptr_cd = &gsl_complex_log10;
						break;
					case OPH_MATH_GSL_COMPLEX_SIN:
						fnptr_cd = &gsl_complex_sin;
						break;
					case OPH_MATH_GSL_COMPLEX_COS:
						fnptr_cd = &gsl_complex_cos;
						break;
					case OPH_MATH_GSL_COMPLEX_TAN:
						fnptr_cd = &gsl_complex_tan;
						break;
					case OPH_MATH_GSL_COMPLEX_SEC:
						fnptr_cd = &gsl_complex_sec;
						break;
					case OPH_MATH_GSL_COMPLEX_CSC:
						fnptr_cd = &gsl_complex_csc;
						break;
					case OPH_MATH_GSL_COMPLEX_COT:
						fnptr_cd = &gsl_complex_cot;
						break;
					case OPH_MATH_GSL_COMPLEX_ASIN:
						fnptr_cd = &gsl_complex_arcsin;
						break;
					case OPH_MATH_GSL_COMPLEX_ACOS:
						fnptr_cd = &gsl_complex_arccos;
						break;
					case OPH_MATH_GSL_COMPLEX_ATAN:
						fnptr_cd = &gsl_complex_arctan;
						break;
					case OPH_MATH_GSL_COMPLEX_ASEC:
						fnptr_cd = &gsl_complex_arcsec;
						break;
					case OPH_MATH_GSL_COMPLEX_ACSC:
						fnptr_cd = &gsl_complex_arccsc;
						break;
					case OPH_MATH_GSL_COMPLEX_ACOT:
						fnptr_cd = &gsl_complex_arccot;
						break;
					case OPH_MATH_GSL_COMPLEX_SINH:
						fnptr_cd = &gsl_complex_sinh;
						break;
					case OPH_MATH_GSL_COMPLEX_COSH:
						fnptr_cd = &gsl_complex_cosh;
						break;
					case OPH_MATH_GSL_COMPLEX_TANH:
						fnptr_cd = &gsl_complex_tanh;
						break;
					case OPH_MATH_GSL_COMPLEX_SECH:
						fnptr_cd = &gsl_complex_sech;
						break;
					case OPH_MATH_GSL_COMPLEX_CSCH:
						fnptr_cd = &gsl_complex_csch;
						break;
					case OPH_MATH_GSL_COMPLEX_COTH:
						fnptr_cd = &gsl_complex_coth;
						break;
					case OPH_MATH_GSL_COMPLEX_ASINH:
						fnptr_cd = &gsl_complex_arcsinh;
						break;
					case OPH_MATH_GSL_COMPLEX_ACOSH:
						fnptr_cd = &gsl_complex_arccosh;
						break;
					case OPH_MATH_GSL_COMPLEX_ATANH:
						fnptr_cd = &gsl_complex_arctanh;
						break;
					case OPH_MATH_GSL_COMPLEX_ASECH:
						fnptr_cd = &gsl_complex_arcsech;
						break;
					case OPH_MATH_GSL_COMPLEX_ACSCH:
						fnptr_cd = &gsl_complex_arccsch;
						break;
					case OPH_MATH_GSL_COMPLEX_ACOTH:
						fnptr_cd = &gsl_complex_arccoth;
						break;
					default:
						pmesg(1, __FILE__, __LINE__, "Operation not recognized\n");
						return -1;
				}
				gsl_complex c, d;
				for (i = 0; i < (byte_array->numelem) * 2; i += 2) {
					c.dat[0] = ((double *) (byte_array->content))[i];	//real part
					c.dat[1] = ((double *) (byte_array->content))[i + 1];	//imag part
					d = fnptr_cd(c);
					if (core_oph_type_cast(&(d.dat[0]), result + i * output->elemsize, byte_array->type, output->type, byte_array->missingvalue)) {
						pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
					if (core_oph_type_cast(&(d.dat[1]), result + (i + 1) * output->elemsize, byte_array->type, output->type, byte_array->missingvalue)) {
						pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
#else
				pmesg(1, __FILE__, __LINE__, "Operation not allowed\n");
#endif
				break;
			}
		case OPH_COMPLEX_FLOAT:{
#ifdef GSL_SUPPORTED
				gsl_complex(*fnptr_cf) (gsl_complex);
				switch (*operation) {
					case OPH_MATH_GSL_COMPLEX_CONJUGATE:
						fnptr_cf = &gsl_complex_conjugate;
						break;
					case OPH_MATH_GSL_COMPLEX_INVERSE:
						fnptr_cf = &gsl_complex_inverse;
						break;
					case OPH_MATH_GSL_COMPLEX_NEGATIVE:
						fnptr_cf = &gsl_complex_negative;
						break;
					case OPH_MATH_GSL_COMPLEX_SQRT:
						fnptr_cf = &gsl_complex_sqrt;
						break;
					case OPH_MATH_GSL_COMPLEX_EXP:
						fnptr_cf = &gsl_complex_exp;
						break;
					case OPH_MATH_GSL_COMPLEX_LOG:
						fnptr_cf = &gsl_complex_log;
						break;
					case OPH_MATH_GSL_COMPLEX_LOG10:
						fnptr_cf = &gsl_complex_log10;
						break;
					case OPH_MATH_GSL_COMPLEX_SIN:
						fnptr_cf = &gsl_complex_sin;
						break;
					case OPH_MATH_GSL_COMPLEX_COS:
						fnptr_cf = &gsl_complex_cos;
						break;
					case OPH_MATH_GSL_COMPLEX_TAN:
						fnptr_cf = &gsl_complex_tan;
						break;
					case OPH_MATH_GSL_COMPLEX_SEC:
						fnptr_cf = &gsl_complex_sec;
						break;
					case OPH_MATH_GSL_COMPLEX_CSC:
						fnptr_cf = &gsl_complex_csc;
						break;
					case OPH_MATH_GSL_COMPLEX_COT:
						fnptr_cf = &gsl_complex_cot;
						break;
					case OPH_MATH_GSL_COMPLEX_ASIN:
						fnptr_cf = &gsl_complex_arcsin;
						break;
					case OPH_MATH_GSL_COMPLEX_ACOS:
						fnptr_cf = &gsl_complex_arccos;
						break;
					case OPH_MATH_GSL_COMPLEX_ATAN:
						fnptr_cf = &gsl_complex_arctan;
						break;
					case OPH_MATH_GSL_COMPLEX_ASEC:
						fnptr_cf = &gsl_complex_arcsec;
						break;
					case OPH_MATH_GSL_COMPLEX_ACSC:
						fnptr_cf = &gsl_complex_arccsc;
						break;
					case OPH_MATH_GSL_COMPLEX_ACOT:
						fnptr_cf = &gsl_complex_arccot;
						break;
					case OPH_MATH_GSL_COMPLEX_SINH:
						fnptr_cf = &gsl_complex_sinh;
						break;
					case OPH_MATH_GSL_COMPLEX_COSH:
						fnptr_cf = &gsl_complex_cosh;
						break;
					case OPH_MATH_GSL_COMPLEX_TANH:
						fnptr_cf = &gsl_complex_tanh;
						break;
					case OPH_MATH_GSL_COMPLEX_SECH:
						fnptr_cf = &gsl_complex_sech;
						break;
					case OPH_MATH_GSL_COMPLEX_CSCH:
						fnptr_cf = &gsl_complex_csch;
						break;
					case OPH_MATH_GSL_COMPLEX_COTH:
						fnptr_cf = &gsl_complex_coth;
						break;
					case OPH_MATH_GSL_COMPLEX_ASINH:
						fnptr_cf = &gsl_complex_arcsinh;
						break;
					case OPH_MATH_GSL_COMPLEX_ACOSH:
						fnptr_cf = &gsl_complex_arccosh;
						break;
					case OPH_MATH_GSL_COMPLEX_ATANH:
						fnptr_cf = &gsl_complex_arctanh;
						break;
					case OPH_MATH_GSL_COMPLEX_ASECH:
						fnptr_cf = &gsl_complex_arcsech;
						break;
					case OPH_MATH_GSL_COMPLEX_ACSCH:
						fnptr_cf = &gsl_complex_arccsch;
						break;
					case OPH_MATH_GSL_COMPLEX_ACOTH:
						fnptr_cf = &gsl_complex_arccoth;
						break;
					default:
						pmesg(1, __FILE__, __LINE__, "Operation not recognized\n");
						return -1;
				}
				gsl_complex c, d;
				for (i = 0; i < (byte_array->numelem) * 2; i += 2) {
					c.dat[0] = (double) ((float *) (byte_array->content))[i];	//real part
					c.dat[1] = (double) ((float *) (byte_array->content))[i + 1];	//imag part
					d = fnptr_cf(c);
					if (core_oph_type_cast(&(d.dat[0]), result + i * output->elemsize, byte_array->type, output->type, byte_array->missingvalue)) {
						pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
					if (core_oph_type_cast(&(d.dat[1]), result + (i + 1) * output->elemsize, byte_array->type, output->type, byte_array->missingvalue)) {
						pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
#else
				pmesg(1, __FILE__, __LINE__, "Operation not allowed\n");
#endif
				break;
			}
		case OPH_COMPLEX_LONG:{
#ifdef GSL_SUPPORTED
				gsl_complex(*fnptr_cl) (gsl_complex);
				switch (*operation) {
					case OPH_MATH_GSL_COMPLEX_CONJUGATE:
						fnptr_cl = &gsl_complex_conjugate;
						break;
					case OPH_MATH_GSL_COMPLEX_INVERSE:
						fnptr_cl = &gsl_complex_inverse;
						break;
					case OPH_MATH_GSL_COMPLEX_NEGATIVE:
						fnptr_cl = &gsl_complex_negative;
						break;
					case OPH_MATH_GSL_COMPLEX_SQRT:
						fnptr_cl = &gsl_complex_sqrt;
						break;
					case OPH_MATH_GSL_COMPLEX_EXP:
						fnptr_cl = &gsl_complex_exp;
						break;
					case OPH_MATH_GSL_COMPLEX_LOG:
						fnptr_cl = &gsl_complex_log;
						break;
					case OPH_MATH_GSL_COMPLEX_LOG10:
						fnptr_cl = &gsl_complex_log10;
						break;
					case OPH_MATH_GSL_COMPLEX_SIN:
						fnptr_cl = &gsl_complex_sin;
						break;
					case OPH_MATH_GSL_COMPLEX_COS:
						fnptr_cl = &gsl_complex_cos;
						break;
					case OPH_MATH_GSL_COMPLEX_TAN:
						fnptr_cl = &gsl_complex_tan;
						break;
					case OPH_MATH_GSL_COMPLEX_SEC:
						fnptr_cl = &gsl_complex_sec;
						break;
					case OPH_MATH_GSL_COMPLEX_CSC:
						fnptr_cl = &gsl_complex_csc;
						break;
					case OPH_MATH_GSL_COMPLEX_COT:
						fnptr_cl = &gsl_complex_cot;
						break;
					case OPH_MATH_GSL_COMPLEX_ASIN:
						fnptr_cl = &gsl_complex_arcsin;
						break;
					case OPH_MATH_GSL_COMPLEX_ACOS:
						fnptr_cl = &gsl_complex_arccos;
						break;
					case OPH_MATH_GSL_COMPLEX_ATAN:
						fnptr_cl = &gsl_complex_arctan;
						break;
					case OPH_MATH_GSL_COMPLEX_ASEC:
						fnptr_cl = &gsl_complex_arcsec;
						break;
					case OPH_MATH_GSL_COMPLEX_ACSC:
						fnptr_cl = &gsl_complex_arccsc;
						break;
					case OPH_MATH_GSL_COMPLEX_ACOT:
						fnptr_cl = &gsl_complex_arccot;
						break;
					case OPH_MATH_GSL_COMPLEX_SINH:
						fnptr_cl = &gsl_complex_sinh;
						break;
					case OPH_MATH_GSL_COMPLEX_COSH:
						fnptr_cl = &gsl_complex_cosh;
						break;
					case OPH_MATH_GSL_COMPLEX_TANH:
						fnptr_cl = &gsl_complex_tanh;
						break;
					case OPH_MATH_GSL_COMPLEX_SECH:
						fnptr_cl = &gsl_complex_sech;
						break;
					case OPH_MATH_GSL_COMPLEX_CSCH:
						fnptr_cl = &gsl_complex_csch;
						break;
					case OPH_MATH_GSL_COMPLEX_COTH:
						fnptr_cl = &gsl_complex_coth;
						break;
					case OPH_MATH_GSL_COMPLEX_ASINH:
						fnptr_cl = &gsl_complex_arcsinh;
						break;
					case OPH_MATH_GSL_COMPLEX_ACOSH:
						fnptr_cl = &gsl_complex_arccosh;
						break;
					case OPH_MATH_GSL_COMPLEX_ATANH:
						fnptr_cl = &gsl_complex_arctanh;
						break;
					case OPH_MATH_GSL_COMPLEX_ASECH:
						fnptr_cl = &gsl_complex_arcsech;
						break;
					case OPH_MATH_GSL_COMPLEX_ACSCH:
						fnptr_cl = &gsl_complex_arccsch;
						break;
					case OPH_MATH_GSL_COMPLEX_ACOTH:
						fnptr_cl = &gsl_complex_arccoth;
						break;
					default:
						pmesg(1, __FILE__, __LINE__, "Operation not recognized\n");
						return -1;
				}
				gsl_complex c, d;
				for (i = 0; i < (byte_array->numelem) * 2; i += 2) {
					c.dat[0] = (double) ((long long *) (byte_array->content))[i];	//real part
					c.dat[1] = (double) ((long long *) (byte_array->content))[i + 1];	//imag part
					d = fnptr_cl(c);
					if (core_oph_type_cast(&(d.dat[0]), result + i * output->elemsize, byte_array->type, output->type, byte_array->missingvalue)) {
						pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
					if (core_oph_type_cast(&(d.dat[1]), result + (i + 1) * output->elemsize, byte_array->type, output->type, byte_array->missingvalue)) {
						pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
#else
				pmesg(1, __FILE__, __LINE__, "Operation not allowed\n");
#endif
				break;
			}
		case OPH_COMPLEX_INT:{
#ifdef GSL_SUPPORTED
				gsl_complex(*fnptr_ci) (gsl_complex);
				switch (*operation) {
					case OPH_MATH_GSL_COMPLEX_CONJUGATE:
						fnptr_ci = &gsl_complex_conjugate;
						break;
					case OPH_MATH_GSL_COMPLEX_INVERSE:
						fnptr_ci = &gsl_complex_inverse;
						break;
					case OPH_MATH_GSL_COMPLEX_NEGATIVE:
						fnptr_ci = &gsl_complex_negative;
						break;
					case OPH_MATH_GSL_COMPLEX_SQRT:
						fnptr_ci = &gsl_complex_sqrt;
						break;
					case OPH_MATH_GSL_COMPLEX_EXP:
						fnptr_ci = &gsl_complex_exp;
						break;
					case OPH_MATH_GSL_COMPLEX_LOG:
						fnptr_ci = &gsl_complex_log;
						break;
					case OPH_MATH_GSL_COMPLEX_LOG10:
						fnptr_ci = &gsl_complex_log10;
						break;
					case OPH_MATH_GSL_COMPLEX_SIN:
						fnptr_ci = &gsl_complex_sin;
						break;
					case OPH_MATH_GSL_COMPLEX_COS:
						fnptr_ci = &gsl_complex_cos;
						break;
					case OPH_MATH_GSL_COMPLEX_TAN:
						fnptr_ci = &gsl_complex_tan;
						break;
					case OPH_MATH_GSL_COMPLEX_SEC:
						fnptr_ci = &gsl_complex_sec;
						break;
					case OPH_MATH_GSL_COMPLEX_CSC:
						fnptr_ci = &gsl_complex_csc;
						break;
					case OPH_MATH_GSL_COMPLEX_COT:
						fnptr_ci = &gsl_complex_cot;
						break;
					case OPH_MATH_GSL_COMPLEX_ASIN:
						fnptr_ci = &gsl_complex_arcsin;
						break;
					case OPH_MATH_GSL_COMPLEX_ACOS:
						fnptr_ci = &gsl_complex_arccos;
						break;
					case OPH_MATH_GSL_COMPLEX_ATAN:
						fnptr_ci = &gsl_complex_arctan;
						break;
					case OPH_MATH_GSL_COMPLEX_ASEC:
						fnptr_ci = &gsl_complex_arcsec;
						break;
					case OPH_MATH_GSL_COMPLEX_ACSC:
						fnptr_ci = &gsl_complex_arccsc;
						break;
					case OPH_MATH_GSL_COMPLEX_ACOT:
						fnptr_ci = &gsl_complex_arccot;
						break;
					case OPH_MATH_GSL_COMPLEX_SINH:
						fnptr_ci = &gsl_complex_sinh;
						break;
					case OPH_MATH_GSL_COMPLEX_COSH:
						fnptr_ci = &gsl_complex_cosh;
						break;
					case OPH_MATH_GSL_COMPLEX_TANH:
						fnptr_ci = &gsl_complex_tanh;
						break;
					case OPH_MATH_GSL_COMPLEX_SECH:
						fnptr_ci = &gsl_complex_sech;
						break;
					case OPH_MATH_GSL_COMPLEX_CSCH:
						fnptr_ci = &gsl_complex_csch;
						break;
					case OPH_MATH_GSL_COMPLEX_COTH:
						fnptr_ci = &gsl_complex_coth;
						break;
					case OPH_MATH_GSL_COMPLEX_ASINH:
						fnptr_ci = &gsl_complex_arcsinh;
						break;
					case OPH_MATH_GSL_COMPLEX_ACOSH:
						fnptr_ci = &gsl_complex_arccosh;
						break;
					case OPH_MATH_GSL_COMPLEX_ATANH:
						fnptr_ci = &gsl_complex_arctanh;
						break;
					case OPH_MATH_GSL_COMPLEX_ASECH:
						fnptr_ci = &gsl_complex_arcsech;
						break;
					case OPH_MATH_GSL_COMPLEX_ACSCH:
						fnptr_ci = &gsl_complex_arccsch;
						break;
					case OPH_MATH_GSL_COMPLEX_ACOTH:
						fnptr_ci = &gsl_complex_arccoth;
						break;
					default:
						pmesg(1, __FILE__, __LINE__, "Operation not recognized\n");
						return -1;
				}
				gsl_complex c, d;
				for (i = 0; i < (byte_array->numelem) * 2; i += 2) {
					c.dat[0] = (double) ((int *) (byte_array->content))[i];	//real part
					c.dat[1] = (double) ((int *) (byte_array->content))[i + 1];	//imag part
					d = fnptr_ci(c);
					if (core_oph_type_cast(&(d.dat[0]), result + i * output->elemsize, byte_array->type, output->type, byte_array->missingvalue)) {
						pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
					if (core_oph_type_cast(&(d.dat[1]), result + (i + 1) * output->elemsize, byte_array->type, output->type, byte_array->missingvalue)) {
						pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
#else
				pmesg(1, __FILE__, __LINE__, "Operation not allowed\n");
#endif
				break;
			}
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			return -1;
	}
	return 0;
}
