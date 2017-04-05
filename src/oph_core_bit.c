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

#include "oph_core_bit.h"

int core_oph_bit_parse(const char *string, char *bit_string, unsigned long numelem)
{
	if (!numelem)
		return 1;
	unsigned long i;
	unsigned char *O = (unsigned char *) bit_string, tmp = OPH_BIT_BLOCK_MSB;
	for (i = 0; i < numelem; ++i)	// loop on bits
	{
		switch (string[i]) {
			case OPH_BIT_ZERO_S:
				*O &= ~tmp;
				break;
			case OPH_BIT_ONE_S:
				*O |= tmp;
				break;
			default:
				pmesg(1, __FILE__, __LINE__, "Invalid symbol '%c'\n", string[i]);
				return 1;
		}
		if (tmp == 1) {
			tmp = OPH_BIT_BLOCK_MSB;
			O++;	// next byte
		} else
			tmp >>= 1;
	}
	return 0;
}

unsigned long core_oph_bit_length(unsigned long numelem)
{
	unsigned long result = numelem / OPH_BITSBYTE;
	if (numelem % OPH_BITSBYTE)
		result++;
	return result;
}

// Current implementation does not consider any padding
unsigned long core_oph_bit_numelem(unsigned long length)
{
	return length * OPH_BITSBYTE;
}

int core_oph_bit_set_op(oph_bit_op * op, char *type, unsigned long *len)
{
	if (!type) {
		*op = DEFAULT_BIT_OP;
		return 0;
	}
	*op = core_oph_bit_get_op(type, len);
	if (*op == INVALID_BIT_OP) {
		pmesg(1, __FILE__, __LINE__, "Operator not recognized\n");
		return -1;
	}
	return 0;
}

oph_bit_op core_oph_bit_get_op(char *type, unsigned long *len)
{
	char type_buff[BUFF_LEN];
	if (core_strncpy(type_buff, type, len)) {
		pmesg(1, __FILE__, __LINE__, "Invalid operator\n");
		return INVALID_BIT_OP;
	}
	//Case insensitive comparison
	if (!strcasecmp(type_buff, "OPH_AND") || !strcasecmp(type_buff, "OPH_CARRY"))
		return OPH_BIT_AND;
	if (!strcasecmp(type_buff, "OPH_OR"))
		return OPH_BIT_OR;
	if (!strcasecmp(type_buff, "OPH_XOR") || !strcasecmp(type_buff, "OPH_SUM") || !strcasecmp(type_buff, "OPH_NOT_EQUAL_TO"))
		return OPH_BIT_XOR;
	if (!strcasecmp(type_buff, "OPH_NAND"))
		return OPH_BIT_NAND;
	if (!strcasecmp(type_buff, "OPH_NOR"))
		return OPH_BIT_NOR;
	if (!strcasecmp(type_buff, "OPH_XNOR") || !strcasecmp(type_buff, "OPH_EQUAL_TO"))
		return OPH_BIT_XNOR;
	if (!strcasecmp(type_buff, "OPH_GREATER_THAN"))
		return OPH_BIT_GREATER_THAN;
	if (!strcasecmp(type_buff, "OPH_GREATER_OR_EQUAL_TO"))
		return OPH_BIT_GREATER_OR_EQUAL_TO;
	if (!strcasecmp(type_buff, "OPH_LESS_THAN"))
		return OPH_BIT_LESS_THAN;
	if (!strcasecmp(type_buff, "OPH_LESS_OR_EQUAL_TO") || !strcasecmp(type_buff, "OPH_IMPLY"))
		return OPH_BIT_LESS_OR_EQUAL_TO;
	pmesg(1, __FILE__, __LINE__, "Invalid operator\n");
	return INVALID_BIT_OP;
}

