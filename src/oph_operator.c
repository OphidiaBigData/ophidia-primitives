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

#include "oph_operator.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_operator_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
        int i = 0;
        if((args->arg_count < 3) || (args->arg_count > 6)){
                strcpy(message, "ERROR: Wrong arguments! oph_operator(input_OPH_TYPE, output_OPH_TYPE, measure, [OPH_OPERATOR], [OPH_HIERARCHY], [order])");
                return 1;
        }
        
        for(i = 0; i < args->arg_count; i++){
		if (i==5)
		{
			if(args->arg_type[i] == STRING_RESULT){
                                strcpy(message, "ERROR: Wrong argument 'order' to oph_operator function");
                                return 1;
                        }
			args->arg_type[i] = REAL_RESULT;
		}
                else if(args->arg_type[i] != STRING_RESULT){
                        strcpy(message, "ERROR: Wrong arguments to oph_operator function");
                        return 1;
                }
        }

	initid->ptr = NULL;
	return 0;
}

void oph_operator_deinit(UDF_INIT *initid)
{
        //Free allocated space
	if(initid->ptr)
	{
		free_oph_generic_param_multi((oph_generic_param_multi*)initid->ptr);
                initid->ptr = NULL;
        }
}

char* oph_operator(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
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

		oph_request req;

		if(args->arg_count < 4) core_set_oper(&req, NULL, 0);
		else core_set_oper(&req, args->args[3], &(args->lengths[3]));
		if(!req.oper)
		{
			param->error = 1;
			pmesg(1,  __FILE__, __LINE__, "Operator not recognized\n");
			*length=0;
			*is_null=0;
			*error=1;
			return NULL;
		}
		switch(req.oper)
		{
			case OPH_COUNT:
				param->core_oph_oper_multi = core_oph_count_multi;
				break;
			case OPH_MAX:
				param->core_oph_oper_multi = core_oph_max_multi;
				break;
			case OPH_MIN:
				param->core_oph_oper_multi = core_oph_min_multi;
				break;
			case OPH_SUM:
				param->core_oph_oper_multi = core_oph_sum_multi;
				break;
			case OPH_AVG:
				param->core_oph_oper_multi = core_oph_avg_multi;
				break;
			case OPH_STD:
				param->core_oph_oper_multi = core_oph_std_multi;
				break;
			case OPH_CMOMENT:
		                param->core_oph_oper_multi = core_oph_cmoment_multi;
		                break;
			case OPH_ACMOMENT:
		                param->core_oph_oper_multi = core_oph_acmoment_multi;
		                break;
			case OPH_RMOMENT:
		                param->core_oph_oper_multi = core_oph_rmoment_multi;
		                break;
			case OPH_ARMOMENT:
		                param->core_oph_oper_multi = core_oph_armoment_multi;
		                break;
			default:
				param->error = 1;
				pmesg(1,  __FILE__, __LINE__, "Unable to recognize operator\n");
				*length=0;
				*is_null=0;
				*error=1;
				return NULL;
		}

		if(args->arg_count < 5) core_set_hier(&req, NULL, 0);
		else core_set_hier(&req, args->args[4], &(args->lengths[4]));
		if(!req.hier)
		{
			param->error = 1;
			pmesg(1,  __FILE__, __LINE__, "Hierarchy not recognized\n");
			*length=0;
			*is_null=0;
			*error=1;
			return NULL;
		}
		switch(req.hier)
		{
			case OPH_ALL:
				param->extend = (void*)1;
				break;
			default:
				param->error = 1;
				pmesg(1,  __FILE__, __LINE__, "Unable to recognize hierarchy\n");
				*length=0;
				*is_null=0;
				*error=1;
				return NULL;
		}

		if(args->arg_count > 5)
		{
			measure->param = *((double*)(args->args[5]));
			if (measure->param<0)
			{
				param->error = 1;
				pmesg(1,  __FILE__, __LINE__, "Wrong parameter value 'order' %f\n",measure->param);
				*length=0;
				*is_null=0;
				*error=1;
				return NULL;
			}
		}
		else measure->param = 2.0;

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
		output->numelem = param->extend ? 1 : 0;
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

	if(!param->error && param->core_oph_oper_multi(param->measure,param->result))
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

