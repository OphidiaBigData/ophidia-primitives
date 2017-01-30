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

#include "oph_quantize.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_quantize_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	if(args->arg_count != 5)
	{
		strcpy(message, "ERROR: Wrong arguments! oph_quantize(input_OPH_TYPE, output_OPH_TYPE, measure, binary quantization levels, binary quantization values)");
		return 1;
	}

	int i;
	for(i = 0; i < 5; i++){
		if(args->arg_type[i] != STRING_RESULT){
			strcpy(message, "ERROR: Wrong arguments to oph_quantize function");
			return 1;
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;
	return 0;
}

void oph_quantize_deinit(UDF_INIT *initid)
{
	//Free allocated space
	if(initid->ptr)
	{
		free_oph_generic_param_multi((oph_generic_param_multi*)initid->ptr);
		initid->ptr = NULL;
	}
	if(initid->extension) {
		free(initid->extension);
		initid->extension = NULL;
	}
}

char* oph_quantize(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
	if (*error)
	{
	        *length=0;
	        *is_null=0;
	        *error=1;
	        return NULL;
	}
	if (*is_null || !args->lengths[2] || !args->lengths[3] || !args->lengths[4])
	{
	        *length=0;
	        *is_null=1;
	        *error=0;
	        return NULL;
	}

	oph_generic_param_multi* param;
	double *quant_levels;
	double *quant_values;
	unsigned long quant_levels_num;
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

		if (args->lengths[4] - args->lengths[3] != sizeof(double)) {
			pmesg(1, __FILE__, __LINE__, "Quantization values must be equal to the number of quantization levels + 1\n");
			*length=0;
			*is_null=0;
			*error=1;
			return NULL;
		}

		initid->extension = (void *)malloc(sizeof(unsigned long));
		if (!initid->extension)
		{
			pmesg(1, __FILE__, __LINE__, "Error in allocating parameters\n");
			*length=0;
			*is_null=0;
			*error=1;
			return NULL;
		}
		*((unsigned long *) initid->extension) = args->lengths[3] / sizeof(double);
	}
	else param = (oph_generic_param_multi*)initid->ptr;

	quant_levels = (double *) args->args[3];
	quant_values = (double *) args->args[4];
	quant_levels_num = *((unsigned long *) initid->extension);

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
    double tmp = 0;
    size_t run_sum1 = 0;
    size_t run_sum2 = 0;
    for (i = 0; i < measure->num_measure; i++) {
    	switch (measure->type[i]) {
    	case OPH_INT:
    		for (k = 0; k < i ; k++) {
    			run_sum1 += measure->elemsize[k];
    			run_sum2 += output->elemsize[k];
    		}
    		for (k = 0; k < measure->numelem; k++) {
    			tmp = (double) *((int *) (measure->content + (k * measure->blocksize) + run_sum1));
    			int q,assigned=0;
    			for (q = 0; q < quant_levels_num; q++) {
					if (tmp <= quant_levels[q]) {
						tmp = quant_values[q];
						assigned=1;
						break;
					}
				}
    			if (!assigned) {
					tmp = quant_values[quant_levels_num];
				}
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
    	case OPH_SHORT:
    		for (k = 0; k < i ; k++) {
    			run_sum1 += measure->elemsize[k];
    			run_sum2 += output->elemsize[k];
    		}
    		for (k = 0; k < measure->numelem; k++) {
    			tmp = (double) *((short *) (measure->content + (k * measure->blocksize) + run_sum1));
    			int q,assigned=0;
    			for (q = 0; q < quant_levels_num; q++) {
					if (tmp <= quant_levels[q]) {
						tmp = quant_values[q];
						assigned=1;
						break;
					}
				}
    			if (!assigned) {
					tmp = quant_values[quant_levels_num];
				}
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
    	case OPH_BYTE:
    		for (k = 0; k < i ; k++) {
    			run_sum1 += measure->elemsize[k];
    			run_sum2 += output->elemsize[k];
    		}
    		for (k = 0; k < measure->numelem; k++) {
    			tmp = (double) *((char *) (measure->content + (k * measure->blocksize) + run_sum1));
    			int q,assigned=0;
    			for (q = 0; q < quant_levels_num; q++) {
					if (tmp <= quant_levels[q]) {
						tmp = quant_values[q];
						assigned=1;
						break;
					}
				}
    			if (!assigned) {
					tmp = quant_values[quant_levels_num];
				}
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
    	case OPH_LONG:
    		for (k = 0; k < i ; k++) {
    			run_sum1 += measure->elemsize[k];
    			run_sum2 += output->elemsize[k];
    		}
    		for (k = 0; k < measure->numelem; k++) {
    			tmp = (double) *((long long *) (measure->content + (k * measure->blocksize) + run_sum1));
    			int q,assigned=0;
    			for (q = 0; q < quant_levels_num; q++) {
					if (tmp <= quant_levels[q]) {
						tmp = quant_values[q];
						assigned=1;
						break;
					}
				}
    			if (!assigned) {
					tmp = quant_values[quant_levels_num];
				}
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
    	case OPH_FLOAT:
    		for (k = 0; k < i ; k++) {
    			run_sum1 += measure->elemsize[k];
    			run_sum2 += output->elemsize[k];
    		}
    		for (k = 0; k < measure->numelem; k++) {
    			tmp = (double) *((float *) (measure->content + (k * measure->blocksize) + run_sum1));
    			int q,assigned=0;
    			for (q = 0; q < quant_levels_num; q++) {
					if (tmp <= quant_levels[q]) {
						tmp = quant_values[q];
						assigned=1;
						break;
					}
				}
    			if (!assigned) {
    				if (!isnan(tmp))
    					tmp = quant_values[quant_levels_num];
				}
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
    	case OPH_DOUBLE:
    		for (k = 0; k < i ; k++) {
    			run_sum1 += measure->elemsize[k];
    			run_sum2 += output->elemsize[k];
    		}
    		for (k = 0; k < measure->numelem; k++) {
    			tmp = *((double *) (measure->content + (k * measure->blocksize) + run_sum1));
    			int q,assigned=0;
    			for (q = 0; q < quant_levels_num; q++) {
					if (tmp <= quant_levels[q]) {
						tmp = quant_values[q];
						assigned=1;
						break;
					}
				}
    			if (!assigned) {
    				if (!isnan(tmp))
    					tmp = quant_values[quant_levels_num];
				}
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

