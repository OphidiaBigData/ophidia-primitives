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

#include "oph_reduce3.h"

#ifdef GSL_SUPPORTED
#include <gsl/gsl_sort.h>
#endif

int msglevel = 1;

int core_oph_quantile (oph_stringPtr byte_array, char *result)
{
#ifdef GSL_SUPPORTED
	if (!byte_array || (byte_array->param<0.0) || (byte_array->param>1.0))
	{
		pmesg(1, __FILE__, __LINE__, "Wrong parameter\n");
		return -1;
	}
        int k;
        switch(byte_array->type)
	{
                case OPH_DOUBLE:{
			double sum;
			memcpy(byte_array->extend, byte_array->content, *byte_array->length);
			gsl_sort((double*)byte_array->extend,1,byte_array->numelem);
			double delta = (byte_array->numelem-1) * byte_array->param;
			k = floor(delta);
			delta = delta - k;
			sum = (1-delta) * (*(((double*)byte_array->extend)+k)) + delta * (*(((double*)byte_array->extend)+k+1));
			memcpy(result, (void*)(&sum), core_sizeof(OPH_DOUBLE));
                        break;
                }
                case OPH_FLOAT:{
			float sum;
			memcpy(byte_array->extend, byte_array->content, *byte_array->length);
                        gsl_sort_float((float*)byte_array->extend,1,byte_array->numelem);
			float delta = (byte_array->numelem-1) * byte_array->param;
			k = floor(delta);
			delta = delta - k;
			sum = (1-delta) * (*(((float*)byte_array->extend)+k)) + delta * (*(((float*)byte_array->extend)+k+1));
			memcpy(result, (void*)(&sum), core_sizeof(OPH_FLOAT));
                        break;
                }
                case OPH_INT:{
			float sum; //The quantile is a float number
			memcpy(byte_array->extend, byte_array->content, *byte_array->length);
			gsl_sort_int((int*)byte_array->extend,1,byte_array->numelem);
			float delta = (byte_array->numelem-1) * byte_array->param;
			k = floor(delta);
			delta = delta - k;
			sum = (1-delta) * (*(((int*)byte_array->extend)+k)) + delta * (*(((int*)byte_array->extend)+k+1));
			memcpy(result, (void*)(&sum), core_sizeof(OPH_FLOAT));
                        break;
                }
                case OPH_SHORT:{
			float sum; //The quantile is a float number
			memcpy(byte_array->extend, byte_array->content, *byte_array->length);
			gsl_sort_short((short*)byte_array->extend,1,byte_array->numelem);
			float delta = (byte_array->numelem-1) * byte_array->param;
			k = floor(delta);
			delta = delta - k;
			sum = (1-delta) * (*(((short*)byte_array->extend)+k)) + delta * (*(((short*)byte_array->extend)+k+1));
			memcpy(result, (void*)(&sum), core_sizeof(OPH_FLOAT));
                        break;
                }
                case OPH_BYTE:{
			float sum; //The quantile is a float number
			memcpy(byte_array->extend, byte_array->content, *byte_array->length);
			gsl_sort_char((char*)byte_array->extend,1,byte_array->numelem);
			float delta = (byte_array->numelem-1) * byte_array->param;
			k = floor(delta);
			delta = delta - k;
			sum = (1-delta) * (*(((char*)byte_array->extend)+k)) + delta * (*(((char*)byte_array->extend)+k+1));
			memcpy(result, (void*)(&sum), core_sizeof(OPH_FLOAT));
                        break;
                }
		case OPH_LONG:{
			double sum; //The quantile is a double number
			memcpy(byte_array->extend, byte_array->content, *byte_array->length);
			gsl_sort_long((long*)byte_array->extend,1,byte_array->numelem);
			double delta = (byte_array->numelem-1) * byte_array->param;
			k = floor(delta);
			delta = delta - k;
			sum = (1-delta) * (*(((long long*)byte_array->extend)+k)) + delta * (*(((long long*)byte_array->extend)+k+1));
			memcpy(result, (void*)(&sum), core_sizeof(OPH_DOUBLE));
                        break;
                }
                default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
        }
        return 0;
#else
	pmesg(1, __FILE__, __LINE__, "Operation not allowed\n");
	return -1;
#endif
}

