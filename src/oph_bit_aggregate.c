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

#include "oph_bit_aggregate.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_bit_aggregate_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
        if(args->arg_count < 3 || args->arg_count > 4){
                strcpy(message, "ERROR: Wrong arguments! oph_bit_aggregate(input_OPH_TYPE, output_OPH_TYPE, bit_measure, [OPH_BIT_OP])");
                return 1;
        }
        
	int i;
        for(i = 0; i < args->arg_count; i++){
                if(args->arg_type[i] != STRING_RESULT){
                        strcpy(message, "ERROR: Wrong arguments to oph_bit_aggregate function");
                        return 1;
                }
        }

	initid->ptr = NULL;

	return 0;
}

void oph_bit_aggregate_deinit(UDF_INIT *initid)
{
        //Free allocated space
        if(initid->ptr)
	{
		oph_bit_param_aggregate* param = (oph_bit_param_aggregate*)initid->ptr;
		if (param->measure) { free(param->measure); param->measure=NULL; }
		if (param->result) { free(param->result); param->result=NULL; }
                free(initid->ptr);
                initid->ptr=NULL;
        }
}

void oph_bit_aggregate_operator_reset( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* error )
{
	oph_bit_aggregate_clear( initid, is_null, error );
	oph_bit_aggregate_add( initid, args, is_null, error );
}

void oph_bit_aggregate_clear( UDF_INIT* initid, char* is_null, char* error )
{
	oph_bit_param_aggregate* param;
	
	if(!initid->ptr)
	{
		initid->ptr=(char*)malloc(sizeof(oph_bit_param_aggregate));
		if(!initid->ptr)
		{
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_bit_param_aggregate\n");
        	        *is_null=0;
                	*error=1;
	                return;
        	}
		param = (oph_bit_param_aggregate*)initid->ptr;
		param->measure = NULL;
		param->result = NULL;
	}
	else param = (oph_bit_param_aggregate*)initid->ptr;

	if (!param->measure)
	{
		param->measure = (oph_bit_string*)malloc(sizeof(oph_bit_string));
		if(!param->measure)
		{
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_bit_string\n");
        	        *is_null=0;
                	*error=1;
	                return;
        	}
		param->result = NULL;
	}

	param->initialized = 0;
}

void oph_bit_aggregate_add( UDF_INIT* initid, UDF_ARGS* args, char* is_null, char* error )
{
	int res = 0;
	oph_bit_param_aggregate* param;
	oph_bit_string* measure;

	if(!initid->ptr)
	{
		initid->ptr=(char*)malloc(sizeof(oph_bit_param_aggregate));
		if(!initid->ptr)
		{
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_bit_param_aggregate\n");
        	        *is_null=0;
                	*error=1;
	                return;
        	}
		param = (oph_bit_param_aggregate*)initid->ptr;
		param->measure = NULL;
		param->result = NULL;
		param->initialized = 0;
	}
	else param = (oph_bit_param_aggregate*)initid->ptr;

	if (!param->measure)
	{
		param->measure = (oph_bit_string*)malloc(sizeof(oph_bit_string));
		if(!param->measure)
		{
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_bit_string\n");
        	        *is_null=0;
                	*error=1;
	                return;
        	}
	}

	measure = param->measure;
	
	if (!param->result)
	{
		if (!args->lengths[2])
		{
        	        *is_null=1;
                	*error=0;
	                return;
        	}
		measure->numelem = core_oph_bit_numelem(args->lengths[2]); // Const
		
		if (args->arg_count < 4)
			res = core_oph_bit_set_op(&param->op, NULL, 0); // Const
		else
			res = core_oph_bit_set_op(&param->op, args->args[3], &(args->lengths[3])); // Const
		if(res)
		{
        	        *is_null=0;
                	*error=1;
	                return;
        	}
		
		param->length = args->lengths[2]; // bytes

		param->result = (char*)malloc(param->length); // Output
		if(!param->result)
		{
			pmesg(1, __FILE__, __LINE__, "Error allocating result\n");
        	        *is_null=0;
                	*error=1;
	                return;
        	}

		param->initialized = 0;
	}

	measure->content = args->args[2]; // Input
	if (!measure->content)
	{
                *is_null=1;
                *error=0;
                return;
        }

        res = core_oph_bit_aggregate(param);
        if(res)
	{
                pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
                *is_null=1;
                *error=1;
                return;
        }
        *error=0;
        *is_null=0;
}

char* oph_bit_aggregate(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
        int res = 0;
	oph_bit_param_aggregate* param;
	oph_bit_string* measure;
	
	if(!initid->ptr)
	{
		pmesg(1, __FILE__, __LINE__, "Error checking the result\n");
		*length=0;
        	*is_null=0;
                *error=1;
	        return NULL;
	}
	else param = (oph_bit_param_aggregate*)initid->ptr;

	*length=param->length;
        *error=0;
        *is_null=0;
        return (result=param->result);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/

