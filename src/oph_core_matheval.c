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

#include "oph_core_matheval.h"

int core_set_comp(oph_comp* op, char* type, unsigned long *len){
        if(!type){
                *op = DEFAULT_COMP;
                return 0;
        }
        *op = core_get_comp(type, len);
        if(*op == INVALID_COMP){
                pmesg(1, __FILE__, __LINE__, "Comparison operator not recognized\n");
                return -1;
        }
        return 0;
}

oph_comp core_get_comp (char* type, unsigned long *len){
        char type_buff[BUFF_LEN];
        if(core_strncpy(type_buff, type, len)){
                pmesg(1, __FILE__, __LINE__, "Invalid operator\n");
                return INVALID_COMP;
        }
        //Case insensitive comparison
        if(!strcasecmp(type_buff, "OPH_GREATER_THAN_ZERO") || !strcasecmp(type_buff, "OPH_POSITIVE") || !strcasecmp(type_buff, ">0"))
                return OPH_GREATER_THAN_ZERO;
        if(!strcasecmp(type_buff, "OPH_LESS_THAN_ZERO") || !strcasecmp(type_buff, "OPH_NEGATIVE") || !strcasecmp(type_buff, "<0"))
                return OPH_LESS_THAN_ZERO;
        if(!strcasecmp(type_buff, "OPH_EQUAL_TO_ZERO") || !strcasecmp(type_buff, "OPH_ZERO") || !strcasecmp(type_buff, "==0"))
                return OPH_EQUAL_TO_ZERO;
        if(!strcasecmp(type_buff, "OPH_GREATER_OR_EQUAL_TO_ZERO") || !strcasecmp(type_buff, "OPH_NOT_NEGATIVE") || !strcasecmp(type_buff, ">=0"))
                return OPH_GREATER_OR_EQUAL_TO_ZERO;
        if(!strcasecmp(type_buff, "OPH_LESS_OR_EQUAL_TO_ZERO") || !strcasecmp(type_buff, "OPH_NOT_POSITIVE") || !strcasecmp(type_buff, "<=0"))
                return OPH_LESS_OR_EQUAL_TO_ZERO;
        if(!strcasecmp(type_buff, "OPH_NOT_EQUAL_TO_ZERO") || !strcasecmp(type_buff, "OPH_NOT_ZERO") || !strcasecmp(type_buff, "!=0"))
                return OPH_NOT_EQUAL_TO_ZERO;
	if(!strcasecmp(type_buff, "OPH_NULL") || !strcasecmp(type_buff, "NULL") || !strcasecmp(type_buff, "NAN") || !strcasecmp(type_buff, "MISSED") || !strcasecmp(type_buff, "MISS") || !strcasecmp(type_buff, "ISNULL") || !strcasecmp(type_buff, "ISNAN") || !strcasecmp(type_buff, "ISMISSED"))
                return OPH_NULL;
        pmesg(1, __FILE__, __LINE__, "Invalid operator '%s'\n",type_buff);
        return INVALID_COMP;
}

