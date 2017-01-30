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

#include "oph_extract.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_extract_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
        int i = 0;
        if(args->arg_count != 4){
                strcpy(message, "ERROR: Wrong arguments! oph_extract(input_OPH_TYPE, output_OPH_TYPE, measure, index_list)");
                return 1;
        }
        
        for(i = 0; i < args->arg_count; i++){
                if(args->arg_type[i] != STRING_RESULT){
                        strcpy(message, "ERROR: Wrong arguments to oph_extract function");
                        return 1;
                }
        }

	initid->ptr = NULL;

	return 0;
}

void oph_extract_deinit(UDF_INIT *initid)
{
        //Free allocated space
        if(initid->ptr)
	{
		oph_extract_param* param = (oph_extract_param*)initid->ptr;
		if (param->measure) { free(param->measure); param->measure=NULL; }
		if (param->index_list) { free(param->index_list); param->index_list=NULL; }
		if (param->result) { free(param->result); param->result=NULL; }
                free(initid->ptr);
                initid->ptr=NULL;
        }
}

char* oph_extract(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
        int res = 0;
	oph_extract_param* param;
	oph_string *measure, *index_list;

	if (*error)
	{
	        *length=0;
	        *is_null=0;
	        *error=1;
	        return NULL;
	}
	if (*is_null || !args->lengths[2] || !args->lengths[3])
	{
	        *length=0;
	        *is_null=1;
	        *error=0;
	        return NULL;
	}

	if(!initid->ptr)
	{
		initid->ptr=(char*)malloc(sizeof(oph_extract_param));
		if(!initid->ptr)
		{
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_extract_param\n");
			param->error = 1;
			*length=0;
        	        *is_null=0;
                	*error=1;
	                return NULL;
        	}
		param = (oph_extract_param*)initid->ptr;
		param->measure = NULL;
		param->index_list = NULL;
		param->result = NULL;
		param->error = 0;
	}
	else param = (oph_extract_param*)initid->ptr;

	if (!param->error && !param->measure && !param->index_list)
	{
		// Measure
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
		measure->length = &(args->lengths[2]);
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
		if (core_set_elemsize(measure))
		{
			pmesg(1,  __FILE__, __LINE__, "Error on setting data element size\n");
			param->error = 1;
		        *length=0;
		        *is_null=0;
		        *error=1;
		        return NULL;
		}
		if(core_set_numelem(measure))
		{
			pmesg(1,  __FILE__, __LINE__, "Error on counting elements\n");
			param->error = 1;
		        *length=0;
		        *is_null=0;
		        *error=1;
		        return NULL;
		}

		// Index list
		param->index_list = malloc(sizeof(oph_string));
		if(!param->index_list)
		{
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_string\n");
			param->error = 1;
			*length=0;
        	        *is_null=0;
                	*error=1;
	                return NULL;
        	}
		index_list = (oph_string*)param->index_list;
		index_list->length = &(args->lengths[3]);
		char long_type[10];
		sprintf(long_type,"oph_long");
		unsigned long len = strlen(long_type); 
		res = core_set_type(index_list, long_type, &len); // Index are always oph_long, i.e. long long
		if (res)
		{
			pmesg(1,  __FILE__, __LINE__, "Error on setting the data type\n");
			param->error = 1;
		        *length=0;
		        *is_null=0;
		        *error=1;
		        return NULL;
		}
		if (core_set_elemsize(index_list))
		{
			pmesg(1,  __FILE__, __LINE__, "Error on setting data element size\n");
			param->error = 1;
		        *length=0;
		        *is_null=0;
		        *error=1;
		        return NULL;
		}
		if(core_set_numelem(index_list))
		{
			pmesg(1,  __FILE__, __LINE__, "Error on counting elements\n");
			param->error = 1;
		        *length=0;
		        *is_null=0;
		        *error=1;
		        return NULL;
		}

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

		// Output
		param->length = output_array.elemsize * index_list->numelem; // bytes
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
	else
	{
		measure = (oph_string*)param->measure;
		index_list = (oph_string*)param->index_list;
	}

	// Set measure->content
        measure->content = args->args[2]; // Input
	if (!measure->content)
	{
                *length=0;
                *is_null=1;
                *error=0;
                return NULL;
        }
	// Set index_list->content
	index_list->content = args->args[3]; // Input
	if (!index_list->content)
	{
	        *length=0;
	        *is_null=1;
	        *error=0;
	        return NULL;
	}

	long long i, index;
	char temporary[measure->elemsize];
	for (i=0; i<index_list->numelem; ++i)
	{
		index = *((long long*)(index_list->content+i*index_list->elemsize))-1; // Non 'C'-like indexing
		memcpy(temporary, measure->content+index*measure->elemsize, measure->elemsize);
		if(core_oph_type_cast(temporary, param->result+i*param->result_elemsize, measure->type, param->result_type, NULL))
		{
                        pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
                        *length=0;
                        *is_null=0;
                        *error=1;
			return NULL;
                }
	}

	*length=param->length;
	*is_null=0;
        *error=0;
	return (result=param->result);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/

