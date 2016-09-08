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

#include "oph_core.h"

#include <inttypes.h>

#define OPH_MULTISTRING_HARD_SEPARATOR "|"
#define OPH_MULTISTRING_SOFT_SEPARATORS "{},; "

extern int msglevel;

/*------------------------------------------------------------------|
 |               Functions implementation (BEGIN)                   |
 |------------------------------------------------------------------*/

/*------------------------------------------------------------------|
 |                  Internal functions (BEGIN)                      |
 |------------------------------------------------------------------*/

int core_strncpy (char* buff, const char* str, unsigned long *str_len){
        if(BUFF_LEN < *str_len){
                pmesg(1, __FILE__, __LINE__, "Invalid string (exceedes buffer length)\n");
                return -1;
        }
        strncpy(buff, str, *str_len);
        buff[*str_len]='\0';
        return 0;
}


size_t core_sizeof (oph_type type){

        switch(type){
                case OPH_BYTE:
                        return sizeof(char);
                case OPH_SHORT:
                        return sizeof(short);
                case OPH_INT:
                        return sizeof(int);
		case OPH_LONG:
                        return sizeof(long long);
                case OPH_FLOAT:
                        return sizeof(float);
                case OPH_DOUBLE:
                        return sizeof(double);
		case OPH_COMPLEX_INT:
                        return 2*sizeof(int);
		case OPH_COMPLEX_LONG:
                        return 2*sizeof(long long);
                case OPH_COMPLEX_FLOAT:
                        return 2*sizeof(float);
                case OPH_COMPLEX_DOUBLE:
                        return 2*sizeof(double);
		case INVALID_TYPE:
			break;
        }
        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
        return 0;
}

int core_set_hier(oph_requestPtr req, char* hier, unsigned long *len){
        if(!hier){
                req->hier = DEFAULT_HIER;
                return 0;
        }
        req->hier = core_get_hier(hier, len);
        if(req->hier == INVALID_HIER){
                pmesg(1, __FILE__, __LINE__, "Hierarchy not recognized\n");
                return -1;
        }
        return 0;
}

int core_set_type(oph_stringPtr str, char* type, unsigned long *len){
        if(!type){
                str->type = DEFAULT_TYPE;
                return 0;
        }
        str->type = core_get_type(type, len);
        if(str->type == INVALID_TYPE){
                pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
                return -1;
        }
        return 0;
}

int core_oph_multistring_cast (oph_multistring* input, oph_multistring* output){
	/*
		Assume that input and output multistrings have the same num_measure; 
		the output numelem controls the number of elements to cast in the input string
	*/
        char *in_tmp;
	char *out_tmp;
	int i,j;
	if(!input || !output){
                pmesg(1, __FILE__, __LINE__, "Null input parameter\n");
		return -1;
	}
	//Check if I can skip the cast phase
	i = 1;
	for (j = 0; j < input->num_measure; j++){
		if(input->type[j] != output->type[j]){
			i=0;
			break;
		}
	}
	if(i){
        	memcpy(output->content, (void*)(input->content), output->numelem*output->blocksize);
		return 0;
	}

	in_tmp = input->content;
	out_tmp = output->content;
	for (j = 0; j < input->num_measure; j++){
		for (i = 0; i < output->numelem; i++){
			if(core_oph_type_cast( (in_tmp)+(i*input->blocksize), (out_tmp)+(i*output->blocksize), input->type[j], output->type[j])){
                		pmesg(1, __FILE__, __LINE__, "Unable to convert elements type\n");
				return -1;
			}
		}
		in_tmp+=input->elemsize[j];
		out_tmp+=output->elemsize[j];
	}
	return 0;
}

int core_set_oph_multistring(oph_multistring **str, char* type, unsigned long *len){

/*
	This function allocates the array of oph_multistring structures and the internal variables.
 	Given the array of oph_multistring structures, this function sets:

	typedef struct oph_multistring{
	        char *content;          //Not set
        	unsigned long *length;  //Not set
	        oph_type *type;         //Yes (need to be deallocate outside)
        	size_t num_measure;     //Yes 
	        size_t *elemsize;       //Yes (need to be deallocate outside)
        	unsigned long numelem;  //Not set
		size_t blocksize;	//Yes
        	unsigned int islast;    //Yes
	} oph_multistring;

*/

        if(!type || !str || !len){
                pmesg(1, __FILE__, __LINE__, "Null input parameter\n");
                return -1;
        }

	char tmp_type1[BUFF_LEN]; 
	char *tmp_type = tmp_type1;
	if (core_strncpy(tmp_type1, type, len)){
		pmesg(1, __FILE__, __LINE__, "Invalid type\n");
                return -1;
	}
	int str_num = 1;

	int i=0,j=0,subtype_num=0;
	for (i = 0; i < *len; i++)
		if(tmp_type[i] == '|')
			str_num++;

	(*str) = (oph_multistring*)calloc((str_num), sizeof(oph_multistring));
	if(!*str){
                pmesg(1, __FILE__, __LINE__, "Memory allocation error\n");
                return -1;
	}

	for (i = 0; i < str_num; i++){
		(*str)[i].islast = 0;
		(*str)[i].content = NULL;
		(*str)[i].length = 0;
		(*str)[i].type = NULL;
		(*str)[i].num_measure = 0;
		(*str)[i].elemsize = NULL;
		(*str)[i].numelem = 0;
		(*str)[i].blocksize = 0;
		(*str)[i].param = 0;
		(*str)[i].extend = NULL;
	}
	(*str)[i-1].islast = 1;

	char *ptr1;
	char *mytype;
	char *ptr_s1;
	char *ptr_s2;
	char *ptr_s3;
	oph_type mytype2;
	char tmp_ptr[BUFF_LEN];
	char *tmp_ptr1;

	for (i = 0; ; i++, tmp_type = NULL) {

		ptr1 = strtok_r(tmp_type, OPH_MULTISTRING_HARD_SEPARATOR, &ptr_s1);
		if (ptr1 == NULL)
                  	break;

		//First tokenize used only for counting the number of basic type
		tmp_ptr1 = tmp_ptr;
		strncpy(tmp_ptr1, ptr1, BUFF_LEN);
		subtype_num=0;
		for (; ; tmp_ptr1 = NULL) {
                	mytype = strtok_r(tmp_ptr1, OPH_MULTISTRING_SOFT_SEPARATORS, &ptr_s2);
                   	if (mytype == NULL)
                       		break;
		   	if (core_get_type2(mytype) == INVALID_TYPE){
                		pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
                		return -1;
		   	}
			else
				subtype_num++;
               	}

		(*str)[i].num_measure = subtype_num;

		(*str)[i].type = (oph_type *)calloc(subtype_num,sizeof(oph_type));
		if(!((*str)[i].type)){
			free_oph_multistring(*str);
			*str=NULL;
                	pmesg(1, __FILE__, __LINE__, "Memory allocation error\n");
                	return -1;
		}
		(*str)[i].elemsize = (size_t *)calloc(subtype_num,sizeof(size_t));
		if(!((*str)[i].elemsize)){
			free_oph_multistring(*str);
			*str=NULL;
                	pmesg(1, __FILE__, __LINE__, "Memory allocation error\n");
                	return -1;
		}

		//Second tokenize used for setting the oph_type variable inside each oph_string structure
		tmp_ptr1 = tmp_ptr;
		strncpy(tmp_ptr1, ptr1, BUFF_LEN);
		j = 0;
		for (; ; tmp_ptr1 = NULL) {
                	mytype = strtok_r(tmp_ptr1, OPH_MULTISTRING_SOFT_SEPARATORS, &ptr_s3);
                   	if (mytype == NULL)
                       		break;
		   	if ((mytype2 = core_get_type2(mytype)) == INVALID_TYPE){
                		pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
                		return -1;
		   	}
			else
			{
				(*str)[i].type[j]=mytype2;
				(*str)[i].elemsize[j]=core_sizeof(mytype2);
				if((*str)[i].elemsize[j] == 0)
				{
					free_oph_multistring(*str);
					*str=NULL;
                			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
                			return -1;
				}
				(*str)[i].blocksize+=(*str)[i].elemsize[j];
				j++;
			}
               	}
	}
	//We must set the oph_string2.content, length, numelem value yet
        return 0;
}

void free_oph_multistring(oph_multistring *str){
	int i=0;
	if(!str)
		return;
	while (1){
		if(str[i].type){
			free(str[i].type);
			str[i].type=NULL;
		}
		if(str[i].elemsize){
			free(str[i].elemsize);
			str[i].elemsize=NULL;
		}
		if(str[i].extend){
			free(str[i].extend);
			str[i].extend=NULL;
		}
		if(str[i].islast)
			break;
		i++;
	}
	free(str);
	str=NULL;
}

void free_oph_generic_param_multi(oph_generic_param_multi* param)
{
	if(!param) return;

	oph_multistring* multimeasure = param->measure; // Inputs
	if (multimeasure)
	{
		free_oph_multistring(multimeasure);
	        param->measure=NULL;
	}

	multimeasure = param->result; // Output
	if (multimeasure)
	{
		if (multimeasure->content) 
		{
			free(multimeasure->content);
			multimeasure->content=NULL;
		}
		free_oph_multistring(multimeasure);
	        param->result=NULL;
	}

	free(param);
	
}

int core_set_elemsize(oph_stringPtr str){
	//Must be called after core_set_type function
	if(!str->type){
                pmesg(1, __FILE__, __LINE__, "Unknown elements size\n");
                return -1;
	}
		
	str->elemsize = core_sizeof(str->type);
	if(!str->elemsize){
                pmesg(1, __FILE__, __LINE__, "Unknown elements size\n");
                return -1;
        }
	return 0;
}

int core_set_oper(oph_requestPtr req, char* oper, unsigned long *len){
        if(!oper){
                req->oper = DEFAULT_OPER;
                return 0;
        }
        req->oper = core_get_oper(oper, len);
        if(req->oper== INVALID_OPER){
                        pmesg(1, __FILE__, __LINE__, "Operator not recognized\n");
                        return -1;
        }
        return 0;
}

int core_set_oper_multi(oph_request_multi* req, char* oper, unsigned long *len){
	oph_oper myoper = DEFAULT_OPER;
	if(!req){
		pmesg(1, __FILE__, __LINE__, "Null input parameter\n");
                return -1;
	}
        if(oper){
		myoper = core_get_oper(oper, len);
        	if(oper== INVALID_OPER){
                        pmesg(1, __FILE__, __LINE__, "Operator not recognized\n");
                        return -1;
        	}
        }
        switch(myoper)
	{
                case OPH_COUNT:
                        req->core_oph_oper = core_oph_count_multi;
                        break;
                case OPH_MAX:
                        req->core_oph_oper = core_oph_max_multi;
                        break;
                case OPH_MIN:
                        req->core_oph_oper = core_oph_min_multi;
                        break;
                case OPH_SUM:
                        req->core_oph_oper = core_oph_sum_multi;
                        break;
                case OPH_AVG:
                        req->core_oph_oper = core_oph_avg_multi;
                        break;
                case OPH_STD:
                        req->core_oph_oper = core_oph_std_multi;
                        break;
		case OPH_VAR:
                        req->core_oph_oper = core_oph_var_multi;
                        break;
		case OPH_CMOMENT:
                        req->core_oph_oper = core_oph_cmoment_multi;
                        break;
		case OPH_ACMOMENT:
                        req->core_oph_oper = core_oph_acmoment_multi;
                        break;
		case OPH_RMOMENT:
                        req->core_oph_oper = core_oph_rmoment_multi;
                        break;
		case OPH_ARMOMENT:
                        req->core_oph_oper = core_oph_armoment_multi;
                        break;
		case OPH_QUANTILE:
                        req->core_oph_oper = NULL;
                        break;
                default:
                        pmesg(1,  __FILE__, __LINE__, "Unable to recognize operator\n");
                        return -1;
        }
        return 0;
}

oph_hier core_get_hier(char* hier, unsigned long *len){
        char hier_buff[BUFF_LEN];
        if(core_strncpy(hier_buff, hier, len)){
                pmesg(1, __FILE__, __LINE__, "Invalid hierarchy\n");
                return INVALID_HIER;
        }
        //Case insensitive comparison
        if(!strcasecmp(hier_buff, "OPH_ALL"))
                return OPH_ALL;
        if(!strcasecmp(hier_buff, "OPH_WEEK"))
                return OPH_WEEK;
        if(!strcasecmp(hier_buff, "OPH_MONTH"))
                return OPH_MONTH;
        if(!strcasecmp(hier_buff, "OPH_YEAR"))
                return OPH_YEAR;
        pmesg(1, __FILE__, __LINE__, "Invalid hierarchy\n");
        return INVALID_HIER;
}

oph_type core_get_type (char* type, unsigned long *len){
        char type_buff[BUFF_LEN];
        if(core_strncpy(type_buff, type, len)){
                pmesg(1, __FILE__, __LINE__, "Invalid type\n");
                return INVALID_TYPE;
        }
	if(!strcasecmp(type_buff, "OPH_BYTE"))
                return OPH_BYTE;
        if(!strcasecmp(type_buff, "OPH_SHORT"))
                return OPH_SHORT;
        if(!strcasecmp(type_buff, "OPH_INT"))
                return OPH_INT;
        if(!strcasecmp(type_buff, "OPH_LONG"))
                return OPH_LONG;
        if(!strcasecmp(type_buff, "OPH_FLOAT"))
                return OPH_FLOAT;
        if(!strcasecmp(type_buff, "OPH_DOUBLE"))
                return OPH_DOUBLE;
        if(!strcasecmp(type_buff, "OPH_COMPLEX_INT"))
                return OPH_COMPLEX_INT;
        if(!strcasecmp(type_buff, "OPH_COMPLEX_LONG"))
                return OPH_COMPLEX_LONG;
        if(!strcasecmp(type_buff, "OPH_COMPLEX_FLOAT"))
                return OPH_COMPLEX_FLOAT;
        if(!strcasecmp(type_buff, "OPH_COMPLEX_DOUBLE"))
                return OPH_COMPLEX_DOUBLE;
        pmesg(1, __FILE__, __LINE__, "Invalid type\n");
        return INVALID_TYPE;
}

oph_type core_get_type2 (char* type_buff){
        //Case insensitive comparison
	if(!strcasecmp(type_buff, "OPH_BYTE"))
                return OPH_BYTE;
	if(!strcasecmp(type_buff, "OPH_SHORT"))
                return OPH_SHORT;
        if(!strcasecmp(type_buff, "OPH_INT"))
                return OPH_INT;
	if(!strcasecmp(type_buff, "OPH_LONG"))
                return OPH_LONG;
        if(!strcasecmp(type_buff, "OPH_FLOAT"))
                return OPH_FLOAT;
        if(!strcasecmp(type_buff, "OPH_DOUBLE"))
                return OPH_DOUBLE;
	if(!strcasecmp(type_buff, "OPH_COMPLEX_INT"))
                return OPH_COMPLEX_INT;
	if(!strcasecmp(type_buff, "OPH_COMPLEX_LONG"))
                return OPH_COMPLEX_LONG;
        if(!strcasecmp(type_buff, "OPH_COMPLEX_FLOAT"))
                return OPH_COMPLEX_FLOAT;
        if(!strcasecmp(type_buff, "OPH_COMPLEX_DOUBLE"))
                return OPH_COMPLEX_DOUBLE;
        pmesg(1, __FILE__, __LINE__, "Invalid type\n");
        return INVALID_TYPE;
}

oph_oper core_get_oper (char* oper, unsigned long *len){
        char oper_buff[BUFF_LEN];
        if(core_strncpy(oper_buff, oper, len)){
                pmesg(1, __FILE__, __LINE__, "Invalid operator\n");
                return INVALID_OPER;
        }
        //Case insensitive comparison
	if(!strcasecmp(oper_buff, "OPH_COUNT"))
                return OPH_COUNT;
        else if(!strcasecmp(oper_buff, "OPH_MAX"))
                return OPH_MAX;
        else if(!strcasecmp(oper_buff, "OPH_MIN"))
                return OPH_MIN;
        else if(!strcasecmp(oper_buff, "OPH_SUM"))
                return OPH_SUM;
        else if(!strcasecmp(oper_buff, "OPH_AVG"))
                return OPH_AVG;
        else if(!strcasecmp(oper_buff, "OPH_STD"))
                return OPH_STD;
        else if(!strcasecmp(oper_buff, "OPH_VAR"))
                return OPH_VAR;
	else if(!strcasecmp(oper_buff, "OPH_CMOMENT"))
                return OPH_CMOMENT;
        else if(!strcasecmp(oper_buff, "OPH_ACMOMENT"))
                return OPH_ACMOMENT;
        else if(!strcasecmp(oper_buff, "OPH_RMOMENT"))
                return OPH_RMOMENT;
        else if(!strcasecmp(oper_buff, "OPH_ARMOMENT"))
                return OPH_ARMOMENT;
	else if(!strcasecmp(oper_buff, "OPH_QUANTILE"))
                return OPH_QUANTILE;
        pmesg(1, __FILE__, __LINE__, "Invalid operator\n");
        return INVALID_OPER;
}

int core_set_numelem(oph_stringPtr str){
	// Must be called at the end of the oph_string initialization, after all other core_set_* routines
	unsigned int res = 0;
	res = (unsigned int)(*(str->length)%((unsigned int)str->elemsize));
	if(res){
                pmesg(1, __FILE__, __LINE__, "Unable to counting array elements\n");
                return -1;
        }
	str->numelem = (unsigned long)((*(str->length))/((unsigned long)str->elemsize));
	return 0;
}	

// Deprecated - use str->numelem
long core_count_array(oph_stringPtr str){

        long count = 0;
        unsigned int res = 0;
        size_t type_size = core_sizeof(str->type);
        if(!type_size){
                return -1;
        }
        res = (unsigned int)(*(str->length)%((unsigned int)type_size));
        pmesg(3, __FILE__, __LINE__, "length %d type_size %d modulo %d\n", *(str->length), type_size, res);
        if(res){
                pmesg(1, __FILE__, __LINE__, "Unable to compute measure length\n");
                return -1;
        }
        count = (long)((*(str->length))/((unsigned long)type_size));

        pmesg(3, __FILE__, __LINE__, "MEASURE ARRAY COUNT: %ld\n", count);
        return count;
}

int core_base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize)
{
   const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
   const uint8_t *data = (const uint8_t *)data_buf;
   size_t resultIndex = 0;
   size_t x;
   uint32_t n = 0;
   int padCount = dataLength % 3;
   uint8_t n0, n1, n2, n3;
 
   /* increment over the length of the string, three characters at a time */
   for (x = 0; x < dataLength; x += 3) 
   {
      /* these three 8-bit (ASCII) characters become one 24-bit number */
      n = ((uint32_t)data[x]) << 16; //parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0
 
      if((x+1) < dataLength)
         n += ((uint32_t)data[x+1]) << 8;//parenthesis needed, compiler depending on flags can do the shifting before conversion to uint32_t, resulting to 0
 
      if((x+2) < dataLength)
         n += data[x+2];
 
      /* this 24-bit number gets separated into four 6-bit numbers */
      n0 = (uint8_t)(n >> 18) & 63;
      n1 = (uint8_t)(n >> 12) & 63;
      n2 = (uint8_t)(n >> 6) & 63;
      n3 = (uint8_t)n & 63;
 
      /*
       * if we have one byte available, then its encoding is spread
       * out over two characters
       */
      if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
      result[resultIndex++] = base64chars[n0];
      if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
      result[resultIndex++] = base64chars[n1];
 
      /*
       * if we have only two bytes available, then their encoding is
       * spread out over three chars
       */
      if((x+1) < dataLength)
      {
         if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
         result[resultIndex++] = base64chars[n2];
      }
 
      /*
       * if we have all three bytes available, then their encoding is spread
       * out over four characters
       */
      if((x+2) < dataLength)
      {
         if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
         result[resultIndex++] = base64chars[n3];
      }
   }  
 
   /*
    * create and add padding that is required if we did not have a multiple of 3
    * number of characters available
    */
   if (padCount > 0) 
   { 
      for (; padCount < 3; padCount++) 
      { 
         if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
         result[resultIndex++] = '=';
      } 
   }
   if(resultIndex >= resultSize) return 1;   /* indicate failure: buffer too small */
   result[resultIndex] = 0;
   return 0;   /* indicate success */
}

#define WHITESPACE 64
#define EQUALS     65
#define INVALID    66
 
static const unsigned char core_d[] = {
    66,66,66,66,66,66,66,66,66,66,64,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,64,66,66,66,66,66,66,66,66,66,66,62,66,66,66,63,52,53,
    54,55,56,57,58,59,60,61,66,66,66,65,66,66,66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,66,66,66,66,66,66,26,27,28,
    29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66
};
 
int _core_base64decode (const char *in, size_t inLen, char *out, size_t *outLen) { 
    const char *end = in + inLen;
    char iter = 0;
    size_t buf = 0, len = 0;
 
    while (in < end) {
        unsigned char c = core_d[(int)(*in++)];
 
        switch (c) {
        case WHITESPACE: continue;   /* skip whitespace */
        case INVALID:    return 1;   /* invalid input, return error */
        case EQUALS:                 /* pad character, end of data */
            in = end;
            continue;
        default:
            buf = buf << 6 | c;
            iter++; // increment the number of iteration
            /* If the buffer is full, split it into bytes */
            if (iter == 4) {
                if ((len += 3) > *outLen) return 1; /* buffer overflow */
                (*out) = (buf >> 16) &255;
		out++;
                (*out) = (buf >> 8) &255;
		out++;
                (*out) = buf &255;
		out++;
                buf = 0; iter = 0;
 
            }   
        }
    }
 
    if (iter == 3) {
        if ((len += 2) > *outLen) return 1; /* buffer overflow */
        (*out) = (buf >> 10) &255;
	out++;
        (*out) = (buf >> 2) &255;
	out++;
    }
    else if (iter == 2) {
        if (++len > *outLen) return 1; /* buffer overflow */
        (*out) = (buf >> 4) &255;
	out++;
    }

    *outLen = len; /* modify to reflect the actual output size */

    return 0;
}