int core_oph_bit_import(oph_bit_param * param)
{
	oph_string *measure = (oph_string *) param->measure;
	unsigned char tmp1 = OPH_BIT_BLOCK_MSB, tmp2 = 0;
	unsigned long i, j = 0;
	switch (measure->type) {
		case OPH_DOUBLE:
			{
				double *d;
				for (i = 0; i < measure->numelem; i++) {
					d = (double *) (measure->content + i * measure->elemsize);
					if (*d)
						tmp2 |= tmp1;
					else
						tmp2 &= ~tmp1;	// bzero(size) could be an alternative
					if (tmp1 == 1) {
						*(param->result + j) = tmp2;
						tmp1 = OPH_BIT_BLOCK_MSB;
						tmp2 = 0;
						j++;
					} else
						tmp1 >>= 1;
				}
				break;
			}
		case OPH_FLOAT:
			{
				float *d;
				for (i = 0; i < measure->numelem; i++) {
					d = (float *) (measure->content + i * measure->elemsize);
					if (*d)
						tmp2 |= tmp1;
					else
						tmp2 &= ~tmp1;	// bzero(size) could be an alternative
					if (tmp1 == 1) {
						*(param->result + j) = tmp2;
						tmp1 = OPH_BIT_BLOCK_MSB;
						tmp2 = 0;
						j++;
					} else
						tmp1 >>= 1;
				}
				break;
			}
		case OPH_INT:
			{
				int *d;
				for (i = 0; i < measure->numelem; i++) {
					d = (int *) (measure->content + i * measure->elemsize);
					if (*d)
						tmp2 |= tmp1;
					else
						tmp2 &= ~tmp1;	// bzero(size) could be an alternative
					if (tmp1 == 1) {
						*(param->result + j) = tmp2;
						tmp1 = OPH_BIT_BLOCK_MSB;
						tmp2 = 0;
						j++;
					} else
						tmp1 >>= 1;
				}
				break;
			}
		case OPH_SHORT:
			{
				short *d;
				for (i = 0; i < measure->numelem; i++) {
					d = (short *) (measure->content + i * measure->elemsize);
					if (*d)
						tmp2 |= tmp1;
					else
						tmp2 &= ~tmp1;	// bzero(size) could be an alternative
					if (tmp1 == 1) {
						*(param->result + j) = tmp2;
						tmp1 = OPH_BIT_BLOCK_MSB;
						tmp2 = 0;
						j++;
					} else
						tmp1 >>= 1;
				}
				break;
			}
		case OPH_BYTE:
			{
				char *d;
				for (i = 0; i < measure->numelem; i++) {
					d = (char *) (measure->content + i * measure->elemsize);
					if (*d)
						tmp2 |= tmp1;
					else
						tmp2 &= ~tmp1;	// bzero(size) could be an alternative
					if (tmp1 == 1) {
						*(param->result + j) = tmp2;
						tmp1 = OPH_BIT_BLOCK_MSB;
						tmp2 = 0;
						j++;
					} else
						tmp1 >>= 1;
				}
				break;
			}
		case OPH_LONG:
			{
				long long *d;
				for (i = 0; i < measure->numelem; i++) {
					d = (long long *) (measure->content + i * measure->elemsize);
					if (*d)
						tmp2 |= tmp1;
					else
						tmp2 &= ~tmp1;	// bzero(size) could be an alternative
					if (tmp1 == 1) {
						*(param->result + j) = tmp2;
						tmp1 = OPH_BIT_BLOCK_MSB;
						tmp2 = 0;
						j++;
					} else
						tmp1 >>= 1;
				}
				break;
			}
			// other types
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			return -1;
	}
	if (measure->numelem && (tmp1 != OPH_BIT_BLOCK_MSB))
		*(param->result + j) = tmp2;
	return 0;
}

