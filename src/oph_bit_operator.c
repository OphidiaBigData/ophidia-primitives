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

#include "oph_bit_operator.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_bit_operator_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
        if(args->arg_count < 4 || args->arg_count > 5){
                strcpy(message, "ERROR: Wrong arguments! oph_bit_operator(input_OPH_TYPE, output_OPH_TYPE, bit_measureA, bit_measureB, [OPH_BIT_OP])");
                return 1;
        }
        
	int i;
        for(i = 0; i < args->arg_count; i++){
                if(args->arg_type[i] != STRING_RESULT){
                        strcpy(message, "ERROR: Wrong arguments to oph_bit_operator function");
                        return 1;
                }
        }
        
	initid->ptr = NULL;

	return 0;
}

void oph_bit_operator_deinit(UDF_INIT *initid)
{
        //Free allocated space
        if(initid->ptr)
	{
		oph_bit_param_operator* param = (oph_bit_param_operator*)initid->ptr;
		if (param->measureA) { free(param->measureA); param->measureA=NULL; }
		if (param->measureB) { free(param->measureB); param->measureB=NULL; }
		if (param->result) { free(param->result); param->result=NULL; }
                free(initid->ptr);
                initid->ptr=NULL;
        }
}

char* oph_bit_operator(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
        int res = 0;
	oph_bit_param_operator* param;
	oph_bit_string* measureA;
	oph_bit_string* measureB;

	if(!initid->ptr)
	{
		initid->ptr=(char*)malloc(sizeof(oph_bit_param_operator));
		if(!initid->ptr)
		{
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_bit_param_operator\n");
			*length=0;
        	        *is_null=0;
                	*error=1;
	                return NULL;
        	}
		param = (oph_bit_param_operator*)initid->ptr;
		param->measureA = NULL;
		param->measureB = NULL;
		param->result = NULL;
	}
	else param = (oph_bit_param_operator*)initid->ptr;

	if (!param->measureA || !param->measureB)
	{
		measureA = param->measureA = (oph_bit_string*)malloc(sizeof(oph_bit_string));
		if(!param->measureA)
		{
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_bit_string\n");
			*length=0;
        	        *is_null=0;
                	*error=1;
	                return NULL;
        	}
		measureB = param->measureB = (oph_bit_string*)malloc(sizeof(oph_bit_string));
		if(!param->measureB)
		{
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_bit_string\n");
			*length=0;
        	        *is_null=0;
                	*error=1;
	                return NULL;
        	}
		if (args->arg_count < 5)
			res = core_oph_bit_set_op(&param->op, NULL, 0); // Const
		else
			res = core_oph_bit_set_op(&param->op, args->args[4], &(args->lengths[4])); // Const
		if(res)
		{
			*length=0;
        	        *is_null=0;
                	*error=1;
	                return NULL;
        	}

		measureA->numelem = core_oph_bit_numelem(args->lengths[2]); // Const
		measureB->numelem = core_oph_bit_numelem(args->lengths[3]); // Const
		
		param->length = args->lengths[2] >= args->lengths[3] ? args->lengths[2] : args->lengths[3]; // bytes
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
			pmesg(1, __FILE__, __LINE__, "Error allocating result\n");
			*length=0;
        	        *is_null=0;
                	*error=1;
	                return NULL;
        	}
	}
	else
	{
		measureA = param->measureA;
		measureB = param->measureB;
	}

        measureA->content = args->args[2]; // Input A
	measureB->content = args->args[3]; // Input B
	if (!measureA->content || !measureB->content)
	{
                *length=0;
                *is_null=1;
                *error=0;
                return NULL;
        }

        res = core_oph_bit_operator(param);
        if(res)
	{
                pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
                *length=0;
                *is_null=1;
                *error=1;
                return NULL;
        }
	*length=param->length;
        *error=0;
        *is_null=0;
        return (result=param->result);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
