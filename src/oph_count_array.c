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

#include "oph_count_array.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_count_array_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	if((args->arg_count < 3) || (args->arg_count > 5)){
		strcpy(message, "ERROR! Wrong arguments! oph_count_array(input_OPH_TYPE, output_OPH_TYPE, measure, [include_missing_values], [missingvalue])");
		return 1;
	}

	int i;
        for(i = 0; i < 3; i++){
                if(args->arg_type[i] != STRING_RESULT){
                        strcpy(message, "ERROR: Wrong arguments to oph_count_array function");
                        return 1;
                }
        }
	if (args->arg_count > 3)
	{
                if(args->arg_type[3] != INT_RESULT){
                        strcpy(message, "ERROR: Wrong arguments to oph_count_array function");
                        return 1;
                }
		if (args->arg_count > 4)
		{
                        if (args->args[4] && (args->arg_type[4] == STRING_RESULT)){
                                strcpy(message, "ERROR: Wrong argument 'missingvalue' to oph_count_array function");
                                return 1;
                        }
			args->arg_type[4] = REAL_RESULT;
                }
	}

	initid->ptr = NULL;

	return 0;
}

void oph_count_array_deinit(UDF_INIT *initid __attribute__((unused)))
{
	if(initid->ptr){
		oph_multistring* multimeasure=(oph_multistring*)(initid->ptr);
		free_oph_multistring(multimeasure);
		multimeasure=NULL;
		initid->ptr = NULL;
	}
}

long long oph_count_array(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
	long long count = 0;

	if (*error)
	{
	        *is_null=0;
	        *error=1;
	        return 0;
	}
	if (*is_null || !args->lengths[2])
	{
	        *is_null=1;
	        *error=0;
	        return 0;
	}

	oph_multistring* measure;
	if(!initid->ptr)
	{
		if(core_set_oph_multistring( ((oph_multistring**)(&(initid->ptr))), args->args[0], &(args->lengths[0])))
		{
 			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*is_null=0;
			*error=1;
			return 0;
		}
		measure = (oph_multistring*)initid->ptr;
		if (!measure->islast)
		{
			pmesg(1, __FILE__, __LINE__, "Wrong number of input measure\n");
			*is_null=0;
			*error=1;
			return 0;
		}
		measure->length = args->lengths[2];
		if(measure->length % measure->blocksize)
		{
			pmesg(1, __FILE__, __LINE__, "Wrong input type or data corrupted\n");
			*is_null=0;
			*error=1;
			return 0;
		}
		measure->numelem = measure->length / measure->blocksize;
	}
	else measure = (oph_multistring*)initid->ptr;

        measure->content = args->args[2];

	int include = 1;
	if (args->arg_count > 3) include = (long long)(args->args[3]);

	double *missingvalue = NULL;
	if (args->arg_count > 4) missingvalue = (double*)(args->args[4]);

	if (include) count = measure->numelem;
	else
	{
		int i,j,stop;
		size_t elemsize=0, offset;
		char *item;
		for (i=0; i<measure->numelem; ++i, elemsize += measure->blocksize)
		{
			stop = offset = 0;
			for (j=0;j<measure->num_measure;++j)
			{
				item = measure->content + elemsize + offset;
				switch(measure->type[j])
				{
					case OPH_DOUBLE:
						if (!isnan(*((double*)item))) stop = 1;
						else if (missingvalue && (*((double*)item) == (double)*missingvalue)) stop = 1;
						break;
					case OPH_FLOAT:
						if (!isnan(*((float*)item))) stop = 1;
						else if (missingvalue && (*((float*)item) == (float)*missingvalue)) stop = 1;
						break;
					case OPH_LONG:
						if (missingvalue && (*((long long*)item) == (long long)*missingvalue)) stop = 1;
						break;
					case OPH_INT:
						if (missingvalue && (*((int*)item) == (int)*missingvalue)) stop = 1;
						break;
					case OPH_SHORT:
						if (missingvalue && (*((short*)item) == (short)*missingvalue)) stop = 1;
						break;
					case OPH_BYTE:
						if (missingvalue && (*((char*)item) == (char)*missingvalue)) stop = 1;
						break;
					default:
						pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
						*is_null=0;
						*error=1;
						return 0;
				}
				if (stop) break;
				offset += measure->elemsize[j];
			}
			if (!stop) count++;
		}
	}

	*is_null=0;
	*error=0;

        return count;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/