int core_base64decode(const char* query, char** new_query)
{
	if (!query || !new_query) return 1;
	size_t len = BUFF_LEN;
	char* _query = (char*)malloc(len*sizeof(char));
	if (!_query)
	{
		pmesg(LOG_ERROR,__FILE__,__LINE__,"Memory allocation error");
		return 1;
	}
	char* query_ = _query;
	int result_code = _core_base64decode(query, strlen(query), _query, &len);
	if (!result_code)
	{
		if (len+1<BUFF_LEN) *(query_+len)=0;
		else
		{
			pmesg(LOG_ERROR,__FILE__,__LINE__,"Space not available");
			return -2;
		}
	}
	*new_query = query_;
	return result_code;
}

/*------------------------------------------------------------------|
 |                  Internal functions (END)                        |
 |------------------------------------------------------------------*/


/*------------------------------------------------------------------|
 |               Single element functions (BEGIN)                   |
 |------------------------------------------------------------------*/

int core_oph_dump (oph_stringPtr byte_array, char *result, int encoding)
{
        char *tmp;
	char measure[INT_DIGIT+PRECISION+2];

        tmp = (char*)malloc(byte_array->elemsize*sizeof(char));
	int i = 0;
	int res;

	*result = 0;

	if (encoding)
	{
		memset(result, 0, (size_t)ceil(*byte_array->length * 1.5 + 1));
		core_base64encode(byte_array->content, *byte_array->length, result, (size_t)ceil(*byte_array->length * 1.5));
	}
        else switch(byte_array->type){
                case OPH_DOUBLE:{
			for(i = 0; i < byte_array->numelem; i++){
                        	double *d;
                        	memcpy(tmp, (void*)((byte_array->content)+(i*byte_array->elemsize)), byte_array->elemsize);
                        	d = (double*)tmp;
				res = snprintf(measure, INT_DIGIT+PRECISION+1, "%.*f", PRECISION, *d);
				if (res >= (INT_DIGIT+PRECISION+1)) res = snprintf(measure, INT_DIGIT+PRECISION+1, "%.*e", INT_DIGIT, *d);
				if(res < 0 || res >= (INT_DIGIT+PRECISION+1)){
					pmesg(1, __FILE__, __LINE__, "Error forming measure string\n");
					free(tmp);
					tmp = NULL;
					return -1;
				}
				strcat(result, measure);
				if(i != byte_array->numelem -1)
					strcat(result, ", ");
			}
                        break;
                }
                case OPH_FLOAT:{
			for(i = 0; i < byte_array->numelem; i++){
                        	float *d;
                        	memcpy(tmp, (void*)((byte_array->content)+(i*byte_array->elemsize)), byte_array->elemsize);
                        	d = (float*)tmp;
				res = snprintf(measure, INT_DIGIT+PRECISION+1, "%.*f", PRECISION, *d);
				if (res >= (INT_DIGIT+PRECISION+1)) res = snprintf(measure, INT_DIGIT+PRECISION+1, "%.*e", INT_DIGIT, *d);
				if(res < 0 || res >= (INT_DIGIT+PRECISION+1)){
					pmesg(1, __FILE__, __LINE__, "Error forming measure string\n");
					free(tmp);
					tmp = NULL;
					return -1;
				}
				strcat(result, measure);
				if(i != byte_array->numelem -1)
					strcat(result, ", ");
			}
                        break;
                }
                case OPH_INT:{
			for(i = 0; i < byte_array->numelem; i++){
                        	int *d;
                        	memcpy(tmp, (void*)((byte_array->content)+(i*byte_array->elemsize)), byte_array->elemsize);
                        	d = (int*)tmp;	
	                	res = snprintf(measure, INT_DIGIT+PRECISION+1, "%d", *d);
				if(res < 0 || res >= (INT_DIGIT+PRECISION+1)){
					pmesg(1, __FILE__, __LINE__, "Error forming measure string\n");
					free(tmp);
					tmp = NULL;
					return -1;
				}
				strcat(result, measure);
				if(i != byte_array->numelem -1)
					strcat(result, ", ");
			}
                        break;
                }
                case OPH_SHORT:{
			for(i = 0; i < byte_array->numelem; i++){
                        	short *d;
                        	memcpy(tmp, (void*)((byte_array->content)+(i*byte_array->elemsize)), byte_array->elemsize);
                        	d = (short*)tmp;
				res = snprintf(measure, INT_DIGIT+PRECISION+1, "%d", *d);
				if(res < 0 || res >= (INT_DIGIT+PRECISION+1)){
					pmesg(1, __FILE__, __LINE__, "Error forming measure string\n");
					free(tmp);
					tmp = NULL;
					return -1;
				}
				strcat(result, measure);
				if(i != byte_array->numelem -1)
					strcat(result, ", ");
			}
                        break;
                }
                case OPH_BYTE:{
			for(i = 0; i < byte_array->numelem; i++){
                        	char *d;
                        	memcpy(tmp, (void*)((byte_array->content)+(i*byte_array->elemsize)), byte_array->elemsize);
                        	d = (char*)tmp;
				res = snprintf(measure, INT_DIGIT+PRECISION+1, "%d", *d);
				if(res < 0 || res >= (INT_DIGIT+PRECISION+1)){
					pmesg(1, __FILE__, __LINE__, "Error forming measure string\n");
					free(tmp);
					tmp = NULL;
					return -1;
				}
				strcat(result, measure);
				if(i != byte_array->numelem -1)
					strcat(result, ", ");
			}
                        break;
                }
		case OPH_LONG:{
			for(i = 0; i < byte_array->numelem; i++){
                        	long long *d;
                        	memcpy(tmp, (void*)((byte_array->content)+(i*byte_array->elemsize)), byte_array->elemsize);
                        	d = (long long*)tmp;
				res = snprintf(measure, INT_DIGIT+PRECISION+1, "%lld", *d);
				if(res < 0 || res >= (INT_DIGIT+PRECISION+1)){
					pmesg(1, __FILE__, __LINE__, "Error forming measure string\n");
					free(tmp);
					tmp = NULL;
					return -1;
				}
				strcat(result, measure);
				if(i != byte_array->numelem -1)
					strcat(result, ", ");
			}
                        break;
                }
                case OPH_COMPLEX_DOUBLE:{
			for(i = 0; i < byte_array->numelem; i++){
                        	double *d;
                        	memcpy(tmp, (void*)((byte_array->content)+(i*byte_array->elemsize)), byte_array->elemsize);
                        	d = (double*)tmp;
                        	strcat(result, "(");
				res = snprintf(measure, INT_DIGIT+PRECISION+1, "%.*f", PRECISION, *d);
				if (res >= (INT_DIGIT+PRECISION+1)) res = snprintf(measure, INT_DIGIT+PRECISION+1, "%.*e", INT_DIGIT, *d);
				if(res < 0 || res >= (INT_DIGIT+PRECISION+1)){
					pmesg(1, __FILE__, __LINE__, "Error forming measure string\n");
					free(tmp);
					tmp = NULL;
					return -1;
				}
				strcat(result, measure);
				strcat(result, ")+(");
				d++;
				res = snprintf(measure, INT_DIGIT+PRECISION+1, "%.*f", PRECISION, *d);
				if (res >= (INT_DIGIT+PRECISION+1)) res = snprintf(measure, INT_DIGIT+PRECISION+1, "%.*e", INT_DIGIT, *d);
				if(res < 0 || res >= (INT_DIGIT+PRECISION+1)){
					pmesg(1, __FILE__, __LINE__, "Error forming measure string\n");
					free(tmp);
					tmp = NULL;
					return -1;
				}
				strcat(result, measure);
				strcat(result, "i)");
				if(i != byte_array->numelem -1)
					strcat(result, ", ");
			}
                        break;
                }
                case OPH_COMPLEX_FLOAT:{
			for(i = 0; i < byte_array->numelem; i++){
                        	float *d;
                        	memcpy(tmp, (void*)((byte_array->content)+(i*byte_array->elemsize)), byte_array->elemsize);
                        	d = (float*)tmp;
                        	strcat(result, "(");
				res = snprintf(measure, INT_DIGIT+PRECISION+1, "%.*f", PRECISION, *d);
				if (res >= (INT_DIGIT+PRECISION+1)) res = snprintf(measure, INT_DIGIT+PRECISION+1, "%.*e", INT_DIGIT, *d);
				if(res < 0 || res >= (INT_DIGIT+PRECISION+1)){
					pmesg(1, __FILE__, __LINE__, "Error forming measure string\n");
					free(tmp);
					tmp = NULL;
					return -1;
				}
				strcat(result, measure);
				strcat(result, ")+(");
				d++;
				res = snprintf(measure, INT_DIGIT+PRECISION+1, "%.*f", PRECISION, *d);
				if (res >= (INT_DIGIT+PRECISION+1)) res = snprintf(measure, INT_DIGIT+PRECISION+1, "%.*e", INT_DIGIT, *d);
				if(res < 0 || res >= (INT_DIGIT+PRECISION+1)){
					pmesg(1, __FILE__, __LINE__, "Error forming measure string\n");
					free(tmp);
					tmp = NULL;
					return -1;
				}
				strcat(result, measure);
				strcat(result, "i)");
				if(i != byte_array->numelem -1)
					strcat(result, ", ");
			}
                        break;
                }
                case OPH_COMPLEX_INT:{
			for(i = 0; i < byte_array->numelem; i++){
                        	int *d;
                        	memcpy(tmp, (void*)((byte_array->content)+(i*byte_array->elemsize)), byte_array->elemsize);
                        	d = (int*)tmp;
                        	strcat(result, "(");
				res = snprintf(measure, INT_DIGIT+PRECISION+1, "%d", *d);
				if(res < 0 || res >= (INT_DIGIT+PRECISION+1)){
					pmesg(1, __FILE__, __LINE__, "Error forming measure string\n");
					free(tmp);
					tmp = NULL;
					return -1;
				}
				strcat(result, measure);
				strcat(result, ")+(");
				d++;
				res = snprintf(measure, INT_DIGIT+PRECISION+1, "%d", *d);
				if(res < 0 || res >= (INT_DIGIT+PRECISION+1)){
					pmesg(1, __FILE__, __LINE__, "Error forming measure string\n");
					free(tmp);
					tmp = NULL;
					return -1;
				}
				strcat(result, measure);
				strcat(result, "i)");
				if(i != byte_array->numelem -1)
					strcat(result, ", ");
			}
                        break;
                }
		case OPH_COMPLEX_LONG:{
			for(i = 0; i < byte_array->numelem; i++){
                        	long long *d;
                        	memcpy(tmp, (void*)((byte_array->content)+(i*byte_array->elemsize)), byte_array->elemsize);
                        	d = (long long*)tmp;
				strcat(result, "(");
				res = snprintf(measure, INT_DIGIT+PRECISION+1, "%lld", *d);
				if(res < 0 || res >= (INT_DIGIT+PRECISION+1)){
					pmesg(1, __FILE__, __LINE__, "Error forming measure string\n");
					free(tmp);
					tmp = NULL;
					return -1;
				}
				strcat(result, measure);
				strcat(result, ")+(");
				d++;
				res = snprintf(measure, INT_DIGIT+PRECISION+1, "%lld", *d);
				if(res < 0 || res >= (INT_DIGIT+PRECISION+1)){
					pmesg(1, __FILE__, __LINE__, "Error forming measure string\n");
					free(tmp);
					tmp = NULL;
					return -1;
				}
				strcat(result, measure);
				strcat(result, "i)");
				if(i != byte_array->numelem -1)
					strcat(result, ", ");
			}
                        break;
                }
                default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        free(tmp);
                        tmp = NULL;
                        return -1;
        }
        free(tmp);
        tmp = NULL;
        return 0;

}

int core_oph_convert (oph_stringPtr byte_array, void *result){
	memcpy(result, (void*)(byte_array->content), byte_array->elemsize);
        return 0;
}

/*------------------------------------------------------------------|
 |               Single element functions (END)                     |
 |------------------------------------------------------------------*/

/*------------------------------------------------------------------|
 |               Array functions (BEGIN)                            |
 |------------------------------------------------------------------*/

int core_oph_type_cast (void *input, char *output, oph_type in_type, oph_type out_type)
{
	switch(out_type){
		case OPH_DOUBLE:{
			switch(in_type){
				case OPH_DOUBLE:{
					*((double*)output) = (*((double*)input));
					return 0;
				}
				case OPH_FLOAT:{
					*((double*)output) = (double)(*((float*)input));
					return 0;
				}
				case OPH_INT:{
					*((double*)output) = (double)(*((int*)input));
					return 0;
				}
				case OPH_SHORT:{
					*((double*)output) = (double)(*((short*)input));
					return 0;
				}
				case OPH_BYTE:{
					*((double*)output) = (double)(*((char*)input));
					return 0;
				}
				case OPH_LONG:{
					*((double*)output) = (double)(*((long long*)input));
					return 0;
				}
				default:
                        		pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        		return -1;
			}
		}
		case OPH_FLOAT:{
			switch(in_type){
				case OPH_DOUBLE:{
					*((float*)output) = (float)(*((double*)input));
					return 0;
				}
				case OPH_FLOAT:{
					*((float*)output) = (*((float*)input));
					return 0;
				}
				case OPH_INT:{
					*((float*)output) = (float)(*((int*)input));
					return 0;
				}
				case OPH_SHORT:{
					*((float*)output) = (float)(*((short*)input));
					return 0;
				}
				case OPH_BYTE:{
					*((float*)output) = (float)(*((char*)input));
					return 0;
				}
				case OPH_LONG:{
					*((float*)output) = (float)(*((long long*)input));
					return 0;
				}
				default:
                        		pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        		return -1;
			}
		}
		case OPH_INT:{
			switch(in_type){
				case OPH_DOUBLE:{
					if (isnan(*((double*)input))) *((int*)output) = 0;
					else *((int*)output) = (*((double*)input));
					return 0;
				}
				case OPH_FLOAT:{
					if (isnan(*((float*)input))) *((int*)output) = 0;
					else *((int*)output) = (*((float*)input));
					return 0;
				}
				case OPH_INT:{
					*((int*)output) = (*((int*)input));
					return 0;
				}
				case OPH_SHORT:{
					*((int*)output) = (int)(*((short*)input));
					return 0;
				}
				case OPH_BYTE:{
					*((int*)output) = (int)(*((char*)input));
					return 0;
				}
				case OPH_LONG:{
					*((int*)output) = (int)(*((long long*)input));
					return 0;
				}
				default:
                        		pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        		return -1;
			}
		}
		case OPH_SHORT:{
			switch(in_type){
				case OPH_DOUBLE:{
					if (isnan(*((double*)input))) *((short*)output) = 0L;
					else *((short*)output) = (*((double*)input));
					return 0;
				}
				case OPH_FLOAT:{
					if (isnan(*((float*)input))) *((short*)output) = 0L;
					else *((short*)output) = (*((float*)input));
					return 0;
				}
				case OPH_INT:{
					*((short*)output) = (short)(*((int*)input));
					return 0;
				}
				case OPH_LONG:{
					*((short*)output) = (short)(*((long long*)input));
					return 0;
				}
				case OPH_BYTE:{
					*((short*)output) = (short)(*((char*)input));
					return 0;
				}
				case OPH_SHORT:{
					*((short*)output) = (*((short*)input));
					return 0;
				}
				default:
                        		pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        		return -1;
			}
		}
		case OPH_BYTE:{
			switch(in_type){
				case OPH_DOUBLE:{
					if (isnan(*((double*)input))) *((char*)output) = 0L;
					else *((char*)output) = (*((double*)input));
					return 0;
				}
				case OPH_FLOAT:{
					if (isnan(*((float*)input))) *((char*)output) = 0L;
					else *((char*)output) = (*((float*)input));
					return 0;
				}
				case OPH_INT:{
					*((char*)output) = (char)(*((int*)input));
					return 0;
				}
				case OPH_SHORT:{
					*((char*)output) = (char)(*((short*)input));
					return 0;
				}
				case OPH_LONG:{
					*((char*)output) = (char)(*((long long*)input));
					return 0;
				}
				case OPH_BYTE:{
					*((char*)output) = (*((char*)input));
					return 0;
				}
				default:
                        		pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        		return -1;
			}
		}
		case OPH_LONG:{
			switch(in_type){
				case OPH_DOUBLE:{
					if (isnan(*((double*)input))) *((long long*)output) = 0L;
					else *((long long*)output) = (*((double*)input));
					return 0;
				}
				case OPH_FLOAT:{
					if (isnan(*((float*)input))) *((long long*)output) = 0L;
					else *((long long*)output) = (*((float*)input));
					return 0;
				}
				case OPH_INT:{
					*((long long*)output) = (long long)(*((int*)input));
					return 0;
				}
				case OPH_SHORT:{
					*((long long*)output) = (long long)(*((short*)input));
					return 0;
				}
				case OPH_BYTE:{
					*((long long*)output) = (long long)(*((char*)input));
					return 0;
				}
				case OPH_LONG:{
					*((long long*)output) = (*((long long*)input));
					return 0;
				}
				default:
                        		pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        		return -1;
			}
		}
		default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
	}
	return 0;
}

int core_oph_count (oph_stringPtr byte_array, char *result)
{
	long long count = 0;

        int i;
        switch(byte_array->type)
	{
                case OPH_DOUBLE:{
                        double *d = (double*)byte_array->content;
                        for (i = 0; i < byte_array->numelem; i++, d++) if (!isnan(*d)) count++;
                        break;
                }
                case OPH_FLOAT:{
                        float *d = (float*)byte_array->content;
                        for (i = 0; i < byte_array->numelem; i++, d++) if (!isnan(*d)) count++;
                        break;
                }
		case OPH_BYTE:
		case OPH_SHORT:
                case OPH_INT:
		case OPH_LONG:
                        count = byte_array->numelem;
                        break;
                default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
        }
	memcpy(result, (void*)(&count), core_sizeof(OPH_LONG));

        return 0;
}
int core_oph_count_multi (oph_multistring* byte_array, oph_multistring *result)
{
	long long count;

        int i,j;
	char *in_string = byte_array->content, *current, *out_string = result->content;
	for(j = 0; j < byte_array->num_measure; j++)
	{
		count = 0;
		current=in_string;
        	switch(byte_array->type[j])
		{
                	case OPH_DOUBLE:
			{
                        	double *d;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (double*)current;
					if (!isnan(*d)) count++;
                        	}
				if (core_oph_type_cast((void*)(&count), out_string, OPH_LONG, result->type[j])) return -1;
				break;
                	}
                	case OPH_FLOAT:{
                        	float *d;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (float*)current;
					if (!isnan(*d)) count++;
                        	}
				if (core_oph_type_cast((void*)(&count), out_string, OPH_LONG, result->type[j])) return -1;
				break;
                	}
			case OPH_BYTE:
			case OPH_SHORT:
		        case OPH_INT:
			case OPH_LONG:
				count = byte_array->numelem;
                        	if (core_oph_type_cast((void*)(&count), out_string, OPH_LONG, result->type[j])) return -1;
                        	break;
        	        default:
                	        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        	return -1;
	        }
		in_string+=byte_array->elemsize[j];
		out_string+=result->elemsize[j];
	}

        return 0;
}