long long get_index_of(long long row_index, long long* list, long long new_size, long long reduced_size)
{
	if (!list) return 0;
	long long i, start=0, stop=0, relative_row_index = row_index%reduced_size;
	for (i=0;i<new_size-1;++i)
	{
		stop += list[i];
		if ((relative_row_index >= start) && (relative_row_index < stop)) break;
		start = stop;
	}
	return i+row_index/reduced_size*new_size;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_reduce3_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
        if((args->arg_count < 3) || (args->arg_count > 8))
	{
		strcpy(message, "ERROR: Wrong arguments! oph_reduce3(input_OPH_TYPE, output_OPH_TYPE, measure, [OPH_REDUCE3_OPERATOR], [binary_count_list], [block_size], [size], [order])");
		return 1;
        }

	int i;
        for(i = 0; i < 3; i++){
                if(args->arg_type[i] != STRING_RESULT){
                        strcpy(message, "ERROR: Wrong arguments to oph_reduce3 function");
                        return 1;
                }
        }

	if (args->arg_count > 3)
	{
		if (args->arg_type[3] != STRING_RESULT)
		{
			strcpy(message, "ERROR: Wrong argument 'operator' to oph_reduce3 function");
			return 1;
		}
		if (args->arg_count > 4)
		{
			if (args->arg_type[4] != STRING_RESULT)
			{
				strcpy(message, "ERROR: Wrong argument 'binary_count_list' to oph_reduce3 function");
				return 1;
			}
			if (args->arg_count > 5)
			{
				if (args->arg_type[5] != INT_RESULT)
				{
					strcpy(message, "ERROR: Wrong argument 'block_size' to oph_reduce3 function");
					return 1;
				}
				if (args->arg_count > 6)
				{
					if (args->arg_type[6] != INT_RESULT)
					{
						strcpy(message, "ERROR: Wrong argument 'size' to oph_reduce3 function");
						return 1;
					}
					if (args->arg_count > 7)
					{
						if (args->arg_type[7] == STRING_RESULT)
						{
							strcpy(message, "ERROR: Wrong argument 'order' to oph_reduce3 function");
							return 1;
						}
						args->arg_type[7] = REAL_RESULT;
					}
				}
			}
		}
	}

	initid->ptr = NULL;
	return 0;
}

void oph_reduce3_deinit(UDF_INIT *initid)
{
        //Free allocated space
        if(initid->ptr){
		oph_reduce3_param* tmp = (oph_reduce3_param*)initid->ptr;
		if (tmp->result) free(tmp->result);
		if (tmp->temp) free(tmp->temp);
		if (tmp->temp2) free(tmp->temp2);
	        free(initid->ptr);
        	initid->ptr=NULL;
	}
}