int core_oph_bit_export(oph_bit_param * param)
{
	oph_string *measure = (oph_string *) param->measure;
	unsigned char tmp1 = OPH_BIT_BLOCK_MSB, tmp2;
	unsigned long i, j = 0;
	switch (measure->type) {
		case OPH_DOUBLE:
			{
				for (i = 0; i < measure->numelem; i++) {
					if (tmp1 == OPH_BIT_BLOCK_MSB)
						tmp2 = *(measure->content + j);
					*(double *) (param->result + i * measure->elemsize) = tmp2 & tmp1 ? OPH_BIT_ONE : OPH_BIT_ZERO;
					if (tmp1 == 1) {
						tmp1 = OPH_BIT_BLOCK_MSB;
						j++;
					} else
						tmp1 >>= 1;
				}
				break;
			}
		case OPH_FLOAT:
			{
				for (i = 0; i < measure->numelem; i++) {
					if (tmp1 == OPH_BIT_BLOCK_MSB)
						tmp2 = *(measure->content + j);
					*(float *) (param->result + i * measure->elemsize) = tmp2 & tmp1 ? OPH_BIT_ONE : OPH_BIT_ZERO;
					if (tmp1 == 1) {
						tmp1 = OPH_BIT_BLOCK_MSB;
						j++;
					} else
						tmp1 >>= 1;
				}
				break;
			}
		case OPH_INT:
			{
				for (i = 0; i < measure->numelem; i++) {
					if (tmp1 == OPH_BIT_BLOCK_MSB)
						tmp2 = *(measure->content + j);
					*(int *) (param->result + i * measure->elemsize) = tmp2 & tmp1 ? OPH_BIT_ONE : OPH_BIT_ZERO;
					if (tmp1 == 1) {
						tmp1 = OPH_BIT_BLOCK_MSB;
						j++;
					} else
						tmp1 >>= 1;
				}
				break;
			}
		case OPH_SHORT:
			{
				for (i = 0; i < measure->numelem; i++) {
					if (tmp1 == OPH_BIT_BLOCK_MSB)
						tmp2 = *(measure->content + j);
					*(short *) (param->result + i * measure->elemsize) = tmp2 & tmp1 ? OPH_BIT_ONE : OPH_BIT_ZERO;
					if (tmp1 == 1) {
						tmp1 = OPH_BIT_BLOCK_MSB;
						j++;
					} else
						tmp1 >>= 1;
				}
				break;
			}
		case OPH_BYTE:
			{
				for (i = 0; i < measure->numelem; i++) {
					if (tmp1 == OPH_BIT_BLOCK_MSB)
						tmp2 = *(measure->content + j);
					*(char *) (param->result + i * measure->elemsize) = tmp2 & tmp1 ? OPH_BIT_ONE : OPH_BIT_ZERO;
					if (tmp1 == 1) {
						tmp1 = OPH_BIT_BLOCK_MSB;
						j++;
					} else
						tmp1 >>= 1;
				}
				break;
			}
		case OPH_LONG:
			{
				for (i = 0; i < measure->numelem; i++) {
					if (tmp1 == OPH_BIT_BLOCK_MSB)
						tmp2 = *(measure->content + j);
					*(long long *) (param->result + i * measure->elemsize) = tmp2 & tmp1 ? OPH_BIT_ONE : OPH_BIT_ZERO;
					if (tmp1 == 1) {
						tmp1 = OPH_BIT_BLOCK_MSB;
						j++;
					} else
						tmp1 >>= 1;
				}
				break;
			}
			// other types
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			return -1;
	}
	return 0;
}

