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

#include "oph_cast.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_cast_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    if((args->arg_count < 3) && (args->arg_count > 5)){
            strcpy(message, "ERROR: Wrong arguments! oph_cast(input_OPH_TYPE, output_OPH_TYPE, measure, [missingvalue], [output_missingvalue])");
            return 1;
    }

	int i;
        for(i = 0; i < args->arg_count; i++){
                if((i == 3) || (i == 4)) {
                        if (args->args[i] && (args->arg_type[i] == STRING_RESULT)){
                                strcpy(message, "ERROR: Wrong argument 'missingvalue' to oph_cast function");
                                return 1;
                        }
			args->arg_type[i] = REAL_RESULT;
                }
		else if(args->arg_type[i] != STRING_RESULT){
                        strcpy(message, "ERROR: Wrong arguments to oph_cast function");
                        return 1;
                }
        }

    initid->ptr = NULL;
    initid->extension = NULL;

    return 0;
}

void oph_cast_deinit(UDF_INIT *initid)
{
    //Free allocated space
    oph_stringPtr output = (oph_stringPtr) initid->ptr;
    oph_stringPtr input = (oph_stringPtr) initid->extension;
    if (input && output){
        if (output->type!=input->type) {
            if(output->content){
                free(output->content);
                output->content=NULL;
            }
        }
    }
    if(initid->ptr)
    {
        if(output->length){
           free(output->length);
           output->length=NULL;
        }
        free(initid->ptr);
        initid->ptr=NULL;
    }
    if(initid->extension)
    {
        if(input->length){
            free(input->length);
            input->length=NULL;
        }
        free(initid->extension);
        initid->extension=NULL;
    }
}