int core_oph_max (oph_stringPtr byte_array, char *result)
{
        int i;
        switch(byte_array->type)
	{
                case OPH_DOUBLE:{
                        double *d = (double*)byte_array->content, *max = 0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (!isnan(*d))
				{
					if (max)
					{
						if (*d > *max) max = d;
					}
					else max = d;
				}
                        }
			if (max) memcpy(result, max, byte_array->elemsize);
			else
			{
				double value=NAN;
				memcpy(result, &value, byte_array->elemsize);
			}
                        break;
                }
                case OPH_FLOAT:{
                        float *d = (float*)byte_array->content, *max = 0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (!isnan(*d))
				{
					if (max)
					{
						if (*d > *max) max = d;
					}
					else max = d;
				}
                        }
			if (max) memcpy(result, max, byte_array->elemsize);
			else
			{
				float value=NAN;
				memcpy(result, &value, byte_array->elemsize);
			}
                        break;
                }
                case OPH_INT:{
                        int *d = (int*)byte_array->content, *max = 0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (max)
				{
					if (*d > *max) max = d;
				}
				else max = d;
                        }
			if (max) memcpy(result, max, byte_array->elemsize);
			else
			{
				pmesg(1, __FILE__, __LINE__, "Error in evaluating max value\n");
				return -1;
			}
                        break;
                }
                case OPH_SHORT:{
                        short *d = (short*)byte_array->content, *max = 0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (max)
				{
					if (*d > *max) max = d;
				}
				else max = d;
                        }
			if (max) memcpy(result, max, byte_array->elemsize);
			else
			{
				pmesg(1, __FILE__, __LINE__, "Error in evaluating max value\n");
				return -1;
			}
                        break;
                }
                case OPH_BYTE:{
                        char *d = (char*)byte_array->content, *max = 0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (max)
				{
					if (*d > *max) max = d;
				}
				else max = d;
                        }
			if (max) memcpy(result, max, byte_array->elemsize);
			else
			{
				pmesg(1, __FILE__, __LINE__, "Error in evaluating max value\n");
				return -1;
			}
                        break;
                }
		case OPH_LONG:{
                        long long *d = (long long*)byte_array->content, *max = 0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (max)
				{
					if (*d > *max) max = d;
				}
				else max = d;
                        }
			if (max) memcpy(result, max, byte_array->elemsize);
			else
			{
				pmesg(1, __FILE__, __LINE__, "Error in evaluating max value\n");
				return -1;
			}
                        break;
                }
                default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
        }
        return 0;
}
int core_oph_max_multi (oph_multistring* byte_array, oph_multistring *result)
{
        int i,j;
	char *in_string = byte_array->content, *current, *out_string = result->content;
	for(j = 0; j < byte_array->num_measure; j++)
	{
		current=in_string;
        	switch(byte_array->type[j])
		{
                	case OPH_DOUBLE:
			{
                        	double *d, *max = NULL;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (double*)current;
					if (!isnan(*d))
					{
						if (max)
						{
							if (*d > *max) max = d;
						}
						else max = d;
					}
                        	}
				if (max){
					//Cast of the output
					if(core_oph_type_cast((void*)max, out_string, byte_array->type[j], result->type[j])) return -1;
				}
				else
				{
					double value=NAN;
					if(core_oph_type_cast((void*)(&value), out_string, byte_array->type[j], result->type[j])) return -1;
				}
                        	break;
                	}
                	case OPH_FLOAT:{
                        	float *d, *max = NULL;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (float*)current;
					if (!isnan(*d))
					{
						if (max)
						{
							if (*d > *max) max = d;
						}
						else max = d;
					}
                        	}
				if (max){
					//Cast of the output
					if(core_oph_type_cast((void*)max, out_string, byte_array->type[j], result->type[j])) return -1;
				}
				else
				{
					float value=NAN;
					if(core_oph_type_cast((void*)(&value), out_string, byte_array->type[j], result->type[j])) return -1;
				}
                        	break;
                	}
                	case OPH_INT:{
                        	int *d, *max = NULL;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (int*)current;
					if (max)
					{
						if (*d > *max) max = d;
					}
					else max = d;
                        	}
				if (!max || core_oph_type_cast((void*)max, out_string, byte_array->type[j], result->type[j]))
				{
					pmesg(1, __FILE__, __LINE__, "Error in evaluating max value\n");
					return -1;
				}
                        	break;
        	        }
                	case OPH_SHORT:{
                        	short *d, *max = NULL;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (short*)current;
					if (max)
					{
						if (*d > *max) max = d;
					}
					else max = d;
                        	}
				if (!max || core_oph_type_cast((void*)max, out_string, byte_array->type[j], result->type[j]))
				{
					pmesg(1, __FILE__, __LINE__, "Error in evaluating max value\n");
					return -1;
				}
                        	break;
        	        }
                	case OPH_BYTE:{
                        	char *d, *max = NULL;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (char*)current;
					if (max)
					{
						if (*d > *max) max = d;
					}
					else max = d;
                        	}
				if (!max || core_oph_type_cast((void*)max, out_string, byte_array->type[j], result->type[j]))
				{
					pmesg(1, __FILE__, __LINE__, "Error in evaluating max value\n");
					return -1;
				}
                        	break;
        	        }
			case OPH_LONG:{
                        	long long *d, *max = NULL;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (long long*)current;
					if (max)
					{
						if (*d > *max) max = d;
					}
					else max = d;
                        	}
				if (!max || core_oph_type_cast((void*)max, out_string, byte_array->type[j], result->type[j]))
				{
					pmesg(1, __FILE__, __LINE__, "Error in evaluating max value\n");
					return -1;
				}
                        	break;
	                }
        	        default:
                	        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        	return -1;
	        }
		in_string+=byte_array->elemsize[j];
		out_string+=result->elemsize[j];
	}
        return 0;
}

int core_oph_min (oph_stringPtr byte_array, char *result)
{
        int i;
        switch(byte_array->type)
	{
                case OPH_DOUBLE:{
                        double *d = (double*)byte_array->content, *min = 0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (!isnan(*d))
				{
					if (min)
					{
						if (*d < *min) min = d;
					}
					else min = d;
				}
                        }
			if (min) memcpy(result, min, byte_array->elemsize);
			else
			{
				double value=NAN;
				memcpy(result, &value, byte_array->elemsize);
			}
                        break;
                }
                case OPH_FLOAT:{
                        float *d = (float*)byte_array->content, *min = 0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (!isnan(*d))
				{
					if (min)
					{
						if (*d < *min) min = d;
					}
					else min = d;
				}
                        }
			if (min) memcpy(result, min, byte_array->elemsize);
			else
			{
				float value=NAN;
				memcpy(result, &value, byte_array->elemsize);
			}
                        break;
                }
                case OPH_INT:{
                        int *d = (int*)byte_array->content, *min = 0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (!isnan(*d))
				{
					if (min)
					{
						if (*d < *min) min = d;
					}
					else min = d;
				}
                        }
			if (min) memcpy(result, min, byte_array->elemsize);
			else
			{
				pmesg(1, __FILE__, __LINE__, "Error in evaluating min value\n");
				return -1;
			}
                        break;
                }
		case OPH_SHORT:{
                        short *d = (short*)byte_array->content, *min = 0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (!isnan(*d))
				{
					if (min)
					{
						if (*d < *min) min = d;
					}
					else min = d;
				}
                        }
			if (min) memcpy(result, min, byte_array->elemsize);
			else
			{
				pmesg(1, __FILE__, __LINE__, "Error in evaluating min value\n");
				return -1;
			}
                        break;
                }
		case OPH_BYTE:{
                        char *d = (char*)byte_array->content, *min = 0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (!isnan(*d))
				{
					if (min)
					{
						if (*d < *min) min = d;
					}
					else min = d;
				}
                        }
			if (min) memcpy(result, min, byte_array->elemsize);
			else
			{
				pmesg(1, __FILE__, __LINE__, "Error in evaluating min value\n");
				return -1;
			}
                        break;
                }
		case OPH_LONG:{
                        long long *d = (long long*)byte_array->content, *min = 0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (!isnan(*d))
				{
					if (min)
					{
						if (*d < *min) min = d;
					}
					else min = d;
				}
                        }
			if (min) memcpy(result, min, byte_array->elemsize);
			else
			{
				pmesg(1, __FILE__, __LINE__, "Error in evaluating min value\n");
				return -1;
			}
                        break;
                }
                default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
        }
        return 0;
}
int core_oph_min_multi (oph_multistring* byte_array, oph_multistring *result)
{
        int i,j;
	char *in_string = byte_array->content, *current, *out_string = result->content;
	for(j = 0; j < byte_array->num_measure; j++)
	{
		current=in_string;
        	switch(byte_array->type[j])
		{
                	case OPH_DOUBLE:
			{
                        	double *d, *min = NULL;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (double*)current;
					if (!isnan(*d))
					{
						if (min)
						{
							if (*d < *min) min = d;
						}
						else min = d;
					}
                        	}
				if (min){
					//Cast of the output
					if(core_oph_type_cast((void*)min, out_string, byte_array->type[j], result->type[j])) return -1;
				}
				else
				{
					double value=NAN;
					if(core_oph_type_cast((void*)(&value), out_string, byte_array->type[j], result->type[j])) return -1;
				}
                        	break;
                	}
                	case OPH_FLOAT:{
                        	float *d, *min = NULL;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (float*)current;
					if (!isnan(*d))
					{
						if (min)
						{
							if (*d < *min) min = d;
						}
						else min = d;
					}
                        	}
				if (min){
					//Cast of the output
					if(core_oph_type_cast((void*)min, out_string, byte_array->type[j], result->type[j])) return -1;
				}
				else
				{
					float value=NAN;
					if(core_oph_type_cast((void*)(&value), out_string, byte_array->type[j], result->type[j])) return -1;
				}
                        	break;
                	}
                	case OPH_INT:{
                        	int *d, *min = NULL;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (int*)current;
					if (min)
					{
						if (*d < *min) min = d;
					}
					else min = d;
                        	}
				if (!min || core_oph_type_cast((void*)min, out_string, byte_array->type[j], result->type[j]))
				{
					pmesg(1, __FILE__, __LINE__, "Error in evaluating min value\n");
					return -1;
				}
                        	break;
        	        }
                	case OPH_SHORT:{
                        	short *d, *min = NULL;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (short*)current;
					if (min)
					{
						if (*d < *min) min = d;
					}
					else min = d;
                        	}
				if (!min || core_oph_type_cast((void*)min, out_string, byte_array->type[j], result->type[j]))
				{
					pmesg(1, __FILE__, __LINE__, "Error in evaluating min value\n");
					return -1;
				}
                        	break;
        	        }
                	case OPH_BYTE:{
                        	char *d, *min = NULL;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (char*)current;
					if (min)
					{
						if (*d < *min) min = d;
					}
					else min = d;
                        	}
				if (!min || core_oph_type_cast((void*)min, out_string, byte_array->type[j], result->type[j]))
				{
					pmesg(1, __FILE__, __LINE__, "Error in evaluating min value\n");
					return -1;
				}
                        	break;
        	        }
			case OPH_LONG:{
                        	long long *d, *min = NULL;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (long long*)current;
					if (min)
					{
						if (*d < *min) min = d;
					}
					else min = d;
                        	}
				if (!min || core_oph_type_cast((void*)min, out_string, byte_array->type[j], result->type[j]))
				{
					pmesg(1, __FILE__, __LINE__, "Error in evaluating min value\n");
					return -1;
				}
                        	break;
	                }
        	        default:
                	        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        	return -1;
	        }
		in_string+=byte_array->elemsize[j];
		out_string+=result->elemsize[j];
	}
        return 0;
}

int core_oph_sum (oph_stringPtr byte_array, char *result)
{
	int i;
        switch(byte_array->type)
	{
                case OPH_DOUBLE:{
			char nan = 1;
                        double *d = (double*)byte_array->content, sum = 0.0;
                        for (i = 0; i < byte_array->numelem; i++, d++) if (!isnan(*d)) { sum = sum + *d; nan = 0; }
			if (nan) sum = NAN;
			memcpy(result, (void*)(&sum), byte_array->elemsize);
                        break;
                }
                case OPH_FLOAT:{
			char nan = 1;
                        float *d = (float*)byte_array->content, sum = 0.0;
                        for (i = 0; i < byte_array->numelem; i++, d++) if (!isnan(*d)) { sum = sum + *d; nan = 0; }
			if (nan) sum = NAN;
			memcpy(result, (void*)(&sum), byte_array->elemsize);
                        break;
                }
                case OPH_INT:{
                        int *d = (int*)byte_array->content, sum = 0;
                        for (i = 0; i < byte_array->numelem; i++, d++) sum = sum + *d;
			memcpy(result, (void*)(&sum), byte_array->elemsize);
                        break;
                }
                case OPH_SHORT:{
                        short *d = (short*)byte_array->content, sum = 0;
                        for (i = 0; i < byte_array->numelem; i++, d++) sum = sum + *d;
			memcpy(result, (void*)(&sum), byte_array->elemsize);
                        break;
                }
                case OPH_BYTE:{
                        char *d = (char*)byte_array->content, sum = 0;
                        for (i = 0; i < byte_array->numelem; i++, d++) sum = sum + *d;
			memcpy(result, (void*)(&sum), byte_array->elemsize);
                        break;
                }
		case OPH_LONG:{
                        long long *d = (long long*)byte_array->content, sum = 0;
                        for (i = 0; i < byte_array->numelem; i++, d++) sum = sum + *d;
			memcpy(result, (void*)(&sum), byte_array->elemsize);
                        break;
                }
                default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
        }
        return 0;
}
int core_oph_sum_multi (oph_multistring* byte_array, oph_multistring *result)
{
        int i,j;
	char *in_string = byte_array->content, *current, *out_string = result->content;
	for(j = 0; j < byte_array->num_measure; j++)
	{
		current=in_string;
        	switch(byte_array->type[j])
		{
                	case OPH_DOUBLE:
			{
				char nan = 1;
                        	double sum = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize) if (!isnan(*(double*)current)) { sum += *(double*)current; nan = 0; }
				if (nan) sum = NAN;
				if(core_oph_type_cast((void*)(&sum), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
                	}
                	case OPH_FLOAT:
			{
				char nan = 1;
                        	float sum = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize) if (!isnan(*(float*)current)) { sum += *(float*)current; nan = 0; }
				if (nan) sum = NAN;
				if(core_oph_type_cast((void*)(&sum), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
                	}
                	case OPH_INT:{
                        	int sum = 0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize) sum += *(int*)current;
				if(core_oph_type_cast((void*)(&sum), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
        	        }
                	case OPH_SHORT:{
                        	short sum = 0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize) sum += *(short*)current;
				if(core_oph_type_cast((void*)(&sum), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
        	        }
                	case OPH_BYTE:{
                        	char sum = 0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize) sum += *(char*)current;
				if(core_oph_type_cast((void*)(&sum), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
        	        }
			case OPH_LONG:{
                        	long long sum = 0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize) sum += *(long long*)current;
				if(core_oph_type_cast((void*)(&sum), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
	                }
        	        default:
                	        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        	return -1;
	        }
		in_string+=byte_array->elemsize[j];
		out_string+=result->elemsize[j];
	}
        return 0;
}

int core_oph_avg (oph_stringPtr byte_array, char *result)
{
        int i;
	unsigned long numelem=0;
        switch(byte_array->type){
                case OPH_DOUBLE:{
                        double *d = (double*)byte_array->content, sum = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (!isnan(*d))
				{
					sum = sum + *d;
					numelem++;
				}
                        }
			if (numelem) sum /= (double)numelem;
			else sum = NAN;
			memcpy(result, (void*)(&sum), byte_array->elemsize);
                        break;
                }
                case OPH_FLOAT:{
                        float *d = (float*)byte_array->content, sum = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (!isnan(*d))
				{
					sum = sum + *d;
					numelem++;
				}
                        }
			if (numelem) sum /= (float)numelem;
			else sum = NAN;
			memcpy(result, (void*)(&sum), byte_array->elemsize);
                        break;
                }
                case OPH_INT:{
			//The average is a float number
                        int *d = (int*)byte_array->content;
			float sum = 0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum = sum + *d;
			sum /= (float)byte_array->numelem;
			memcpy(result, (void*)(&sum), core_sizeof(OPH_FLOAT));
                        break;
                }
                case OPH_SHORT:{
			//The average is a float number
                        short *d = (short*)byte_array->content;
			float sum = 0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum = sum + *d;
			sum /= (float)byte_array->numelem;
			memcpy(result, (void*)(&sum), core_sizeof(OPH_FLOAT));
                        break;
                }
                case OPH_BYTE:{
			//The average is a float number
                        char *d = (char*)byte_array->content;
			float sum = 0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum = sum + *d;
			sum /= (float)byte_array->numelem;
			memcpy(result, (void*)(&sum), core_sizeof(OPH_FLOAT));
                        break;
                }
		case OPH_LONG:{
			//The average is a float number
                        long long *d = (long long*)byte_array->content;
			double sum = 0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum = sum + *d;
			sum /= (double)byte_array->numelem;
			memcpy(result, (void*)(&sum), core_sizeof(OPH_DOUBLE));
                        break;
                }
                default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
        }
        return 0;
}
int core_oph_avg_multi (oph_multistring* byte_array, oph_multistring *result)
{
        int i,j;
	unsigned long numelem;
	char *in_string = byte_array->content, *current, *out_string = result->content;
	for(j = 0; j < byte_array->num_measure; j++)
	{
		numelem=0;
		current=in_string;
        	switch(byte_array->type[j])
		{
                	case OPH_DOUBLE:
			{
                        	double *d, sum = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (double*)current;
					if (!isnan(*d))
					{
						sum += *d;
						numelem++;
					}
                        	}
				if (numelem) sum /= (double)numelem;
				else sum = NAN;
				if(core_oph_type_cast((void*)(&sum), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
                	}
                	case OPH_FLOAT:{
                        	float *d, sum = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (float*)current;
					if (!isnan(*d))
					{
						sum += *d;
						numelem++;
					}
                        	}
				if (numelem) sum /= (float)numelem;
				else sum = NAN;
				if(core_oph_type_cast((void*)(&sum), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
                	}
                	case OPH_INT:{
                        	float sum = 0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize) sum += *(int*)current;
				sum /= (float)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
                	case OPH_SHORT:{
                        	float sum = 0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize) sum += *(short*)current;
				sum /= (float)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
			case OPH_BYTE:{
                        	float sum = 0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize) sum += *(char*)current;
				sum /= (float)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
	                }
			case OPH_LONG:{
                        	double sum = 0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize) sum += *(long long*)current;
				sum /= (double)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum), out_string, OPH_DOUBLE, result->type[j])) return -1;
                        	break;
	                }
        	        default:
                	        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        	return -1;
	        }
		in_string+=byte_array->elemsize[j];
		out_string+=result->elemsize[j];
	}
        return 0;
}

int core_oph_std (oph_stringPtr byte_array, char *result)
{
	if (core_oph_var(byte_array,result)) return -1;

	switch(byte_array->type)
	{
		case OPH_LONG:
                case OPH_DOUBLE:
		{
                        double *d = (double*)result;
			double stdev = sqrt(fabs(*d));
			memcpy(result, (void*)(&stdev), core_sizeof(OPH_DOUBLE));
                        break;
                }
		case OPH_SHORT:
                case OPH_BYTE:
		case OPH_INT:
                case OPH_FLOAT:
		{
                        float *d = (float*)result;
			float stdev = sqrtf(fabsf(*d));
			memcpy(result, (void*)(&stdev), core_sizeof(OPH_FLOAT));
                        break;
                }
                default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
        }

	return 0;
}
int core_oph_std_multi (oph_multistring* byte_array, oph_multistring *result)
{
        int i,j;
	unsigned long numelem;
	char *in_string = byte_array->content, *current, *out_string = result->content;
	for(j = 0; j < byte_array->num_measure; j++)
	{
		numelem=0;
		current = in_string;
        	switch(byte_array->type[j])
		{
                	case OPH_DOUBLE:
			{
                        	double *d, sum = 0.0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (double*)current;
					if (!isnan(*d))
					{
						sum += *d;
						sum2 += (*d)*(*d);
						numelem++;
					}
                        	}
				if (!numelem) sum2 = NAN;
				else
				{
					sum /= numelem;
					sum2 /= numelem;
					sum2 -= sum*sum;
					if (numelem>1) sum2 *= numelem/(numelem-1.0);
					sum2 = sqrt(fabs(sum2));
				}
				if(core_oph_type_cast((void*)(&sum2), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
                	}
                	case OPH_FLOAT:{
                        	float *d, sum = 0.0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (float*)current;
					if (!isnan(*d))
					{
						sum += *d;
						sum2 += (*d)*(*d);
						numelem++;
					}
                        	}
				if (!numelem) sum2 = NAN;
				else
				{
					sum /= numelem;
					sum2 /= numelem;
					sum2 -= sum*sum;
					if (numelem>1) sum2 *= numelem/(numelem-1.0);
					sum2 = sqrtf(fabsf(sum2));
				}
				if(core_oph_type_cast((void*)(&sum2), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
                	}
                	case OPH_INT:{
				int *d;
				float sum = 0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (int*)current;
					sum += *d;
					sum2 += ((float)(*d))*((float)(*d));
				}
				sum /= (float)byte_array->numelem;
				sum2 /= (float)byte_array->numelem;
				sum2 -= sum*sum;
				if (byte_array->numelem>1) sum2 *= byte_array->numelem/(byte_array->numelem-1.0);
				sum2 = sqrtf(fabsf(sum2));
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
                	case OPH_SHORT:{
				short *d;
				float sum = 0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (short*)current;
					sum += *d;
					sum2 += ((float)(*d))*((float)(*d));
				}
				sum /= (float)byte_array->numelem;
				sum2 /= (float)byte_array->numelem;
				sum2 -= sum*sum;
				if (byte_array->numelem>1) sum2 *= byte_array->numelem/(byte_array->numelem-1.0);
				sum2 = sqrtf(fabsf(sum2));
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
                	case OPH_BYTE:{
				char *d;
				float sum = 0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (char*)current;
					sum += *d;
					sum2 += ((float)(*d))*((float)(*d));
				}
				sum /= (float)byte_array->numelem;
				sum2 /= (float)byte_array->numelem;
				sum2 -= sum*sum;
				if (byte_array->numelem>1) sum2 *= byte_array->numelem/(byte_array->numelem-1.0);
				sum2 = sqrtf(fabsf(sum2));
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
			case OPH_LONG:{
				long long *d;
				double sum = 0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++)
				{
					d = (long long*)current;
					sum += *d;
					sum2 += ((double)(*d))*((double)(*d));
				}
				sum /= (double)byte_array->numelem;
				sum2 /= (double)byte_array->numelem;
				sum2 -= sum*sum;
				if (byte_array->numelem>1) sum2 *= byte_array->numelem/(byte_array->numelem-1.0);
				sum2 = sqrt(fabs(sum2));
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_DOUBLE, result->type[j])) return -1;
                        	break;
	                }
        	        default:
                	        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        	return -1;
	        }
		in_string+=byte_array->elemsize[j];
		out_string+=result->elemsize[j];
	}
        return 0;
}

