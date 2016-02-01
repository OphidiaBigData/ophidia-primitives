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

#include "oph_moving_avg.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_moving_avg_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
        int i = 0;
        if(args->arg_count < 3 || args->arg_count > 5){
                strcpy(message, "ERROR: Wrong arguments! oph_moving_avg(input_OPH_TYPE, output_OPH_TYPE, measure, [param], [operation])");
                return 1;
        }
        
        for(i = 0; i < args->arg_count; i++){
                if(i == 3){
                        if(args->arg_type[i] != INT_RESULT && args->arg_type[i] != DECIMAL_RESULT)
			{
                                strcpy(message, "ERROR: Wrong arguments to oph_moving_avg function");
                                return 1;
                        }
			else args->arg_type[i] = REAL_RESULT;
                }
                else{
                        if(args->arg_type[i] != STRING_RESULT){
                                strcpy(message, "ERROR: Wrong arguments to oph_moving_avg function");
                                return 1;
                        }
                }
        }

	initid->ptr = NULL;
	return 0;
}

void oph_moving_avg_deinit(UDF_INIT *initid)
{
        //Free allocated space
	if(initid->ptr)
	{
		oph_generic_param_multi* param = (oph_generic_param_multi*)initid->ptr;
		free_oph_generic_param_multi(param);
		if (param->extend)
		{
			free(param->extend);
			param->extend = NULL;
		}
                initid->ptr = NULL;
        }
}

char* oph_moving_avg(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
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
			pmesg(1, __FILE__, __LINE__, "Wrong number of input data type\n");
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

		oph_ma_oper oper;
		if(args->arg_count < 5) oper = OPH_SMA;
		else
		{
			char oper_buff[BUFF_LEN];
			if(core_strncpy(oper_buff, args->args[4],  &(args->lengths[4]))) pmesg(1, __FILE__, __LINE__, "Invalid operator\n");
			else
			{        
				//Case insensitive comparison
				if(!strcasecmp(oper_buff, "OPH_SMA")) oper = OPH_SMA;
				else if(!strcasecmp(oper_buff, "OPH_EWMA")) oper = OPH_EWMA;
				else{
					oper = INVALID_MA_OPER;
					pmesg(1, __FILE__, __LINE__, "Invalid operator\n");
				}		   
			}
		}
		switch(oper)
		{
			case OPH_SMA:
				param->core_oph_oper_multi_ext = core_oph_simple_moving_avg_multi;
				break;
			case OPH_EWMA:
				param->core_oph_oper_multi_ext = core_oph_exponential_moving_avg_multi;
				break;
			default:
				param->error = 1;
				*length=0;
				*is_null=0;
				*error=1;
				return NULL;
		}

		param->extend = malloc(sizeof(double));
		if(args->arg_count < 4) *((double*)param->extend) = 1.0;
		else
		{
			*((double*)param->extend) = *((double*) (args->args[3]));
			switch(oper)
			{
				case OPH_SMA:
				{
					if((*((double*)param->extend) < 1.0) || (*((double*)param->extend) > measure->numelem))
					{
						param->error = 1;
						pmesg(1,  __FILE__, __LINE__, "Moving set size is not correct\n");
						*length=0;
						*is_null=0;
						*error=1;
						return NULL;
					}
					break;
				}
				case OPH_EWMA:
				{
					if(*((double*)param->extend) < 0.0 || *((double*)param->extend) > 1.0)
					{
						param->error = 1;
						pmesg(1,  __FILE__, __LINE__, "Degree of weighting decrease alfa has to be in [0,1]\n");
						*length=0;
						*is_null=0;
						*error=1;
						return NULL;
					}
					break;
				}
			}
		}

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
		output->numelem = measure->numelem;
		output->length = output->numelem * output->blocksize;
		if(!output->length)
		{
			*length=0;
			*is_null=1;
			*error=0;
			return NULL;
		}
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

	if(!param->error && param->core_oph_oper_multi_ext(param->measure,param->result,param->extend))
	{
		param->error = 1;
		pmesg(1,  __FILE__, __LINE__, "Unable to compute result\n");
		*length=0;
		*is_null=0;
		*error=1;
		return NULL;
        }

	*length=output->length;
        *error=0;
        *is_null=0;

        return (result = output->content);
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/

