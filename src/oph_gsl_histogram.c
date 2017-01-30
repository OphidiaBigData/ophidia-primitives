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

#include "oph_gsl_histogram.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_gsl_histogram_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	int i = 0;
    /* oph_gsl_histogram(input_OPH_TYPE,output_OPH_TYPE,measure,nbin,[RANGES|FREQS|ALL],[ABS|PDF|CDF]) */
    if(args->arg_count > 6 || args->arg_count < 4 ){
            strcpy(message, "ERROR: Wrong arguments! oph_gsl_histogram(input_OPH_TYPE,output_OPH_TYPE,measure,nbin,[RANGES|FREQS|ALL],[ABS|PDF|CDF])");
            return 1;
    }

    for(i = 0; i < 3; i++){
            if(args->arg_type[i] != STRING_RESULT){
                    strcpy(message, "ERROR: Wrong arguments to oph_gsl_histogram function");
                    return 1;
            }
    }

    if(args->arg_type[3] != INT_RESULT){
            strcpy(message, "ERROR: Wrong arguments to oph_gsl_histogram function");
            return 1;
    }

    if (args->arg_count > 4) {
        if(args->arg_type[4] != STRING_RESULT){
                strcpy(message, "ERROR: Wrong arguments to oph_gsl_histogram function");
                return 1;
        }
    }
    if (args->arg_count > 5) {
        if(args->arg_type[5] != STRING_RESULT){
                strcpy(message, "ERROR: Wrong arguments to oph_gsl_histogram function");
                return 1;
        }
    }

    initid->ptr = NULL;
    initid->extension = NULL;

    return 0;
}

void oph_gsl_histogram_deinit(UDF_INIT *initid)
{
	//Free allocated space
	if(initid->ptr){
		oph_stringPtr output = (oph_stringPtr) initid->ptr;
		if(output->content){
			free(output->content);
			output->content=NULL;
		}
		if(output->length){
			free(output->length);
			output->length=NULL;
		}
		free(initid->ptr);
		initid->ptr=NULL;
	}
	if(initid->extension){
		oph_gsl_histogram_extraspace *extra = (oph_gsl_histogram_extraspace *) initid->extension;
		if(extra->hist){
			gsl_histogram_free(extra->hist);
			extra->hist=NULL;
		}
		free(initid->extension);
		initid->extension=NULL;
	}
}