int core_oph_var (oph_stringPtr byte_array, char *result)
{
        int i;
	unsigned long numelem=0;

        switch(byte_array->type){
                case OPH_DOUBLE:{
                        double *d = (double*)byte_array->content, sum = 0.0, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++)
			{
                                if (!isnan(*d))
				{
					sum += *d;
					sum2 += (*d)*(*d);
					numelem++;
				}
                        }
			if (!numelem) sum2 = NAN;
			else
			{
				sum /= numelem;
				sum2 /= numelem;
				sum2 -= sum*sum;
				if (numelem>1) sum2 *= numelem/(numelem-1.0);
			}
			memcpy(result, (void*)(&sum2), byte_array->elemsize);
                        break;
                }
                case OPH_FLOAT:{
                        float *d = (float*)byte_array->content, sum = 0.0, sum2 = 0.0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
                                if (!isnan(*d))
				{
					sum += *d;
					sum2 += (*d)*(*d);
					numelem++;
				}
                        }
			if (!numelem) sum2 = NAN;
			else
			{
				sum /= numelem;
				sum2 /= numelem;
				sum2 -= sum*sum;
				if (numelem>1) sum2 *= numelem/(numelem-1.0);
			}
			memcpy(result, (void*)(&sum2), byte_array->elemsize);
                        break;
                }
                case OPH_INT:{
			//The standard deviation is a float number
                        int *d = (int*)byte_array->content;
			float sum = 0, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++)
			{
				sum += *d;
				sum2 += ((float)(*d))*((float)(*d));
			}
			sum /= (float)byte_array->numelem;
			sum2 /= (float)byte_array->numelem;
			sum2 -= sum*sum;
			if (byte_array->numelem>1) sum2 *= byte_array->numelem/(byte_array->numelem-1.0);
			memcpy(result, (void*)(&sum2), core_sizeof(OPH_FLOAT));
                        break;
                }
                case OPH_SHORT:{
			//The standard deviation is a float number
                        short *d = (short*)byte_array->content;
			float sum = 0, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++)
			{
				sum += *d;
				sum2 += ((float)(*d))*((float)(*d));
			}
			sum /= (float)byte_array->numelem;
			sum2 /= (float)byte_array->numelem;
			sum2 -= sum*sum;
			if (byte_array->numelem>1) sum2 *= byte_array->numelem/(byte_array->numelem-1.0);
			memcpy(result, (void*)(&sum2), core_sizeof(OPH_FLOAT));
                        break;
                }
                case OPH_BYTE:{
			//The standard deviation is a float number
                        char *d = (char*)byte_array->content;
			float sum = 0, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++)
			{
				sum += *d;
				sum2 += ((float)(*d))*((float)(*d));
			}
			sum /= (float)byte_array->numelem;
			sum2 /= (float)byte_array->numelem;
			sum2 -= sum*sum;
			if (byte_array->numelem>1) sum2 *= byte_array->numelem/(byte_array->numelem-1.0);
			memcpy(result, (void*)(&sum2), core_sizeof(OPH_FLOAT));
                        break;
                }
		case OPH_LONG:{
			//The standard deviation is a double number
                        long long *d = (long long*)byte_array->content;
			double sum = 0, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++)
			{
				sum += *d;
				sum2 += ((double)(*d))*((double)(*d));
			}
			sum /= (double)byte_array->numelem;
			sum2 /= (double)byte_array->numelem;
			sum2 -= sum*sum;
			if (byte_array->numelem>1) sum2 *= byte_array->numelem/(byte_array->numelem-1.0);
                        memcpy(result, (void*)(&sum2), core_sizeof(OPH_DOUBLE));
                        break;
                }
                default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
        }
        return 0;
}
int core_oph_var_multi (oph_multistring* byte_array, oph_multistring *result)
{
        int i,j;
	unsigned long numelem;
	char *in_string = byte_array->content, *current, *out_string = result->content;
	for(j = 0; j < byte_array->num_measure; j++)
	{
		numelem=0;
		current = in_string;
        	switch(byte_array->type[j])
		{
                	case OPH_DOUBLE:
			{
                        	double *d, sum = 0.0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (double*)current;
					if (!isnan(*d))
					{
						sum += *d;
						sum2 += (*d)*(*d);
						numelem++;
					}
                        	}
				if (!numelem) sum2 = NAN;
				else
				{
					sum /= numelem;
					sum2 /= numelem;
					sum2 -= sum*sum;
					if (numelem>1) sum2 *= numelem/(numelem-1.0);
				}
				if(core_oph_type_cast((void*)(&sum2), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
                	}
                	case OPH_FLOAT:{
                        	float *d, sum = 0.0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (float*)current;
					if (!isnan(*d))
					{
						sum += *d;
						sum2 += (*d)*(*d);
						numelem++;
					}
                        	}
				if (!numelem) sum2 = NAN;
				else
				{
					sum /= numelem;
					sum2 /= numelem;
					sum2 -= sum*sum;
					if (numelem>1) sum2 *= numelem/(numelem-1.0);
				}
				if(core_oph_type_cast((void*)(&sum2), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
                	}
                	case OPH_INT:{
				int *d;
				float sum = 0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (int*)current;
					sum += *d;
					sum2 += ((float)(*d))*((float)(*d));
				}
				sum /= (float)byte_array->numelem;
				sum2 /= (float)byte_array->numelem;
				sum2 -= sum*sum;
				if (byte_array->numelem>1) sum2 *= byte_array->numelem/(byte_array->numelem-1.0);
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
                	case OPH_SHORT:{
				short *d;
				float sum = 0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (short*)current;
					sum += *d;
					sum2 += ((float)(*d))*((float)(*d));
				}
				sum /= (float)byte_array->numelem;
				sum2 /= (float)byte_array->numelem;
				sum2 -= sum*sum;
				if (byte_array->numelem>1) sum2 *= byte_array->numelem/(byte_array->numelem-1.0);
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
                	case OPH_BYTE:{
				char *d;
				float sum = 0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (char*)current;
					sum += *d;
					sum2 += ((float)(*d))*((float)(*d));
				}
				sum /= (float)byte_array->numelem;
				sum2 /= (float)byte_array->numelem;
				sum2 -= sum*sum;
				if (byte_array->numelem>1) sum2 *= byte_array->numelem/(byte_array->numelem-1.0);
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
			case OPH_LONG:{
				long long *d;
				double sum = 0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++)
				{
					d = (long long*)current;
					sum += *d;
					sum2 += ((double)(*d))*((double)(*d));
				}
				sum /= (double)byte_array->numelem;
				sum2 /= (double)byte_array->numelem;
				sum2 -= sum*sum;
				if (byte_array->numelem>1) sum2 *= byte_array->numelem/(byte_array->numelem-1.0);
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_DOUBLE, result->type[j])) return -1;
                        	break;
	                }
        	        default:
                	        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        	return -1;
	        }
		in_string+=byte_array->elemsize[j];
		out_string+=result->elemsize[j];
	}
        return 0;
}


int core_oph_cmoment (oph_stringPtr byte_array, char *result)
{
	if (!byte_array || (byte_array->param<0.0))
	{
		pmesg(1, __FILE__, __LINE__, "Wrong parameter\n");
		return -1;
	}
        int i;
	unsigned long numelem=0;
        switch(byte_array->type){
                case OPH_DOUBLE:{
                        double *d = (double*)byte_array->content, sum = 0.0, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++)
			{
                                if (!isnan(*d))
				{
					sum += *d;
					numelem++;
				}
                        }
			if (!numelem) sum2 = NAN;
			else
			{
				sum /= numelem;
				d = (double*)byte_array->content;
				for (i = 0; i < byte_array->numelem; i++, d++) if (!isnan(*d)) sum2 += pow((*d)-sum, byte_array->param);
				sum2 /= numelem;
			}
			memcpy(result, (void*)(&sum2), byte_array->elemsize);
                        break;
                }
                case OPH_FLOAT:{
                        float *d = (float*)byte_array->content, sum = 0.0, sum2 = 0.0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
                                if (!isnan(*d))
				{
					sum += *d;
					numelem++;
				}
                        }
			if (!numelem) sum2 = NAN;
			else
			{
				sum /= numelem;
				d = (float*)byte_array->content;
				for (i = 0; i < byte_array->numelem; i++, d++) if (!isnan(*d)) sum2 += powf((*d)-sum, (float)byte_array->param);
				sum2 /= numelem;
			}
			memcpy(result, (void*)(&sum2), byte_array->elemsize);
                        break;
                }
                case OPH_INT:{
			//The standard deviation is a float number
                        int *d = (int*)byte_array->content;
			float sum = 0, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum += *d;
			sum /= (float)byte_array->numelem;
			d = (int*)byte_array->content;
			for (i = 0; i < byte_array->numelem; i++, d++) sum2 += powf((*d)-sum, (float)byte_array->param);
			sum2 /= (float)byte_array->numelem;
			memcpy(result, (void*)(&sum2), core_sizeof(OPH_FLOAT));
                        break;
                }
                case OPH_SHORT:{
			//The standard deviation is a float number
                        short *d = (short*)byte_array->content;
			float sum = 0, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum += *d;
			sum /= (float)byte_array->numelem;
			d = (short*)byte_array->content;
			for (i = 0; i < byte_array->numelem; i++, d++) sum2 += powf((*d)-sum, (float)byte_array->param);
			sum2 /= (float)byte_array->numelem;
			memcpy(result, (void*)(&sum2), core_sizeof(OPH_FLOAT));
                        break;
                }
                case OPH_BYTE:{
			//The standard deviation is a float number
                        char *d = (char*)byte_array->content;
			float sum = 0, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum += *d;
			sum /= (float)byte_array->numelem;
			d = (char*)byte_array->content;
			for (i = 0; i < byte_array->numelem; i++, d++) sum2 += powf((*d)-sum, (float)byte_array->param);
			sum2 /= (float)byte_array->numelem;
			memcpy(result, (void*)(&sum2), core_sizeof(OPH_FLOAT));
                        break;
                }
		case OPH_LONG:{
			//The standard deviation is a double number
                        long long *d = (long long*)byte_array->content;
			double sum = 0, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum += *d;
			sum /= (double)byte_array->numelem;
			d = (long long*)byte_array->content;
			for (i = 0; i < byte_array->numelem; i++, d++) sum2 += pow((*d)-sum, byte_array->param);
			sum2 /= (double)byte_array->numelem;
                        memcpy(result, (void*)(&sum2), core_sizeof(OPH_DOUBLE));
                        break;
                }
                default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
        }
        return 0;
}
int core_oph_cmoment_multi (oph_multistring* byte_array, oph_multistring *result)
{
	if (!byte_array || (byte_array->param<0.0))
	{
		pmesg(1, __FILE__, __LINE__, "Wrong parameter\n");
		return -1;
	}
        int i,j;
	unsigned long numelem;
	char *in_string = byte_array->content, *current, *out_string = result->content;
	for(j = 0; j < byte_array->num_measure; j++)
	{
		numelem=0;
		current = in_string;
        	switch(byte_array->type[j])
		{
                	case OPH_DOUBLE:
			{
                        	double *d, sum = 0.0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (double*)current;
					if (!isnan(*d))
					{
						sum += *d;
						numelem++;
					}
                        	}
				if (!numelem) sum2 = NAN;
				else
				{
					sum /= numelem;
					current = in_string;
					for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
					{
						d = (double*)current;
						if (!isnan(*d)) sum2 += pow((*d)-sum, byte_array->param);
					}
					sum2 /= numelem;
				}
				if(core_oph_type_cast((void*)(&sum2), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
                	}
                	case OPH_FLOAT:{
                        	float *d, sum = 0.0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (float*)current;
					if (!isnan(*d))
					{
						sum += *d;
						numelem++;
					}
                        	}
				if (!numelem) sum2 = NAN;
				else
				{
					sum /= numelem;
					current = in_string;
					for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
					{
						d = (float*)current;
						if (!isnan(*d)) sum2 += powf((*d)-sum, (float)byte_array->param);
					}
					sum2 /= numelem;
				}
				if(core_oph_type_cast((void*)(&sum2), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
                	}
                	case OPH_INT:{
				int *d;
				float sum = 0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (int*)current;
					sum += *d;
				}
				sum /= (float)byte_array->numelem;
				current = in_string;
				for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (int*)current;
					sum2 += powf((*d)-sum, (float)byte_array->param);
				}
				sum2 /= (float)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
                	case OPH_SHORT:{
				short *d;
				float sum = 0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (short*)current;
					sum += *d;
				}
				sum /= (float)byte_array->numelem;
				current = in_string;
				for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (short*)current;
					sum2 += powf((*d)-sum, (float)byte_array->param);
				}
				sum2 /= (float)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
                	case OPH_BYTE:{
				char *d;
				float sum = 0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (char*)current;
					sum += *d;
				}
				sum /= (float)byte_array->numelem;
				current = in_string;
				for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (char*)current;
					sum2 += powf((*d)-sum, (float)byte_array->param);
				}
				sum2 /= (float)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
			case OPH_LONG:{
				long long *d;
				double sum = 0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++)
				{
					d = (long long*)current;
					sum += *d;
				}
				sum /= (double)byte_array->numelem;
				current = in_string;
				for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (long long*)current;
					sum2 += pow((*d)-sum, byte_array->param);
				}
				sum2 /= (double)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_DOUBLE, result->type[j])) return -1;
                        	break;
	                }
        	        default:
                	        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        	return -1;
	        }
		in_string+=byte_array->elemsize[j];
		out_string+=result->elemsize[j];
	}
        return 0;
}


int core_oph_acmoment (oph_stringPtr byte_array, char *result)
{
	if (!byte_array || (byte_array->param<0.0))
	{
		pmesg(1, __FILE__, __LINE__, "Wrong parameter\n");
		return -1;
	}
        int i;
	unsigned long numelem=0;
        switch(byte_array->type){
                case OPH_DOUBLE:{
                        double *d = (double*)byte_array->content, sum = 0.0, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++)
			{
                                if (!isnan(*d))
				{
					sum += *d;
					numelem++;
				}
                        }
			if (!numelem) sum2 = NAN;
			else
			{
				sum /= numelem;
				d = (double*)byte_array->content;
				for (i = 0; i < byte_array->numelem; i++, d++) if (!isnan(*d)) sum2 += pow(abs((*d)-sum), byte_array->param);
				sum2 /= numelem;
			}
			memcpy(result, (void*)(&sum2), byte_array->elemsize);
                        break;
                }
                case OPH_FLOAT:{
                        float *d = (float*)byte_array->content, sum = 0.0, sum2 = 0.0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
                                if (!isnan(*d))
				{
					sum += *d;
					numelem++;
				}
                        }
			if (!numelem) sum2 = NAN;
			else
			{
				sum /= numelem;
				d = (float*)byte_array->content;
				for (i = 0; i < byte_array->numelem; i++, d++) if (!isnan(*d)) sum2 += powf(fabs((*d)-sum), (float)byte_array->param);
				sum2 /= numelem;
			}
			memcpy(result, (void*)(&sum2), byte_array->elemsize);
                        break;
                }
                case OPH_INT:{
			//The standard deviation is a float number
                        int *d = (int*)byte_array->content;
			float sum = 0, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum += *d;
			sum /= (float)byte_array->numelem;
			d = (int*)byte_array->content;
			for (i = 0; i < byte_array->numelem; i++, d++) sum2 += powf(fabs((*d)-sum), (float)byte_array->param);
			sum2 /= (float)byte_array->numelem;
			memcpy(result, (void*)(&sum2), core_sizeof(OPH_FLOAT));
                        break;
                }
                case OPH_SHORT:{
			//The standard deviation is a float number
                        short *d = (short*)byte_array->content;
			float sum = 0, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum += *d;
			sum /= (float)byte_array->numelem;
			d = (short*)byte_array->content;
			for (i = 0; i < byte_array->numelem; i++, d++) sum2 += powf(fabs((*d)-sum), (float)byte_array->param);
			sum2 /= (float)byte_array->numelem;
			memcpy(result, (void*)(&sum2), core_sizeof(OPH_FLOAT));
                        break;
                }
                case OPH_BYTE:{
			//The standard deviation is a float number
                        char *d = (char*)byte_array->content;
			float sum = 0, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum += *d;
			sum /= (float)byte_array->numelem;
			d = (char*)byte_array->content;
			for (i = 0; i < byte_array->numelem; i++, d++) sum2 += powf(fabs((*d)-sum), (float)byte_array->param);
			sum2 /= (float)byte_array->numelem;
			memcpy(result, (void*)(&sum2), core_sizeof(OPH_FLOAT));
                        break;
                }
		case OPH_LONG:{
			//The standard deviation is a double number
                        long long *d = (long long*)byte_array->content;
			double sum = 0, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum += *d;
			sum /= (double)byte_array->numelem;
			d = (long long*)byte_array->content;
			for (i = 0; i < byte_array->numelem; i++, d++) sum2 += pow(abs((*d)-sum), byte_array->param);
			sum2 /= (double)byte_array->numelem;
                        memcpy(result, (void*)(&sum2), core_sizeof(OPH_DOUBLE));
                        break;
                }
                default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
        }
        return 0;
}
int core_oph_acmoment_multi (oph_multistring* byte_array, oph_multistring *result)
{
	if (!byte_array || (byte_array->param<0.0))
	{
		pmesg(1, __FILE__, __LINE__, "Wrong parameter\n");
		return -1;
	}
        int i,j;
	unsigned long numelem;
	char *in_string = byte_array->content, *current, *out_string = result->content;
	for(j = 0; j < byte_array->num_measure; j++)
	{
		numelem=0;
		current = in_string;
        	switch(byte_array->type[j])
		{
                	case OPH_DOUBLE:
			{
                        	double *d, sum = 0.0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (double*)current;
					if (!isnan(*d))
					{
						sum += *d;
						numelem++;
					}
                        	}
				if (!numelem) sum2 = NAN;
				else
				{
					sum /= numelem;
					current = in_string;
					for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
					{
						d = (double*)current;
						if (!isnan(*d)) sum2 += pow(abs((*d)-sum), byte_array->param);
					}
					sum2 /= numelem;
				}
				if(core_oph_type_cast((void*)(&sum2), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
                	}
                	case OPH_FLOAT:{
                        	float *d, sum = 0.0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (float*)current;
					if (!isnan(*d))
					{
						sum += *d;
						numelem++;
					}
                        	}
				if (!numelem) sum2 = NAN;
				else
				{
					sum /= numelem;
					current = in_string;
					for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
					{
						d = (float*)current;
						if (!isnan(*d)) sum2 += powf(fabs((*d)-sum), (float)byte_array->param);
					}
					sum2 /= numelem;
				}
				if(core_oph_type_cast((void*)(&sum2), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
                	}
                	case OPH_INT:{
				int *d;
				float sum = 0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (int*)current;
					sum += *d;
				}
				sum /= (float)byte_array->numelem;
				current = in_string;
				for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (int*)current;
					sum2 += powf(fabs((*d)-sum), (float)byte_array->param);
				}
				sum2 /= (float)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
                	case OPH_SHORT:{
				short *d;
				float sum = 0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (short*)current;
					sum += *d;
				}
				sum /= (float)byte_array->numelem;
				current = in_string;
				for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (short*)current;
					sum2 += powf(fabs((*d)-sum), (float)byte_array->param);
				}
				sum2 /= (float)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
                	case OPH_BYTE:{
				char *d;
				float sum = 0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (char*)current;
					sum += *d;
				}
				sum /= (float)byte_array->numelem;
				current = in_string;
				for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (char*)current;
					sum2 += powf(fabs((*d)-sum), (float)byte_array->param);
				}
				sum2 /= (float)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
			case OPH_LONG:{
				long long *d;
				double sum = 0, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++)
				{
					d = (long long*)current;
					sum += *d;
				}
				sum /= (double)byte_array->numelem;
				current = in_string;
				for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (long long*)current;
					sum2 += pow(abs((*d)-sum), byte_array->param);
				}
				sum2 /= (double)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_DOUBLE, result->type[j])) return -1;
                        	break;
	                }
        	        default:
                	        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        	return -1;
	        }
		in_string+=byte_array->elemsize[j];
		out_string+=result->elemsize[j];
	}
        return 0;
}


int core_oph_rmoment (oph_stringPtr byte_array, char *result)
{
	if (!byte_array || (byte_array->param<0.0))
	{
		pmesg(1, __FILE__, __LINE__, "Wrong parameter\n");
		return -1;
	}
        int i;
	unsigned long numelem=0;
        switch(byte_array->type){
                case OPH_DOUBLE:{
		        double *d = (double*)byte_array->content, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (!isnan(*d))
				{
					sum2 += pow(*d, byte_array->param);
					numelem++;
				}
			}
			if (!numelem) sum2 = NAN;
			else sum2 /= numelem;
			memcpy(result, (void*)(&sum2), byte_array->elemsize);
                        break;
                }
                case OPH_FLOAT:{
                        float *d = (float*)byte_array->content, sum2 = 0.0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (!isnan(*d))
				{
					sum2 += powf(*d, (float)byte_array->param);
					numelem++;
				}
			}
			if (!numelem) sum2 = NAN;
			else sum2 /= numelem;
			memcpy(result, (void*)(&sum2), byte_array->elemsize);
                        break;
                }
                case OPH_INT:{
			//The standard deviation is a float number
                        int *d = (int*)byte_array->content;
			float sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum2 += powf(*d, (float)byte_array->param);
			sum2 /= (float)byte_array->numelem;
			memcpy(result, (void*)(&sum2), core_sizeof(OPH_FLOAT));
                        break;
                }
                case OPH_SHORT:{
			//The standard deviation is a float number
                        short *d = (short*)byte_array->content;
			float sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum2 += powf(*d, (float)byte_array->param);
			sum2 /= (float)byte_array->numelem;
			memcpy(result, (void*)(&sum2), core_sizeof(OPH_FLOAT));
                        break;
                }
                case OPH_BYTE:{
			//The standard deviation is a float number
                        char *d = (char*)byte_array->content;
			float sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum2 += powf(*d, (float)byte_array->param);
			sum2 /= (float)byte_array->numelem;
			memcpy(result, (void*)(&sum2), core_sizeof(OPH_FLOAT));
                        break;
                }
		case OPH_LONG:{
			//The standard deviation is a double number
                        long long *d = (long long*)byte_array->content;
			double sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum2 += pow(*d, byte_array->param);
			sum2 /= (double)byte_array->numelem;
                        memcpy(result, (void*)(&sum2), core_sizeof(OPH_DOUBLE));
                        break;
                }
                default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
        }
        return 0;
}
int core_oph_rmoment_multi (oph_multistring* byte_array, oph_multistring *result)
{
	if (!byte_array || (byte_array->param<0.0))
	{
		pmesg(1, __FILE__, __LINE__, "Wrong parameter\n");
		return -1;
	}
        int i,j;
	unsigned long numelem;
	char *in_string = byte_array->content, *current, *out_string = result->content;
	for(j = 0; j < byte_array->num_measure; j++)
	{
		numelem=0;
		current = in_string;
        	switch(byte_array->type[j])
		{
                	case OPH_DOUBLE:
			{
                        	double *d, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (double*)current;
					if (!isnan(*d))
					{
						sum2 += pow(*d, byte_array->param);
						numelem++;
					}
                        	}
				if (!numelem) sum2 = NAN;
				else sum2 /= numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
                	}
                	case OPH_FLOAT:{
                        	float *d, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (float*)current;
					if (!isnan(*d))
					{
						sum2 += powf(*d, (float)byte_array->param);
						numelem++;
					}
                        	}
				if (!numelem) sum2 = NAN;
				else sum2 /= numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
                	}
                	case OPH_INT:{
				int *d;
				float sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (int*)current;
					sum2 += powf(*d, (float)byte_array->param);
				}
				sum2 /= (float)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
                	case OPH_SHORT:{
				short *d;
				float sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (short*)current;
					sum2 += powf(*d, (float)byte_array->param);
				}
				sum2 /= (float)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
                	case OPH_BYTE:{
				char *d;
				float sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (char*)current;
					sum2 += powf(*d, (float)byte_array->param);
				}
				sum2 /= (float)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
			case OPH_LONG:{
				long long *d;
				double sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++)
				{
					d = (long long*)current;
					sum2 += pow(*d, byte_array->param);
				}
				sum2 /= (double)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_DOUBLE, result->type[j])) return -1;
                        	break;
	                }
        	        default:
                	        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        	return -1;
	        }
		in_string+=byte_array->elemsize[j];
		out_string+=result->elemsize[j];
	}
        return 0;
}


