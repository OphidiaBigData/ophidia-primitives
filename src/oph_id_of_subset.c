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

#include "oph_id_of_subset.h"

int msglevel = 1;

int core_oph_id_of_subset_set_flags(oph_subset** subset, long long* flags, unsigned long size, unsigned long* total, oph_id_of_subset_block* blocks, int number)
{
	if (!subset || !flags || !size || !total || !blocks)
	{
		pmesg(1, __FILE__, __LINE__, "Null pointer\n");
		return OPH_SUBSET_NULL_POINTER_ERR;
	}

	unsigned long i, j, k;
	*total=0;
	for (j=0; j<size; ++j)
	{
		for (k=0; k<number; ++k)
		{
			for (i=0; i<subset[k]->number; ++i)
				if (oph_subset_is_in_subset(oph_subset_id_to_index2(j, blocks[k].block_size, blocks[k].max), subset[k]->start[i], subset[k]->stride[i], subset[k]->end[i])) break;
			if (!(flags[j] = i<subset[k]->number)) break; // Element skipped
		}
		if (flags[j]) flags[j] = ++(*total);
	}
	return OPH_SUBSET_OK;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_id_of_subset_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	int i;
        if(args->arg_count < 2)
	{
                strcpy(message, "ERROR: Wrong arguments! oph_id_of_subset(id, total_size, [subset_string1, blocksize1, size1], [subset_string2, blocksize2, size2], ...)");
                return 1;
        }
        
        if(args->arg_type[0] != INT_RESULT)
	{
        	strcpy(message, "ERROR: Wrong arguments to oph_id_of_subset function");
        	return 1;
        }

        if(args->arg_type[1] != INT_RESULT)
	{
        	strcpy(message, "ERROR: Wrong arguments to oph_id_of_subset function");
        	return 1;
        }

	if(args->arg_count > 2)
	{
		if ((args->arg_count-2)%3)
		{
			strcpy(message, "ERROR: Wrong arguments to oph_id_of_subset function");
			return 1;
		}
		for (i=2; i<args->arg_count; )
		{
			if(args->arg_type[i] != STRING_RESULT)
			{
				strcpy(message, "ERROR: Wrong arguments to oph_id_of_subset function");
				return 1;
			}
			i++;
			if(args->arg_type[i] != INT_RESULT)
			{
				strcpy(message, "ERROR: Wrong arguments to oph_id_of_subset function");
				return 1;
			}
			i++;
			if(args->arg_type[i] != INT_RESULT)
			{
				strcpy(message, "ERROR: Wrong arguments to oph_id_of_subset function");
				return 1;
			}
			i++;
		}
	}

	initid->ptr = NULL;

	return 0;
}

void oph_id_of_subset_deinit(UDF_INIT *initid)
{
        //Free allocated space
        if(initid->ptr)
	{
		oph_id_of_subset_param* param = (oph_id_of_subset_param*)initid->ptr;
		if (param->subset) { oph_subset_vector_free(param->subset,param->number); param->subset=NULL; param->number=0; }
		if (param->flags) { free(param->flags); param->flags=NULL; }
		free(initid->ptr);
		initid->ptr=NULL;
        }
}

long long oph_id_of_subset(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
        int i;
	oph_id_of_subset_param* param;
	unsigned long total;
	oph_id_of_subset_block* blocks = 0;
	long long value;

	if (*error)
	{
	        *is_null=0;
	        *error=1;
	        return 0;
	}
	if (*is_null || !args->args[0] || !args->lengths[0] || !args->args[1] || !args->lengths[1])
	{
	        *is_null=1;
	        *error=0;
	        return 0;
	}

	if(args->arg_count < 3)
	{
	        *is_null=0;
	        *error=0;
		long long value = *((long long*)args->args[0]);
		return value <= *((long long*)args->args[1]) ? value : 0;
	}

	if(!initid->ptr)
	{
		initid->ptr=(char*)malloc(sizeof(oph_id_of_subset_param));
		if(!initid->ptr)
		{
			pmesg(1, __FILE__, __LINE__, "Error allocating oph_id_of_subset_param\n");
			param->error = 1;
        	        *is_null=0;
                	*error=1;
	                return 0;
        	}
		param = (oph_id_of_subset_param*)initid->ptr;
		param->subset = NULL;
		param->number = 0;
		param->flags = NULL;
		param->id = -1;
		param->error = 0;
	}
	else param = (oph_id_of_subset_param*)initid->ptr;

	if (param->error)
	{
	        *is_null=0;
        	*error=1;
                return 0;
	}

	if (param->id<0)
	{
		param->id = *((long long*)args->args[0]);
		param->numelem = *((long long*)args->args[1]);

		param->number = (args->arg_count-2)/3;
		param->subset = (oph_subset**)malloc(param->number*sizeof(oph_subset*));
		blocks = (oph_id_of_subset_block*)malloc(param->number*sizeof(oph_id_of_subset_block));
		for (i=0; i<param->number; ++i)
		{
			blocks[i].subset_string = args->args[i*3+2];
			blocks[i].subset_strings_length = args->lengths[i*3+2];
			blocks[i].block_size = *((long long*)args->args[i*3+3]);
			blocks[i].max = *((long long*)args->args[i*3+4]);
		}

		value = 1;
		for (i=0;i<param->number;++i)
		{
			// Set subsetting structures
			if (oph_subset_init(&(param->subset[i])))
			{
				pmesg(1,  __FILE__, __LINE__, "Error in oph_subset initialization\n");
				param->error = 1;
				*is_null=0;
				*error=1;
				return 0;
			}
			if (oph_subset_parse(blocks[i].subset_string ? blocks[i].subset_string : OPH_SUBSET_ALL, blocks[i].subset_strings_length ? blocks[i].subset_strings_length : strlen(OPH_SUBSET_ALL), param->subset[i], blocks[i].max))
			{
				pmesg(1,  __FILE__, __LINE__, "Error in parsing subset_string\n");
				param->error = 1;
				*is_null=0;
				*error=1;
				return 0;
			}
			value *= blocks[i].max;
		}
		if (value > param->numelem)
		{
			pmesg(1,  __FILE__, __LINE__, "Wrong parameter 'total_size'\n");
			param->error = 1;
			*is_null=0;
			*error=1;
			return 0;
		}

		// New implementation with flags
		param->flags = malloc(param->numelem*sizeof(long long));
		if(!param->flags)
		{
			pmesg(1, __FILE__, __LINE__, "Error allocating flag array\n");
			param->error = 1;
        	        *is_null=0;
                	*error=1;
	                return 0;
        	}
		if (core_oph_id_of_subset_set_flags(param->subset, param->flags, param->numelem, &total, blocks, param->number))
		{
			pmesg(1,  __FILE__, __LINE__, "Error in setting flag array\n");
			param->error = 1;
		        *is_null=0;
		        *error=1;
		        return 0;
		}
		if (blocks) free(blocks);
	}
	else param->id = *((long long*)args->args[0]);

	*is_null=0;
        *error=0;
	return param->flags[param->id-1]; // Non 'C'-like indexing
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/

