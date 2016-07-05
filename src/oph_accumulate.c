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

#include "oph_accumulate.h"

int msglevel = 1;

int core_oph_accumulate_multi(oph_multistring* byte_array, oph_multistring* result, int id)
{
        int i,j,js,je;

	if (id<=0) { js=0; je=byte_array->num_measure; }
	else if (id>byte_array->num_measure)
	{
		pmesg(1, __FILE__, __LINE__, "Index out of boundaries\n");
		return -1;
	}
	else { js=id-1; je=id; }

	for (j=js;j<je;++j)
	{
		switch(byte_array->type[j])
		{
		        case OPH_DOUBLE:{
				double tmp = 0;
		                for (i = 0; i < byte_array->numelem; i++){
		                        tmp += *(double*)((byte_array->content)+(i*byte_array->elemsize[j]));
					if(core_oph_type_cast((void*)(&tmp), (result->content)+(i*result->elemsize[j]), byte_array->type[j], result->type[j])) return -1;
				}
				break;
			}
		        case OPH_FLOAT:{
				float tmp = 0;
		                for (i = 0; i < byte_array->numelem; i++){
		                        tmp += *(float*)((byte_array->content)+(i*byte_array->elemsize[j]));
					if(core_oph_type_cast((void*)(&tmp), (result->content)+(i*result->elemsize[j]), byte_array->type[j], result->type[j])) return -1;
				}
				break;
			}
		        case OPH_INT:{
				int tmp = 0;
		                for (i = 0; i < byte_array->numelem; i++){
		                        tmp += *(int*)((byte_array->content)+(i*byte_array->elemsize[j]));
					if(core_oph_type_cast((void*)(&tmp), (result->content)+(i*result->elemsize[j]), byte_array->type[j], result->type[j])) return -1;
				}
				break;
			}
		        case OPH_LONG:{
				long long tmp = 0;
		                for (i = 0; i < byte_array->numelem; i++){
		                        tmp += *(long long*)((byte_array->content)+(i*byte_array->elemsize[j]));
					if(core_oph_type_cast((void*)(&tmp), (result->content)+(i*result->elemsize[j]), byte_array->type[j], result->type[j])) return -1;
				}
				break;
			}
			case OPH_SHORT:{
				short tmp = 0;
		                for (i = 0; i < byte_array->numelem; i++){
		                        tmp += *(short*)((byte_array->content)+(i*byte_array->elemsize[j]));
					if(core_oph_type_cast((void*)(&tmp), (result->content)+(i*result->elemsize[j]), byte_array->type[j], result->type[j])) return -1;
				}
				break;
			}
			case OPH_BYTE:{
				char tmp = 0;
		                for (i = 0; i < byte_array->numelem; i++){
		                        tmp += *(char*)((byte_array->content)+(i*byte_array->elemsize[j]));
					if(core_oph_type_cast((void*)(&tmp), (result->content)+(i*result->elemsize[j]), byte_array->type[j], result->type[j])) return -1;
				}
				break;
			}
		        default:
		                pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
		                return -1;
		}
	}
        return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_accumulate_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
        int i = 0;
        if(args->arg_count < 3 || args->arg_count > 4){
                strcpy(message, "ERROR: Wrong arguments! oph_accumulate(input_OPH_TYPE, output_OPH_TYPE, measure, [id_measure])");
                return 1;
        }

	for(i = 0; i < 3; i++){
		if(args->arg_type[i] != STRING_RESULT){
        		strcpy(message, "ERROR: Wrong arguments to oph_accumulate function");
        		return 1;
		}
	}

	if (args->arg_count>3)
	{
		if(args->arg_type[3] == STRING_RESULT){
			strcpy(message, "ERROR: Wrong arguments to oph_accumulate function");
			return 1;
		}
		args->arg_type[3] = INT_RESULT;
	}

	initid->ptr = NULL;
	initid->extension = NULL;
	return 0;
}

void oph_accumulate_deinit(UDF_INIT *initid)
{
        //Free allocated space
        oph_multistring* multimeasure;
        if(initid->ptr){
                multimeasure=(oph_multistring*)(initid->ptr);
                free(multimeasure->content);
                multimeasure->content=NULL;
                free_oph_multistring(multimeasure);
                multimeasure=NULL;
                initid->ptr = NULL;
        }
        if(initid->extension){
                multimeasure=(oph_multistring*)(initid->extension);
                free_oph_multistring(multimeasure);
                multimeasure=NULL;
                initid->extension = NULL;
        }
}

char* oph_accumulate(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
	oph_multistring *multim;
        oph_multistring *output;

	int id = 0;

        if(!initid->extension){
                if(core_set_oph_multistring( (oph_multistring**)(&(initid->extension)), args->args[0], &(args->lengths[0]))){
                        pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
                        *length=0;
                        *is_null=0;
                        *error=1;
                        return NULL;
                }
        }

        if(!initid->ptr){
                if(core_set_oph_multistring( ((oph_multistring**)(&(initid->ptr))), args->args[1], &(args->lengths[1]))){
                        pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
                        *length=0;
                        *is_null=0;
                        *error=1;
                        return NULL;
                }
                //Check
                if( ((oph_multistring*)(initid->ptr))->num_measure != ((oph_multistring*)(initid->extension))->num_measure){
                	pmesg(1, __FILE__, __LINE__, "Wrong input or output type\n");
                	*length=0;
                     	*is_null=0;
                	*error=1;
                	return NULL;
                }
	}
        multim = (oph_multistring*)(initid->extension);
        multim->content = args->args[2];
        multim->length = args->lengths[2];

        //Check
        if(multim->length%multim->blocksize){
        	pmesg(1,  __FILE__, __LINE__, "Wrong input type or data corrupted\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
	}
        multim->numelem = multim->length/multim->blocksize;
        output = (oph_multistring*)(initid->ptr);
	output->numelem = multim->numelem;

	if (args->arg_count>3)
	{
		id = *((long long*)args->args[3]);
		if ((id < 0) || (id > ((oph_multistring*)(initid->ptr))->num_measure))
		{
			pmesg(1,  __FILE__, __LINE__, "Wrong measure identifier. Correct values are integers in [1, %d].\n",((oph_multistring*)(initid->ptr))->num_measure);
		        *length=0;
		        *is_null=0;
		        *error=1;
		        return NULL;
		}
	}

	/* Allocate the right space for the result set */
        if(!output->content){
                output->content=(char *)calloc(1,(output->numelem)*(output->blocksize));
                if(!output->content){
                        pmesg(1,  __FILE__, __LINE__, "Error allocating measures string\n");
                        *length=0;
                        *is_null=0;
                        *error=1;
                        return NULL;
                }
        }

        if(core_oph_accumulate_multi(multim, output, id)){
        	pmesg(1,  __FILE__, __LINE__, "Unable to compute result\n");
                *length=0;
                *is_null=1;
                *error=1;
                return NULL;
        }
	*length=(output->numelem)*(output->blocksize);
        *error=0;
        *is_null=0;
        return (result = ((oph_multistring*)initid->ptr)->content);

}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/

