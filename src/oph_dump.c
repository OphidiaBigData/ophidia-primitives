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

#include "oph_dump.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_dump_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
        int i = 0;
        if((args->arg_count < 3) || (args->arg_count > 4)) {
                strcpy(message, "ERROR: Wrong arguments! oph_dump(input_OPH_TYPE, output_OPH_TYPE, measure, [encoding])");
                return 1;
        }
        
        for(i = 0; i < args->arg_count; i++){
                if(args->arg_type[i] != STRING_RESULT){
                        strcpy(message, "ERROR: Wrong arguments to oph_dump function");
                        return 1;
                }
        }

	initid->ptr = NULL;
	return 0;
}

void oph_dump_deinit(UDF_INIT *initid)
{
        //Free allocated space
        if(initid->ptr){
                free(initid->ptr);
                initid->ptr=NULL;
        }
}

char* oph_dump(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
        oph_string measure;

        int res = 0;

	if (*error)
	{
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
	}
	if (*is_null)
	{
                *length=0;
                *is_null=1;
                *error=0;
                return NULL;
	}

        core_set_type(&(measure), args->args[0], &(args->lengths[0]));

        measure.content = args->args[2];
	if (!measure.content)
	{
                *length=0;
                *is_null=1;
                *error=0;
                return NULL;
        }

        measure.length = &(args->lengths[2]);

        if(core_set_elemsize(&(measure))){
                pmesg(1,  __FILE__, __LINE__, "Error on setting elements size\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
        }

        if(core_set_numelem(&(measure))){
                pmesg(1,  __FILE__, __LINE__, "Error on counting elements\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
        }


	if(!initid->ptr){
		unsigned int space = INT_DIGIT+PRECISION+1;
		switch(measure.type){
			case OPH_COMPLEX_INT:
			case OPH_COMPLEX_LONG:
			case OPH_COMPLEX_FLOAT:
			case OPH_COMPLEX_DOUBLE:
				space = 2*space+6;
			break;
			default:;
		}
		initid->ptr=(char *)malloc(measure.numelem*space + 2*(measure.numelem - 1)); 
		if(!initid->ptr){
			pmesg(1,  __FILE__, __LINE__, "Error allocating measures string\n");
			*length=0;
        	        *is_null=1;
                	*error=1;
	                return NULL;
        	}
	}

	int encoding = 0;
	if (args->arg_count > 3)
	{
		if ((args->lengths[3] == 7) && !strncasecmp(args->args[3],"decimal",args->lengths[3])) encoding = 0;
		else if ((args->lengths[3] == 6) && !strncasecmp(args->args[3],"base64",args->lengths[3])) encoding = 1;
		else
		{
			pmesg(1,  __FILE__, __LINE__, "Unknown encoding type\n");
		        *length=0;
		        *is_null=0;
		        *error=1;
		        return NULL;
		}
	}

        res = core_oph_dump(&measure, initid->ptr, encoding);
        if(res){
                pmesg(1,  __FILE__, __LINE__, "Unable to convert supplied measure\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
        }
        *length=strlen(initid->ptr);
        *error=0;
        *is_null=0;
        return initid->ptr;

}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/

