/*
    Ophidia Primitives
    Copyright (C) 2012-2016 CMCC Foundation

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

/*/ Standard C headers */
#ifndef __OPH_CORE__
#define __OPH_CORE__

#include <stdlib.h>
#include <stdio.h>
#include <zlib.h>
#include <math.h>
#include <string.h>
#include "oph_debug.h"

typedef enum oph_hier {INVALID_HIER, OPH_ALL, OPH_WEEK, OPH_MONTH, OPH_YEAR} oph_hier;
typedef enum oph_type {INVALID_TYPE, OPH_INT, OPH_LONG, OPH_SHORT, OPH_BYTE, OPH_FLOAT, OPH_DOUBLE, OPH_COMPLEX_INT, OPH_COMPLEX_LONG, OPH_COMPLEX_FLOAT, OPH_COMPLEX_DOUBLE} oph_type;
typedef enum oph_oper {INVALID_OPER, OPH_COUNT, OPH_MAX, OPH_MIN, OPH_SUM, OPH_AVG, OPH_STD, OPH_VAR, OPH_CMOMENT, OPH_ACMOMENT, OPH_RMOMENT, OPH_ARMOMENT, OPH_QUANTILE } oph_oper;
typedef enum oph_ma_oper {INVALID_MA_OPER, OPH_SMA, OPH_EWMA} oph_ma_oper;

#define UNUSED(x) {(void)(x);}

#define BUFF_LEN 65536

#define DEFAULT_HIER OPH_ALL
#define DEFAULT_HIER_LEN 10

#define DEFAULT_TYPE OPH_DOUBLE
#define DEFAULT_TYPE_LEN 10

#define DEFAULT_OPER OPH_MAX
#define DEFAULT_OPER_LEN 15

#define DEFAULT_COMPLEX_TYPE "OPH_COMPLEX_DOUBLE"
#define DEFAULT_COMPLEX_TYPE_LEN 18

/* define number of integer digits (max value) and number of decimal digits (exact value) for string representation */
#define INT_DIGIT 8 //TODO: set it to 64 and, then, show empty chars
#define PRECISION 10

#define COMPRESS_OFFSET 4

extern int msglevel;

/* Struct used to contain string parameter provided in oph_operator statement; I need it because if args->args[i] is 
   an argument of type STRING_RESULT handbook says: "Do not assume that the string is null-terminated". */

typedef struct oph_string{
        char *content;
        unsigned long *length;
        oph_type type;
        size_t elemsize;
        unsigned long numelem;
	double *missingvalue;
	double param;
	void* extend;
} oph_string;
typedef struct oph_string *oph_stringPtr;

/* New struct used to store informations about multimeasure arrays */

typedef struct oph_multistring{
        char* content;		//Contains the entire binary array
        unsigned long length;	//Length (in byte) of the entire binary array
        oph_type *type;		//Array containing the single basic type for each measure
	size_t num_measure;	//Number of elements of the array type (or number of different 'basic data types')
	size_t *elemsize;	//Array containing the size of each type
	unsigned long numelem;	//Number of blocks (multimeasure block) in the array
	size_t blocksize;	//Number of byte between two subsequent measure value in the multimeasure array (=SUM(elemsize[i]))
	unsigned int islast;	//Flag used to check if it is the last element in case of array of multistring
	double *missingvalue;	//Pointer to the value to be considered as missing value; no value or NAN in case it is NULL
	double param;		//Free param
	void* extend;		//Free space
} oph_multistring;
typedef struct oph_multistring *oph_multistringPtr;

/* Struct used to contain all parameters provided from query string */

typedef struct oph_request{
        oph_string measure;
        oph_hier hier;
        oph_oper oper;
} oph_request;
typedef struct oph_request *oph_requestPtr;

/* New Struct used to contain all parameters provided from query string using oph_multistring */

typedef struct oph_request_multi{
        oph_multistring* measure;	//Array of oph_string2 structures (in case of more than one measure passed as input)
        int (*core_oph_oper) (oph_multistring* byte_array, oph_multistring* res);
} oph_request_multi;
typedef struct oph_request_multi *oph_request_multiPtr;

typedef struct {
	char error;
	oph_string *measure;
	char *result;
	unsigned long length;	// size in bytes
	oph_type result_type;
	size_t result_elemsize;
} oph_generic_param;

typedef struct {
	char error;
	oph_multistring *measure;
	oph_multistring *result;
	void* extend;
	int (*core_oph_oper) (char* valueA, char* valueB, char* result, oph_type type);
	int (*core_oph_oper_multi) (oph_multistring* byte_array, oph_multistring* res);
	int (*core_oph_oper_multi_ext) (oph_multistring* byte_array, oph_multistring* res, void* extend);
} oph_generic_param_multi;

/*------------------------------------------------------------------|
 |               Functions declarations (BEGIN)                     |
 |------------------------------------------------------------------*/

/*------------------------------------------------------------------|
 |                  Internal functions (BEGIN)                      |
 |------------------------------------------------------------------*/


