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

#include "oph_mask.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_mask_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
        if(args->arg_count < 3 || args->arg_count > 5)
	{
                strcpy(message, "ERROR: Wrong arguments! oph_mask(input_OPH_TYPE, output_OPH_TYPE, measure, [bit_mask], [filling])");
                return 1;
        }

	int i;
        for(i = 0; i < 3; i++){
                if(args->arg_type[i] != STRING_RESULT){
                        strcpy(message, "ERROR: Wrong arguments to oph_mask function");
                        return 1;
                }
        }

	if(args->arg_count > 3)
	{
		if(args->arg_type[3] != STRING_RESULT)
		{
			strcpy(message, "ERROR: Wrong arguments to oph_mask function");
			return 1;
		}
		if(args->arg_count > 4) args->arg_type[4] = REAL_RESULT;
	}

	initid->ptr = NULL;

	return 0;
}

void oph_mask_deinit(UDF_INIT *initid)
{
        //Free allocated space
        if(initid->ptr)
	{
		oph_mask_param* param = (oph_mask_param*)initid->ptr;
		if (param->measure) { free(param->measure); param->measure=NULL; }
		if (param->mask) { free(param->mask); param->mask=NULL; }
		if (param->result) { free(param->result); param->result=NULL; }
                free(initid->ptr);
                initid->ptr=NULL;
        }
}

char* oph_mask(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
        int res = 0;
	oph_mask_param* param;
	oph_string* measure;
	oph_bit_string* mask;

	if (*error)
	{
	        *length=0;
	        *is_null=0;
	        *error=1;
	        return NULL;
	}
	if (*is_null || !args->lengths[2])
	{
	        *length=0;
	        *is_null=1;
	        *error=0;
	        return NULL;
	}

	if(!initid->ptr)
	{
		initid->ptr=(char*)malloc(sizeof(oph_mask_param));
		if(!initid->ptr)
		{
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_mask_param\n");
			param->error = 1;
			*length=0;
        	        *is_null=0;
                	*error=1;
	                return NULL;
        	}
		param = (oph_mask_param*)initid->ptr;
		param->measure = NULL;
		param->mask = NULL;
		param->result = NULL;
		param->error = 0;

		// Filling
		if (args->arg_count > 4)
		{
			if (args->args[4]) param->filling = *((double*)args->args[4]);
			else param->filling = nan("");
		}
		else param->filling = 0.0;

		initid->ptr = (char*)param;
	}
	else param = (oph_mask_param*)initid->ptr;

	if (!param->error && !param->measure)
	{
		// Input
		param->measure = malloc(sizeof(oph_string));
		if(!param->measure)
		{
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_string\n");
			param->error = 1;
			*length=0;
        	        *is_null=0;
                	*error=1;
	                return NULL;
        	}
		measure = (oph_string*)param->measure;

		// Set measure->length
		measure->length = &(args->lengths[2]);

		// Set measure->type
		res = core_set_type(measure, args->args[0], &(args->lengths[0]));
		if (res)
		{
			pmesg(1,  __FILE__, __LINE__, "Error on setting the data type\n");
			param->error = 1;
		        *length=0;
		        *is_null=0;
		        *error=1;
		        return NULL;
		}

		// Set measure->elemsize
		if (core_set_elemsize(measure))
		{
			pmesg(1,  __FILE__, __LINE__, "Error on setting data element size\n");
			param->error = 1;
		        *length=0;
		        *is_null=0;
		        *error=1;
		        return NULL;
		}

		// Set measure->numelem
		if(core_set_numelem(measure))
		{
			pmesg(1,  __FILE__, __LINE__, "Error on counting elements\n");
			param->error = 1;
		        *length=0;
		        *is_null=0;
		        *error=1;
		        return NULL;
		}

		// Output
		oph_string output_array;
		core_set_type(&output_array, args->args[1], &(args->lengths[1]));
		if(!output_array.type){
		        pmesg(1,  __FILE__, __LINE__, "Unable to recognize measures type\n");
			param->error = 1;
		        *length=0;
		        *is_null=0;
		        *error=1;
		        return NULL;
		}
		if(core_set_elemsize(&output_array)){
		        pmesg(1,  __FILE__, __LINE__, "Unable to recognize measures type\n");
			param->error = 1;
		        *length=0;
		        *is_null=0;
		        *error=1;
		        return NULL;
		}
		param->result_type = output_array.type;
		param->result_elemsize = output_array.elemsize;

		param->length = output_array.elemsize * measure->numelem; // bytes
		if (!param->length)
		{
			*length=0;
        	        *is_null=1;
                	*error=0;
	                return NULL;
        	}
		param->result = (char*)malloc(param->length);
		if(!param->result)
		{
			pmesg(1, __FILE__, __LINE__, "Error allocating result\n");
			param->error = 1;
			*length=0;
        	        *is_null=0;
                	*error=1;
	                return NULL;
        	}
	}
	else measure = (oph_string*)param->measure;

	if (!param->error && !param->mask)
	{
		param->mask = malloc(sizeof(oph_bit_string));
		if(!param->mask)
		{
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_bit_string\n");
			param->error = 1;
			*length=0;
		        *is_null=0;
	        	*error=1;
		        return NULL;
		}
	
		mask = (oph_bit_string*)param->mask;

		if (args->arg_count > 3)
			mask->numelem = core_oph_bit_numelem(args->lengths[3]); // Const
		else
			mask->numelem = 0;
	}
	else mask = (oph_bit_string*)param->mask;

	// Set measure->content
        measure->content = args->args[2]; // Input
	if (!measure->content)
	{
                *length=0;
                *is_null=1;
                *error=0;
                return NULL;
        }

	if (args->arg_count > 3)
	{
		mask->content = args->args[3]; // Input
		if (!mask->content)
		{
		        *length=0;
		        *is_null=1;
		        *error=0;
		        return NULL;
		}
	}
	else mask->content=NULL;

        res = core_oph_mask(param);
        if(res)
	{
                pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
        }

	*length=param->length;
	*is_null=0;
        *error=0;
	return (result=param->result);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/

