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

#include "oph_bit_sequence.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_bit_sequence_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
        if((args->arg_count < 3) || (args->arg_count > 5)){
                strcpy(message, "ERROR: Wrong arguments! oph_bit_sequence(input_OPH_TYPE, output_OPH_TYPE, bit_measure, [mask], [value])");
                return 1;
        }
        
	int i;
        for(i = 0; i < 3; i++){
                if(args->arg_type[i] != STRING_RESULT){
                        strcpy(message, "ERROR: Wrong arguments to oph_bit_sequence function");
                        return 1;
                }
        }

	if(args->arg_count > 3)
	{
		if(args->arg_type[3] != STRING_RESULT)
		{
			strcpy(message, "ERROR: Wrong arguments to oph_bit_sequence function");
			return 1;
		}
		if(args->arg_count > 4)
		{
			if(args->arg_type[4] != INT_RESULT)
			{
				strcpy(message, "ERROR: Wrong arguments to oph_bit_sequence function");
				return 1;
			}
		}
	}

	initid->ptr = NULL;

	return 0;
}

void oph_bit_sequence_deinit(UDF_INIT *initid)
{
        //Free allocated space
        if(initid->ptr)
	{
		oph_bit_param* param = (oph_bit_param*)initid->ptr;
		if (param->measure) { free(param->measure); param->measure=NULL; }
		if (param->result) { free(param->result); param->result=NULL; }
                free(initid->ptr);
                initid->ptr=NULL;
        }
}

char* oph_bit_sequence(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
        int res = 0;
	unsigned long i = 0;
	oph_bit_param* param;
	oph_string* measure;

	char* mask = NULL;
	unsigned long mask_length = 1;
	unsigned char value = OPH_BIT_BLOCK_MAX;
	if (args->arg_count > 3)
	{
		mask = args->args[3];
		if (mask)
		{
			mask_length = 0;
			if (args->lengths[3] <= OPH_BIT_SEQUENCE_STATS_NUMBER) for (i=0; i<args->lengths[3]; ++i)
			{
				if (mask[i]=='1') mask_length++;
				else if (mask[i]!='0') break;
			}
			if (!mask_length || (i<args->lengths[3]))
			{
				pmesg(1,  __FILE__, __LINE__, "Wrong argument 'mask'\n");
				*length=0;
				*is_null=0;
				*error=1;
				return NULL;
			}
		}
	}
	if (args->arg_count > 4) value = *((long long*)args->args[4]) ? OPH_BIT_BLOCK_MAX : 0;

	if(!initid->ptr)
	{
		initid->ptr=(char*)malloc(sizeof(oph_bit_param));
		if(!initid->ptr)
		{
			pmesg(1,  __FILE__, __LINE__, "Error allocating oph_bit_param\n");
			*length=0;
        	        *is_null=0;
                	*error=1;
	                return NULL;
        	}
		param = (oph_bit_param*)initid->ptr;
		param->measure = NULL;
		param->result = NULL;
	}
	else param = (oph_bit_param*)initid->ptr;

	if (!param->measure)
	{
		measure = param->measure = (oph_string*)malloc(sizeof(oph_string));
		if(!param->measure)
		{
			pmesg(1,  __FILE__, __LINE__, "Error allocating oph_string\n");
			*length=0;
        	        *is_null=0;
                	*error=1;
	                return NULL;
        	}

		// Input
		measure->numelem = core_oph_bit_numelem(args->lengths[2]); // Const

		// Output
		res = core_set_type(measure, args->args[1], &(args->lengths[1])); // Const
		if(res)
		{
			*length=0;
        	        *is_null=0;
                	*error=1;
	                return NULL;
        	}
		res = core_set_elemsize(measure); // Const
		if(res)
		{
			*length=0;
        	        *is_null=0;
                	*error=1;
	                return NULL;
        	}
		param->length = mask_length * measure->elemsize; // bytes
		if (!param->length)
		{
			*length=0;
        	        *is_null=1;
                	*error=0;
	                return NULL;
        	}
		param->result = (char*)malloc(param->length); // Output
		if(!param->result)
		{
			pmesg(1,  __FILE__, __LINE__, "Error allocating result\n");
			*length=0;
        	        *is_null=0;
                	*error=1;
	                return NULL;
        	}
	}
	else measure = (oph_string*)param->measure;

	if (!args->args[2])
	{
                *length=0;
                *is_null=1;
                *error=0;
                return NULL;
        }
	*(param->result) = 0;

	long long count = 0, sum = 0, maximum = 0, minimum = measure->numelem, current = 0;
	unsigned char *A = (unsigned char*)args->args[2], tmp=OPH_BIT_BLOCK_MSB;
	for (i=0;i<measure->numelem;++i) // loop on bits
	{
		if ((*A ^ ~value) & tmp) current++;
		else if (current)
		{
			sum += current;
			count++;
			if (maximum < current) maximum = current;
			if (minimum > current) minimum = current;
			current = 0;
		}
		if (tmp==1)
		{
			tmp=OPH_BIT_BLOCK_MSB;
			A++; // next byte
		}
		else tmp>>=1;
	}
	if (current)
	{
		sum += current;
		count++;
		if (maximum < current) maximum = current;
		if (minimum > current) minimum = current;
		current = 0;
	}

	if (!mask || (mask[0]=='1'))
	{
		core_oph_type_cast(&count, param->result + current, OPH_LONG, measure->type);
		current += measure->elemsize;
	}
	if (mask && (mask[1]=='1'))
	{
		double mean = (double)sum/(double)count;
		core_oph_type_cast(&mean, param->result + current, OPH_DOUBLE, measure->type);
		current += measure->elemsize;
	}
	if (mask && (mask[2]=='1'))
	{
		core_oph_type_cast(&maximum, param->result + current, OPH_LONG, measure->type);
		current += measure->elemsize;
	}
	if (mask && (mask[3]=='1'))
	{
		core_oph_type_cast(&minimum, param->result + current, OPH_LONG, measure->type);
		//current += measure->elemsize; // To be uncomment in case of additional stats
	}

	*length=param->length;
        *error=0;
        *is_null=0;
        return (result=param->result);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/