int core_oph_bit_operator(oph_bit_param_operator * param)
{
	oph_bit_string *measureA = (oph_bit_string *) param->measureA;
	oph_bit_string *measureB = (oph_bit_string *) param->measureB;
	unsigned long j;
	unsigned char _A = 0, _B = 0, *A, *B, *O;

	A = (unsigned char *) measureA->content;
	B = (unsigned char *) measureB->content;
	O = (unsigned char *) param->result;

	for (j = 0; j < param->length; ++j, A++, B++, O++) {
		if (j >= measureA->numelem)
			A = &_A;
		if (j >= measureB->numelem)
			B = &_B;

		switch (param->op) {
			case OPH_BIT_AND:
				{
					*O = OPH_BIT_VALUE_OF(A) & OPH_BIT_VALUE_OF(B);
					break;
				}
			case OPH_BIT_OR:
				{
					*O = OPH_BIT_VALUE_OF(A) | OPH_BIT_VALUE_OF(B);
					break;
				}
			case OPH_BIT_XOR:
				{
					*O = OPH_BIT_VALUE_OF(A) ^ OPH_BIT_VALUE_OF(B);
					break;
				}
			case OPH_BIT_NAND:
				{
					*O = ~(OPH_BIT_VALUE_OF(A) & OPH_BIT_VALUE_OF(B));
					break;
				}
			case OPH_BIT_NOR:
				{
					*O = ~(OPH_BIT_VALUE_OF(A) | OPH_BIT_VALUE_OF(B));
					break;
				}
			case OPH_BIT_XNOR:
				{
					*O = ~(OPH_BIT_VALUE_OF(A) ^ OPH_BIT_VALUE_OF(B));
					break;
				}
			case OPH_BIT_GREATER_THAN:
				{
					*O = OPH_BIT_VALUE_OF(A) & ~OPH_BIT_VALUE_OF(B);
					break;
				}
			case OPH_BIT_GREATER_OR_EQUAL_TO:
				{
					*O = OPH_BIT_VALUE_OF(A) | ~OPH_BIT_VALUE_OF(B);
					break;
				}
			case OPH_BIT_LESS_THAN:
				{
					*O = ~OPH_BIT_VALUE_OF(A) & OPH_BIT_VALUE_OF(B);
					break;
				}
			case OPH_BIT_LESS_OR_EQUAL_TO:
				{
					*O = ~OPH_BIT_VALUE_OF(A) | OPH_BIT_VALUE_OF(B);
					break;
				}
			default:
				pmesg(1, __FILE__, __LINE__, "Operator not recognized\n");
				return -1;
		}
	}

	return 0;
}

int core_oph_bit_not(oph_bit_param * param)
{
	unsigned long j;
	unsigned char *A = (unsigned char *) ((oph_bit_string *) param->measure)->content, *O = (unsigned char *) param->result;
	for (j = 0; j < param->length; ++j, A++, O++)
		*O = ~(*A);
	return 0;
}

int core_oph_bit_shift(oph_bit_param * param, long long offset, unsigned char filling, char round)
{
	int byte_shift;
	long long part1, part2, i;
	oph_bit_string *measure = (oph_bit_string *) param->measure;
	unsigned char *O, tmp, tmp2;
	if (offset > 0) {

		byte_shift = (offset % OPH_BITSBYTE);
		part2 = offset - byte_shift;	// bits
		part1 = (measure->numelem - part2) / OPH_BITSBYTE;	// bytes
		part2 /= OPH_BITSBYTE;
		// pre-shift
		if (part1)
			memcpy(param->result + part2, measure->content, part1);
		if (part2) {
			// rotate
			if (round)
				memcpy(param->result, measure->content + part1, part2);
			// filling
			else
				for (i = 0, O = (unsigned char *) param->result; i < part2; ++i, O++)
					*O = filling;
		}
		// post-shift
		if (byte_shift) {
			O = (unsigned char *) param->result;
			if (round)
				tmp = *(O + param->length - 1) << (OPH_BITSBYTE - byte_shift);
			else
				tmp = filling << (OPH_BITSBYTE - byte_shift);
			for (i = 0; i < param->length; ++i, O++) {
				tmp2 = *O << (OPH_BITSBYTE - byte_shift);
				*O = (*O >> byte_shift) | tmp;
				tmp = tmp2;
			}
		}
	} else if (offset < 0) {
		byte_shift = ((-offset) % OPH_BITSBYTE);
		part2 = (-offset) - byte_shift;	// bits
		part1 = (measure->numelem - part2) / OPH_BITSBYTE;	// bytes
		part2 /= OPH_BITSBYTE;	// bytes
		// pre-shift
		if (part1)
			memcpy(param->result, measure->content + part2, part1);
		if (part2) {
			// rotate
			if (round)
				memcpy(param->result + part1, measure->content, part2);
			// filling
			else
				for (i = 0, O = (unsigned char *) (param->result + part1); i < part2; ++i, O++)
					*O = filling;
		}
		// post-shift
		if (byte_shift) {
			O = (unsigned char *) param->result;
			if (round)
				tmp = *O >> (OPH_BITSBYTE - byte_shift);
			else
				tmp = filling >> (OPH_BITSBYTE - byte_shift);
			O += param->length - 1;
			for (i = 0; i < param->length; ++i, O--) {
				tmp2 = *O >> (OPH_BITSBYTE - byte_shift);
				*O = (*O << byte_shift) | tmp;
				tmp = tmp2;
			}
		}
	} else
		memcpy(param->result, measure->content, param->length);
	return 0;
}