// Fill buffer with string contained in oph_string struct; buffer will be null terminated
int core_strncpy (char* buff, const char *str, unsigned long *str_len);

// Return the sizeof type defined in the oph_string struct
size_t core_sizeof (oph_type type);

// Set values for hierarchy, measure type and operator
int core_set_hier(oph_requestPtr req, char* hier, unsigned long *len);
int core_set_type(oph_stringPtr str, char* type, unsigned long *len);
int core_set_oph_multistring(oph_multistring **str, char* type, unsigned long *len);
void free_oph_multistring(oph_multistring* str);
int core_set_oper(oph_requestPtr req, char* oper, unsigned long *len);
int core_set_oper_multi(oph_request_multi* req, char* oper, unsigned long *len);
int core_oph_type_cast (void *input, char *output, oph_type in_type, oph_type out_type, double *missingvalue);
int core_oph_multistring_cast (oph_multistring* input, oph_multistring* output, double *missingvalue);
// Set value of elemsize as the sizeof elements in byte string - Must be called after core_set_type function
int core_set_elemsize(oph_stringPtr str);

// Set value of numelem as the number of elements in byte string - Must be called at the end of the oph_string initialization, after all other core_set_* routines
int core_set_numelem(oph_stringPtr str);

// Get the hierarchy in enum oph_hier form
oph_hier core_get_hier (char* hier, unsigned long *len);

// Get the type in enum oph_type form
oph_type core_get_type (char* type, unsigned long *len);
oph_type core_get_type2 (char* type);

// Get the operator in enum oph_oper form
oph_oper core_get_oper (char* oper, unsigned long *len);

// Deprecated - use str->numelem
//Return the number of elements of an array
long core_count_array(oph_stringPtr str);

int core_oph_sum2_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result);
int core_oph_sum2_array_multi(char* valueA, char* valueB, char* result, oph_type type);

int core_oph_sum3_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result);
int core_oph_sum3_array_multi(char* valueA, char* valueB, char* result, oph_type type);

int core_oph_sum4_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result);
int core_oph_sum4_array_multi(char* valueA, char* valueB, char* result, oph_type type);

void free_oph_generic_param_multi(oph_generic_param_multi* param);

/*------------------------------------------------------------------|
 |                  Internal functions (END)                        |
 |------------------------------------------------------------------*/

/*------------------------------------------------------------------|
 |               Single element functions (BEGIN)                   |
 |------------------------------------------------------------------*/

// Return the values stored in byte_array converted in string format; numbers are separeted by ", " chars
int core_oph_dump (oph_stringPtr byte_array, char *result, int encoding);

// Return the 'single value' stored in byte_array converted in numerical format (int, long, float, double)
int core_oph_convert (oph_stringPtr byte_array, void *result);

// Return (in byte array form) the sum of the scalar number with every element of byte_array. Cast scalar to byte_array elements type
int core_oph_sum_scalar(oph_stringPtr byte_array, double scalar, char* result);

// Return (in byte array form) the multiplication of the scalar number with every element of byte_array. Cast scalar to byte_array elements type
int core_oph_mul_scalar(oph_stringPtr byte_array, double scalar, char* result);

/*------------------------------------------------------------------|
 |               Single element functions (END)                     |
 |------------------------------------------------------------------*/


/*------------------------------------------------------------------|
 |               Array functions (BEGIN)                            |
 |------------------------------------------------------------------*/

// Return the number of non-null values in an array (as result string)
int core_oph_count (oph_stringPtr byte_array, char *result);
int core_oph_count_multi (oph_multistring* byte_array, oph_multistring *result);

// Return the maximum value in an array (as result string)
int core_oph_max (oph_stringPtr byte_array, char *result);
int core_oph_max_multi (oph_multistring* byte_array, oph_multistring *result);

// Return the minimum value in an array (as result string)
int core_oph_min (oph_stringPtr byte_array, char *result);
int core_oph_min_multi (oph_multistring* byte_array, oph_multistring *result);

// Return the sum of the array values (as result string)
int core_oph_sum (oph_stringPtr byte_array, char *result);
int core_oph_sum_multi (oph_multistring* byte_array, oph_multistring *result);

// Return the average value in an array (as result string)
int core_oph_avg (oph_stringPtr byte_array, char *result);
int core_oph_avg_multi (oph_multistring* byte_array, oph_multistring *result);

// Return the standard deviation value in an array (as result string)
int core_oph_std (oph_stringPtr byte_array, char *result);
int core_oph_std_multi (oph_multistring* byte_array, oph_multistring *result);

// Return the standard deviation value in an array (as result string)
int core_oph_var (oph_stringPtr byte_array, char *result);
int core_oph_var_multi (oph_multistring* byte_array, oph_multistring *result);

// Return the central moment value in an array (as result string)
int core_oph_cmoment (oph_stringPtr byte_array, char *result);
int core_oph_cmoment_multi (oph_multistring* byte_array, oph_multistring *result);