char* oph_gsl_histogram(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
    gsl_set_error_handler_off();

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

    if (!initid->ptr) {

        initid->ptr=(char *)calloc(1,sizeof(oph_string));
        if(!initid->ptr){
            pmesg(1,  __FILE__, __LINE__, "Error allocating result\n");
            *length=0;
            *is_null=1;
            *error=1;
            return NULL;
        }

        oph_stringPtr output = (oph_stringPtr) initid->ptr;
        core_set_type(output,NULL,0); // output always double

        output->length = (unsigned long *)calloc(1,sizeof(unsigned long));
        if (!output->length) {
            pmesg(1,  __FILE__, __LINE__, "Error allocating length\n");
            *length=0;
            *is_null=1;
            *error=1;
            return NULL;
        }

        initid->extension=(void *)calloc(1,sizeof(oph_gsl_histogram_extraspace));
        if(!initid->extension){
            pmesg(1,  __FILE__, __LINE__, "Error allocating extension\n");
            *length=0;
            *is_null=1;
            *error=1;
            return NULL;
        }

        oph_gsl_histogram_extraspace *extra = (oph_gsl_histogram_extraspace *) initid->extension;
        extra->hist = gsl_histogram_alloc((size_t) *((long long *) args->args[3]));
        if(!extra->hist){
            pmesg(1,  __FILE__, __LINE__, "Error allocating histogram\n");
            *length=0;
            *is_null=1;
            *error=1;
            return NULL;
        }

        extra->mode = OPH_GSL_HISTOGRAM_ALL_MODE; //default
        extra->flag = OPH_GSL_HISTOGRAM_ABS_FREQS_FLAG; //default

        char buffer[BUFF_LEN];
        core_strncpy(buffer,args->args[0],&(args->lengths[0]));
        if (!strcasecmp(buffer,"OPH_BYTE")) {
        	extra->intype = OPH_BYTE;
        } else if (!strcasecmp(buffer,"OPH_SHORT")) {
        	extra->intype = OPH_SHORT;
	} else if (!strcasecmp(buffer,"OPH_INT")) {
        	extra->intype = OPH_INT;
	} else if (!strcasecmp(buffer,"OPH_LONG")) {
        	extra->intype = OPH_LONG;
        } else if (!strcasecmp(buffer,"OPH_FLOAT")) {
        	extra->intype = OPH_FLOAT;
        } else if (!strcasecmp(buffer,"OPH_DOUBLE")) {
        	extra->intype = OPH_DOUBLE;
        } else {
        	pmesg(1,  __FILE__, __LINE__, "ERROR: Invalid input type\n");
        	*length=0;
        	*is_null=1;
        	*error=1;
        	return NULL;
        }

        int i;
        for (i = 4; i < args->arg_count; i++) {
            core_strncpy(buffer,args->args[i],&(args->lengths[i]));
            if (!strcasecmp(buffer,OPH_GSL_HISTOGRAM_RANGES)) {
                extra->mode = OPH_GSL_HISTOGRAM_RANGES_MODE;
            } else if (!strcasecmp(buffer,OPH_GSL_HISTOGRAM_FREQS)) {
                extra->mode = OPH_GSL_HISTOGRAM_FREQS_MODE;
            } else if (!strcasecmp(buffer,OPH_GSL_HISTOGRAM_ALL)) {
                extra->mode = OPH_GSL_HISTOGRAM_ALL_MODE;
            } else if (!strcasecmp(buffer,OPH_GSL_HISTOGRAM_ABS_FREQS)) {
                extra->flag = OPH_GSL_HISTOGRAM_ABS_FREQS_FLAG;
            } else if (!strcasecmp(buffer,OPH_GSL_HISTOGRAM_REL_FREQS)) {
                extra->flag = OPH_GSL_HISTOGRAM_REL_FREQS_FLAG;
            } else if (!strcasecmp(buffer,OPH_GSL_HISTOGRAM_CUM_FREQS)) {
                extra->flag = OPH_GSL_HISTOGRAM_CUM_FREQS_FLAG;
            } else {
                pmesg(1,  __FILE__, __LINE__, "ERROR: Wrong arguments to oph_gsl_histogram function\n");
                *length=0;
                *is_null=1;
                *error=1;
                return NULL;
            }
        }

        if(core_set_elemsize(output)){
            pmesg(1,  __FILE__, __LINE__, "Error on setting element size\n");
            *length=0;
            *is_null=0;
            *error=1;
            return NULL;
        }

        switch (extra->mode) {
            case OPH_GSL_HISTOGRAM_RANGES_MODE:
                *(output->length)= ((unsigned long) *((long long *) args->args[3]) + 1) * sizeof(double); //nbin+1 doubles
                break;
            case OPH_GSL_HISTOGRAM_FREQS_MODE:
                *(output->length)= ((unsigned long) *((long long *) args->args[3])) * sizeof(double); //nbin doubles
                break;
            case OPH_GSL_HISTOGRAM_ALL_MODE:
                *(output->length)= (2 * ((unsigned long) *((long long *) args->args[3])) + 1) * sizeof(double); //(2*nbin)+1 doubles
                break;
            default:
                pmesg(1,  __FILE__, __LINE__, "Mode not recognized\n");
                *length=0;
                *is_null=1;
                *error=1;
                return NULL;
        }

        if(core_set_numelem(output)){
            pmesg(1,  __FILE__, __LINE__, "Error on counting result elements\n");
            *length=0;
            *is_null=0;
            *error=1;
            return NULL;
        }

        output->content = (char *)calloc(1,*(output->length));
        if(!output->content){
            pmesg(1,  __FILE__, __LINE__, "Error allocating result string\n");
            *length=0;
            *is_null=1;
            *error=1;
            return NULL;
        }
    }

    oph_stringPtr output = (oph_stringPtr) initid->ptr;
    oph_gsl_histogram_extraspace *extra = (oph_gsl_histogram_extraspace *) initid->extension;

    char *measure = args->args[2];
    unsigned long measurelen = args->lengths[2];
    unsigned long elem;

    switch (extra->intype) {
        case OPH_INT:
        {
            int imin,imax;
            elem = measurelen/sizeof(int);
            gsl_stats_int_minmax(&imin,&imax,(int *) measure,1,elem);
            imax+=1;
            gsl_histogram_set_ranges_uniform(extra->hist,(double)imin,(double)imax); //reset bins and set ranges
            switch (extra->mode) {
                case OPH_GSL_HISTOGRAM_RANGES_MODE:
                    break;
                case OPH_GSL_HISTOGRAM_FREQS_MODE:
                case OPH_GSL_HISTOGRAM_ALL_MODE:
                {
                    int i;
                    for (i = 0; i < elem; i++) {
                        gsl_histogram_increment(extra->hist,(double) ((int *)measure)[i]);
                    }
                }
            }
            break;
        }
        case OPH_SHORT:
        {
            short imin,imax;
            elem = measurelen/sizeof(short);
            gsl_stats_short_minmax(&imin,&imax,(short *) measure,1,elem);
            imax+=1;
            gsl_histogram_set_ranges_uniform(extra->hist,(double)imin,(double)imax); //reset bins and set ranges
            switch (extra->mode) {
                case OPH_GSL_HISTOGRAM_RANGES_MODE:
                    break;
                case OPH_GSL_HISTOGRAM_FREQS_MODE:
                case OPH_GSL_HISTOGRAM_ALL_MODE:
                {
                    int i;
                    for (i = 0; i < elem; i++) {
                        gsl_histogram_increment(extra->hist,(double) ((short *)measure)[i]);
                    }
                }
            }
            break;
        }
        case OPH_BYTE:
        {
            char imin,imax;
            elem = measurelen/sizeof(int);
            gsl_stats_char_minmax(&imin,&imax,(char *) measure,1,elem);
            imax+=1;
            gsl_histogram_set_ranges_uniform(extra->hist,(double)imin,(double)imax); //reset bins and set ranges
            switch (extra->mode) {
                case OPH_GSL_HISTOGRAM_RANGES_MODE:
                    break;
                case OPH_GSL_HISTOGRAM_FREQS_MODE:
                case OPH_GSL_HISTOGRAM_ALL_MODE:
                {
                    int i;
                    for (i = 0; i < elem; i++) {
                        gsl_histogram_increment(extra->hist,(double) ((char *)measure)[i]);
                    }
                }
            }
            break;
        }
        case OPH_LONG:
        {
            long lmin,lmax;
            elem = measurelen/sizeof(long long);
            gsl_stats_long_minmax(&lmin,&lmax,(long *) measure,1,elem);
            lmax+=1;
            gsl_histogram_set_ranges_uniform(extra->hist,(double)lmin,(double)lmax);
            switch (extra->mode) {
                case OPH_GSL_HISTOGRAM_RANGES_MODE:
                    break;
                case OPH_GSL_HISTOGRAM_FREQS_MODE:
                case OPH_GSL_HISTOGRAM_ALL_MODE:
                {
                    int i;
                    for (i = 0; i < elem; i++) {
                        gsl_histogram_increment(extra->hist,(double) ((long long *)measure)[i]);
                    }
                }
            }
            break;
        }
        case OPH_FLOAT:
        {
            float fmin,fmax;
            elem = measurelen/sizeof(float);
            gsl_stats_float_minmax(&fmin,&fmax,(float *) measure,1,elem);
            fmax+=1;
            gsl_histogram_set_ranges_uniform(extra->hist,(double)fmin,(double)fmax);
            switch (extra->mode) {
                case OPH_GSL_HISTOGRAM_RANGES_MODE:
                    break;
                case OPH_GSL_HISTOGRAM_FREQS_MODE:
                case OPH_GSL_HISTOGRAM_ALL_MODE:
                {
                    int i;
                    for (i = 0; i < elem; i++) {
                        gsl_histogram_increment(extra->hist,(double) ((float *)measure)[i]);
                    }
                }
            }
            break;
        }
        case OPH_DOUBLE:
        {
            double dmin,dmax;
            elem = measurelen/sizeof(double);
            gsl_stats_minmax(&dmin,&dmax,(double *) measure,1,elem);
            dmax+=1;
            gsl_histogram_set_ranges_uniform(extra->hist,dmin,dmax);
            switch (extra->mode) {
                case OPH_GSL_HISTOGRAM_RANGES_MODE:
                    break;
                case OPH_GSL_HISTOGRAM_FREQS_MODE:
                case OPH_GSL_HISTOGRAM_ALL_MODE:
                {
                    int i;
                    for (i = 0; i < elem; i++) {
                        gsl_histogram_increment(extra->hist, ((double *)measure)[i]);
                    }
                }
            }
            break;
        }
        default:
            pmesg(1,  __FILE__, __LINE__, "Input type not valid\n");
            *length=0;
            *is_null=1;
            *error=1;
            return NULL;
    }

    int i;
    switch (extra->mode) {
        case OPH_GSL_HISTOGRAM_RANGES_MODE:
            for (i = 0; i < output->numelem; i++) {
                ((double *) output->content)[i] = extra->hist->range[i];
            }
            break;
        case OPH_GSL_HISTOGRAM_FREQS_MODE:
            if (extra->flag == OPH_GSL_HISTOGRAM_ABS_FREQS_FLAG) { //abs
                for (i = 0; i < output->numelem; i++) {
                    ((double *) output->content)[i] = extra->hist->bin[i];
                }
            } else if (extra->flag == OPH_GSL_HISTOGRAM_REL_FREQS_FLAG){ //pdf
                for (i = 0; i < output->numelem; i++) {
                    ((double *) output->content)[i] = extra->hist->bin[i] / elem;
                }
            } else { //cdf
                for (i = 0; i < output->numelem; i++) {
                    if (i==0) {
                        ((double *) output->content)[i] = extra->hist->bin[i] / elem;
                    } else {
                        ((double *) output->content)[i] = (extra->hist->bin[i] / elem) + ((double *) output->content)[i-1];
                    }
                }
            }
            break;
        case OPH_GSL_HISTOGRAM_ALL_MODE:
        {
            i=0;
            int j;
            if (extra->flag == OPH_GSL_HISTOGRAM_ABS_FREQS_FLAG) { //abs
                for (j = 0; j < extra->hist->n; j++) {
                    ((double *) output->content)[i] = extra->hist->range[j];
                    i++;
                    ((double *) output->content)[i] = extra->hist->bin[j];
                    i++;
                }
                ((double *) output->content)[i] = extra->hist->range[j];
            } else if (extra->flag == OPH_GSL_HISTOGRAM_REL_FREQS_FLAG){ //pdf
                for (j = 0; j < extra->hist->n; j++) {
                    ((double *) output->content)[i] = extra->hist->range[j];
                    i++;
                    ((double *) output->content)[i] = extra->hist->bin[j] / elem;
                    i++;
                }
                ((double *) output->content)[i] = extra->hist->range[j];
            } else { //cdf
                for (j = 0; j < extra->hist->n; j++) {
                    ((double *) output->content)[i] = extra->hist->range[j];
                    i++;
                    if (j==0) {
                        ((double *) output->content)[i] = extra->hist->bin[j] / elem;
                    } else {
                        ((double *) output->content)[i] = (extra->hist->bin[j] / elem) + ((double *) output->content)[i-2];
                    }
                    i++;
                }
                ((double *) output->content)[i] = extra->hist->range[j];
            }
        }
    }

    *length = *(output->length);
    *error=0;
    *is_null=0;

    return output->content;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