void core_oph_bit_reverse_(unsigned char *A, unsigned char *O)
{
	unsigned char tmp1 = 1, tmp2 = OPH_BIT_BLOCK_MSB;
	do {
		if (*A & tmp2)
			*O |= tmp1;
		else
			*O &= ~tmp1;
		tmp1 <<= 1;
		tmp2 >>= 1;
	}
	while (tmp2 > 0);
}

int core_oph_bit_reverse(oph_bit_param * param)
{
	unsigned long i;
	unsigned char *A = (unsigned char *) (((oph_bit_string *) param->measure)->content + param->length - 1), *O = (unsigned char *) param->result;
	for (i = 0; i < param->length; ++i, O++, A--)
		core_oph_bit_reverse_(A, O);	// loop on bytes
	return 0;
}

int core_oph_bit_count(oph_bit_param * param, unsigned char value, long long *result)
{
	unsigned long i;
	oph_bit_string *measure = (oph_bit_string *) param->measure;
	unsigned char *A = (unsigned char *) measure->content, tmp = OPH_BIT_BLOCK_MSB;
	*result = 0;
	for (i = 0; i < measure->numelem; ++i)	// loop on bits
	{
		//if ((*A & tmp & value) | ~(*A | ~tmp | value)) (*result)++;
		if ((*A ^ ~value) & tmp)
			(*result)++;
		if (tmp == 1) {
			tmp = OPH_BIT_BLOCK_MSB;
			A++;	// next byte
		} else
			tmp >>= 1;
	}
	return 0;
}

int core_oph_bit_subarray(oph_bit_param_subarray * param)
{
	unsigned long i, tot = 0;
	oph_bit_string *measure = (oph_bit_string *) param->measure;
	unsigned char *A, *O, tmpA, tmpO, tmp;

	O = (unsigned char *) param->result;
	tmpO = OPH_BIT_BLOCK_MSB;

	if (param->step > 1) {
		for (i = param->start; tot < param->size; tot++, i += param->step)	// loop on bits
		{
			if (i >= measure->numelem) {
				pmesg(1, __FILE__, __LINE__, "Index out of range\n");
				return -1;
			}
			A = (unsigned char *) (measure->content + (i / OPH_BITSBYTE));
			tmpA = OPH_BIT_BLOCK_MSB;
			if ((tmp = i % OPH_BITSBYTE))
				tmpA >>= tmp;
			if (*A & tmpA)
				*O |= tmpO;
			else
				*O &= ~tmpO;
			if (tmpO == 1) {
				tmpO = OPH_BIT_BLOCK_MSB;
				O++;
			} else
				tmpO >>= 1;
		}
	} else			// more optimized for step = 1
	{
		A = (unsigned char *) (measure->content + (param->start / OPH_BITSBYTE));
		tmpA = OPH_BIT_BLOCK_MSB;
		if ((tmp = param->start % OPH_BITSBYTE))
			tmpA >>= tmp;
		for (i = param->start; tot < param->size; tot++, i++)	// loop on bits
		{
			if (i >= measure->numelem) {
				pmesg(1, __FILE__, __LINE__, "Index out of range\n");
				return -1;
			}
			if (*A & tmpA)
				*O |= tmpO;
			else
				*O &= ~tmpO;
			if (tmpA == 1) {
				tmpA = OPH_BIT_BLOCK_MSB;
				A++;
			} else
				tmpA >>= 1;
			if (tmpO == 1) {
				tmpO = OPH_BIT_BLOCK_MSB;
				O++;
			} else
				tmpO >>= 1;
		}
	}
	if (tmpO < OPH_BIT_BLOCK_MSB)
		while (tmpO) {
			*O &= ~tmpO;
			tmpO >>= 1;
		}		// padding

	return 0;
}