char* oph_reduce3(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
        oph_request inp_req;

        long long hierarchy_set = 0;
        long long result_length = 0;
	long long block_size = 1, numelement_to_reduce, max_size = 0, new_size = 1, reduced_size = 0;
	long long* list = NULL;
        int i = 0, j, jj;
	unsigned long lll, olll, k;

	//Function pointer
	int (*core_oph_oper) (oph_stringPtr byte_array, char *res);

	if (args->arg_count > 5)
	{
		block_size = *((long long*)(args->args[5]));
		if(block_size<=0)
		{
		        pmesg(1,  __FILE__, __LINE__, "Wrong arguments to oph_reduce3 function\n");
		        *length=0;
		        *is_null=0;
		        *error=1;
		        return NULL;
		}
	}

        core_set_type(&(inp_req.measure), args->args[0], &(args->lengths[0]));

	if(!inp_req.measure.type){
                pmesg(1,  __FILE__, __LINE__, "Unable to recognize measures type\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
        }

        if(args->arg_count < 4){
                core_set_oper(&inp_req, NULL, 0);
        }
        else{
                core_set_oper(&inp_req, args->args[3], &(args->lengths[3]));
        }

	if(!inp_req.oper){
                pmesg(1,  __FILE__, __LINE__, "Operator not recognized\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
        }

        /** SET TO DEFAULT HIERARCHY - maybe useful for further extensions **/
        core_set_hier(&inp_req, NULL, 0);
	if(!inp_req.hier){
                pmesg(1,  __FILE__, __LINE__, "Hierarchy not recognized\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
        }

        inp_req.measure.content = args->args[2];
        inp_req.measure.length = &(args->lengths[2]);

	if(core_set_elemsize(&(inp_req.measure))){
                pmesg(1,  __FILE__, __LINE__, "Unable to recognize measures type\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
        }

        if(core_set_numelem(&(inp_req.measure))){
        	pmesg(1,  __FILE__, __LINE__, "Error on counting elements\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
        }

	if (args->arg_count < 6)
		numelement_to_reduce = (long long)inp_req.measure.numelem;
	else
	{
		if(((long long)inp_req.measure.numelem) % block_size)
		{
			pmesg(1,  __FILE__, __LINE__, "Error: 'block_size' %lld not applicable on %ld elements\n", block_size, inp_req.measure.numelem);
		        *length=0;
		        *is_null=0;
		        *error=1;
		        return NULL;
		}
		numelement_to_reduce = (long long)inp_req.measure.numelem / block_size;
	}

	if (args->arg_count > 6) max_size = *((long long*)(args->args[6]));
	if (!max_size) max_size = numelement_to_reduce;

        if(args->arg_count < 5)
	{
                hierarchy_set = (long long) (inp_req.measure.numelem);
		result_length = 1;
	}
        else
	{
		list = (long long*)(args->args[4]);
		hierarchy_set = list ? *list : 0;
		if (!hierarchy_set || (hierarchy_set>max_size))
		{
			hierarchy_set = max_size;
			result_length = inp_req.measure.numelem/max_size;
		}
		else if (hierarchy_set<0)
		{
			pmesg(1,  __FILE__, __LINE__, "Wrong parameter value 'count' %lld\n",hierarchy_set);
		        *length=0;
		        *is_null=0;
		        *error=1;
		        return NULL;
		}
		else
		{
			new_size = args->lengths[4] / sizeof(long long);
			reduced_size = hierarchy_set;
			for (jj=1;jj<new_size;++jj)
			{
				reduced_size += list[jj];
				if (list[jj] > hierarchy_set) hierarchy_set=list[jj];
			}

			/* Check if I can apply the hierarchy */
			if(numelement_to_reduce%reduced_size)
			{
				pmesg(1,  __FILE__, __LINE__, "Error: hierarchy_set %lld not applicable on %ld elements\n", reduced_size, inp_req.measure.numelem);
				*length=0;
				*is_null=0;
				*error=1;
				return NULL;
			}

			result_length = inp_req.measure.numelem/reduced_size*new_size;
		}
        }

	oph_type result_type = inp_req.measure.type;
	size_t result_size = inp_req.measure.elemsize;
        switch(inp_req.oper)
	{
                case OPH_COUNT:
			core_oph_oper = core_oph_count;
			break;
                case OPH_MAX:
			core_oph_oper = core_oph_max;
			break;
                case OPH_MIN:
			core_oph_oper = core_oph_min;
			break;
                case OPH_SUM:
			core_oph_oper = core_oph_sum;
			break;
		case OPH_AVG:
			core_oph_oper = core_oph_avg;
			break;
		case OPH_STD:
			core_oph_oper = core_oph_std;
			break;
		case OPH_VAR:
			core_oph_oper = core_oph_var;
			break;
		case OPH_CMOMENT:
			core_oph_oper = core_oph_cmoment;
			break;
		case OPH_ACMOMENT:
			core_oph_oper = core_oph_acmoment;
			break;
		case OPH_RMOMENT:
			core_oph_oper = core_oph_rmoment;
			break;
		case OPH_ARMOMENT:
			core_oph_oper = core_oph_armoment;
			break;
		case OPH_QUANTILE:
			core_oph_oper = core_oph_quantile;
			break;
		default:
                        pmesg(1,  __FILE__, __LINE__, "Unable to recognize operator\n");
                        *length=0;
                        *is_null=0;
                        *error=1;
                        return NULL;
        }
	switch(inp_req.oper)
	{
		case OPH_COUNT:
			result_type = OPH_LONG;
			result_size = core_sizeof(OPH_LONG);
			break;
		case OPH_AVG:
		case OPH_STD:
		case OPH_VAR:
		case OPH_CMOMENT:
		case OPH_ACMOMENT:
		case OPH_RMOMENT:
		case OPH_ARMOMENT:
		case OPH_QUANTILE:
			if(inp_req.measure.type == OPH_BYTE || inp_req.measure.type == OPH_SHORT || inp_req.measure.type == OPH_INT)
			{
				result_type = OPH_FLOAT;
				result_size = core_sizeof(OPH_FLOAT);
			}
			else if(inp_req.measure.type == OPH_LONG)
			{
				result_type = OPH_DOUBLE;
				result_size = core_sizeof(OPH_DOUBLE);
			}
			break;
		default:;
	}

	oph_string output_array;
        core_set_type(&output_array, args->args[1], &(args->lengths[1]));
	if(!output_array.type){
                pmesg(1,  __FILE__, __LINE__, "Unable to recognize measures type\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
        }
	if(core_set_elemsize(&output_array)){
                pmesg(1,  __FILE__, __LINE__, "Unable to recognize measures type\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
        }
	olll = result_length*output_array.elemsize;

        /* Allocate the right space for the result set */
	oph_reduce3_param* tmp;
        if(!initid->ptr)
	{
                tmp = (oph_reduce3_param*)malloc(sizeof(oph_reduce3_param));
		initid->ptr = (char*)tmp;
		if(!initid->ptr)
		{
			pmesg(1,  __FILE__, __LINE__, "Error allocating measures string\n");
	                *length=0;
		        *is_null=0;
        		*error=1;
                	return NULL;
		}
		tmp->result = (char *)malloc(olll);
		if(!tmp->result)
		{
			pmesg(1,  __FILE__, __LINE__, "Error allocating measures string\n");
	                *length=0;
		        *is_null=0;
        		*error=1;
                	return NULL;
		}
		if (block_size>1)
		{
			tmp->temp = (char *)malloc(hierarchy_set*inp_req.measure.elemsize);
			if(!tmp->temp)
			{
				pmesg(1,  __FILE__, __LINE__, "Error allocating measures string\n");
				*length=0;
				*is_null=0;
				*error=1;
				return NULL;
			}
		}
		else tmp->temp=NULL;
		tmp->temp2 = (char *)malloc(*inp_req.measure.length);
		if (inp_req.oper == OPH_QUANTILE)
		{
			tmp->temp2 = (char *)malloc(*inp_req.measure.length);
			if(!tmp->temp2)
			{
				pmesg(1,  __FILE__, __LINE__, "Error allocating measures string\n");
				*length=0;
				*is_null=0;
				*error=1;
				return NULL;
			}
		}
		else tmp->temp2 = NULL;
        }
	else tmp = (oph_reduce3_param*)initid->ptr;

	oph_string curr_array;
        curr_array.type = inp_req.measure.type;
        curr_array.elemsize = inp_req.measure.elemsize;
        curr_array.length = &lll;
	if(args->arg_count > 7)
	{
		curr_array.param = *((double*)(args->args[7]));
		if (curr_array.param<0)
		{
			pmesg(1,  __FILE__, __LINE__, "Wrong parameter value 'order' %f\n",curr_array.param);
		        *length=0;
		        *is_null=0;
		        *error=1;
		        return NULL;
		}
	}
	else curr_array.param = 2.0;
	curr_array.extend = (void*)tmp->temp2;

	k=0;
	char temporary[result_size];
	for(i=0; i < result_length; i++)
	{
		curr_array.numelem = list ? list[(i/block_size)%new_size] : hierarchy_set;
		*curr_array.length = (unsigned long)(curr_array.numelem*curr_array.elemsize);

		if (block_size==1)
		{
			curr_array.content = inp_req.measure.content+k*inp_req.measure.elemsize;
			k += curr_array.numelem;
		}
		else
		{
			for (j=0, k=0; k<inp_req.measure.numelem; ++k) // Loop on the input array
			{
				if (k%block_size + get_index_of(k/block_size,list,new_size,reduced_size)*block_size == i)
				{
					memcpy(tmp->temp+j*inp_req.measure.elemsize,inp_req.measure.content+k*inp_req.measure.elemsize,inp_req.measure.elemsize);
					++j;
					if (j>=curr_array.numelem) break; // Found all the elements to be reduced
				}
			}
			if (k>=inp_req.measure.numelem)
			{
				pmesg(1,  __FILE__, __LINE__, "Unable to find an element to be aggregated\n");
				*length=0;
				*is_null=0;
				*error=1;
				return NULL;
			}
			curr_array.content = tmp->temp;
		}

                if(core_oph_oper(&curr_array, temporary))
		{
                        pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
                        *length=0;
                        *is_null=0;
                        *error=1;
			return NULL;
                }
		if(core_oph_type_cast(temporary, tmp->result+i*output_array.elemsize, result_type, output_array.type))
		{
                        pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
                        *length=0;
                        *is_null=0;
                        *error=1;
			return NULL;
                }
        }
        *length = (unsigned long) (olll);

	*is_null=0;
	*error=0;

        return tmp->result;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/