// Return the absolute central moment value in an array (as result string)
int core_oph_acmoment (oph_stringPtr byte_array, char *result);
int core_oph_acmoment_multi (oph_multistring* byte_array, oph_multistring *result);

// Return the raw moment value in an array (as result string)
int core_oph_rmoment (oph_stringPtr byte_array, char *result);
int core_oph_rmoment_multi (oph_multistring* byte_array, oph_multistring *result);

// Return the absolute raw moment value in an array (as result string)
int core_oph_armoment (oph_stringPtr byte_array, char *result);
int core_oph_armoment_multi (oph_multistring* byte_array, oph_multistring *result);

// Return the byte sub-array of byte_array from start position to size+start position (as result string) (start is 0 based)
int core_oph_get_subarray(oph_stringPtr byte_array, char* result, long long start, long long size);

// Return a byte array resulting from adding byte_arraya with byte_arrayb: result[i] = byte_arraya[i] + byte_arrayb[i]
int core_oph_sum_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result);
int core_oph_sum_array2(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result, int* count);

// Return a byte array resulting from increasing byte_arraya by 1 if byte_arrayb is a number: result[i] = byte_arraya[i] + (isnan(byte_arrayb[i]) ? 0 : 1)
int core_oph_count_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result);

// Return a byte array resulting from multiplying byte_arraya with byte_arrayb: result[i] = byte_arraya[i] * byte_arrayb[i]
int core_oph_mul_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result);

// Return a byte array resulting from masking byte_arraya with byte_arrayb: result[i] = (byte_arrayb[i]==1)?byte_arraya[i]:undef
int core_oph_mask_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result);

// Return a byte array resulting from subtracting byte_arrayb from byte_arraya: result[i] = byte_arraya[i] - byte_arrayb[i]
int core_oph_sub_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result);

// Return a byte array resulting from dividing byte_arraya for byte_arrayb: result[i] = byte_arraya[i] / byte_arrayb[i]
int core_oph_div_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result);

// Return a byte array resulting from computing the modulus of byte_arraya and byte_arrayb: result[i] = sqrt((byte_arraya[i])^2 + (byte_arrayb[i])^2)
int core_oph_abs_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result);

// Return a byte array resulting from computing the phase of byte_arraya and byte_arrayb: result[i] = atan2(byte_arrayb[i],byte_arraya[i])
int core_oph_arg_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result);

// Evaluates the maximum values comparing each elements of two arrays. Return an array containing the maximum values
int core_oph_max_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result);

// Evaluates the minimum values comparing each elements of two arrays. Return an array containing the minimum values
int core_oph_min_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result);

// Return the number of elements in byte_array which have distance from value <= distance
int core_oph_find(oph_stringPtr byte_array, double value, double distance, long *count);

// Return the array string in a compressed format
int core_oph_compress(const unsigned char *src, const unsigned long src_length, unsigned char **dest, unsigned long *dest_length, int compression_level);

// Return the array string uncompressing the source
int core_oph_uncompress(const unsigned char *source, unsigned long source_length, unsigned long uncompressed_length, unsigned char **dest);

unsigned long core_compressed_length (const unsigned long src_length);

// Return an array (as result string) shifted of a quantity '+offset' towards right (or '-offset' towards left)
int core_oph_shift(oph_generic_param* param, long long offset, double filling, int round);

//Compute the simple moving average indicator on the array
int core_oph_simple_moving_avg (oph_generic_param *param, long long k);
int core_oph_simple_moving_avg_multi (oph_multistring* byte_array, oph_multistring* res, void* k);

//Compute the exponential weighted moving average indicator on the array
int core_oph_exponential_moving_avg (oph_generic_param *param, double a);
int core_oph_exponential_moving_avg_multi (oph_multistring* byte_array, oph_multistring* res, void* a);

// Compute a binary operation on the array
int core_oph_oper_array_multi(oph_generic_param_multi* param);
int core_oph_sum_array_multi(char* valueA, char* valueB, char* valueO, oph_type type);
int core_oph_mul_array_multi(char* valueA, char* valueB, char* valueO, oph_type type);
int core_oph_mask_array_multi(char* valueA, char* valueB, char* valueO, oph_type type);
int core_oph_sub_array_multi(char* valueA, char* valueB, char* valueO, oph_type type);
int core_oph_div_array_multi(char* valueA, char* valueB, char* valueO, oph_type type);
int core_oph_abs_array_multi(char* valueA, char* valueB, char* valueO, oph_type type);
int core_oph_arg_array_multi(char* valueA, char* valueB, char* valueO, oph_type type);
int core_oph_max_array_multi(char* valueA, char* valueB, char* valueO, oph_type type);
int core_oph_min_array_multi(char* valueA, char* valueB, char* valueO, oph_type type);

/*------------------------------------------------------------------|
 |               Array functions (END)                              |
 |------------------------------------------------------------------*/

/*------------------------------------------------------------------|
 |               Functions declarations (END)                       |
 |------------------------------------------------------------------*/

#endif