int core_oph_predicate(oph_stringPtr byte_array, char* result)
{
	unsigned long i;
	unsigned short r;
	double res;
	oph_predicate_param* _result = (oph_predicate_param*)result;
	char temporary[byte_array->elemsize];
	switch(byte_array->type)
	{
                case OPH_DOUBLE:
		{
			for (i=0; i<byte_array->numelem; ++i)
			{
				res = evaluator_evaluate_x (_result->f[1], *((double*)(byte_array->content+i*byte_array->elemsize)));
				switch (_result->op)
				{
					case OPH_GREATER_THAN_ZERO:
					{
						r = res > 0.0;
						break;
					}
					case OPH_LESS_THAN_ZERO:
					{
						r = res < 0.0;
						break;
					}
					case OPH_EQUAL_TO_ZERO:
					{
						r = res == 0.0;
						break;
					}
					case OPH_GREATER_OR_EQUAL_TO_ZERO:
					{
						r = res >= 0.0;
						break;
					}
					case OPH_LESS_OR_EQUAL_TO_ZERO:
					{
						r = res <= 0.0;
						break;
					}
					case OPH_NOT_EQUAL_TO_ZERO:
					{
						r = res != 0.0;
						break;
					}
					case OPH_NULL:
					{
						r = isnan(res);
						break;
					}
					default:
						pmesg(1, __FILE__, __LINE__, "Comparison value non recognized\n");
                        			return -1;
				}
				*((double*)temporary) = evaluator_evaluate_x (_result->f[r?2:3], *((double*)(byte_array->content+i*byte_array->elemsize)));
				if(core_oph_type_cast(temporary, (((char*)_result->f[0])+i*_result->result_elemsize), byte_array->type, _result->result_type))
				{
				        pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		case OPH_FLOAT:
		{
			for (i=0; i<byte_array->numelem; ++i)
			{
				res = evaluator_evaluate_x (_result->f[1], *((float*)(byte_array->content+i*byte_array->elemsize)));
				switch (_result->op)
				{
					case OPH_GREATER_THAN_ZERO:
					{
						r = res > 0.0;
						break;
					}
					case OPH_LESS_THAN_ZERO:
					{
						r = res < 0.0;
						break;
					}
					case OPH_EQUAL_TO_ZERO:
					{
						r = res == 0.0;
						break;
					}
					case OPH_GREATER_OR_EQUAL_TO_ZERO:
					{
						r = res >= 0.0;
						break;
					}
					case OPH_LESS_OR_EQUAL_TO_ZERO:
					{
						r = res <= 0.0;
						break;
					}
					case OPH_NOT_EQUAL_TO_ZERO:
					{
						r = res != 0.0;
						break;
					}
					case OPH_NULL:
					{
						r = isnan(res);
						break;
					}
					default:
						pmesg(1, __FILE__, __LINE__, "Comparison value non recognized\n");
                        			return -1;
				}
				*((float*)temporary) = (float)evaluator_evaluate_x (_result->f[r?2:3], *((float*)(byte_array->content+i*byte_array->elemsize)));
				if(core_oph_type_cast(temporary, (((char*)_result->f[0])+i*_result->result_elemsize), byte_array->type, _result->result_type))
				{
				        pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		case OPH_INT:
		{
			for (i=0; i<byte_array->numelem; ++i)
			{
				res = evaluator_evaluate_x (_result->f[1], *((int*)(byte_array->content+i*byte_array->elemsize)));
				switch (_result->op)
				{
					case OPH_GREATER_THAN_ZERO:
					{
						r = res > 0.0;
						break;
					}
					case OPH_LESS_THAN_ZERO:
					{
						r = res < 0.0;
						break;
					}
					case OPH_EQUAL_TO_ZERO:
					{
						r = res == 0.0;
						break;
					}
					case OPH_GREATER_OR_EQUAL_TO_ZERO:
					{
						r = res >= 0.0;
						break;
					}
					case OPH_LESS_OR_EQUAL_TO_ZERO:
					{
						r = res <= 0.0;
						break;
					}
					case OPH_NOT_EQUAL_TO_ZERO:
					{
						r = res != 0.0;
						break;
					}
					case OPH_NULL:
					{
						r = isnan(res);
						break;
					}
					default:
						pmesg(1, __FILE__, __LINE__, "Comparison value non recognized\n");
                        			return -1;
				}
				*((int*)temporary) = (int)evaluator_evaluate_x (_result->f[r?2:3], *((int*)(byte_array->content+i*byte_array->elemsize)));
				if(core_oph_type_cast(temporary, (((char*)_result->f[0])+i*_result->result_elemsize), byte_array->type, _result->result_type))
				{
				        pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		case OPH_SHORT:
		{
			for (i=0; i<byte_array->numelem; ++i)
			{
				res = evaluator_evaluate_x (_result->f[1], *((short*)(byte_array->content+i*byte_array->elemsize)));
				switch (_result->op)
				{
					case OPH_GREATER_THAN_ZERO:
					{
						r = res > 0.0;
						break;
					}
					case OPH_LESS_THAN_ZERO:
					{
						r = res < 0.0;
						break;
					}
					case OPH_EQUAL_TO_ZERO:
					{
						r = res == 0.0;
						break;
					}
					case OPH_GREATER_OR_EQUAL_TO_ZERO:
					{
						r = res >= 0.0;
						break;
					}
					case OPH_LESS_OR_EQUAL_TO_ZERO:
					{
						r = res <= 0.0;
						break;
					}
					case OPH_NOT_EQUAL_TO_ZERO:
					{
						r = res != 0.0;
						break;
					}
					case OPH_NULL:
					{
						r = isnan(res);
						break;
					}
					default:
						pmesg(1, __FILE__, __LINE__, "Comparison value non recognized\n");
                        			return -1;
				}
				*((short*)temporary) = (short)evaluator_evaluate_x (_result->f[r?2:3], *((short*)(byte_array->content+i*byte_array->elemsize)));
				if(core_oph_type_cast(temporary, (((char*)_result->f[0])+i*_result->result_elemsize), byte_array->type, _result->result_type))
				{
				        pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		case OPH_BYTE:
		{
			for (i=0; i<byte_array->numelem; ++i)
			{
				res = evaluator_evaluate_x (_result->f[1], *((char*)(byte_array->content+i*byte_array->elemsize)));
				switch (_result->op)
				{
					case OPH_GREATER_THAN_ZERO:
					{
						r = res > 0.0;
						break;
					}
					case OPH_LESS_THAN_ZERO:
					{
						r = res < 0.0;
						break;
					}
					case OPH_EQUAL_TO_ZERO:
					{
						r = res == 0.0;
						break;
					}
					case OPH_GREATER_OR_EQUAL_TO_ZERO:
					{
						r = res >= 0.0;
						break;
					}
					case OPH_LESS_OR_EQUAL_TO_ZERO:
					{
						r = res <= 0.0;
						break;
					}
					case OPH_NOT_EQUAL_TO_ZERO:
					{
						r = res != 0.0;
						break;
					}
					case OPH_NULL:
					{
						r = isnan(res);
						break;
					}
					default:
						pmesg(1, __FILE__, __LINE__, "Comparison value non recognized\n");
                        			return -1;
				}
				*((char*)temporary) = (char)evaluator_evaluate_x (_result->f[r?2:3], *((char*)(byte_array->content+i*byte_array->elemsize)));
				if(core_oph_type_cast(temporary, (((char*)_result->f[0])+i*_result->result_elemsize), byte_array->type, _result->result_type))
				{
				        pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		case OPH_LONG:
		{
			for (i=0; i<byte_array->numelem; ++i)
			{
				res = evaluator_evaluate_x (_result->f[1], *((long long*)(byte_array->content+i*byte_array->elemsize)));
				switch (_result->op)
				{
					case OPH_GREATER_THAN_ZERO:
					{
						r = res > 0.0;
						break;
					}
					case OPH_LESS_THAN_ZERO:
					{
						r = res < 0.0;
						break;
					}
					case OPH_EQUAL_TO_ZERO:
					{
						r = res == 0.0;
						break;
					}
					case OPH_GREATER_OR_EQUAL_TO_ZERO:
					{
						r = res >= 0.0;
						break;
					}
					case OPH_LESS_OR_EQUAL_TO_ZERO:
					{
						r = res <= 0.0;
						break;
					}
					case OPH_NOT_EQUAL_TO_ZERO:
					{
						r = res != 0.0;
						break;
					}
					case OPH_NULL:
					{
						r = isnan(res);
						break;
					}
					default:
						pmesg(1, __FILE__, __LINE__, "Comparison value non recognized\n");
                        			return -1;
				}
				*((long long*)temporary) = (long long)evaluator_evaluate_x (_result->f[r?2:3], *((long long*)(byte_array->content+i*byte_array->elemsize)));
				if(core_oph_type_cast(temporary, (((char*)_result->f[0])+i*_result->result_elemsize), byte_array->type, _result->result_type))
				{
				        pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
	}
	return 0;
}

