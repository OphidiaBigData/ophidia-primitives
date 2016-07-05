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

#include "oph_compress.h"

int msglevel = 1;

uLongf core_compressed_length (const unsigned long src_length){
        return src_length + 12 + ceil((double) (src_length/5));
}

int core_oph_compress(const unsigned char *src, const unsigned long src_length, unsigned char **dest, unsigned long *dest_length, int compression_level)
{
    if(!src){
        pmesg(1, __FILE__, __LINE__, "Input buffer is empty\n");
        return -1;
    }
    if( (compress2((Bytef *)(*dest), (uLongf *)dest_length, (const Bytef *) src, (uLong) src_length, compression_level)) != Z_OK){
        pmesg(1, __FILE__, __LINE__, "Error during compression phase\n");
        return -1;
    }
    return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_compress_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
        if(args->arg_count != 3 ){
                strcpy(message, "ERROR: Wrong arguments! oph_compress(input_OPH_TYPE, output_OPH_TYPE, measure)");
                return 1;
        }
        
        if(args->arg_type[2] != STRING_RESULT){
                strcpy(message, "ERROR: Wrong arguments to oph_compress function");
                return 1;
        }

	initid->ptr = NULL;

	return 0;
}

void oph_compress_deinit(UDF_INIT *initid)
{
        //Free allocated space
	if(initid->ptr){
	        free(initid->ptr);
        	initid->ptr=NULL;
	}

}

char* oph_compress(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{

	unsigned long sizeof_var = 0;
	sizeof_var = args->lengths[2];
        uLongf cmpr_length = core_compressed_length(sizeof_var);
	uLongf cmpr_size = 0;
	uLongf buffer_length = cmpr_length + 5;
	uLongf buffer_size = 0;
	Bytef *body_buffer = NULL;
	Bytef *cmpr_buffer = NULL;
	int rc;

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

        if(!initid->ptr){
		initid->ptr = (Bytef*)calloc(buffer_length, sizeof(unsigned char));
		if(!initid->ptr){
                        pmesg(1,  __FILE__, __LINE__, "Error allocating compressed string\n");
                        *length=0;
                        *is_null=1;
                        *error=1;
                        return NULL;
                }
	}
	cmpr_buffer = (Bytef*) (initid->ptr);
	body_buffer = cmpr_buffer + COMPRESS_OFFSET;
        *((unsigned long*) (cmpr_buffer))= (unsigned long) (sizeof_var & 0x3FFFFFFF);
	cmpr_size = cmpr_length;
        memset((Bytef*)body_buffer, 0, (uLongf)(cmpr_size+1));
        //Compress array
        rc = compress2((Bytef*)body_buffer, (uLong*)&cmpr_size, (const Bytef*)args->args[2], (uLong)sizeof_var, Z_DEFAULT_COMPRESSION);
	if(rc != Z_OK){
                pmesg(1,  __FILE__, __LINE__, "Error on compressing string\n");
                *length=0;
		*is_null=1;
                *error=1;
		cmpr_buffer = NULL;
		return NULL;
        }
	buffer_size = cmpr_size + COMPRESS_OFFSET;

        *length = (unsigned long) buffer_size;
	cmpr_buffer = NULL;
        return initid->ptr;
                
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/