int core_oph_armoment (oph_stringPtr byte_array, char *result)
{
	if (!byte_array || (byte_array->param<0.0))
	{
		pmesg(1, __FILE__, __LINE__, "Wrong parameter\n");
		return -1;
	}
        int i;
	unsigned long numelem=0;
        switch(byte_array->type){
                case OPH_DOUBLE:{
		        double *d = (double*)byte_array->content, sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (!isnan(*d))
				{
					sum2 += pow(abs(*d), byte_array->param);
					numelem++;
				}
			}
			if (!numelem) sum2 = NAN;
			else sum2 /= numelem;
			memcpy(result, (void*)(&sum2), byte_array->elemsize);
                        break;
                }
                case OPH_FLOAT:{
                        float *d = (float*)byte_array->content, sum2 = 0.0;
                        for (i = 0; i < byte_array->numelem; i++, d++)
			{
				if (!isnan(*d))
				{
					sum2 += powf(fabs(*d), (float)byte_array->param);
					numelem++;
				}
			}
			if (!numelem) sum2 = NAN;
			else sum2 /= numelem;
			memcpy(result, (void*)(&sum2), byte_array->elemsize);
                        break;
                }
                case OPH_INT:{
			//The standard deviation is a float number
                        int *d = (int*)byte_array->content;
			float sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum2 += powf(fabs(*d), (float)byte_array->param);
			sum2 /= (float)byte_array->numelem;
			memcpy(result, (void*)(&sum2), core_sizeof(OPH_FLOAT));
                        break;
                }
                case OPH_SHORT:{
			//The standard deviation is a float number
                        short *d = (short*)byte_array->content;
			float sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum2 += powf(fabs(*d), (float)byte_array->param);
			sum2 /= (float)byte_array->numelem;
			memcpy(result, (void*)(&sum2), core_sizeof(OPH_FLOAT));
                        break;
                }
                case OPH_BYTE:{
			//The standard deviation is a float number
                        char *d = (char*)byte_array->content;
			float sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum2 += powf(fabs(*d), (float)byte_array->param);
			sum2 /= (float)byte_array->numelem;
			memcpy(result, (void*)(&sum2), core_sizeof(OPH_FLOAT));
                        break;
                }
		case OPH_LONG:{
			//The standard deviation is a double number
                        long long *d = (long long*)byte_array->content;
			double sum2 = 0.0;
			for (i = 0; i < byte_array->numelem; i++, d++) sum2 += pow(abs(*d), byte_array->param);
			sum2 /= (double)byte_array->numelem;
                        memcpy(result, (void*)(&sum2), core_sizeof(OPH_DOUBLE));
                        break;
                }
                default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
        }
        return 0;
}
int core_oph_armoment_multi (oph_multistring* byte_array, oph_multistring *result)
{
	if (!byte_array || (byte_array->param<0.0))
	{
		pmesg(1, __FILE__, __LINE__, "Wrong parameter\n");
		return -1;
	}
        int i,j;
	unsigned long numelem;
	char *in_string = byte_array->content, *current, *out_string = result->content;
	for(j = 0; j < byte_array->num_measure; j++)
	{
		numelem=0;
		current = in_string;
        	switch(byte_array->type[j])
		{
                	case OPH_DOUBLE:
			{
                        	double *d, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (double*)current;
					if (!isnan(*d))
					{
						sum2 += pow(abs(*d), byte_array->param);
						numelem++;
					}
                        	}
				if (!numelem) sum2 = NAN;
				else sum2 /= numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
                	}
                	case OPH_FLOAT:{
                        	float *d, sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (float*)current;
					if (!isnan(*d))
					{
						sum2 += powf(fabs(*d), (float)byte_array->param);
						numelem++;
					}
                        	}
				if (!numelem) sum2 = NAN;
				else sum2 /= numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, byte_array->type[j], result->type[j])) return -1;
                        	break;
                	}
                	case OPH_INT:{
				int *d;
				float sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (int*)current;
					sum2 += powf(fabs(*d), (float)byte_array->param);
				}
				sum2 /= (float)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
                	case OPH_SHORT:{
				short *d;
				float sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (short*)current;
					sum2 += powf(fabs(*d), (float)byte_array->param);
				}
				sum2 /= (float)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
                	case OPH_BYTE:{
				char *d;
				float sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++, current+=byte_array->blocksize)
				{
					d = (char*)current;
					sum2 += powf(fabs(*d), (float)byte_array->param);
				}
				sum2 /= (float)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_FLOAT, result->type[j])) return -1;
                        	break;
        	        }
			case OPH_LONG:{
				long long *d;
				double sum2 = 0.0;
                        	for (i = 0; i < byte_array->numelem; i++)
				{
					d = (long long*)current;
					sum2 += pow(abs(*d), byte_array->param);
				}
				sum2 /= (double)byte_array->numelem;
				if(core_oph_type_cast((void*)(&sum2), out_string, OPH_DOUBLE, result->type[j])) return -1;
                        	break;
	                }
        	        default:
                	        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        	return -1;
	        }
		in_string+=byte_array->elemsize[j];
		out_string+=result->elemsize[j];
	}
        return 0;
}


// Not modified
int core_oph_get_subarray(oph_stringPtr byte_array, char* result, long long start, long long size)
{
        if(byte_array->numelem < start+size){
                pmesg(1, __FILE__, __LINE__, "Subarray size over array limits\n");
                return -1;
        }
        memcpy(result, (void*)( (byte_array->content)+(start*byte_array->elemsize)), size*byte_array->elemsize);
        return 0;

}

int core_oph_sum_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result)
{
	return core_oph_sum_array2(byte_arraya, byte_arrayb, result, NULL);
}
int core_oph_sum_array2(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result, int* count)
{
    int j;
    switch (byte_arraya->type) {
    case OPH_INT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((int *)(result))[j] = ((int *)(byte_arraya->content))[j] + (byte_arrayb ? ((int *)(byte_arrayb->content))[j] : 0);
        }
	if (count) count[0]++; // Only count[0] is updated to improve performance
        break;
    }
    case OPH_SHORT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((short *)(result))[j] = ((short *)(byte_arraya->content))[j] + (byte_arrayb ? ((short *)(byte_arrayb->content))[j] : 0);
        }
	if (count) count[0]++; // Only count[0] is updated to improve performance
        break;
    }
    case OPH_BYTE:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((char *)(result))[j] = ((char *)(byte_arraya->content))[j] + (byte_arrayb ? ((char *)(byte_arrayb->content))[j] : 0);
        }
	if (count) count[0]++; // Only count[0] is updated to improve performance
        break;
    }
    case OPH_LONG:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((long long *)(result))[j] = ((long long *)(byte_arraya->content))[j] + (byte_arrayb ? ((long long *)(byte_arrayb->content))[j] : 0);
        }
	if (count) count[0]++; // Only count[0] is updated to improve performance
        break;
    }
    case OPH_FLOAT:
    {
	for (j = 0; j < byte_arraya->numelem; j++)
	{
		if (!isnan(((float *)(byte_arraya->content))[j]))
		{
			if (byte_arrayb)
			{
				if (!isnan(((float *)(byte_arrayb->content))[j]))
				    ((float *)(result))[j] = ((float *)(byte_arraya->content))[j] + ((float *)(byte_arrayb->content))[j];
				else ((float *)(result))[j] = ((float *)(byte_arraya->content))[j];
			}
			else ((float *)(result))[j] = ((float *)(byte_arraya->content))[j];
			if (count) count[j]++;
		}
		else
		{
			if (byte_arrayb) ((float *)(result))[j] = ((float *)(byte_arrayb->content))[j];
			else ((float *)(result))[j] = NAN;
		}
	}
        break;
    }
    case OPH_DOUBLE:
    {
        for (j = 0; j < byte_arraya->numelem; j++)
	{
		if (!isnan(((double *)(byte_arraya->content))[j]))
		{
			if (byte_arrayb)
			{
				if (!isnan(((double *)(byte_arrayb->content))[j]))
				    ((double *)(result))[j] = ((double *)(byte_arraya->content))[j] + ((double *)(byte_arrayb->content))[j];
				else ((double *)(result))[j] = ((double *)(byte_arraya->content))[j];
			}
			else ((double *)(result))[j] = ((double *)(byte_arraya->content))[j];
			if (count) count[j]++;
		}
		else
		{
			if (byte_arrayb) ((double *)(result))[j] = ((double *)(byte_arrayb->content))[j];
			else ((double *)(result))[j] = NAN;
		}
	}
        break;
    }
    default:
        pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
        return -1;
    }
    return 0;
}

int core_oph_sub_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result){
    int j;
    switch (byte_arraya->type) {
    case OPH_INT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((int *)(result))[j] = ((int *)(byte_arraya->content))[j] - ((int *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_SHORT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((short *)(result))[j] = ((short *)(byte_arraya->content))[j] - ((short *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_BYTE:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((char *)(result))[j] = ((char *)(byte_arraya->content))[j] - ((char *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_LONG:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((long long *)(result))[j] = ((long long *)(byte_arraya->content))[j] - ((long long *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_FLOAT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (!isnan(((float *)(byte_arraya->content))[j]))
	    {
		if (!isnan(((float *)(byte_arrayb->content))[j]))
	            ((float *)(result))[j] = ((float *)(byte_arraya->content))[j] - ((float *)(byte_arrayb->content))[j];
		else ((float *)(result))[j] = ((float *)(byte_arraya->content))[j];
	    }
	    else ((float *)(result))[j] = - ((float *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_DOUBLE:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (!isnan(((double *)(byte_arraya->content))[j]))
	    {
		if (!isnan(((double *)(byte_arrayb->content))[j]))
	            ((double *)(result))[j] = ((double *)(byte_arraya->content))[j] - ((double *)(byte_arrayb->content))[j];
		else ((double *)(result))[j] = ((double *)(byte_arraya->content))[j];
	    }
	    else ((double *)(result))[j] = - ((double *)(byte_arrayb->content))[j];
        }
        break;
    }
    default:
        pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
        return -1;
    }
    return 0;
}

int core_oph_mul_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result){
    int j;
    switch (byte_arraya->type) {
    case OPH_INT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((int *)(result))[j] = ((int *)(byte_arraya->content))[j] * ((int *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_SHORT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((short *)(result))[j] = ((short *)(byte_arraya->content))[j] * ((short *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_BYTE:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((char *)(result))[j] = ((char *)(byte_arraya->content))[j] * ((char *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_LONG:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((long long *)(result))[j] = ((long long *)(byte_arraya->content))[j] * ((long long *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_FLOAT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (!isnan(((float *)(byte_arraya->content))[j]))
	    {
		if (!isnan(((float *)(byte_arrayb->content))[j]))
	            ((float *)(result))[j] = ((float *)(byte_arraya->content))[j] * ((float *)(byte_arrayb->content))[j];
		else ((float *)(result))[j] = ((float *)(byte_arraya->content))[j];
	    }
	    else ((float *)(result))[j] = ((float *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_DOUBLE:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (!isnan(((double *)(byte_arraya->content))[j]))
	    {
		if (!isnan(((double *)(byte_arrayb->content))[j]))
	            ((double *)(result))[j] = ((double *)(byte_arraya->content))[j] * ((double *)(byte_arrayb->content))[j];
		else ((double *)(result))[j] = ((double *)(byte_arraya->content))[j];
	    }
	    else ((double *)(result))[j] = ((double *)(byte_arrayb->content))[j];
        }
        break;
    }
    default:
        pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
        return -1;
    }
    return 0;
}

int core_oph_mask_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result){
    int j;
    switch (byte_arraya->type) {
    case OPH_INT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((int *)(result))[j] = (((int *)(byte_arrayb->content))[j] == 1)?((int *)(byte_arraya->content))[j]:0;
        }
        break;
    }
    case OPH_SHORT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((short *)(result))[j] = (((short *)(byte_arrayb->content))[j] == 1)?((short *)(byte_arraya->content))[j]:0;
        }
        break;
    }
    case OPH_BYTE:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((char *)(result))[j] = (((char *)(byte_arrayb->content))[j] == 1)?((char *)(byte_arraya->content))[j]:0;
        }
        break;
    }
    case OPH_LONG:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((long long *)(result))[j] = (((long long *)(byte_arrayb->content))[j] == 1)?((long long *)(byte_arraya->content))[j]:0;
        }
        break;
    }
    case OPH_FLOAT:
    {
    	for (j = 0; j < byte_arraya->numelem; j++) {
    		((float *)(result))[j] = (((float *)(byte_arrayb->content))[j] == 1.0)?((float *)(byte_arraya->content))[j]:NAN;
    	}
        break;
    }
    case OPH_DOUBLE:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
        	((double *)(result))[j] = (((double *)(byte_arrayb->content))[j] == 1.0)?((double *)(byte_arraya->content))[j]:NAN;
        }
        break;
    }
    default:
        pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
        return -1;
    }
    return 0;
}

int core_oph_div_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result){
    int j;
    switch (byte_arraya->type) {
    case OPH_INT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((int *)(result))[j] = ((int *)(byte_arraya->content))[j] / ((int *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_SHORT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((short *)(result))[j] = ((short *)(byte_arraya->content))[j] / ((short *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_BYTE:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((char *)(result))[j] = ((char *)(byte_arraya->content))[j] / ((char *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_LONG:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((long long *)(result))[j] = ((long long *)(byte_arraya->content))[j] / ((long long *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_FLOAT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (!isnan(((float *)(byte_arraya->content))[j]))
	    {
		if (!isnan(((float *)(byte_arrayb->content))[j]))
	            ((float *)(result))[j] = ((float *)(byte_arraya->content))[j] / ((float *)(byte_arrayb->content))[j];
		else ((float *)(result))[j] = ((float *)(byte_arraya->content))[j];
	    }
	    else ((float *)(result))[j] = 1.0 / ((float *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_DOUBLE:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (!isnan(((double *)(byte_arraya->content))[j]))
	    {
		if (!isnan(((double *)(byte_arrayb->content))[j]))
	            ((double *)(result))[j] = ((double *)(byte_arraya->content))[j] / ((double *)(byte_arrayb->content))[j];
		else ((double *)(result))[j] = ((double *)(byte_arraya->content))[j];
	    }
	    else ((double *)(result))[j] = 1.0 / ((double *)(byte_arrayb->content))[j];
        }
        break;
    }
    default:
        pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
        return -1;
    }
    return 0;
}

int core_oph_abs_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result){
    int j;
    switch (byte_arraya->type) {
    case OPH_INT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((int *)(result))[j] = (int) sqrt(fabs(((int *)(byte_arraya->content))[j] * ((int *)(byte_arraya->content))[j] + ((int *)(byte_arrayb->content))[j] * ((int *)(byte_arrayb->content))[j]));
        }
        break;
    }
    case OPH_SHORT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((short *)(result))[j] = (short) sqrt(fabs(((short *)(byte_arraya->content))[j] * ((short *)(byte_arraya->content))[j] + ((short *)(byte_arrayb->content))[j] * ((short *)(byte_arrayb->content))[j]));
        }
        break;
    }
    case OPH_BYTE:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((char *)(result))[j] = (char) sqrt(fabs(((char *)(byte_arraya->content))[j] * ((char *)(byte_arraya->content))[j] + ((char *)(byte_arrayb->content))[j] * ((char *)(byte_arrayb->content))[j]));
        }
        break;
    }
    case OPH_LONG:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((long long *)(result))[j] = (long long) sqrt(fabs(((long long *)(byte_arraya->content))[j] * ((long long *)(byte_arraya->content))[j] + ((long long *)(byte_arrayb->content))[j] * ((long long *)(byte_arrayb->content))[j]));
        }
        break;
    }
    case OPH_FLOAT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (isnan(((float *)(byte_arraya->content))[j]) || isnan(((float *)(byte_arrayb->content))[j])) ((float *)(result))[j] = NAN;
	    else ((float *)(result))[j] = (float) sqrt(fabs(((float *)(byte_arraya->content))[j] * ((float *)(byte_arraya->content))[j] + ((float *)(byte_arrayb->content))[j] * ((float *)(byte_arrayb->content))[j]));
        }
        break;
    }
    case OPH_DOUBLE:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (isnan(((double *)(byte_arraya->content))[j]) || isnan(((double *)(byte_arrayb->content))[j])) ((double *)(result))[j] = NAN;
	    else ((double *)(result))[j] = (double) sqrt(fabs(((double *)(byte_arraya->content))[j] * ((double *)(byte_arraya->content))[j] + ((double *)(byte_arrayb->content))[j] * ((double *)(byte_arrayb->content))[j]));
        }
        break;
    }
    default:
        pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
        return -1;
    }
    return 0;
}

int core_oph_arg_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result){
    int j;
    switch (byte_arraya->type) {
    case OPH_INT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((int *)(result))[j] = (int) atan2((double)(((int *)(byte_arrayb->content))[j]),(double)((int *)(byte_arraya->content))[j]);
        }
        break;
    }
    case OPH_SHORT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((short *)(result))[j] = (short) atan2((double)(((short *)(byte_arrayb->content))[j]),(double)((short *)(byte_arraya->content))[j]);
        }
        break;
    }
    case OPH_BYTE:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((char *)(result))[j] = (char) atan2((double)(((char *)(byte_arrayb->content))[j]),(double)((char *)(byte_arraya->content))[j]);
        }
        break;
    }
    case OPH_LONG:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            ((long long *)(result))[j] = (long long) atan2((double)(((long long *)(byte_arrayb->content))[j]),(double)((long long *)(byte_arraya->content))[j]);
        }
        break;
    }
    case OPH_FLOAT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (!isnan(((float *)(byte_arraya->content))[j]))
	    {
		if (!isnan(((float *)(byte_arrayb->content))[j]))
	            ((float *)(result))[j] = (float) atan2((double)(((float *)(byte_arrayb->content))[j]),(double)((float *)(byte_arraya->content))[j]);
		else ((float *)(result))[j] = 0.0;
	    }
	    else ((float *)(result))[j] = (float)(M_PI/2);
        }
        break;
    }
    case OPH_DOUBLE:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (!isnan(((double *)(byte_arraya->content))[j]))
	    {
		if (!isnan(((double *)(byte_arrayb->content))[j]))
	            ((double *)(result))[j] = atan2(((double *)(byte_arrayb->content))[j],((double *)(byte_arraya->content))[j]);
		else ((double *)(result))[j] = 0.0;
	    }
	    else ((double *)(result))[j] = (double)(M_PI/2);
        }
        break;
    }
    default:
        pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
        return -1;
    }
    return 0;
}

