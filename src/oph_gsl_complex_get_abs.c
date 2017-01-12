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

#include "oph_gsl_complex_get_abs.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_gsl_complex_get_abs_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    int i = 0;
    /* oph_gsl_complex_get_abs(input_OPH_TYPE, output_OPH_TYPE, measure) */
    if(args->arg_count != 3){
        strcpy(message, "ERROR: Wrong arguments! oph_gsl_complex_get_abs(input_OPH_TYPE, output_OPH_TYPE, measure)");
        return 1;
    }

    for(i = 0; i < args->arg_count; i++){
            if(args->arg_type[i] != STRING_RESULT){
                    strcpy(message, "ERROR: Wrong arguments to oph_gsl_complex_get_abs function");
                    return 1;
            }
    }

    initid->ptr = NULL;
    return 0;
}

void oph_gsl_complex_get_abs_deinit(UDF_INIT *initid)
{
	//Free allocated space
	if(initid->ptr)
	{
		free_oph_generic_param_multi((oph_generic_param_multi*)initid->ptr);
		initid->ptr = NULL;
	}
}

char* oph_gsl_complex_get_abs(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
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

	oph_generic_param_multi* param;
	if (!initid->ptr)
	{
		param = (oph_generic_param_multi*)malloc(sizeof(oph_generic_param_multi));
		if (!param)
		{
			pmesg(1, __FILE__, __LINE__, "Error in allocating parameters\n");
			*length=0;
			*is_null=0;
			*error=1;
			return NULL;
		}
		param->measure = NULL;
		param->result = NULL;
		param->error = 0;
		param->extend = NULL;

		initid->ptr = (char*)param;
	}
	else param = (oph_generic_param_multi*)initid->ptr;

	if (param->error)
	{
		*length=0;
		*is_null=0;
		*error=1;
		return NULL;
	}

	oph_multistring* measure;
	if (!param->error && !param->measure)
	{
		if(core_set_oph_multistring(&measure, args->args[0], &(args->lengths[0])))
		{
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*length=0;
			*is_null=0;
			*error=1;
			return NULL;
		}
		if (!measure->islast)
		{
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Wrong number of input measure\n");
			*length=0;
			*is_null=0;
			*error=1;
			return NULL;
		}
		measure->length = args->lengths[2];
		if(measure->length % measure->blocksize)
		{
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Wrong input type or data corrupted\n");
			*length=0;
			*is_null=0;
			*error=1;
			return NULL;
		}
		measure->numelem = measure->length / measure->blocksize;

		param->measure = measure;
	}
	else measure = param->measure;

	measure->content = args->args[2];

	oph_multistring* output;
	if (!param->error && !param->result)
	{
		if(core_set_oph_multistring(&output, args->args[1], &(args->lengths[1])))
		{
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*length=0;
			*is_null=0;
			*error=1;
			return NULL;
		}
		if (output->num_measure != measure->num_measure)
		{
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Wrong number of output data type\n");
			*length=0;
			*is_null=0;
			*error=1;
			return NULL;
		}
		output->numelem = measure->numelem;
		output->length = output->numelem * output->blocksize;

		output->content = (char *)malloc(output->length);
		if(!output->content)
		{
			param->error = 1;
			pmesg(1,  __FILE__, __LINE__, "Error allocating measures string\n");
			*length=0;
			*is_null=0;
			*error=1;
			return NULL;
		}

		param->result = output;
	}
	else output = param->result;

    int i,k;
    gsl_complex z;
    double tmp = 0;
    size_t run_sum1 = 0;
    size_t run_sum2 = 0;
    for (i = 0; i < measure->num_measure; i++) {
    	switch (measure->type[i]) {
    	case OPH_COMPLEX_INT:
    		for (k = 0; k < i ; k++) {
    			run_sum1 += measure->elemsize[k];
    			run_sum2 += output->elemsize[k];
    		}
    		for (k = 0; k < measure->numelem; k++) {
    			z.dat[0] = (double) *((int *) (measure->content + (k * measure->blocksize) + run_sum1));   //real part
    			z.dat[1] = (double) *((int *) (measure->content + (k * measure->blocksize) + run_sum1 + sizeof(int)));   //imag part
    			tmp = gsl_complex_abs(z);
    			if(core_oph_type_cast(&tmp, output->content + (k * output->blocksize) + run_sum2, OPH_DOUBLE, output->type[i], NULL)) {
    				param->error = 1;
    				pmesg(1,  __FILE__, __LINE__, "Error casting output\n");
    				*length=0;
    				*is_null=0;
    				*error=1;
    				return NULL;
    			}
    		}
			run_sum1 = 0;
			run_sum2 = 0;
    		break;
    	case OPH_COMPLEX_LONG:
    		for (k = 0; k < i ; k++) {
    			run_sum1 += measure->elemsize[k];
    			run_sum2 += output->elemsize[k];
    		}
    		for (k = 0; k < measure->numelem; k++) {
    			z.dat[0] = (double) *((long long *) (measure->content + (k * measure->blocksize) + run_sum1));   //real part
    			z.dat[1] = (double) *((long long *) (measure->content + (k * measure->blocksize) + run_sum1 + sizeof(long long)));   //imag part
    			tmp = gsl_complex_abs(z);
    			if(core_oph_type_cast(&tmp, output->content + (k * output->blocksize) + run_sum2, OPH_DOUBLE, output->type[i], NULL)) {
    				param->error = 1;
    				pmesg(1,  __FILE__, __LINE__, "Error casting output\n");
    				*length=0;
    				*is_null=0;
    				*error=1;
    				return NULL;
    			}
    		}
			run_sum1 = 0;
			run_sum2 = 0;
    		break;
    	case OPH_COMPLEX_FLOAT:
    		for (k = 0; k < i ; k++) {
    			run_sum1 += measure->elemsize[k];
    			run_sum2 += output->elemsize[k];
    		}
    		for (k = 0; k < measure->numelem; k++) {
    			z.dat[0] = (double) *((float *) (measure->content + (k * measure->blocksize) + run_sum1));   //real part
    			z.dat[1] = (double) *((float *) (measure->content + (k * measure->blocksize) + run_sum1 + sizeof(float)));   //imag part
    			tmp = gsl_complex_abs(z);
    			if(core_oph_type_cast(&tmp, output->content + (k * output->blocksize) + run_sum2, OPH_DOUBLE, output->type[i], NULL)) {
    				param->error = 1;
    				pmesg(1,  __FILE__, __LINE__, "Error casting output\n");
    				*length=0;
    				*is_null=0;
    				*error=1;
    				return NULL;
    			}
    		}
			run_sum1 = 0;
			run_sum2 = 0;
    		break;
    	case OPH_COMPLEX_DOUBLE:
    		for (k = 0; k < i ; k++) {
    			run_sum1 += measure->elemsize[k];
    			run_sum2 += output->elemsize[k];
    		}
    		for (k = 0; k < measure->numelem; k++) {
    			z.dat[0] = *((double *) (measure->content + (k * measure->blocksize) + run_sum1));   //real part
    			z.dat[1] = *((double *) (measure->content + (k * measure->blocksize) + run_sum1 + sizeof(double)));   //imag part
    			tmp = gsl_complex_abs(z);
    			if(core_oph_type_cast(&tmp, output->content + (k * output->blocksize) + run_sum2, OPH_DOUBLE, output->type[i], NULL)) {
    				param->error = 1;
    				pmesg(1,  __FILE__, __LINE__, "Error casting output\n");
    				*length=0;
    				*is_null=0;
    				*error=1;
    				return NULL;
    			}
    		}
			run_sum1 = 0;
			run_sum2 = 0;
    		break;
    	default:
			param->error = 1;
			pmesg(1, __FILE__, __LINE__, "Input Type not allowed\n");
			*length=0;
			*is_null=0;
			*error=1;
			return NULL;
    	}
	}

    *length=output->length;
    *error=0;
    *is_null=0;

    return (result = output->content);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/