char* oph_cast(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
    if (!initid->ptr) {
        *error=0;
        *is_null=0;

        initid->ptr=(char *)calloc(1,sizeof(oph_string));
        if(!initid->ptr){
            pmesg(1,  __FILE__, __LINE__, "Error allocating result\n");
            *length=0;
            *is_null=1;
            *error=1;
            return NULL;
        }

        initid->extension=calloc(1,sizeof(oph_string));
        if(!initid->extension){
            pmesg(1,  __FILE__, __LINE__, "Error allocating extension\n");
            *length=0;
            *is_null=1;
            *error=1;
            return NULL;
        }

        oph_stringPtr input = (oph_stringPtr) initid->extension;
	input->content = NULL;
	input->length = NULL;
        core_set_type(input,args->args[0],&(args->lengths[0]));
        input->length = (unsigned long *)calloc(1,sizeof(unsigned long));
        if (!input->length) {
            pmesg(1,  __FILE__, __LINE__, "Error allocating length\n");
            *length=0;
            *is_null=1;
            *error=1;
            return NULL;
        }
        *(input->length) = args->lengths[2];

        oph_stringPtr output = (oph_stringPtr) initid->ptr;
	output->content = NULL;
	output->length = NULL;
        core_set_type(output,args->args[1],&(args->lengths[1]));
        output->length = (unsigned long *)calloc(1,sizeof(unsigned long));
        if (!output->length) {
            pmesg(1,  __FILE__, __LINE__, "Error allocating length\n");
            *length=0;
            *is_null=1;
            *error=1;
            return NULL;
        }

        if (input->type == output->type) {
            //nothing to do
            *(output->length) = *(input->length);
        } else {

            if(core_set_elemsize(input)){
                pmesg(1,  __FILE__, __LINE__, "Error on setting element size\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
            }

            if(core_set_numelem(input)){
                pmesg(1,  __FILE__, __LINE__, "Error on counting result elements\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
            }

            switch (input->type) {
            case OPH_BYTE:
            case OPH_SHORT:
            case OPH_INT:
            case OPH_LONG:
            case OPH_FLOAT:
            case OPH_DOUBLE:

                output->elemsize = core_sizeof(output->type);
                output->numelem = input->numelem;
                *(output->length) = output->numelem * output->elemsize;
                break;

            case OPH_COMPLEX_INT:
            case OPH_COMPLEX_LONG:
            case OPH_COMPLEX_FLOAT:
            case OPH_COMPLEX_DOUBLE:

                switch (output->type) {
                case OPH_BYTE:
                case OPH_SHORT:
                case OPH_INT:
                case OPH_LONG:
                case OPH_FLOAT:
                case OPH_DOUBLE:

                    pmesg(1,  __FILE__, __LINE__, "Cast not permitted\n");
                    *length=0;
                    *is_null=0;
                    *error=1;
                    return NULL;

                case OPH_COMPLEX_INT:
                case OPH_COMPLEX_LONG:
                case OPH_COMPLEX_FLOAT:
                case OPH_COMPLEX_DOUBLE:

                    output->elemsize = core_sizeof(output->type);
                    output->numelem = input->numelem;
                    *(output->length) = output->numelem * output->elemsize;
                    break;

                default:
                    pmesg(1,  __FILE__, __LINE__, "Type not recognized\n");
                    *length=0;
                    *is_null=0;
                    *error=1;
                    return NULL;
                }
                break;

            default:
                pmesg(1,  __FILE__, __LINE__, "Type not recognized\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
            }

            output->content = (char *)calloc(*(output->length),1);
            if(!output->content){
                pmesg(1,  __FILE__, __LINE__, "Error allocating result string\n");
                *length=0;
                *is_null=1;
                *error=1;
                return NULL;
            }
        }
    }

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

    oph_stringPtr output = (oph_stringPtr) initid->ptr;
    oph_stringPtr input = (oph_stringPtr) initid->extension;
    if (!input && !output)
    {
        *length=0;
        *is_null=0;
        *error=1;
        return NULL;
    }

    input->content = args->args[2];

	double *missingvalue = NULL;
	if((args->arg_count > 3) && args->args[3])
		missingvalue = (double*)(args->args[3]);

	double *output_missingvalue = missingvalue;
	if((args->arg_count > 4) && args->args[4])
		output_missingvalue = (double*)(args->args[4]);

    int i,j=0;
    if (input->type == output->type) {
        output->content = input->content;
    } else {

        switch (input->type) {
        case OPH_INT:
            switch (output->type) {
            case OPH_LONG: //int->long
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((int *)(input->content))[i] == (int)*missingvalue))
				((long long *)(output->content))[i] = (long long)*output_missingvalue;
			else ((long long *)(output->content))[i] = (long long) ((int *)(input->content))[i];
                }
                break;
            case OPH_SHORT: //int->short
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((int *)(input->content))[i] == (int)*missingvalue))
				((short *)(output->content))[i] = (short)*output_missingvalue;
			else ((short *)(output->content))[i] = (short) ((int *)(input->content))[i];
                }
                break;
            case OPH_BYTE: //int->byte
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((int *)(input->content))[i] == (int)*missingvalue))
				((char *)(output->content))[i] = (char)*output_missingvalue;
			else ((char *)(output->content))[i] = (char) ((int *)(input->content))[i];
                }
                break;
            case OPH_FLOAT: //int->float
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((int *)(input->content))[i] == (int)*missingvalue))
				((float *)(output->content))[i] = args->args[4] ? (float)*output_missingvalue : NAN;
			else ((float *)(output->content))[i] = (float) ((int *)(input->content))[i];
                }
                break;
            case OPH_DOUBLE: //int->double
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((int *)(input->content))[i] == (int)*missingvalue))
				((double *)(output->content))[i] = args->args[4] ? (double)*output_missingvalue : NAN;
			else ((double *)(output->content))[i] = (double) ((int *)(input->content))[i];
                }
                break;
            case OPH_COMPLEX_INT: //int->cint
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((int *)(input->content))[i] == (int)*missingvalue))
				((int *)(output->content))[j] = ((int *)(output->content))[j+1] = (int)*output_missingvalue;
			else {
				((int *)(output->content))[j] = ((int *)(input->content))[i];
				((int *)(output->content))[j+1] = 0;
			}
			j+=2;
                }
                break;
            case OPH_COMPLEX_LONG: //int->clong
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((int *)(input->content))[i] == (int)*missingvalue))
				((long long *)(output->content))[j] = ((long long *)(output->content))[j+1] = (long long)*output_missingvalue;
			else {
				((long long *)(output->content))[j] = (long long) ((int *)(input->content))[i];
				((long long *)(output->content))[j+1] = 0;
			}
			j+=2;
                }
                break;
            case OPH_COMPLEX_FLOAT: //int->cfloat
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((int *)(input->content))[i] == (int)*missingvalue))
				((float *)(output->content))[j] = ((float *)(output->content))[j+1] = args->args[4] ? (float)*output_missingvalue : NAN;
			else {
				((float *)(output->content))[j] = (float) ((int *)(input->content))[i];
				((float *)(output->content))[j+1] = 0;
			}
			j+=2;
                }
                break;
            case OPH_COMPLEX_DOUBLE: //int->cdouble
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((int *)(input->content))[i] == (int)*missingvalue))
				((double *)(output->content))[j] = ((double *)(output->content))[j+1] = args->args[4] ? (double)*output_missingvalue : NAN;
			else {
				((double *)(output->content))[j] = (double) ((int *)(input->content))[i];
				((double *)(output->content))[j+1] = 0;
			}
			j+=2;
                }
                break;
            default:
                pmesg(1,  __FILE__, __LINE__, "Type not recognized\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
            }
            break;
        case OPH_SHORT:
            switch (output->type) {
            case OPH_LONG: //short->long
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((short *)(input->content))[i] == (short)*missingvalue))
				((long long *)(output->content))[i] = (long long)*output_missingvalue;
			else ((long long *)(output->content))[i] = (long long) ((short *)(input->content))[i];
                }
                break;
            case OPH_INT: //short->int
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((short *)(input->content))[i] == (short)*missingvalue))
				((int *)(output->content))[i] = (int)*output_missingvalue;
			else ((int *)(output->content))[i] = (int) ((short *)(input->content))[i];
                }
                break;
            case OPH_BYTE: //short->byte
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((short *)(input->content))[i] == (short)*missingvalue))
				((char *)(output->content))[i] = (char)*output_missingvalue;
			else ((char *)(output->content))[i] = (char) ((short *)(input->content))[i];
                }
                break;
            case OPH_FLOAT: //short->float
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((short *)(input->content))[i] == (short)*missingvalue))
				((float *)(output->content))[i] = args->args[4] ? (float)*output_missingvalue : NAN;
			else ((float *)(output->content))[i] = (float) ((short *)(input->content))[i];
                }
                break;
            case OPH_DOUBLE: //short->double
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((short *)(input->content))[i] == (short)*missingvalue))
				((double *)(output->content))[i] = args->args[4] ? (double)*output_missingvalue : NAN;
			else ((double *)(output->content))[i] = (double) ((short *)(input->content))[i];
                }
                break;
            case OPH_COMPLEX_INT: //short->cint
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((short *)(input->content))[i] == (short)*missingvalue))
				((int *)(output->content))[j] = ((int *)(output->content))[j+1] = (int)*output_missingvalue;
			else {
				((int *)(output->content))[j] = (int) ((short *)(input->content))[i];
				((int *)(output->content))[j+1] = 0;
			}
			j+=2;
                }
                break;
            case OPH_COMPLEX_LONG: //short->clong
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((short *)(input->content))[i] == (short)*missingvalue))
				((long long *)(output->content))[j] = ((long long *)(output->content))[j+1] = (long long)*output_missingvalue;
			else {
				((long long *)(output->content))[j] = (long long) ((short *)(input->content))[i];
				((long long *)(output->content))[j+1] = 0;
			}
			j+=2;
                }
                break;
            case OPH_COMPLEX_FLOAT: //short->cfloat
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((short *)(input->content))[i] == (short)*missingvalue))
				((float *)(output->content))[j] = ((float *)(output->content))[j+1] = args->args[4] ? (float)*output_missingvalue : NAN;
			else {
				((float *)(output->content))[j] = (float) ((short *)(input->content))[i];
				((float *)(output->content))[j+1] = 0;
			}
			j+=2;
                }
                break;
            case OPH_COMPLEX_DOUBLE: //short->cdouble
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((short *)(input->content))[i] == (short)*missingvalue))
				((double *)(output->content))[j] = ((double *)(output->content))[j+1] = args->args[4] ? (double)*output_missingvalue : NAN;
			else {
				((double *)(output->content))[j] = (double) ((short *)(input->content))[i];
				((double *)(output->content))[j+1] = 0;
			}
			j+=2;
                }
                break;
            default:
                pmesg(1,  __FILE__, __LINE__, "Type not recognized\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
            }
            break;
        case OPH_BYTE:
            switch (output->type) {
            case OPH_LONG: //byte->long
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((char *)(input->content))[i] == (char)*missingvalue))
				((long long *)(output->content))[i] = (long long)*output_missingvalue;
			else ((long long *)(output->content))[i] = (long long) ((char *)(input->content))[i];
                }
                break;
            case OPH_SHORT: //byte->short
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((char *)(input->content))[i] == (char)*missingvalue))
				((short *)(output->content))[i] = (short)*output_missingvalue;
			else ((short *)(output->content))[i] = (short) ((char *)(input->content))[i];
                }
                break;
            case OPH_INT: //byte->int
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((char *)(input->content))[i] == (char)*missingvalue))
				((int *)(output->content))[i] = (int)*output_missingvalue;
			else ((int *)(output->content))[i] = (int) ((char *)(input->content))[i];
                }
                break;
            case OPH_FLOAT: //byte->float
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((char *)(input->content))[i] == (char)*missingvalue))
				((float *)(output->content))[i] = args->args[4] ? (float)*output_missingvalue : NAN;
			else ((float *)(output->content))[i] = (float) ((char *)(input->content))[i];
                }
                break;
            case OPH_DOUBLE: //byte->double
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((char *)(input->content))[i] == (char)*missingvalue))
				((double *)(output->content))[i] = args->args[4] ? (double)*output_missingvalue : NAN;
			else ((double *)(output->content))[i] = (double) ((char *)(input->content))[i];
                }
                break;
            case OPH_COMPLEX_INT: //byte->cint
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((char *)(input->content))[i] == (char)*missingvalue))
				((int *)(output->content))[j] = ((int *)(output->content))[j+1] = (int)*output_missingvalue;
			else {
				((int *)(output->content))[j] = ((char *)(input->content))[i];
				((int *)(output->content))[j+1] = 0;
			}
			j+=2;
                }
                break;
            case OPH_COMPLEX_LONG: //byte->clong
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((char *)(input->content))[i] == (char)*missingvalue))
				((long long *)(output->content))[j] = ((long long *)(output->content))[j+1] = (long long)*output_missingvalue;
			else {
				((long long *)(output->content))[j] = (long long) ((char *)(input->content))[i];
				((long long *)(output->content))[j+1] = 0;
			}
			j+=2;
                }
                break;
            case OPH_COMPLEX_FLOAT: //byte->cfloat
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((char *)(input->content))[i] == (char)*missingvalue))
				((float *)(output->content))[j] = ((float *)(output->content))[j+1] = args->args[4] ? (float)*output_missingvalue : NAN;
			else {
				((float *)(output->content))[j] = (float) ((char *)(input->content))[i];
				((float *)(output->content))[j+1] = 0;
			}
			j+=2;
                }
                break;
            case OPH_COMPLEX_DOUBLE: //byte->cdouble
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((char *)(input->content))[i] == (char)*missingvalue))
				((double *)(output->content))[j] = ((double *)(output->content))[j+1] = args->args[4] ? (double)*output_missingvalue : NAN;
			else {
				((double *)(output->content))[j] = (double) ((char *)(input->content))[i];
				((double *)(output->content))[j+1] = 0;
			}
			j+=2;
                }
                break;
            default:
                pmesg(1,  __FILE__, __LINE__, "Type not recognized\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
            }
            break;
        case OPH_LONG:
            switch (output->type) {
            case OPH_INT: //long->int
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((long long *)(input->content))[i] == (long long)*missingvalue))
				((int *)(output->content))[i] = (int)*output_missingvalue;
			else ((int *)(output->content))[i] = (int) ((long long *)(input->content))[i];
                }
                break;
            case OPH_SHORT: //long->short
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((long long *)(input->content))[i] == (long long)*missingvalue))
				((short *)(output->content))[i] = (short)*output_missingvalue;
			else ((short *)(output->content))[i] = (short) ((long long *)(input->content))[i];
                }
                break;
            case OPH_BYTE: //long->byte
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((long long *)(input->content))[i] == (long long)*missingvalue))
				((char *)(output->content))[i] = (char)*output_missingvalue;
			else ((char *)(output->content))[i] = (char) ((long long *)(input->content))[i];
                }
                break;
            case OPH_FLOAT: //long->float
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((long long *)(input->content))[i] == (long long)*missingvalue))
				((float *)(output->content))[i] = args->args[4] ? (float)*output_missingvalue : NAN;
			else ((float *)(output->content))[i] = (float) ((long long *)(input->content))[i];
                }
                break;
            case OPH_DOUBLE: //long->double
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((long long *)(input->content))[i] == (long long)*missingvalue))
				((double *)(output->content))[i] = args->args[4] ? (double)*output_missingvalue : NAN;
			else ((double *)(output->content))[i] = (double) ((long long *)(input->content))[i];
                }
                break;
            case OPH_COMPLEX_INT: //long->cint
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((long long *)(input->content))[i] == (long long)*missingvalue))
				((int *)(output->content))[j] = ((int *)(output->content))[j+1] = (int)*output_missingvalue;
			else {
				((int *)(output->content))[j] = (int) ((long long *)(input->content))[i];
				((int *)(output->content))[j+1] = 0;
			}
			j+=2;
                }
                break;
            case OPH_COMPLEX_LONG: //long->clong
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((long long *)(input->content))[i] == (long long)*missingvalue))
				((long long *)(output->content))[j] = ((long long *)(output->content))[j+1] = (long long)*output_missingvalue;
			else {
				((long long *)(output->content))[j] = ((long long *)(input->content))[i];
				((long long *)(output->content))[j+1] = 0;
			}
			j+=2;
                }
                break;
            case OPH_COMPLEX_FLOAT: //long->cfloat
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((long long *)(input->content))[i] == (long long)*missingvalue))
				((float *)(output->content))[j] = ((float *)(output->content))[j+1] = args->args[4] ? (float)*output_missingvalue : NAN;
			else {
				((float *)(output->content))[j] = (float) ((long long *)(input->content))[i];
				((float *)(output->content))[j+1] = 0;
			}
			j+=2;
                }
                break;
            case OPH_COMPLEX_DOUBLE: //long->cdouble
                for (i = 0; i < input->numelem; i++) {
			if (missingvalue && (((long long *)(input->content))[i] == (long long)*missingvalue))
				((double *)(output->content))[j] = ((double *)(output->content))[j+1] = args->args[4] ? (double)*output_missingvalue : NAN;
			else {
				((double *)(output->content))[j] = (double) ((long long *)(input->content))[i];
				((double *)(output->content))[j+1] = 0;
			}
			j+=2;
                }
                break;
            default:
                pmesg(1,  __FILE__, __LINE__, "Type not recognized\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
            }
            break;
        case OPH_FLOAT:
            switch (output->type) {
            case OPH_INT: //float->int
                for (i = 0; i < input->numelem; i++) {
                    if (isnanf(((float *)(input->content))[i]) || (missingvalue && (((float *)(input->content))[i] == (float)*missingvalue))) {
			if (output_missingvalue)
				((int *)(output->content))[i] = (float)*output_missingvalue;
			else {
		                pmesg(1,  __FILE__, __LINE__, "Cannot convert NaNs to ints\n");
		                *length=0;
		                *is_null=0;
		                *error=1;
		                return NULL;
			}
                    } else {
                        ((int *)(output->content))[i] = (int) ((float *)(input->content))[i];
                    }
                }
                break;
            case OPH_SHORT: //float->short
                for (i = 0; i < input->numelem; i++) {
                    if (isnanf(((float *)(input->content))[i]) || (missingvalue && (((float *)(input->content))[i] == (float)*missingvalue))) {
			if (output_missingvalue)
				((short *)(output->content))[i] = (short)*output_missingvalue;
			else {
		                pmesg(1,  __FILE__, __LINE__, "Cannot convert NaNs to ints\n");
		                *length=0;
		                *is_null=0;
		                *error=1;
		                return NULL;
			}
                    } else {
                        ((short *)(output->content))[i] = (short) ((float *)(input->content))[i];
                    }
                }
                break;
            case OPH_BYTE: //float->byte
                for (i = 0; i < input->numelem; i++) {
                    if (isnanf(((float *)(input->content))[i]) || (missingvalue && (((float *)(input->content))[i] == (float)*missingvalue))) {
			if (output_missingvalue)
				((char *)(output->content))[i] = (char)*output_missingvalue;
			else {
		                pmesg(1,  __FILE__, __LINE__, "Cannot convert NaNs to ints\n");
		                *length=0;
		                *is_null=0;
		                *error=1;
		                return NULL;
			}
                    } else {
                        ((char *)(output->content))[i] = (char) ((float *)(input->content))[i];
                    }
                }
                break;
            case OPH_LONG: //float->long
                for (i = 0; i < input->numelem; i++) {
                    if (isnanf(((float *)(input->content))[i]) || (missingvalue && (((float *)(input->content))[i] == (float)*missingvalue))) {
			if (output_missingvalue)
				((long long *)(output->content))[i] = (long long)*output_missingvalue;
			else {
		                pmesg(1,  __FILE__, __LINE__, "Cannot convert NaNs to longs\n");
		                *length=0;
		                *is_null=0;
		                *error=1;
		                return NULL;
			}
                    } else {
                        ((long long *)(output->content))[i] = (long long) ((float *)(input->content))[i];
                    }
                }
                break;
            case OPH_DOUBLE: //float->double
                for (i = 0; i < input->numelem; i++) {
                    if (isnanf(((float *)(input->content))[i]) || (missingvalue && (((float *)(input->content))[i] == (float)*missingvalue))) {
                        ((double *)(output->content))[i] = args->args[4] ? (double)*output_missingvalue : NAN;
                    } else {
                        ((double *)(output->content))[i] = (double) ((float *)(input->content))[i];
                    }
                }
                break;
            case OPH_COMPLEX_INT: //float->cint
                for (i = 0; i < input->numelem; i++) {
                    if (isnanf(((float *)(input->content))[i]) || (missingvalue && (((float *)(input->content))[i] == (float)*missingvalue))) {
			if (output_missingvalue) {
				((int *)(output->content))[j] = ((int *)(output->content))[j+1] = (int)*output_missingvalue;
			} else {
		                pmesg(1,  __FILE__, __LINE__, "Cannot convert NaNs to ints\n");
		                *length=0;
		                *is_null=0;
		                *error=1;
		                return NULL;
			}
                    } else {
                        ((int *)(output->content))[j] = (int) ((float *)(input->content))[i];
                        ((int *)(output->content))[j+1] = 0;
                    }
		    j+=2;
                }
                break;
            case OPH_COMPLEX_LONG: //float->clong
                for (i = 0; i < input->numelem; i++) {
                    if (isnanf(((float *)(input->content))[i]) || (missingvalue && (((float *)(input->content))[i] == (float)*missingvalue))) {
			if (output_missingvalue) {
				((long long *)(output->content))[j] = ((long long *)(output->content))[j+1] = (long long)*output_missingvalue;
			} else {
		                pmesg(1,  __FILE__, __LINE__, "Cannot convert NaNs to longs\n");
		                *length=0;
		                *is_null=0;
		                *error=1;
		                return NULL;
			}
                    } else {
                        ((long long *)(output->content))[j] = (long long) ((float *)(input->content))[i];
                        ((long long *)(output->content))[j+1] = 0;
                    }
		    j+=2;
                }
                break;
            case OPH_COMPLEX_FLOAT: //float->cfloat
                for (i = 0; i < input->numelem; i++) {
                    if (isnanf(((float *)(input->content))[i]) || (missingvalue && (((float *)(input->content))[i] == (float)*missingvalue))) {
                        ((float *)(output->content))[j] = ((float *)(output->content))[j+1] = args->args[4] ? (float)*output_missingvalue : NAN;
                    } else {
                        ((float *)(output->content))[j] = ((float *)(input->content))[i];
                        ((float *)(output->content))[j+1] = 0;
                    }
		    j+=2;
                }
                break;
            case OPH_COMPLEX_DOUBLE: //float->cdouble
                for (i = 0; i < input->numelem; i++) {
                    if (isnanf(((float *)(input->content))[i]) || (missingvalue && (((float *)(input->content))[i] == (float)*missingvalue))) {
                        ((double *)(output->content))[j] = ((double *)(output->content))[j+1] = args->args[4] ? (double)*output_missingvalue : NAN;
                    } else {
                        ((double *)(output->content))[j] = (double) ((float *)(input->content))[i];
                        ((double *)(output->content))[j+1] = 0;
                    }
		    j+=2;
                }
                break;
            default:
                pmesg(1,  __FILE__, __LINE__, "Type not recognized\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
            }
            break;
        case OPH_DOUBLE:
            switch (output->type) {
            case OPH_INT: //double->int
                for (i = 0; i < input->numelem; i++) {
                    if (isnan(((double *)(input->content))[i]) || (missingvalue && (((double *)(input->content))[i] == (double)*missingvalue))) {
			if (output_missingvalue)
				((int *)(output->content))[i] = (int)*output_missingvalue;
			else {
		                pmesg(1,  __FILE__, __LINE__, "Cannot convert NaNs to ints\n");
		                *length=0;
		                *is_null=0;
		                *error=1;
		                return NULL;
			}
                    } else {
                        ((int *)(output->content))[i] = (int) ((double *)(input->content))[i];
                    }
                }
                break;
            case OPH_SHORT: //double->int
                for (i = 0; i < input->numelem; i++) {
                    if (isnan(((double *)(input->content))[i]) || (missingvalue && (((double *)(input->content))[i] == (double)*missingvalue))) {
			if (output_missingvalue)
				((short *)(output->content))[i] = (short)*output_missingvalue;
			else {
		                pmesg(1,  __FILE__, __LINE__, "Cannot convert NaNs to ints\n");
		                *length=0;
		                *is_null=0;
		                *error=1;
		                return NULL;
			}
                    } else {
                        ((short *)(output->content))[i] = (short) ((double *)(input->content))[i];
                    }
                }
                break;
            case OPH_BYTE: //double->int
                for (i = 0; i < input->numelem; i++) {
                    if (isnan(((double *)(input->content))[i]) || (missingvalue && (((double *)(input->content))[i] == (double)*missingvalue))) {
			if (output_missingvalue)
				((char *)(output->content))[i] = (char)*output_missingvalue;
			else {
		                pmesg(1,  __FILE__, __LINE__, "Cannot convert NaNs to ints\n");
		                *length=0;
		                *is_null=0;
		                *error=1;
		                return NULL;
			}
                    } else {
                        ((char *)(output->content))[i] = (char) ((double *)(input->content))[i];
                    }
                }
                break;
            case OPH_LONG: //double->long
                for (i = 0; i < input->numelem; i++) {
                    if (isnan(((double *)(input->content))[i]) || (missingvalue && (((double *)(input->content))[i] == (double)*missingvalue))) {
			if (output_missingvalue)
				((long long *)(output->content))[i] = (long long)*output_missingvalue;
			else {
		                pmesg(1,  __FILE__, __LINE__, "Cannot convert NaNs to longs\n");
		                *length=0;
		                *is_null=0;
		                *error=1;
		                return NULL;
			}
                    } else {
                        ((long long *)(output->content))[i] = (long long) ((double *)(input->content))[i];
                    }
                }
                break;
            case OPH_FLOAT: //double->float
                for (i = 0; i < input->numelem; i++) {
                    if (isnan(((double *)(input->content))[i]) || (missingvalue && (((double *)(input->content))[i] == (double)*missingvalue))) {
                        ((float *)(output->content))[i] = args->args[4] ? (float)*output_missingvalue : NAN;
                    } else {
                        ((float *)(output->content))[i] = (float) ((double *)(input->content))[i];
                    }
                }
                break;
            case OPH_COMPLEX_INT: //double->cint
                for (i = 0; i < input->numelem; i++) {
                    if (isnan(((double *)(input->content))[i]) || (missingvalue && (((double *)(input->content))[i] == (double)*missingvalue))) {
			if (output_missingvalue) {
				((int *)(output->content))[j] = ((int *)(output->content))[j+1] = (int)*output_missingvalue;
			} else {
		                pmesg(1,  __FILE__, __LINE__, "Cannot convert NaNs to ints\n");
		                *length=0;
		                *is_null=0;
		                *error=1;
		                return NULL;
			}
                    } else {
                        ((int *)(output->content))[j] = (int) ((double *)(input->content))[i];
                        ((int *)(output->content))[j+1] = 0;
                    }
		    j+=2;
                }
                break;
            case OPH_COMPLEX_LONG: //double->clong
                for (i = 0; i < input->numelem; i++) {
                    if (isnan(((double *)(input->content))[i]) || (missingvalue && (((double *)(input->content))[i] == (double)*missingvalue))) {
			if (output_missingvalue) {
				((long long *)(output->content))[j] = ((long long *)(output->content))[j+1] = (long long)*output_missingvalue;
			} else {
		                pmesg(1,  __FILE__, __LINE__, "Cannot convert NaNs to longs\n");
		                *length=0;
		                *is_null=0;
		                *error=1;
		                return NULL;
			}
                    } else {
                        ((long long *)(output->content))[j] = (long long) ((double *)(input->content))[i];
                        ((long long *)(output->content))[j+1] = 0;
                    }
		    j+=2;
                }
                break;
            case OPH_COMPLEX_FLOAT: //double->cfloat
                for (i = 0; i < input->numelem; i++) {
                    if (isnan(((double *)(input->content))[i]) || (missingvalue && (((double *)(input->content))[i] == (double)*missingvalue))) {
                        ((float *)(output->content))[j] = ((float *)(output->content))[j+1] = args->args[4] ? (float)*output_missingvalue : NAN;
                    } else {
                        ((float *)(output->content))[j] = (float) ((double *)(input->content))[i];
                        ((float *)(output->content))[j+1] = 0;
                    }
		    j+=2;
                }
                break;
            case OPH_COMPLEX_DOUBLE: //double->cdouble
                for (i = 0; i < input->numelem; i++) {
                    if (isnan(((double *)(input->content))[i]) || (missingvalue && (((double *)(input->content))[i] == (double)*missingvalue))) {
                        ((double *)(output->content))[j] = ((double *)(output->content))[j+1] = args->args[4] ? (double)*output_missingvalue : NAN;
                    } else {
                        ((double *)(output->content))[j] = ((double *)(input->content))[i];
                        ((double *)(output->content))[j+1] = 0;
                    }
		    j+=2;
                }
                break;
            default:
                pmesg(1,  __FILE__, __LINE__, "Type not recognized\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
            }
            break;
        case OPH_COMPLEX_INT: {
	int input_numelem = (input->numelem)*2;
            switch (output->type) {
            case OPH_COMPLEX_LONG: //cint->clong
                for (i = 0; i < input_numelem; i++) {
			if (missingvalue && (((int *)(input->content))[i] == (int)*missingvalue))
				((long long *)(output->content))[i] = (long long)*output_missingvalue;
			else ((long long *)(output->content))[i] = (long long) ((int *)(input->content))[i];
                }
                break;
            case OPH_COMPLEX_FLOAT: //cint->cfloat
                for (i = 0; i < input_numelem; i++) {
			if (missingvalue && (((int *)(input->content))[i] == (int)*missingvalue))
				((float *)(output->content))[i] = args->args[4] ? (float)*output_missingvalue : NAN;
			else ((float *)(output->content))[i] = (float) ((int *)(input->content))[i];
                }
                break;
            case OPH_COMPLEX_DOUBLE: //cint->cdouble
                for (i = 0; i < input_numelem; i++) {
			if (missingvalue && (((int *)(input->content))[i] == (int)*missingvalue))
				((double *)(output->content))[i] = args->args[4] ? (double)*output_missingvalue : NAN;
			else ((double *)(output->content))[i] = (double) ((int *)(input->content))[i];
                }
                break;
            default:
                pmesg(1,  __FILE__, __LINE__, "Type not recognized\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
            }
            break;
	}
        case OPH_COMPLEX_LONG: {
	int input_numelem = (input->numelem)*2;
            switch (output->type) {
            case OPH_COMPLEX_INT: //clong->cint
                for (i = 0; i < input_numelem; i++) {
			if (missingvalue && (((long long *)(input->content))[i] == (long long)*missingvalue))
				((int *)(output->content))[i] = (int)*output_missingvalue;
			else ((int *)(output->content))[i] = (int) ((long long *)(input->content))[i];
                }
                break;
            case OPH_COMPLEX_FLOAT: //clong->cfloat
                for (i = 0; i < input_numelem; i++) {
			if (missingvalue && (((long long *)(input->content))[i] == (long long)*missingvalue))
				((float *)(output->content))[i] = args->args[4] ? (float)*output_missingvalue : NAN;
			else ((float *)(output->content))[i] = (float) ((long long *)(input->content))[i];
                }
                break;
            case OPH_COMPLEX_DOUBLE: //clong->cdouble
                for (i = 0; i < input_numelem; i++) {
			if (missingvalue && (((long long *)(input->content))[i] == (long long)*missingvalue))
				((double *)(output->content))[i] = args->args[4] ? (double)*output_missingvalue : NAN;
			else ((double *)(output->content))[i] = (double) ((long long *)(input->content))[i];
                }
                break;
            default:
                pmesg(1,  __FILE__, __LINE__, "Type not recognized\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
            }
            break;
	}
        case OPH_COMPLEX_FLOAT: {
	int input_numelem = (input->numelem)*2;
            switch (output->type) {
            case OPH_COMPLEX_INT: //cfloat->cint
                for (i = 0; i < input_numelem; i++) {
                    if (isnanf(((float *)(input->content))[i]) || (missingvalue && (((float *)(input->content))[i] == (float)*missingvalue))) {
			if (output_missingvalue)
				((int *)(output->content))[i] = (int)*output_missingvalue;
			else {
		                pmesg(1,  __FILE__, __LINE__, "Cannot convert NaNs to ints\n");
		                *length=0;
		                *is_null=0;
		                *error=1;
		                return NULL;
			}
                    } else {
                        ((int *)(output->content))[i] = (int) ((float *)(input->content))[i];
                    }
                }
                break;
            case OPH_COMPLEX_LONG: //cfloat->clong
                for (i = 0; i < input_numelem; i++) {
                    if (isnanf(((float *)(input->content))[i]) || (missingvalue && (((float *)(input->content))[i] == (float)*missingvalue))) {
			if (output_missingvalue)
				((long long *)(output->content))[i] = (long long)*output_missingvalue;
			else {
		                pmesg(1,  __FILE__, __LINE__, "Cannot convert NaNs to longs\n");
		                *length=0;
		                *is_null=0;
		                *error=1;
		                return NULL;
			}
                    } else {
                        ((long long *)(output->content))[i] = (long long) ((float *)(input->content))[i];
                    }
                }
                break;
            case OPH_COMPLEX_DOUBLE: //cfloat->cdouble
                for (i = 0; i < input_numelem; i++) {
                    if (isnanf(((float *)(input->content))[i]) || (missingvalue && (((float *)(input->content))[i] == (float)*missingvalue))) {
                        ((double *)(output->content))[i] = args->args[4] ? (double)*output_missingvalue : NAN;
                    } else {
                        ((double *)(output->content))[i] = (double) ((float *)(input->content))[i];
                    }
                }
                break;
            default:
                pmesg(1,  __FILE__, __LINE__, "Type not recognized\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
            }
            break;
	}
        case OPH_COMPLEX_DOUBLE: {
	int input_numelem = (input->numelem)*2;
            switch (output->type) {
            case OPH_COMPLEX_INT: //cdouble->cint
                for (i = 0; i < input_numelem; i++) {
                    if (isnan(((double *)(input->content))[i]) || (missingvalue && (((double *)(input->content))[i] == (double)*missingvalue))) {
			if (output_missingvalue)
				((int *)(output->content))[i] = (int)*output_missingvalue;
			else {
		                pmesg(1,  __FILE__, __LINE__, "Cannot convert NaNs to ints\n");
		                *length=0;
		                *is_null=0;
		                *error=1;
		                return NULL;
			}
                    } else {
                        ((int *)(output->content))[i] = (int) ((double *)(input->content))[i];
                    }
                }
                break;
            case OPH_COMPLEX_LONG: //cdouble->clong
                for (i = 0; i < input_numelem; i++) {
                    if (isnan(((double *)(input->content))[i]) || (missingvalue && (((double *)(input->content))[i] == (double)*missingvalue))) {
			if (output_missingvalue)
				((long long *)(output->content))[i] = (long long)*output_missingvalue;
			else {
		                pmesg(1,  __FILE__, __LINE__, "Cannot convert NaNs to longs\n");
		                *length=0;
		                *is_null=0;
		                *error=1;
		                return NULL;
			}
                    } else {
                        ((long long *)(output->content))[i] = (long long) ((double *)(input->content))[i];
                    }
                }
                break;
            case OPH_COMPLEX_FLOAT: //cdouble->cfloat
                for (i = 0; i < input_numelem; i++) {
                    if (isnan(((double *)(input->content))[i]) || (missingvalue && (((double *)(input->content))[i] == (double)*missingvalue))) {
                        ((float *)(output->content))[i] = args->args[4] ? (double)*output_missingvalue : NAN;
                    } else {
                        ((float *)(output->content))[i] = (float) ((double *)(input->content))[i];
                    }
                }
                break;
            default:
                pmesg(1,  __FILE__, __LINE__, "Type not recognized\n");
                *length=0;
                *is_null=0;
                *error=1;
                return NULL;
            }
            break;
	}
        default:
            pmesg(1,  __FILE__, __LINE__, "Type not recognized\n");
            *length=0;
            *is_null=0;
            *error=1;
            return NULL;
        }

    }

    *length = *(output->length);
    return output->content;

}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
