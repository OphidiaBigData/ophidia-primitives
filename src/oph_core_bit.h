/*
    Ophidia Primitives
    Copyright (C) 2012-2017 CMCC Foundation

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
#ifndef __OPH_CORE_BIT__
#define __OPH_CORE_BIT__

#include "oph_core.h"

#define OPH_BITSBYTE 8

#define OPH_BIT_BLOCK_MSB 128
#define OPH_BIT_BLOCK_MAX 255

typedef enum { INVALID_BIT_OP, OPH_BIT_AND, OPH_BIT_OR, OPH_BIT_XOR, OPH_BIT_NAND, OPH_BIT_NOR, OPH_BIT_XNOR, OPH_BIT_GREATER_THAN, OPH_BIT_GREATER_OR_EQUAL_TO, OPH_BIT_LESS_THAN,
	OPH_BIT_LESS_OR_EQUAL_TO
} oph_bit_op;

#define DEFAULT_BIT_OP OPH_BIT_AND
#define OPH_BIT_VALUE_OF(a) (*(unsigned char*)(a))

#define OPH_BIT_ZERO 0
#define OPH_BIT_ONE 1
#define OPH_BIT_ZERO_S '0'
#define OPH_BIT_ONE_S '1'

typedef struct {
	char *content;
	unsigned long numelem;
} oph_bit_string;

typedef struct {
	void *measure;
	char *result;
	unsigned long length;	// size in bytes
} oph_bit_param;

typedef struct {
	oph_bit_string *measure;
	long long start;
	long long size;
	long long step;
	char *result;
	unsigned long length;	// size in bytes
} oph_bit_param_subarray;

typedef struct {
	oph_bit_string *measure;
	oph_bit_op op;
	unsigned long count;
	char *result;
	unsigned long length;	// size in bytes
} oph_bit_param_reduce;

typedef struct {
	oph_bit_string *measure;
	oph_bit_op op;
	char *result;
	unsigned long length;	// size in bytes
	char initialized;
} oph_bit_param_aggregate;

typedef struct {
	oph_bit_string *measureA;
	oph_bit_string *measureB;
	oph_bit_op op;
	char *result;
	unsigned long length;	// size in bytes
} oph_bit_param_operator;

typedef struct {
	oph_bit_string *measureA;
	oph_bit_string *measureB;
	char *result;
	unsigned long length;	// size in bytes
} oph_bit_param2;

typedef struct {
	char error;
	oph_string *measure;
	oph_bit_string *mask;
	double filling;
	char *result;
	unsigned long length;	// size in bytes
	oph_type result_type;
	size_t result_elemsize;
} oph_mask_param;

int core_oph_bit_import(oph_bit_param * param);
int core_oph_bit_export(oph_bit_param * param);
int core_oph_bit_operator(oph_bit_param_operator * param);
int core_oph_bit_not(oph_bit_param * param);
int core_oph_bit_shift(oph_bit_param * param, long long offset, unsigned char filling, char round);
int core_oph_bit_reverse(oph_bit_param * param);
int core_oph_bit_count(oph_bit_param * param, unsigned char value, long long *result);
int core_oph_bit_subarray(oph_bit_param_subarray * param);
int core_oph_bit_concat(oph_bit_param2 * param);
int core_oph_bit_find(oph_bit_param2 * param);
int core_oph_bit_reduce(oph_bit_param_reduce * param);
int core_oph_bit_aggregate(oph_bit_param_aggregate * param);
int core_oph_bit_dump(oph_bit_param * param);
int core_oph_mask(oph_mask_param * param);

// Set/Get the operator in enum oph_comp form
int core_oph_bit_set_op(oph_bit_op * op, char *oper, unsigned long *len);
oph_bit_op core_oph_bit_get_op(char *type, unsigned long *len);

// Return the occupation of a bit_measure in bytes given its size (in bits)
unsigned long core_oph_bit_length(unsigned long numelem);

// Return the number of data bit given the total size in bytes. Current implementation does not consider any padding
unsigned long core_oph_bit_numelem(unsigned long length);

// Return a bit representation of a char vector of 0 and 1
int core_oph_bit_parse(const char *string, char *bit_string, unsigned long numelem);

#endif