// This implementation should be changed in case padding!=0
int core_oph_bit_concat(oph_bit_param2 * param)
{
	oph_bit_string *measureA = (oph_bit_string *) param->measureA;
	oph_bit_string *measureB = (oph_bit_string *) param->measureB;

	memcpy(param->result, measureA->content, measureA->numelem);
	memcpy(param->result + measureA->numelem, measureB->content, measureB->numelem);

	return 0;
}

int core_oph_bit_find(oph_bit_param2 * param)
{
	unsigned long i, m = 0, size = core_sizeof(OPH_LONG);	// Da ripensare!!!!!!
	oph_bit_string *measure = (oph_bit_string *) param->measureA;
	oph_bit_string *pattern = (oph_bit_string *) param->measureB;
	unsigned char *A = (unsigned char *) measure->content, *AA, tmpA = OPH_BIT_BLOCK_MSB, tmpAA, *P = (unsigned char *) pattern->content, tmpP = OPH_BIT_BLOCK_MSB;

	long long position = 0;

	param->length = 0;
	for (i = 0; i < measure->numelem; ++i)	// loop on bits
	{
		if (((*A & tmpA) && (*P & tmpP)) || ((~(*A) & tmpA) && (~(*P) & tmpP))) {
			if (!m) {
				position = i;
				AA = A;
				tmpAA = tmpA;
			}
			m++;
			if (m == pattern->numelem) {
				pmesg(3, __FILE__, __LINE__, "Find a matching pattern\n");
				m = 0;
				tmpP = OPH_BIT_BLOCK_MSB;
				P = (unsigned char *) pattern->content;
				i = position;	// Reset
				position++;
				memcpy(param->result + param->length, &position, size);
				param->length += size;
				A = AA;	// Reset
				tmpA = tmpAA;	// Reset
			}
		} else if (m)	// Reset
		{
			m = 0;
			tmpP = OPH_BIT_BLOCK_MSB;
			P = (unsigned char *) pattern->content;
			i = position;	// Reset
			A = AA;	// Reset
			tmpA = tmpAA;	// Reset
		}

		if (m) {
			if (tmpP == 1) {
				tmpP = OPH_BIT_BLOCK_MSB;
				P++;	// next byte
			} else
				tmpP >>= 1;
		}
		if (tmpA == 1) {
			tmpA = OPH_BIT_BLOCK_MSB;
			A++;	// next byte
		} else
			tmpA >>= 1;
	}

	return 0;
}

int core_oph_bit_reduce(oph_bit_param_reduce * param)
{
	oph_bit_string *measure = (oph_bit_string *) param->measure;
	unsigned long i = 0, j, k;
	unsigned char *A = (unsigned char *) measure->content, tmpA = OPH_BIT_BLOCK_MSB, *O = (unsigned char *) param->result, tmpO = OPH_BIT_BLOCK_MSB;
	*O = 0;

	for (j = 0; j < measure->numelem; ++j) {
		switch (param->op) {
			case OPH_BIT_AND:
				{
					if (~(*A) & tmpA)	// Found a 0
					{
						k = param->count - i - 1;	// Go to the next group
						j += k;
						while (k) {
							if (tmpA == 1) {
								tmpA = OPH_BIT_BLOCK_MSB;
								A++;	// next byte
							} else
								tmpA >>= 1;
							k--;
						}
						i = param->count;	// This group is completed
						*O &= ~tmpO;
					} else if (~(*O) & tmpO)
						*O |= tmpO;
					break;
				}
			case OPH_BIT_OR:
				{
					if (*A & tmpA)	// Found a 1
					{
						k = param->count - i - 1;	// Go to the next group
						j += k;
						while (k) {
							if (tmpA == 1) {
								tmpA = OPH_BIT_BLOCK_MSB;
								A++;	// next byte
							} else
								tmpA >>= 1;
							k--;
						}
						i = param->count;	// This group is completed
						*O |= tmpO;
					} else if (*O & tmpO)
						*O &= ~tmpO;
					break;
				}
			default:
				pmesg(1, __FILE__, __LINE__, "Operator not recognized or not supported\n");
				return -1;
		}
		i++;		// Go to the next bit of the group
		if (i >= param->count)	// This group is completed. Go to the next group
		{
			i = 0;
			if (tmpO == 1) {
				tmpO = OPH_BIT_BLOCK_MSB;
				O++;	// next byte
				*O = 0;
			} else
				tmpO >>= 1;
		}
		if (tmpA == 1) {
			tmpA = OPH_BIT_BLOCK_MSB;
			A++;	// next byte
		} else
			tmpA >>= 1;
	}
/*
	if (i>0) switch(param->op)
	{
		case OPH_BIT_AND:
		{
			*O &= ~tmpO; // paddings is 0
			break;
		}
		case OPH_BIT_OR:
		{
			// No final operations
			break;
		}
		default:
	                pmesg(1, __FILE__, __LINE__, "Operator not recognized or not supported\n");
	                return -1;
	}
*/
	return 0;
}