int core_oph_sum2_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result){
    int j;
    switch (byte_arraya->type) {
    case OPH_INT:
    {
        int val_i;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_i = ((int *)(byte_arraya->content))[j];
            ((int *)(result))[j] = (val_i*val_i) + ((int *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_SHORT:
    {
        short val_i;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_i = ((short *)(byte_arraya->content))[j];
            ((short *)(result))[j] = (val_i*val_i) + ((short *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_BYTE:
    {
        char val_i;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_i = ((char *)(byte_arraya->content))[j];
            ((char *)(result))[j] = (val_i*val_i) + ((char *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_LONG:
    {
        long long val_l;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_l = ((long long *)(byte_arraya->content))[j];
            ((long long *)(result))[j] = (val_l*val_l) + ((long long *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_FLOAT:
    {
        float val_f;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_f = ((float *)(byte_arraya->content))[j];
            if (!isnan(val_f))
	    {
		if (!isnan(((float *)(byte_arrayb->content))[j]))
	            ((float *)(result))[j] = val_f*val_f + ((float *)(byte_arrayb->content))[j];
		else ((float *)(result))[j] = val_f*val_f;
	    }
	    else ((float *)(result))[j] = ((float *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_DOUBLE:
    {
        double val_d;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_d = ((double *)(byte_arraya->content))[j];
            if (!isnan(val_d))
	    {
		if (!isnan(((double *)(byte_arrayb->content))[j]))
	            ((double *)(result))[j] = val_d*val_d + ((double *)(byte_arrayb->content))[j];
		else ((double *)(result))[j] = val_d*val_d;
	    }
	    else ((double *)(result))[j] = ((double *)(byte_arrayb->content))[j];
        }
        break;
    }
    default:
        pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
        return -1;
    }
    return 0;
}

int core_oph_sum3_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result){
    int j;
    switch (byte_arraya->type) {
    case OPH_INT:
    {
        int val_i;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_i = ((int *)(byte_arraya->content))[j];
            ((int *)(result))[j] = (val_i*val_i*val_i) + ((int *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_SHORT:
    {
        short val_i;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_i = ((short *)(byte_arraya->content))[j];
            ((short *)(result))[j] = (val_i*val_i*val_i) + ((short *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_BYTE:
    {
        char val_i;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_i = ((char *)(byte_arraya->content))[j];
            ((char *)(result))[j] = (val_i*val_i*val_i) + ((char *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_LONG:
    {
        long long val_l;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_l = ((long long *)(byte_arraya->content))[j];
            ((long long *)(result))[j] = (val_l*val_l*val_l) + ((long long *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_FLOAT:
    {
        float val_f;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_f = ((float *)(byte_arraya->content))[j];
            if (!isnan(val_f))
	    {
		if (!isnan(((float *)(byte_arrayb->content))[j]))
	            ((float *)(result))[j] = val_f*val_f*val_f + ((float *)(byte_arrayb->content))[j];
		else ((float *)(result))[j] = val_f*val_f*val_f;
	    }
	    else ((float *)(result))[j] = ((float *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_DOUBLE:
    {
        double val_d;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_d = ((double *)(byte_arraya->content))[j];
            if (!isnan(val_d))
	    {
		if (!isnan(((double *)(byte_arrayb->content))[j]))
	            ((double *)(result))[j] = val_d*val_d*val_d + ((double *)(byte_arrayb->content))[j];
		else ((double *)(result))[j] = val_d*val_d*val_d;
	    }
	    else ((double *)(result))[j] = ((double *)(byte_arrayb->content))[j];
        }
        break;
    }
    default:
        pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
        return -1;
    }
    return 0;
}

int core_oph_sum4_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result){
    int j;
    switch (byte_arraya->type) {
    case OPH_INT:
    {
        int val_i;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_i = ((int *)(byte_arraya->content))[j];
            ((int *)(result))[j] = (val_i*val_i*val_i*val_i) + ((int *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_SHORT:
    {
        short val_i;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_i = ((short *)(byte_arraya->content))[j];
            ((short *)(result))[j] = (val_i*val_i*val_i*val_i) + ((short *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_BYTE:
    {
        char val_i;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_i = ((char *)(byte_arraya->content))[j];
            ((char *)(result))[j] = (val_i*val_i*val_i*val_i) + ((char *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_LONG:
    {
        long long val_l;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_l = ((long long *)(byte_arraya->content))[j];
            ((long long *)(result))[j] = (val_l*val_l*val_l*val_l) + ((long long *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_FLOAT:
    {
        float val_f;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_f = ((float *)(byte_arraya->content))[j];
            if (!isnan(val_f))
	    {
		if (!isnan(((float *)(byte_arrayb->content))[j]))
	            ((float *)(result))[j] = val_f*val_f*val_f*val_f + ((float *)(byte_arrayb->content))[j];
		else ((float *)(result))[j] = val_f*val_f*val_f*val_f;
	    }
	    else ((float *)(result))[j] = ((float *)(byte_arrayb->content))[j];
        }
        break;
    }
    case OPH_DOUBLE:
    {
        double val_d;
        for (j = 0; j < byte_arraya->numelem; j++) {
            val_d = ((double *)(byte_arraya->content))[j];
            if (!isnan(val_d))
	    {
		if (!isnan(((double *)(byte_arrayb->content))[j]))
	            ((double *)(result))[j] = val_d*val_d*val_d*val_d + ((double *)(byte_arrayb->content))[j];
		else ((double *)(result))[j] = val_d*val_d*val_d*val_d;
	    }
	    else ((double *)(result))[j] = ((double *)(byte_arrayb->content))[j];
        }
        break;
    }
    default:
        pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
        return -1;
    }
    return 0;
}

int core_oph_count_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result){
    int j;
    switch (byte_arraya->type) {
    case OPH_INT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
		if (byte_arrayb) ((int *)(result))[j] = 1 + ((int *)(byte_arrayb->content))[j];
		else ((int *)(result))[j] = 1;
        }
        break;
    }
    case OPH_SHORT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
		if (byte_arrayb) ((short *)(result))[j] = 1 + ((short *)(byte_arrayb->content))[j];
		else ((short *)(result))[j] = 1;
        }
        break;
    }
    case OPH_BYTE:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
		if (byte_arrayb) ((char *)(result))[j] = 1 + ((char *)(byte_arrayb->content))[j];
		else ((char *)(result))[j] = 1;
        }
        break;
    }
    case OPH_LONG:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
		if (byte_arrayb) ((long long *)(result))[j] = 1 + ((long long *)(byte_arrayb->content))[j];
		else ((long long *)(result))[j] = 1;
        }
        break;
    }
    case OPH_FLOAT:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (!isnan(((float *)(byte_arraya->content))[j]))
	    {
		if (byte_arrayb)
		{
			if (!isnan(((float *)(byte_arrayb->content))[j]))
			    ((float *)(result))[j] = 1 + ((float *)(byte_arrayb->content))[j];
			else ((float *)(result))[j] = 1;
		}
		else ((float *)(result))[j] = 1;
	    }
	    else
	    {
		if (byte_arrayb) ((float *)(result))[j] = ((float *)(byte_arrayb->content))[j];
		else ((float *)(result))[j] = 0;
	    }
        }
        break;
    }
    case OPH_DOUBLE:
    {
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (!isnan(((double *)(byte_arraya->content))[j]))
	    {
		if (byte_arrayb)
		{
			if (!isnan(((double *)(byte_arrayb->content))[j]))
			    ((double *)(result))[j] = 1 + ((double *)(byte_arrayb->content))[j];
			else ((double *)(result))[j] = 1;
		}
		else ((double *)(result))[j] = 1;
	    }
	    else
	    {
		if (byte_arrayb) ((double *)(result))[j] = ((double *)(byte_arrayb->content))[j];
		else ((double *)(result))[j] = 0;
	    }
        }
        break;
    }
    default:
        pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
        return -1;
    }
    return 0;
}

int core_oph_max_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result){
    int j;
    switch (byte_arraya->type) {
    case OPH_INT:
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (((int *)(byte_arraya->content))[j] >= ((int *)(byte_arrayb->content))[j]) {
                ((int *)(result))[j] = ((int *)(byte_arraya->content))[j];
            } else {
                ((int *)(result))[j] = ((int *)(byte_arrayb->content))[j];
            }
        }
        break;
    case OPH_SHORT:
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (((short *)(byte_arraya->content))[j] >= ((short *)(byte_arrayb->content))[j]) {
                ((short *)(result))[j] = ((short *)(byte_arraya->content))[j];
            } else {
                ((short *)(result))[j] = ((short *)(byte_arrayb->content))[j];
            }
        }
        break;
    case OPH_BYTE:
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (((char *)(byte_arraya->content))[j] >= ((char *)(byte_arrayb->content))[j]) {
                ((char *)(result))[j] = ((char *)(byte_arraya->content))[j];
            } else {
                ((char *)(result))[j] = ((char *)(byte_arrayb->content))[j];
            }
        }
        break;
    case OPH_LONG:
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (((long long *)(byte_arraya->content))[j] >= ((long long *)(byte_arrayb->content))[j]) {
                ((long long *)(result))[j] = ((long long *)(byte_arraya->content))[j];
            } else {
                ((long long *)(result))[j] = ((long long *)(byte_arrayb->content))[j];
            }
        }
        break;
    case OPH_FLOAT:
        for (j = 0; j < byte_arraya->numelem; j++) {
	    if (!isnan(((float *)(byte_arraya->content))[j]))
	    {
	    	if (!isnan(((float *)(byte_arrayb->content))[j]))
		{
			if (((float *)(byte_arraya->content))[j] >= ((float *)(byte_arrayb->content))[j]) {
				((float *)(result))[j] = ((float *)(byte_arraya->content))[j];
			} else {
				((float *)(result))[j] = ((float *)(byte_arrayb->content))[j];
			}
		}
		else ((float *)(result))[j] = ((float *)(byte_arraya->content))[j];
	    }
	    else ((float *)(result))[j] = ((float *)(byte_arrayb->content))[j];
        }
        break;
    case OPH_DOUBLE:
        for (j = 0; j < byte_arraya->numelem; j++) {
	    if (!isnan(((double *)(byte_arraya->content))[j]))
	    {
	    	if (!isnan(((double *)(byte_arrayb->content))[j]))
		{
			if (((double *)(byte_arraya->content))[j] >= ((double *)(byte_arrayb->content))[j]) {
				((double *)(result))[j] = ((double *)(byte_arraya->content))[j];
			} else {
				((double *)(result))[j] = ((double *)(byte_arrayb->content))[j];
			}
		}
		else ((double *)(result))[j] = ((double *)(byte_arraya->content))[j];
	    }
	    else ((double *)(result))[j] = ((double *)(byte_arrayb->content))[j];
        }
        break;
    default:
        pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
        return -1;
    }
    return 0;
}

int core_oph_min_array(oph_stringPtr byte_arraya, oph_stringPtr byte_arrayb, char* result){
    int j;
    switch (byte_arraya->type) {
    case OPH_INT:
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (((int *)(byte_arraya->content))[j] < ((int *)(byte_arrayb->content))[j]) {
                ((int *)(result))[j] = ((int *)(byte_arraya->content))[j];
            } else {
                ((int *)(result))[j] = ((int *)(byte_arrayb->content))[j];
            }
        }
        break;
    case OPH_SHORT:
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (((short *)(byte_arraya->content))[j] < ((short *)(byte_arrayb->content))[j]) {
                ((short *)(result))[j] = ((short *)(byte_arraya->content))[j];
            } else {
                ((short *)(result))[j] = ((short *)(byte_arrayb->content))[j];
            }
        }
        break;
    case OPH_BYTE:
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (((char *)(byte_arraya->content))[j] < ((char *)(byte_arrayb->content))[j]) {
                ((char *)(result))[j] = ((char *)(byte_arraya->content))[j];
            } else {
                ((char *)(result))[j] = ((char *)(byte_arrayb->content))[j];
            }
        }
        break;
    case OPH_LONG:
        for (j = 0; j < byte_arraya->numelem; j++) {
            if (((long long *)(byte_arraya->content))[j] < ((long long *)(byte_arrayb->content))[j]) {
                ((long long *)(result))[j] = ((long long *)(byte_arraya->content))[j];
            } else {
                ((long long *)(result))[j] = ((long long *)(byte_arrayb->content))[j];
            }
        }
        break;
    case OPH_FLOAT:
        for (j = 0; j < byte_arraya->numelem; j++) {
	    if (!isnan(((float *)(byte_arraya->content))[j]))
	    {
	    	if (!isnan(((float *)(byte_arrayb->content))[j]))
		{
			if (((float *)(byte_arraya->content))[j] < ((float *)(byte_arrayb->content))[j]) {
				((float *)(result))[j] = ((float *)(byte_arraya->content))[j];
			} else {
				((float *)(result))[j] = ((float *)(byte_arrayb->content))[j];
			}
		}
		else ((float *)(result))[j] = ((float *)(byte_arraya->content))[j];
	    }
	    else ((float *)(result))[j] = ((float *)(byte_arrayb->content))[j];
        }
        break;
    case OPH_DOUBLE:
        for (j = 0; j < byte_arraya->numelem; j++) {
	    if (!isnan(((double *)(byte_arraya->content))[j]))
	    {
	    	if (!isnan(((double *)(byte_arrayb->content))[j]))
		{
			if (((double *)(byte_arraya->content))[j] < ((double *)(byte_arrayb->content))[j]) {
				((double *)(result))[j] = ((double *)(byte_arraya->content))[j];
			} else {
				((double *)(result))[j] = ((double *)(byte_arrayb->content))[j];
			}
		}
		else ((double *)(result))[j] = ((double *)(byte_arraya->content))[j];
	    }
	    else ((double *)(result))[j] = ((double *)(byte_arrayb->content))[j];
        }
        break;
    default:
        pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
        return -1;
    }
    return 0;
}

int core_oph_sum_scalar(oph_stringPtr byte_array, double scalar, char* result){
        int i;

        switch(byte_array->type){
                case OPH_DOUBLE:{
                        double d;
                        for (i = 0; i < byte_array->numelem; i++){
                                d = *(double*)((byte_array->content)+(i*byte_array->elemsize)) + scalar;
                                memcpy(result + (i*byte_array->elemsize), (void*)(&d), byte_array->elemsize);
			}
			break;
		}
                
                case OPH_FLOAT:{
                        float d;
                        for (i = 0; i < byte_array->numelem; i++){
				d = *(float*)((byte_array->content)+(i*byte_array->elemsize)) + (float)scalar;
                                memcpy(result + (i*byte_array->elemsize), (void*)(&d), byte_array->elemsize);
			}
			break;
		}
                case OPH_INT:{
                        int d;
                        for (i = 0; i < byte_array->numelem; i++){
                                d = *(int*)((byte_array->content)+(i*byte_array->elemsize)) + (int)scalar;
                                memcpy(result + (i*byte_array->elemsize), (void*)(&d), byte_array->elemsize);
			}
			break;
		}
                case OPH_SHORT:{
                        short d;
                        for (i = 0; i < byte_array->numelem; i++){
                                d = *(short*)((byte_array->content)+(i*byte_array->elemsize)) + (short)scalar;
                                memcpy(result + (i*byte_array->elemsize), (void*)(&d), byte_array->elemsize);
			}
			break;
		}
                case OPH_BYTE:{
                        char d;
                        for (i = 0; i < byte_array->numelem; i++){
                                d = *(char*)((byte_array->content)+(i*byte_array->elemsize)) + (char)scalar;
                                memcpy(result + (i*byte_array->elemsize), (void*)(&d), byte_array->elemsize);
			}
			break;
		}
                case OPH_LONG:{
                        long long d;
                        for (i = 0; i < byte_array->numelem; i++){
                                d = *(long long*)((byte_array->content)+(i*byte_array->elemsize)) + (long long)scalar;
                                memcpy(result + (i*byte_array->elemsize), (void*)(&d), byte_array->elemsize);
			}
			break;
		}
                
                default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
	}
        return 0;
}

int core_oph_mul_scalar(oph_stringPtr byte_array, double scalar, char* result){
        int i;

        switch(byte_array->type){
                case OPH_DOUBLE:{
                        double d;
                        for (i = 0; i < byte_array->numelem; i++){
                                d = *(double*)((byte_array->content)+(i*byte_array->elemsize)) * scalar;
                                memcpy(result + (i*byte_array->elemsize), (void*)(&d), byte_array->elemsize);
			}
			break;
		}
                
                case OPH_FLOAT:{
                        float d;
                        for (i = 0; i < byte_array->numelem; i++){
                                d = *(float*)((byte_array->content)+(i*byte_array->elemsize)) * (float)scalar;
                                memcpy(result + (i*byte_array->elemsize), (void*)(&d), byte_array->elemsize);
			}
			break;
		}
                case OPH_INT:{
                        int d;
                        for (i = 0; i < byte_array->numelem; i++){
                                d = *(int*)((byte_array->content)+(i*byte_array->elemsize)) * (int)scalar;
                                memcpy(result + (i*byte_array->elemsize), (void*)(&d), byte_array->elemsize);
			}
			break;
		}
                case OPH_SHORT:{
                        short d;
                        for (i = 0; i < byte_array->numelem; i++){
                                d = *(short*)((byte_array->content)+(i*byte_array->elemsize)) * (short)scalar;
                                memcpy(result + (i*byte_array->elemsize), (void*)(&d), byte_array->elemsize);
			}
			break;
		}
                case OPH_BYTE:{
                        char d;
                        for (i = 0; i < byte_array->numelem; i++){
                                d = *(char*)((byte_array->content)+(i*byte_array->elemsize)) * (char)scalar;
                                memcpy(result + (i*byte_array->elemsize), (void*)(&d), byte_array->elemsize);
			}
			break;
		}
                case OPH_LONG:{
                        long long d;
                        for (i = 0; i < byte_array->numelem; i++){
                                d = *(long long*)((byte_array->content)+(i*byte_array->elemsize)) * (long long)scalar;
                                memcpy(result + (i*byte_array->elemsize), (void*)(&d), byte_array->elemsize);
			}
			break;
		}
                
                default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
	}	
        return 0;
}

int core_oph_find(oph_stringPtr byte_array, double value, double distance, long *count){
        int i;

        switch(byte_array->type){
                case OPH_DOUBLE:{
                        for (i = 0; i < byte_array->numelem; i++){
				if( fabs(*(double*)((byte_array->content)+(i*byte_array->elemsize)) - value) <= distance ){
					(*count)++;
				}
			}
			break;
		}
		case OPH_FLOAT:{
			for (i = 0; i < byte_array->numelem; i++){
				if( fabs(*(float*)((byte_array->content)+(i*byte_array->elemsize)) - value) <= distance ){
                                        (*count)++;
                                }
			}
			break;
		}
		case OPH_INT:{
			for (i = 0; i < byte_array->numelem; i++){
				if( fabs(*(int*)((byte_array->content)+(i*byte_array->elemsize)) - value) <= distance ){
                                        (*count)++;
                                }
			}
			break;
		}
		case OPH_SHORT:{
			for (i = 0; i < byte_array->numelem; i++){
				if( fabs(*(short*)((byte_array->content)+(i*byte_array->elemsize)) - value) <= distance ){
                                        (*count)++;
                                }
			}
			break;
		}
		case OPH_BYTE:{
			for (i = 0; i < byte_array->numelem; i++){
				if( fabs(*(char*)((byte_array->content)+(i*byte_array->elemsize)) - value) <= distance ){
                                        (*count)++;
                                }
			}
			break;
		}
		case OPH_LONG:{
			for (i = 0; i < byte_array->numelem; i++){
				if( fabs(*(long long*)((byte_array->content)+(i*byte_array->elemsize)) - value) <= distance ){
                                        (*count)++;
                                }
			}
			break;
		}
		default:
                        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
                        return -1;
	}

	return 0;

}

int core_oph_shift(oph_generic_param* param, long long offset, double filling, int round)
{
	oph_string* byte_array = (oph_string*)param->measure;
	char* result = param->result;
	long long part1, part2, i, j, opart1, opart2;
	if (offset>0)
	{
		opart2 = offset * byte_array->elemsize;
		opart1 = *(byte_array->length) - opart2;
		part2 = offset * param->result_elemsize;
		part1 = param->length - part2;
		if (part1)
		{
			if (byte_array->type != param->result_type)
			{
				for (i=0, j=0; j<part1; i+=byte_array->elemsize, j+=param->result_elemsize)
				{
					if(core_oph_type_cast(byte_array->content+i, result+part2+j, byte_array->type, param->result_type))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
			}
			else memcpy(result+part2, byte_array->content, part1);
		}
		if (round)
		{
			if (byte_array->type != param->result_type)
			{
				for (i=0, j=0; j<part2; i+=byte_array->elemsize, j+=param->result_elemsize)
				{
					if(core_oph_type_cast(byte_array->content+opart1+i, result+j, byte_array->type, param->result_type))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
			}
			else memcpy(result, byte_array->content+opart1, part2);
		}
		else switch(byte_array->type)
		{
			case OPH_DOUBLE:
			{
				for (i=0;i<offset;++i)
				{
					if(core_oph_type_cast(&filling, result+i*param->result_elemsize, byte_array->type, param->result_type))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
			        break;
			}
			case OPH_FLOAT:
			{
				float value = (float)filling;
				for (i=0;i<offset;++i)
				{
					if(core_oph_type_cast(&value, result+i*param->result_elemsize, byte_array->type, param->result_type))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_INT:
			{
				int value = (int)filling;
				for (i=0;i<offset;++i)
				{
					if(core_oph_type_cast(&value, result+i*param->result_elemsize, byte_array->type, param->result_type))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_SHORT:
			{
				short value = (short)filling;
				for (i=0;i<offset;++i)
				{
					if(core_oph_type_cast(&value, result+i*param->result_elemsize, byte_array->type, param->result_type))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_BYTE:
			{
				char value = (char)filling;
				for (i=0;i<offset;++i)
				{
					if(core_oph_type_cast(&value, result+i*param->result_elemsize, byte_array->type, param->result_type))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_LONG:
			{
				long long value = (long long)filling;
				for (i=0;i<offset;++i)
				{
					if(core_oph_type_cast(&value, result+i*param->result_elemsize, byte_array->type, param->result_type))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			/*
				Other types
			*/
			default:
			        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
			        return  -1;
		}
	}
	else if (offset<0)
	{
		opart2 = (-offset) * byte_array->elemsize;
		opart1 = *(byte_array->length) - opart2;
		part2 = (-offset) * param->result_elemsize;
		part1 = param->length - part2;
		if (part1)
		{
			if (byte_array->type != param->result_type)
			{
				for (i=0, j=0; j<part1; i+=byte_array->elemsize, j+=param->result_elemsize)
				{
					if(core_oph_type_cast(byte_array->content+opart2+i, result+j, byte_array->type, param->result_type))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
			}
			else memcpy(result, byte_array->content+opart2, part1);
		}
		if (round)
		{
			if (byte_array->type != param->result_type)
			{
				for (i=0, j=0; j<part2; i+=byte_array->elemsize, j+=param->result_elemsize)
				{
					if(core_oph_type_cast(byte_array->content+i, result+part1+j, byte_array->type, param->result_type))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
			}
			else memcpy(result+part1, byte_array->content, part2);
		}
		else switch(byte_array->type)
		{
			case OPH_DOUBLE:
			{
				for (i=0;i<-offset;++i)
				{
					if(core_oph_type_cast(&filling, result+part1+i*param->result_elemsize, byte_array->type, param->result_type))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
			        break;
			}
			case OPH_FLOAT:
			{
				float value = (float)filling;
				for (i=0;i<-offset;++i)
				{
					if(core_oph_type_cast(&value, result+part1+i*param->result_elemsize, byte_array->type, param->result_type))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_INT:
			{
				int value = (int)filling;
				for (i=0;i<-offset;++i)
				{
					if(core_oph_type_cast(&value, result+part1+i*param->result_elemsize, byte_array->type, param->result_type))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_SHORT:
			{
				short value = (short)filling;
				for (i=0;i<-offset;++i)
				{
					if(core_oph_type_cast(&value, result+part1+i*param->result_elemsize, byte_array->type, param->result_type))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_BYTE:
			{
				char value = (char)filling;
				for (i=0;i<-offset;++i)
				{
					if(core_oph_type_cast(&value, result+part1+i*param->result_elemsize, byte_array->type, param->result_type))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_LONG:
			{
				long long value = (long long)filling;
				for (i=0;i<-offset;++i)
				{
					if(core_oph_type_cast(&value, result+part1+i*param->result_elemsize, byte_array->type, param->result_type))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			/*
				Other types
			*/
			default:
			        pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
			        return  -1;
		}
	}
	else if (byte_array->type != param->result_type)
	{
		for (i=0;i<byte_array->numelem;++i)
		{
			if(core_oph_type_cast(byte_array->content+i*byte_array->elemsize, result+i*param->result_elemsize, byte_array->type, param->result_type))
			{
				pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
				return 1;
			}
		}
	}
	else memcpy(result, byte_array->content, param->length);

	return 0;
}

int core_oph_simple_moving_avg (oph_generic_param *param, long long k)
{
	oph_stringPtr byte_array = param->measure;
	char* result = param->result;
	long long i;
	//Used to count not NaN values (for float and double types)
	long long count = 0;
	short int not_nan = 0; 

	switch(byte_array->type){
		case OPH_DOUBLE:{
			double *d = (double*)byte_array->content, moving_sum = 0.0, moving_avg;
			for (i = 0; i < byte_array->numelem; i++)
			{
				if(i < (k-1))
				{
					if (!isnan(d[i])){
						moving_sum += d[i];
						count++;
					}
					moving_avg = NAN;
				}
				else if( i == (k-1))
				{
					if (!isnan(d[i])){
						moving_sum += d[i];
						count++;
					}

					if(count){
						moving_avg = moving_sum/(double)count;					
					} else{
						moving_avg = NAN;	
					}
				}
				else{
					not_nan = (!isnan(d[i - k]) ? 1 : 0);
					if (!isnan(d[i])){
						moving_sum = moving_sum + d[i] - (not_nan ? d[i - k] : 0);
						count = count + 1 - (not_nan ? 1 : 0);
					} else{
						moving_sum -= (not_nan ? d[i - k] : 0);
						count = count - (not_nan ? 1 : 0);
					}
					if(count){
						moving_avg = moving_sum/(double)count;					
					} else{
						moving_avg = NAN;	
					}			
				}
				if(core_oph_type_cast(&moving_avg, result+i*param->result_elemsize, byte_array->type, param->result_type))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		case OPH_FLOAT:{
			float *d = (float*)byte_array->content, moving_sum = 0.0, moving_avg;
			for (i = 0; i < byte_array->numelem; i++)
			{
				if(i < (k-1))
				{
					if (!isnan(d[i])){
						moving_sum += d[i];
						count++;
					}
					moving_avg = NAN;
				}
				else if( i == (k-1))
				{
					if (!isnan(d[i])){
						moving_sum += d[i];
						count++;
					}

					if(count)
					{
						moving_avg = moving_sum/(float)count;					
					} else{
						moving_avg = NAN;
					}
				}
				else{
					not_nan = (!isnan(d[i - k]) ? 1 : 0);
					if (!isnan(d[i])){
						moving_sum = moving_sum + d[i] - (not_nan ? d[i - k] : 0);
						count = count + 1 - (not_nan ? 1 : 0);
					} else{
						moving_sum -= (not_nan ? d[i - k] : 0);
						count = count - (not_nan ? 1 : 0);
					}

					if(count){
						moving_avg = moving_sum/(float)count;
					} else{
						moving_avg = NAN;
					}			
				}
				if(core_oph_type_cast(&moving_avg, result+i*param->result_elemsize, byte_array->type, param->result_type))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		case OPH_INT:{
			//The average is a float number
			int *d = (int*)byte_array->content, moving_sum = 0;
			float moving_avg;
			for (i = 0; i < byte_array->numelem; i++)
			{
				if(i < (k-1))
				{
					moving_sum += d[i];
					moving_avg = NAN;
				}
				else if( i == (k-1))
				{
					moving_sum += d[i];
					moving_avg = (float)moving_sum/(float)k;					
				}
				else{
					moving_sum = moving_sum + d[i] - d[i - k];
					moving_avg = (float)moving_sum/(float)k;
				}
				if(core_oph_type_cast(&moving_avg, result+i*param->result_elemsize, OPH_FLOAT, param->result_type))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		case OPH_SHORT:{
			//The average is a float number
			short *d = (short*)byte_array->content, moving_sum = 0;
			float moving_avg;
			for (i = 0; i < byte_array->numelem; i++)
			{
				if(i < (k-1))
				{
					moving_sum += d[i];
					moving_avg = NAN;
				}
				else if( i == (k-1))
				{
					moving_sum += d[i];
					moving_avg = (float)moving_sum/(float)k;					
				}
				else{
					moving_sum = moving_sum + d[i] - d[i - k];
					moving_avg = (float)moving_sum/(float)k;
				}
				if(core_oph_type_cast(&moving_avg, result+i*param->result_elemsize, OPH_FLOAT, param->result_type))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		case OPH_BYTE:{
			//The average is a float number
			char *d = (char*)byte_array->content, moving_sum = 0;
			float moving_avg;
			for (i = 0; i < byte_array->numelem; i++)
			{
				if(i < (k-1))
				{
					moving_sum += d[i];
					moving_avg = NAN;
				}
				else if( i == (k-1))
				{
					moving_sum += d[i];
					moving_avg = (float)moving_sum/(float)k;					
				}
				else{
					moving_sum = moving_sum + d[i] - d[i - k];
					moving_avg = (float)moving_sum/(float)k;
				}
				if(core_oph_type_cast(&moving_avg, result+i*param->result_elemsize, OPH_FLOAT, param->result_type))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		case OPH_LONG:{
			//The average is a double number
			long long *d = (long long*)byte_array->content, moving_sum = 0;
			double moving_avg;
			for (i = 0; i < byte_array->numelem; i++)
			{
				if(i < (k -1)){
					moving_sum += d[i];
					moving_avg = NAN;
				}
				else if( i == (k -1)){
					moving_sum += d[i];
					moving_avg = (double)moving_sum/(double)k;					
				}
				else{
					moving_sum = moving_sum + d[i] - d[i - k];
					moving_avg = (double)moving_sum/(double)k;
				}
				if(core_oph_type_cast(&moving_avg, result+i*param->result_elemsize, OPH_DOUBLE, param->result_type))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		default:
			pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
			return -1;
	}
    return 0;
}
int core_oph_simple_moving_avg_multi (oph_multistring* byte_array, oph_multistring* result, void* kk)
{
	long long i;
	//Used to count not NaN values (for float and double types)
	long long count = 0;
	short int not_nan = 0;

	long long k = (long long)(*(double*)kk);

	int j;
	char* in_string = byte_array->content;
	char* out_string = result->content;
	for (j=0;j<byte_array->num_measure;++j)
	{
		switch(byte_array->type[j])
		{
			case OPH_DOUBLE:{
				double *d, moving_sum = 0.0, moving_avg;
				for (i = 0; i < byte_array->numelem; i++)
				{
					d = (double*)(in_string+i*byte_array->blocksize);
					if(i < (k-1))
					{
						if (!isnan(*d)){
							moving_sum += *d;
							count++;
						}
						moving_avg = NAN;
					}
					else if( i == (k-1))
					{
						if (!isnan(*d)){
							moving_sum += *d;
							count++;
						}

						if(count){
							moving_avg = moving_sum/(double)count;					
						} else{
							moving_avg = NAN;	
						}
					}
					else{
						not_nan = (!isnan(*(double*)(in_string+(i-k)*byte_array->blocksize)) ? 1 : 0);
						if (!isnan(*d)){
							moving_sum = moving_sum + *d - (not_nan ? *(double*)(in_string+(i-k)*byte_array->blocksize) : 0);
							count = count + 1 - (not_nan ? 1 : 0);
						} else{
							moving_sum -= (not_nan ? *(double*)(in_string+(i-k)*byte_array->blocksize) : 0);
							count = count - (not_nan ? 1 : 0);
						}
						if(count){
							moving_avg = moving_sum/(double)count;					
						} else{
							moving_avg = NAN;	
						}			
					}
					if(core_oph_type_cast(&moving_avg, out_string+i*result->blocksize, byte_array->type[j], result->type[j]))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_FLOAT:{
				float *d, moving_sum = 0.0, moving_avg;
				for (i = 0; i < byte_array->numelem; i++)
				{
					d = (float*)(in_string+i*byte_array->blocksize);
					if(i < (k-1))
					{
						if (!isnan(*d)){
							moving_sum += *d;
							count++;
						}
						moving_avg = NAN;
					}
					else if( i == (k-1))
					{
						if (!isnan(*d)){
							moving_sum += *d;
							count++;
						}

						if(count)
						{
							moving_avg = moving_sum/(float)count;					
						} else{
							moving_avg = NAN;
						}
					}
					else{
						not_nan = (!isnan(*(float*)(in_string+(i-k)*byte_array->blocksize)) ? 1 : 0);
						if (!isnan(*d)){
							moving_sum = moving_sum + *d - (not_nan ? *(float*)(in_string+(i-k)*byte_array->blocksize) : 0);
							count = count + 1 - (not_nan ? 1 : 0);
						} else{
							moving_sum -= (not_nan ? *(float*)(in_string+(i-k)*byte_array->blocksize) : 0);
							count = count - (not_nan ? 1 : 0);
						}

						if(count){
							moving_avg = moving_sum/(float)count;
						} else{
							moving_avg = NAN;
						}			
					}
					if(core_oph_type_cast(&moving_avg, out_string+i*result->blocksize, byte_array->type[j], result->type[j]))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_INT:{
				//The average is a float number
				int *d, moving_sum = 0;
				float moving_avg;
				for (i = 0; i < byte_array->numelem; i++)
				{
					d = (int*)(in_string+i*byte_array->blocksize);
					if(i < (k-1))
					{
						moving_sum += *d;
						moving_avg = NAN;
					}
					else if( i == (k-1))
					{
						moving_sum += *d;
						moving_avg = (float)moving_sum/(float)k;					
					}
					else{
						moving_sum = moving_sum + *d - *(int*)(in_string+(i-k)*byte_array->blocksize);
						moving_avg = (float)moving_sum/(float)k;
					}
					if(core_oph_type_cast(&moving_avg, out_string+i*result->blocksize, OPH_FLOAT, result->type[j]))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_SHORT:{
				//The average is a float number
				short *d, moving_sum = 0;
				float moving_avg;
				for (i = 0; i < byte_array->numelem; i++)
				{
					d = (short*)(in_string+i*byte_array->blocksize);
					if(i < (k-1))
					{
						moving_sum += *d;
						moving_avg = NAN;
					}
					else if( i == (k-1))
					{
						moving_sum += *d;
						moving_avg = (float)moving_sum/(float)k;					
					}
					else{
						moving_sum = moving_sum + *d - *(short*)(in_string+(i-k)*byte_array->blocksize);
						moving_avg = (float)moving_sum/(float)k;
					}
					if(core_oph_type_cast(&moving_avg, out_string+i*result->blocksize, OPH_FLOAT, result->type[j]))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_BYTE:{
				//The average is a float number
				char *d, moving_sum = 0;
				float moving_avg;
				for (i = 0; i < byte_array->numelem; i++)
				{
					d = (char*)(in_string+i*byte_array->blocksize);
					if(i < (k-1))
					{
						moving_sum += *d;
						moving_avg = NAN;
					}
					else if( i == (k-1))
					{
						moving_sum += *d;
						moving_avg = (float)moving_sum/(float)k;					
					}
					else{
						moving_sum = moving_sum + *d - *(char*)(in_string+(i-k)*byte_array->blocksize);
						moving_avg = (float)moving_sum/(float)k;
					}
					if(core_oph_type_cast(&moving_avg, out_string+i*result->blocksize, OPH_FLOAT, result->type[j]))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_LONG:{
				//The average is a double number
				long long *d, moving_sum = 0;
				double moving_avg;
				for (i = 0; i < byte_array->numelem; i++)
				{
					d = (long long*)(in_string+i*byte_array->blocksize);
					if(i < (k -1)){
						moving_sum += *d;
						moving_avg = NAN;
					}
					else if( i == (k -1)){
						moving_sum += *d;
						moving_avg = (double)moving_sum/(double)k;					
					}
					else{
						moving_sum = moving_sum + *d - *(long long*)(in_string+(i-k)*byte_array->blocksize);
						moving_avg = (double)moving_sum/(double)k;
					}
					if(core_oph_type_cast(&moving_avg, out_string+i*result->blocksize, OPH_DOUBLE, result->type[j]))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			default:
				pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
				return -1;
		}
		in_string += byte_array->elemsize[j];
		out_string += result->elemsize[j];
	}
	return 0;
}

int core_oph_exponential_moving_avg (oph_generic_param *param, double a)
{
	oph_stringPtr byte_array = param->measure;
	char* result = param->result;
	long long i;
	switch(byte_array->type){
		case OPH_DOUBLE:{
			double *d = (double*)byte_array->content;
			if(core_oph_type_cast(d, result, byte_array->type, param->result_type))
			{
				pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
				return 1;
			}
			//Compute first element
			double prev_elem = d[0];
			double moving_avg;
			for (i = 1; i < byte_array->numelem; i++)
			{
				if (!isnan(d[i]))
				{
					moving_avg = (1 - a) *prev_elem + a*d[i];
				}
				else{
					moving_avg = prev_elem ;
				}	
				prev_elem = moving_avg;				
				if(core_oph_type_cast(&moving_avg, result+i*param->result_elemsize, byte_array->type, param->result_type))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		case OPH_FLOAT:{
			float *d = (float*)byte_array->content;
			if(core_oph_type_cast(d, result, byte_array->type, param->result_type))
			{
				pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
				return 1;
			}
			//Compute first element
			float prev_elem = d[0], moving_avg;
			for (i = 1; i < byte_array->numelem; i++)
			{
				if (!isnan(d[i]))
				{
					moving_avg = (1 - a) *prev_elem + a*d[i];
				}
				else{
					moving_avg = prev_elem ;
				}	
				prev_elem = moving_avg;								
				if(core_oph_type_cast(&moving_avg, result+i*param->result_elemsize, byte_array->type, param->result_type))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		case OPH_INT:{
			//The average is a float number
			int *d = (int*)byte_array->content;
			//Compute first element
			float prev_elem = (float)d[0], moving_avg;
			if(core_oph_type_cast(&prev_elem, result, OPH_FLOAT, param->result_type))
			{
				pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
				return 1;
			}
			for (i = 1; i < byte_array->numelem; i++)
			{
				moving_avg = (1 - a) *prev_elem + (float)a*d[i];
				prev_elem = moving_avg;
				if(core_oph_type_cast(&moving_avg, result+i*param->result_elemsize, OPH_FLOAT, param->result_type))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		case OPH_SHORT:{
			//The average is a float number
			short *d = (short*)byte_array->content;
			//Compute first element
			float prev_elem = (float)d[0], moving_avg;
			if(core_oph_type_cast(&prev_elem, result, OPH_FLOAT, param->result_type))
			{
				pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
				return 1;
			}
			for (i = 1; i < byte_array->numelem; i++)
			{
				moving_avg = (1 - a) *prev_elem + (float)a*d[i];
				prev_elem = moving_avg;
				if(core_oph_type_cast(&moving_avg, result+i*param->result_elemsize, OPH_FLOAT, param->result_type))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		case OPH_BYTE:{
			//The average is a float number
			char *d = (char*)byte_array->content;
			//Compute first element
			float prev_elem = (float)d[0], moving_avg;
			if(core_oph_type_cast(&prev_elem, result, OPH_FLOAT, param->result_type))
			{
				pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
				return 1;
			}
			for (i = 1; i < byte_array->numelem; i++)
			{
				moving_avg = (1 - a) *prev_elem + (float)a*d[i];
				prev_elem = moving_avg;
				if(core_oph_type_cast(&moving_avg, result+i*param->result_elemsize, OPH_FLOAT, param->result_type))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		case OPH_LONG:{
			//The average is a double number
			long long *d = (long long*)byte_array->content;
			//Compute first element
			double prev_elem = (double)d[0], moving_avg;
			if(core_oph_type_cast(&prev_elem, result, OPH_DOUBLE, param->result_type))
			{
				pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
				return 1;
			}
			for (i = 1; i < byte_array->numelem; i++)
			{
				moving_avg = (1 - a) *prev_elem + (double)a*d[i];
				prev_elem = moving_avg;
				if(core_oph_type_cast(&moving_avg, result+i*param->result_elemsize, OPH_DOUBLE, param->result_type))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
			}
			break;
		}
		default:
			pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
			return -1;
	}
    return 0;
}
int core_oph_exponential_moving_avg_multi (oph_multistring* byte_array, oph_multistring* result, void* aa)
{
	long long i;

	double a = (*(double*)aa);

	int j;
	char* in_string = byte_array->content;
	char* out_string = result->content;
	for (j=0;j<byte_array->num_measure;++j)
	{
		switch(byte_array->type[j])
		{
			case OPH_DOUBLE:{
				double *d = (double*)in_string;
				if(core_oph_type_cast(d, out_string, byte_array->type[j], result->type[j]))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
				//Compute first element
				double prev_elem = *d;
				double moving_avg;
				for (i = 1; i < byte_array->numelem; i++)
				{
					d = (double*)(in_string+i*byte_array->blocksize);
					if (!isnan(*d))
					{
						moving_avg = (1 - a)*prev_elem + a*(*d);
					}
					else{
						moving_avg = prev_elem ;
					}	
					prev_elem = moving_avg;				
					if(core_oph_type_cast(&moving_avg, out_string+i*result->blocksize, byte_array->type[j], result->type[j]))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_FLOAT:{
				float *d = (float*)in_string;
				if(core_oph_type_cast(d, out_string, byte_array->type[j], result->type[j]))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
				//Compute first element
				float prev_elem = *d, moving_avg;
				for (i = 1; i < byte_array->numelem; i++)
				{
					d = (float*)(in_string+i*byte_array->blocksize);
					if (!isnan(*d))
					{
						moving_avg = (1 - a)*prev_elem + a*(*d);
					}
					else{
						moving_avg = prev_elem ;
					}	
					prev_elem = moving_avg;								
					if(core_oph_type_cast(&moving_avg, out_string+i*result->blocksize, byte_array->type[j], result->type[j]))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_INT:{
				//The average is a float number
				int *d = (int*)in_string;
				//Compute first element
				float prev_elem = (float)(*d), moving_avg;
				if(core_oph_type_cast(&prev_elem, out_string, OPH_FLOAT, result->type[j]))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
				for (i = 1; i < byte_array->numelem; i++)
				{
					d = (int*)(in_string+i*byte_array->blocksize);
					moving_avg = (1 - a)*prev_elem + (float)(a*(*d));
					prev_elem = moving_avg;
					if(core_oph_type_cast(&moving_avg, out_string+i*result->blocksize, OPH_FLOAT, result->type[j]))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_SHORT:{
				//The average is a float number
				short *d = (short*)in_string;
				//Compute first element
				float prev_elem = (float)(*d), moving_avg;
				if(core_oph_type_cast(&prev_elem, out_string, OPH_FLOAT, result->type[j]))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
				for (i = 1; i < byte_array->numelem; i++)
				{
					d = (short*)(in_string+i*byte_array->blocksize);
					moving_avg = (1 - a)*prev_elem + (float)(a*(*d));
					prev_elem = moving_avg;
					if(core_oph_type_cast(&moving_avg, out_string+i*result->blocksize, OPH_FLOAT, result->type[j]))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_BYTE:{
				//The average is a float number
				char *d = (char*)in_string;
				//Compute first element
				float prev_elem = (float)(*d), moving_avg;
				if(core_oph_type_cast(&prev_elem, out_string, OPH_FLOAT, result->type[j]))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
				for (i = 1; i < byte_array->numelem; i++)
				{
					d = (char*)(in_string+i*byte_array->blocksize);
					moving_avg = (1 - a)*prev_elem + (float)(a*(*d));
					prev_elem = moving_avg;
					if(core_oph_type_cast(&moving_avg, out_string+i*result->blocksize, OPH_FLOAT, result->type[j]))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			case OPH_LONG:{
				//The average is a double number
				long long *d = (long long*)in_string;
				//Compute first element
				double prev_elem = (double)(*d), moving_avg;
				if(core_oph_type_cast(&prev_elem, out_string, OPH_DOUBLE, result->type[j]))
				{
					pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
					return 1;
				}
				for (i = 1; i < byte_array->numelem; i++)
				{
					d = (long long*)(in_string+i*byte_array->blocksize);
					moving_avg = (1 - a)*prev_elem + (double)(a*(*d));
					prev_elem = moving_avg;
					if(core_oph_type_cast(&moving_avg, out_string+i*result->blocksize, OPH_DOUBLE, result->type[j]))
					{
						pmesg(1,  __FILE__, __LINE__, "Unable to find result\n");
						return 1;
					}
				}
				break;
			}
			default:
				pmesg(1, __FILE__, __LINE__, "Type non recognized\n");
				return -1;
		}
		in_string += byte_array->elemsize[j];
		out_string += result->elemsize[j];
	}
	return 0;
}

int core_oph_oper_array_multi(oph_generic_param_multi* param)
{
	oph_multistring *measure = param->measure, *result = param->result;
	char *valueA, *valueB;
	size_t offset, offsetO;
	int i, j;
	offset=offsetO=0;
	for (i=0;i<measure->num_measure;++i)
	{
		char temporary[measure->elemsize[i]];
		for (j=0;j<measure->numelem;++j)
		{
			valueA = measure->content + offset + j*measure->blocksize;
			valueB = measure[1].content + offset + j*measure->blocksize;
			if (param->core_oph_oper(valueA, valueB, temporary, measure->type[i]))
			{
				pmesg(1, __FILE__, __LINE__, "Error in compute array\n");
				return 1;
			}
			if(core_oph_type_cast(temporary, result->content + offsetO + j*result->blocksize, measure->type[i], result->type[i]))
			{
				pmesg(1, __FILE__, __LINE__, "Error in compute array\n");
				return 1;
			}
		}
		offset += measure->elemsize[i];
		offsetO += result->elemsize[i];
	}
	
	return 0;
}

int core_oph_sum_array_multi(char* valueA, char* valueB, char* result, oph_type type)
{
	switch (type)
	{
		case OPH_INT:
		{
			*((int *)(result)) = *((int *)(valueA)) + *((int *)(valueB));
			break;
		}
		case OPH_SHORT:
		{
			*((short *)(result)) = *((short *)(valueA)) + *((short *)(valueB));
			break;
		}
		case OPH_BYTE:
		{
			*((char *)(result)) = *((char *)(valueA)) + *((char *)(valueB));
			break;
		}
		case OPH_LONG:
		{
			*((long long *)(result)) = *((long long *)(valueA)) + *((long long *)(valueB));
			break;
		}
		case OPH_FLOAT:
		{
			if (!isnan(*((float *)(valueA))))
			{
				if (!isnan(*((float *)(valueB)))) *((float *)(result)) = *((float *)(valueA)) + *((float *)(valueB));
				else *((float *)(result)) = *((float *)(valueA));
			}
			else *((float *)(result)) = *((float *)(valueB));
			break;
		}
		case OPH_DOUBLE:
		{
			if (!isnan(*((double *)(valueA))))
			{
				if (!isnan(*((double *)(valueB)))) *((double *)(result)) = *((double *)(valueA)) + *((double *)(valueB));
				else *((double *)(result)) = *((double *)(valueA));
			}
			else *((double *)(result)) = *((double *)(valueB));
			break;
		}
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			return -1;
	}
	return 0;
}
int core_oph_sum2_array_multi(char* valueA, char* valueB, char* result, oph_type type)
{
	switch (type)
	{
		case OPH_INT:
		{
			*((int *)(result)) = *((int *)(valueA)) * *((int *)(valueA)) + *((int *)(valueB)) * *((int *)(valueB));
			break;
		}
		case OPH_SHORT:
		{
			*((short *)(result)) = *((short *)(valueA)) * *((short *)(valueA)) + *((short *)(valueB)) * *((short *)(valueB));
			break;
		}
		case OPH_BYTE:
		{
			*((char *)(result)) = *((char *)(valueA)) * *((char *)(valueA)) + *((char *)(valueB)) * *((char *)(valueB));
			break;
		}
		case OPH_LONG:
		{
			*((long long *)(result)) = *((long long *)(valueA)) * *((long long *)(valueA)) + *((long long *)(valueB)) * *((long long *)(valueB));
			break;
		}
		case OPH_FLOAT:
		{
			if (!isnan(*((float *)(valueA))))
			{
				if (!isnan(*((float *)(valueB)))) *((float *)(result)) = *((float *)(valueA)) * *((float *)(valueA)) + *((float *)(valueB)) * *((float *)(valueB));
				else *((float *)(result)) = *((float *)(valueA)) * *((float *)(valueA));
			}
			else *((float *)(result)) = *((float *)(valueB)) * *((float *)(valueB));
			break;
		}
		case OPH_DOUBLE:
		{
			if (!isnan(*((double *)(valueA))))
			{
				if (!isnan(*((double *)(valueB)))) *((double *)(result)) = *((double *)(valueA)) * *((double *)(valueA)) + *((double *)(valueB)) * *((double *)(valueB));
				else *((double *)(result)) = *((double *)(valueA)) * *((double *)(valueA));
			}
			else *((double *)(result)) = *((double *)(valueB)) * *((double *)(valueB));
			break;
		}
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			return -1;
	}
	return 0;
}
int core_oph_sum3_array_multi(char* valueA, char* valueB, char* result, oph_type type)
{
	switch (type)
	{
		case OPH_INT:
		{
			*((int *)(result)) = *((int *)(valueA)) * *((int *)(valueA)) * *((int *)(valueA)) + *((int *)(valueB)) * *((int *)(valueB)) * *((int *)(valueB));
			break;
		}
		case OPH_SHORT:
		{
			*((short *)(result)) = *((short *)(valueA)) * *((short *)(valueA)) * *((short *)(valueA)) + *((short *)(valueB)) * *((short *)(valueB)) * *((short *)(valueB));
			break;
		}
		case OPH_BYTE:
		{
			*((char *)(result)) = *((char *)(valueA)) * *((char *)(valueA)) * *((char *)(valueA)) + *((char *)(valueB)) * *((char *)(valueB)) * *((char *)(valueB));
			break;
		}
		case OPH_LONG:
		{
			*((long long *)(result)) = *((long long *)(valueA)) * *((long long *)(valueA)) * *((long long *)(valueA)) + *((long long *)(valueB)) * *((long long *)(valueB)) * *((long long *)(valueB));
			break;
		}
		case OPH_FLOAT:
		{
			if (!isnan(*((float *)(valueA))))
			{
				if (!isnan(*((float *)(valueB)))) *((float *)(result)) = *((float *)(valueA)) * *((float *)(valueA)) * *((float *)(valueA)) + *((float *)(valueB)) * *((float *)(valueB)) * *((float *)(valueB));
				else *((float *)(result)) = *((float *)(valueA)) * *((float *)(valueA)) * *((float *)(valueA));
			}
			else *((float *)(result)) = *((float *)(valueB)) * *((float *)(valueB)) * *((float *)(valueB));
			break;
		}
		case OPH_DOUBLE:
		{
			if (!isnan(*((double *)(valueA))))
			{
				if (!isnan(*((double *)(valueB)))) *((double *)(result)) = *((double *)(valueA)) * *((double *)(valueA)) * *((double *)(valueA)) + *((double *)(valueB)) * *((double *)(valueB)) * *((double *)(valueB));
				else *((double *)(result)) = *((double *)(valueA)) * *((double *)(valueA)) * *((double *)(valueA));
			}
			else *((double *)(result)) = *((double *)(valueB)) * *((double *)(valueB)) * *((double *)(valueB));
			break;
		}
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			return -1;
	}
	return 0;
}
int core_oph_sum4_array_multi(char* valueA, char* valueB, char* result, oph_type type)
{
	switch (type)
	{
		case OPH_INT:
		{
			*((int *)(result)) = *((int *)(valueA)) * *((int *)(valueA)) * *((int *)(valueA)) * *((int *)(valueA)) + *((int *)(valueB)) * *((int *)(valueB)) * *((int *)(valueB)) * *((int *)(valueB));
			break;
		}
		case OPH_SHORT:
		{
			*((short *)(result)) = *((short *)(valueA)) * *((short *)(valueA)) * *((short *)(valueA)) * *((short *)(valueA)) + *((short *)(valueB)) * *((short *)(valueB)) * *((short *)(valueB)) * *((short *)(valueB));
			break;
		}
		case OPH_BYTE:
		{
			*((char *)(result)) = *((char *)(valueA)) * *((char *)(valueA)) * *((char *)(valueA)) * *((char *)(valueA)) + *((char *)(valueB)) * *((char *)(valueB)) * *((char *)(valueB)) * *((char *)(valueB));
			break;
		}
		case OPH_LONG:
		{
			*((long long *)(result)) = *((long long *)(valueA)) * *((long long *)(valueA)) * *((long long *)(valueA)) * *((long long *)(valueA)) + *((long long *)(valueB)) * *((long long *)(valueB)) * *((long long *)(valueB)) * *((long long *)(valueB));
			break;
		}
		case OPH_FLOAT:
		{
			if (!isnan(*((float *)(valueA))))
			{
				if (!isnan(*((float *)(valueB)))) *((float *)(result)) = *((float *)(valueA)) * *((float *)(valueA)) * *((float *)(valueA)) * *((float *)(valueA)) + *((float *)(valueB)) * *((float *)(valueB)) * *((float *)(valueB)) * *((float *)(valueB));
				else *((float *)(result)) = *((float *)(valueA)) * *((float *)(valueA)) * *((float *)(valueA)) * *((float *)(valueA));
			}
			else *((float *)(result)) = *((float *)(valueB)) * *((float *)(valueB)) * *((float *)(valueB)) * *((float *)(valueB));
			break;
		}
		case OPH_DOUBLE:
		{
			if (!isnan(*((double *)(valueA))))
			{
				if (!isnan(*((double *)(valueB)))) *((double *)(result)) = *((double *)(valueA)) * *((double *)(valueA)) * *((double *)(valueA)) * *((double *)(valueA)) + *((double *)(valueB)) * *((double *)(valueB)) * *((double *)(valueB)) * *((double *)(valueB));
				else *((double *)(result)) = *((double *)(valueA)) * *((double *)(valueA)) * *((double *)(valueA)) * *((double *)(valueA));
			}
			else *((double *)(result)) = *((double *)(valueB)) * *((double *)(valueB)) * *((double *)(valueB)) * *((double *)(valueB));
			break;
		}
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			return -1;
	}
	return 0;
}
int core_oph_mul_array_multi(char* valueA, char* valueB, char* result, oph_type type)
{
	switch (type)
	{
		case OPH_INT:
		{
			*((int *)(result)) = *((int *)(valueA)) * *((int *)(valueB));
			break;
		}
		case OPH_SHORT:
		{
			*((short *)(result)) = *((short *)(valueA)) * *((short *)(valueB));
			break;
		}
		case OPH_BYTE:
		{
			*((char *)(result)) = *((char *)(valueA)) * *((char *)(valueB));
			break;
		}
		case OPH_LONG:
		{
			*((long long *)(result)) = *((long long *)(valueA)) * *((long long *)(valueB));
			break;
		}
		case OPH_FLOAT:
		{
			if (!isnan(*((float *)(valueA))))
			{
				if (!isnan(*((float *)(valueB)))) *((float *)(result)) = *((float *)(valueA)) * *((float *)(valueB));
				else *((float *)(result)) = *((float *)(valueA));
			}
			else *((float *)(result)) = *((float *)(valueB));
			break;
		}
		case OPH_DOUBLE:
		{
			if (!isnan(*((double *)(valueA))))
			{
				if (!isnan(*((double *)(valueB)))) *((double *)(result)) = *((double *)(valueA)) * *((double *)(valueB));
				else *((double *)(result)) = *((double *)(valueA));
			}
			else *((double *)(result)) = *((double *)(valueB));
			break;
		}
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			return -1;
	}
	return 0;
}
int core_oph_mask_array_multi(char* valueA, char* valueB, char* result, oph_type type)
{
	switch (type)
	{
		case OPH_INT:
		{
			*((int *)(result)) = *((int *)(valueB)) ? *((int *)(valueA)) : (int)0;
			break;
		}
		case OPH_SHORT:
		{
			*((short *)(result)) = *((short *)(valueB)) ? *((short *)(valueA)) : (short)0;
			break;
		}
		case OPH_BYTE:
		{
			*((char *)(result)) = *((char *)(valueB)) ? *((char *)(valueA)) : (char)0;
			break;
		}
		case OPH_LONG:
		{
			*((long long *)(result)) = *((long long *)(valueB)) ? *((long long *)(valueA)) : (long long)0;
			break;
		}
		case OPH_FLOAT:
		{
			if (isnan(*((float *)(valueA))) || isnan(*((float *)(valueB)))) *((float *)(result)) = NAN;
			else *((float *)(result)) = *((float *)(valueB)) ? *((float *)(valueA)) : NAN;
			break;
		}
		case OPH_DOUBLE:
		{
			if (isnan(*((double *)(valueA))) || isnan(*((double *)(valueB)))) *((double *)(result)) = NAN;
			else *((double *)(result)) = *((double *)(valueB)) ? *((double *)(valueA)) : NAN;
			break;
		}
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			return -1;
	}
	return 0;
}
int core_oph_sub_array_multi(char* valueA, char* valueB, char* result, oph_type type)
{
	switch (type)
	{
		case OPH_INT:
		{
			*((int *)(result)) = *((int *)(valueA)) - *((int *)(valueB));
			break;
		}
		case OPH_SHORT:
		{
			*((short *)(result)) = *((short *)(valueA)) - *((short *)(valueB));
			break;
		}
		case OPH_BYTE:
		{
			*((char *)(result)) = *((char *)(valueA)) - *((char *)(valueB));
			break;
		}
		case OPH_LONG:
		{
			*((long long *)(result)) = *((long long *)(valueA)) - *((long long *)(valueB));
			break;
		}
		case OPH_FLOAT:
		{
			if (!isnan(*((float *)(valueA))))
			{
				if (!isnan(*((float *)(valueB)))) *((float *)(result)) = *((float *)(valueA)) - *((float *)(valueB));
				else *((float *)(result)) = - *((float *)(valueA));
			}
			else *((float *)(result)) = - *((float *)(valueB));
			break;
		}
		case OPH_DOUBLE:
		{
			if (!isnan(*((double *)(valueA))))
			{
				if (!isnan(*((double *)(valueB)))) *((double *)(result)) = *((double *)(valueA)) - *((double *)(valueB));
				else *((double *)(result)) = - *((double *)(valueA));
			}
			else *((double *)(result)) = - *((double *)(valueB));
			break;
		}
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			return -1;
	}
	return 0;
}
int core_oph_div_array_multi(char* valueA, char* valueB, char* result, oph_type type)
{
	switch (type)
	{
		case OPH_INT:
		{
			*((int *)(result)) = *((int *)(valueA)) / *((int *)(valueB));
			break;
		}
		case OPH_SHORT:
		{
			*((short *)(result)) = *((short *)(valueA)) / *((short *)(valueB));
			break;
		}
		case OPH_BYTE:
		{
			*((char *)(result)) = *((char *)(valueA)) / *((char *)(valueB));
			break;
		}
		case OPH_LONG:
		{
			*((long long *)(result)) = *((long long *)(valueA)) / *((long long *)(valueB));
			break;
		}
		case OPH_FLOAT:
		{
			if (!isnan(*((float *)(valueA))))
			{
				if (!isnan(*((float *)(valueB)))) *((float *)(result)) = *((float *)(valueA)) / *((float *)(valueB));
				else *((float *)(result)) = 1.0/ *((float *)(valueA));
			}
			else *((float *)(result)) = 1.0/ *((float *)(valueB));
			break;
		}
		case OPH_DOUBLE:
		{
			if (!isnan(*((double *)(valueA))))
			{
				if (!isnan(*((double *)(valueB)))) *((double *)(result)) = *((double *)(valueA)) / *((double *)(valueB));
				else *((double *)(result)) = 1.0/ *((double *)(valueA));
			}
			else *((double *)(result)) = 1.0/ *((double *)(valueB));
			break;
		}
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			return -1;
	}
	return 0;
}
int core_oph_abs_array_multi(char* valueA, char* valueB, char* result, oph_type type)
{
	switch (type)
	{
		case OPH_INT:
		{
			*((int *)(result)) = (int)sqrt(fabs( *((int *)(valueA)) * *((int *)(valueA)) + *((int *)(valueB)) * *((int *)(valueB))) );
			break;
		}
		case OPH_SHORT:
		{
			*((short *)(result)) = (short)sqrt(fabs( *((short *)(valueA)) * *((short *)(valueA)) + *((short *)(valueB)) * *((short *)(valueB))) );
			break;
		}
		case OPH_BYTE:
		{
			*((char *)(result)) = (char)sqrt(fabs( *((char *)(valueA)) * *((char *)(valueA)) + *((char *)(valueB)) * *((char *)(valueB))) );
			break;
		}
		case OPH_LONG:
		{
			*((long long *)(result)) = (long long)sqrt(fabs( *((long long *)(valueA)) * *((long long *)(valueA)) + *((long long *)(valueB)) * *((long long *)(valueB))) );
			break;
		}
		case OPH_FLOAT:
		{
			if (isnan(*((float *)(valueA))) || isnan(*((float *)(valueB)))) *((float *)(result)) = NAN;
			else *((float *)(result)) = (float)sqrt(fabs( *((float *)(valueA)) * *((float *)(valueA)) + *((float *)(valueB)) * *((float *)(valueB))) );
			break;
		}
		case OPH_DOUBLE:
		{
			if (isnan(*((double *)(valueA))) || isnan(*((double *)(valueB)))) *((float *)(result)) = NAN;
			else *((double *)(result)) = (double)sqrt(fabs( *((double *)(valueA)) * *((double *)(valueA)) + *((double *)(valueB)) * *((double *)(valueB))) );
			break;
		}
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			return -1;
	}
	return 0;
}
int core_oph_arg_array_multi(char* valueA, char* valueB, char* result, oph_type type)
{
	switch (type)
	{
		case OPH_INT:
		{
			*((int *)(result)) = (int)atan2( *((int *)(valueA)), *((int *)(valueB)));
			break;
		}
		case OPH_SHORT:
		{
			*((short *)(result)) = (short)atan2( *((short *)(valueA)), *((short *)(valueB)));
			break;
		}
		case OPH_BYTE:
		{
			*((char *)(result)) = (char)atan2( *((char *)(valueA)), *((char *)(valueB)));
			break;
		}
		case OPH_LONG:
		{
			*((long long *)(result)) = (long long)atan2( *((long long *)(valueA)), *((long long *)(valueB)) );
			break;
		}
		case OPH_FLOAT:
		{
			if (isnan(*((float *)(valueA))) || isnan(*((float *)(valueB)))) *((float *)(result)) = NAN;
			else *((float *)(result)) = (float)atan2( *((float *)(valueA)), *((float *)(valueB)));
			break;
		}
		case OPH_DOUBLE:
		{
			if (isnan(*((double *)(valueA))) || isnan(*((double *)(valueB)))) *((float *)(result)) = NAN;
			else *((double *)(result)) = (double)atan2( *((double *)(valueA)), *((double *)(valueB)));
			break;
		}
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			return -1;
	}
	return 0;
}
int core_oph_max_array_multi(char* valueA, char* valueB, char* result, oph_type type)
{
	switch (type)
	{
		case OPH_INT:
		{
			*((int *)(result)) = *((int *)(valueA)) > *((int *)(valueB)) ? *((int *)(valueA)) : *((int *)(valueB));
			break;
		}
		case OPH_SHORT:
		{
			*((short *)(result)) = *((short *)(valueA)) > *((short *)(valueB)) ? *((short *)(valueA)) : *((short *)(valueB));
			break;
		}
		case OPH_BYTE:
		{
			*((int *)(result)) = *((char *)(valueA)) > *((char *)(valueB)) ? *((char *)(valueA)) : *((char *)(valueB));
			break;
		}
		case OPH_LONG:
		{
			*((long long *)(result)) = *((long long *)(valueA)) > *((long long *)(valueB)) ? *((long long *)(valueA)) : *((long long *)(valueB));
			break;
		}
		case OPH_FLOAT:
		{
			if (!isnan(*((float *)(valueA))))
			{
				if (!isnan(*((float *)(valueB)))) *((float *)(result)) = *((float *)(valueA)) > *((float *)(valueB)) ? *((float *)(valueA)) : *((float *)(valueB));
				else *((float *)(result)) = *((float *)(valueA));
			}
			else *((float *)(result)) = *((float *)(valueB));
			break;
		}
		case OPH_DOUBLE:
		{
			if (!isnan(*((double *)(valueA))))
			{
				if (!isnan(*((double *)(valueB)))) *((double *)(result)) = *((double *)(valueA)) > *((double *)(valueB)) ? *((double *)(valueA)) : *((double *)(valueB));
				else *((double *)(result)) = *((double *)(valueA));
			}
			else *((double *)(result)) = *((double *)(valueB));
			break;
		}
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			return -1;
	}
	return 0;
}
int core_oph_min_array_multi(char* valueA, char* valueB, char* result, oph_type type)
{
	switch (type)
	{
		case OPH_INT:
		{
			*((int *)(result)) = *((int *)(valueA)) < *((int *)(valueB)) ? *((int *)(valueA)) : *((int *)(valueB));
			break;
		}
		case OPH_SHORT:
		{
			*((short *)(result)) = *((short *)(valueA)) < *((short *)(valueB)) ? *((short *)(valueA)) : *((short *)(valueB));
			break;
		}
		case OPH_BYTE:
		{
			*((char *)(result)) = *((char *)(valueA)) < *((char *)(valueB)) ? *((char *)(valueA)) : *((char *)(valueB));
			break;
		}
		case OPH_LONG:
		{
			*((long long *)(result)) = *((long long *)(valueA)) < *((long long *)(valueB)) ? *((long long *)(valueA)) : *((long long *)(valueB));
			break;
		}
		case OPH_FLOAT:
		{
			if (!isnan(*((float *)(valueA))))
			{
				if (!isnan(*((float *)(valueB)))) *((float *)(result)) = *((float *)(valueA)) < *((float *)(valueB)) ? *((float *)(valueA)) : *((float *)(valueB));
				else *((float *)(result)) = *((float *)(valueA));
			}
			else *((float *)(result)) = *((float *)(valueB));
			break;
		}
		case OPH_DOUBLE:
		{
			if (!isnan(*((double *)(valueA))))
			{
				if (!isnan(*((double *)(valueB)))) *((double *)(result)) = *((double *)(valueA)) < *((double *)(valueB)) ? *((double *)(valueA)) : *((double *)(valueB));
				else *((double *)(result)) = *((double *)(valueA));
			}
			else *((double *)(result)) = *((double *)(valueB));
			break;
		}
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			return -1;
	}
	return 0;
}

/*------------------------------------------------------------------|
 |               Array functions (END)                              |
 |------------------------------------------------------------------*/

/*------------------------------------------------------------------|
 |               Functions implementation (END)                     |
 |------------------------------------------------------------------*/