int core_oph_bit_aggregate(oph_bit_param_aggregate * param)
{
	oph_bit_string *measure = (oph_bit_string *) param->measure;
	unsigned long i;
	unsigned char *A = (unsigned char *) measure->content, *O, tmp = OPH_BIT_BLOCK_MSB;

	if (!param->initialized) {
		O = (unsigned char *) param->result;
		switch (param->op) {
			case OPH_BIT_AND:
				{
					for (i = 0; i < param->length; ++i, ++O)
						*O = OPH_BIT_BLOCK_MAX;
					break;
				}
			case OPH_BIT_OR:
				{
					for (i = 0; i < param->length; ++i, ++O)
						*O = 0;
					break;
				}
			default:
				pmesg(1, __FILE__, __LINE__, "Operator not recognized or not supported\n");
				return -1;
		}
		param->initialized = 1;
	}

	O = (unsigned char *) param->result;
	switch (param->op) {
		case OPH_BIT_AND:
			{
				for (i = 0; i < measure->numelem; ++i) {
					if (*O & tmp)
						*O &= ~tmp | *A;
					if (tmp == 1) {
						tmp = OPH_BIT_BLOCK_MSB;
						A++;	// next byte
						O++;
					} else
						tmp >>= 1;
				}
				break;
			}
		case OPH_BIT_OR:
			{
				for (i = 0; i < measure->numelem; ++i) {
					if (~(*O) & tmp)
						*O |= tmp & *A;
					if (tmp == 1) {
						tmp = OPH_BIT_BLOCK_MSB;
						A++;	// next byte
						O++;
					} else
						tmp >>= 1;
				}
				break;
			}
		default:
			pmesg(1, __FILE__, __LINE__, "Operator not recognized or not supported\n");
			return -1;
	}

	return 0;
}

int core_oph_mask(oph_mask_param * param)
{
	oph_string *measure = (oph_string *) param->measure;
	oph_bit_string *mask = (oph_bit_string *) param->mask;
	unsigned char *A = (unsigned char *) mask->content, tmp = OPH_BIT_BLOCK_MSB;
	unsigned long i;

	if (mask->numelem) {
		switch (measure->type) {
			case OPH_DOUBLE:
				{
					double *pointer;
					for (i = 0; i < measure->numelem; ++i)	// loop on elements of 'measure' and on 'bits'
					{
						if ((i >= mask->numelem) || (~(*A) & tmp))
							pointer = &param->filling;
						else
							pointer = (double *) measure->content + i;

						if (core_oph_type_cast(pointer, param->result + i * param->result_elemsize, measure->type, param->result_type, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}

						if (i < mask->numelem) {
							if (tmp == 1) {
								tmp = OPH_BIT_BLOCK_MSB;
								A++;	// next byte
							} else
								tmp >>= 1;
						}
					}
					break;
				}

			case OPH_FLOAT:
				{
					float data, *pointer;
					for (i = 0; i < measure->numelem; ++i)	// loop on elements of 'measure' and on 'bits'
					{
						if ((i >= mask->numelem) || (~(*A) & tmp)) {
							data = (float) param->filling;
							pointer = &data;
						} else
							pointer = (float *) measure->content + i;

						if (core_oph_type_cast(pointer, param->result + i * param->result_elemsize, measure->type, param->result_type, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}

						if (i < mask->numelem) {
							if (tmp == 1) {
								tmp = OPH_BIT_BLOCK_MSB;
								A++;	// next byte
							} else
								tmp >>= 1;
						}
					}
					break;
				}

			case OPH_INT:
				{
					int data, *pointer;
					for (i = 0; i < measure->numelem; ++i)	// loop on elements of 'measure' and on 'bits'
					{
						if ((i >= mask->numelem) || (~(*A) & tmp)) {
							data = (int) param->filling;
							pointer = &data;
						} else
							pointer = (int *) measure->content + i;

						if (core_oph_type_cast(pointer, param->result + i * param->result_elemsize, measure->type, param->result_type, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}

						if (i < mask->numelem) {
							if (tmp == 1) {
								tmp = OPH_BIT_BLOCK_MSB;
								A++;	// next byte
							} else
								tmp >>= 1;
						}
					}
					break;
				}

			case OPH_SHORT:
				{
					short data, *pointer;
					for (i = 0; i < measure->numelem; ++i)	// loop on elements of 'measure' and on 'bits'
					{
						if ((i >= mask->numelem) || (~(*A) & tmp)) {
							data = (short) param->filling;
							pointer = &data;
						} else
							pointer = (short *) measure->content + i;

						if (core_oph_type_cast(pointer, param->result + i * param->result_elemsize, measure->type, param->result_type, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}

						if (i < mask->numelem) {
							if (tmp == 1) {
								tmp = OPH_BIT_BLOCK_MSB;
								A++;	// next byte
							} else
								tmp >>= 1;
						}
					}
					break;
				}

			case OPH_BYTE:
				{
					char data, *pointer;
					for (i = 0; i < measure->numelem; ++i)	// loop on elements of 'measure' and on 'bits'
					{
						if ((i >= mask->numelem) || (~(*A) & tmp)) {
							data = (char) param->filling;
							pointer = &data;
						} else
							pointer = (char *) measure->content + i;

						if (core_oph_type_cast(pointer, param->result + i * param->result_elemsize, measure->type, param->result_type, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}

						if (i < mask->numelem) {
							if (tmp == 1) {
								tmp = OPH_BIT_BLOCK_MSB;
								A++;	// next byte
							} else
								tmp >>= 1;
						}
					}
					break;
				}

			case OPH_LONG:
				{
					long long data, *pointer;
					for (i = 0; i < measure->numelem; ++i)	// loop on elements of 'measure' and on 'bits'
					{
						if ((i >= mask->numelem) || (~(*A) & tmp)) {
							data = (long long) param->filling;
							pointer = &data;
						} else
							pointer = (long long *) measure->content + i;

						if (core_oph_type_cast(pointer, param->result + i * param->result_elemsize, measure->type, param->result_type, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
							return 1;
						}

						if (i < mask->numelem) {
							if (tmp == 1) {
								tmp = OPH_BIT_BLOCK_MSB;
								A++;	// next byte
							} else
								tmp >>= 1;
						}
					}
					break;
				}

			default:
				pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
				return -1;
		}
	} else if (measure->type != param->result_type)	// Casting only
	{
		for (i = 0; i < measure->numelem; ++i) {
			if (core_oph_type_cast(measure->content + i * measure->elemsize, param->result + i * param->result_elemsize, measure->type, param->result_type, NULL)) {
				pmesg(1, __FILE__, __LINE__, "Unable to find result\n");
				return 1;
			}
		}
	} else
		memcpy(param->result, measure->content, param->length);	// Identity

	return 0;
}

int core_oph_bit_dump(oph_bit_param * param)
{
	unsigned long i;
	oph_bit_string *measure = (oph_bit_string *) param->measure;
	unsigned char *A = (unsigned char *) measure->content, tmp = OPH_BIT_BLOCK_MSB;
	for (i = 0; i < measure->numelem; ++i)	// loop on bits
	{
		param->result[i] = *A & tmp ? OPH_BIT_ONE_S : OPH_BIT_ZERO_S;
		if (tmp == 1) {
			tmp = OPH_BIT_BLOCK_MSB;
			A++;	// next byte
		} else
			tmp >>= 1;
	}
	return 0;
}
