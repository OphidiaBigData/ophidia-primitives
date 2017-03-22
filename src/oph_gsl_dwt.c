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

#include "oph_gsl_dwt.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_gsl_dwt_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	/* oph_gsl_dwt(input_OPH_TYPE, output_OPH_TYPE, measure,[WAVELET_FAMILY],[WAVELET_MEMBER]) */
	if (args->arg_count < 3 || args->arg_count > 5) {
		strcpy(message, "ERROR: Wrong arguments! oph_gsl_dwt(input_OPH_TYPE, output_OPH_TYPE, measure,[WAVELET_FAMILY],[WAVELET_MEMBER])");
		return 1;
	}

	for (i = 0; i < 3; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_gsl_dwt function");
			return 1;
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;

	return 0;
}

void oph_gsl_dwt_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		oph_stringPtr output = (oph_stringPtr) initid->ptr;
		if (output->content) {
			free(output->content);
			output->content = NULL;
		}
		if (output->length) {
			free(output->length);
			output->length = NULL;
		}
		free(initid->ptr);
		initid->ptr = NULL;
	}
	if (initid->extension) {
		oph_gsl_dwt_extraspace *extra = (oph_gsl_dwt_extraspace *) initid->extension;
		if (extra->ws) {
			gsl_wavelet_workspace_free(extra->ws);
			extra->ws = NULL;
		}
		if (extra->w) {
			gsl_wavelet_free(extra->w);
			extra->w = NULL;
		}
		free(initid->extension);
		initid->extension = NULL;
	}
}

char *oph_gsl_dwt(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	gsl_set_error_handler_off();

	if (*error) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	if (*is_null || !args->lengths[2]) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
	}

	if (!initid->ptr) {

		initid->ptr = (char *) calloc(1, sizeof(oph_string));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}

		oph_stringPtr output = (oph_stringPtr) initid->ptr;

		core_set_type(output, args->args[1], &(args->lengths[1]));
		if (output->type != OPH_DOUBLE) {
			pmesg(1, __FILE__, __LINE__, "Invalid type: oph_double required\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}

		core_set_type(output, args->args[0], &(args->lengths[0]));
		if (output->type != OPH_DOUBLE) {
			pmesg(1, __FILE__, __LINE__, "Invalid type: oph_double required\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}

		output->length = (unsigned long *) calloc(1, sizeof(unsigned long));
		if (!output->length) {
			pmesg(1, __FILE__, __LINE__, "Error allocating length\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}

		*(output->length) = args->lengths[2];

		if (core_set_elemsize(output)) {
			pmesg(1, __FILE__, __LINE__, "Error on setting element size\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		if (core_set_numelem(output)) {
			pmesg(1, __FILE__, __LINE__, "Error on counting result elements\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		// input numbers must be a power of 2 and at least 2!
		if (output->numelem == 1 || (output->numelem & (output->numelem - 1))) {
			pmesg(1, __FILE__, __LINE__, "input numbers must be a power of 2 and at least 2!\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		output->content = (char *) calloc(1, *(output->length));
		if (!output->content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result string\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}

		initid->extension = calloc(1, sizeof(oph_gsl_dwt_extraspace));
		if (!initid->extension) {
			pmesg(1, __FILE__, __LINE__, "Error allocating extra space\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}

		oph_gsl_dwt_extraspace *extra = (oph_gsl_dwt_extraspace *) initid->extension;

		extra->wfamily = *(DEFAULT_WAVELET_FAMILY);
		extra->wmember = DEFAULT_WAVELET_MEMBER;

		if (args->arg_count == 4) {
			if (args->arg_type[3] == STRING_RESULT) {
				if (!strcasecmp(args->args[3], "DAUBECHIES")) {
					extra->wfamily = *(DAUBECHIES);
					extra->wmember = 4;
				} else if (!strcasecmp(args->args[3], "DAUBECHIES_C")) {
					extra->wfamily = *(DAUBECHIES_C);
					extra->wmember = 4;
				} else if (!strcasecmp(args->args[3], "HAAR")) {
					extra->wfamily = *(HAAR);
					extra->wmember = 2;
				} else if (!strcasecmp(args->args[3], "HAAR_C")) {
					extra->wfamily = *(HAAR_C);
					extra->wmember = 2;
				} else if (!strcasecmp(args->args[3], "BSPLINE")) {
					extra->wfamily = *(BSPLINE);
					extra->wmember = 103;
				} else if (!strcasecmp(args->args[3], "BSPLINE_C")) {
					extra->wfamily = *(BSPLINE_C);
					extra->wmember = 103;
				} else {
					pmesg(1, __FILE__, __LINE__, "Invalid wavelet family\n");
					*length = 0;
					*is_null = 1;
					*error = 1;
					return NULL;
				}
			} else if (args->arg_type[3] == INT_RESULT) {
				extra->wmember = *((long long *) args->args[3]);
			} else {
				pmesg(1, __FILE__, __LINE__, "Invalid parameter type\n");
				*length = 0;
				*is_null = 1;
				*error = 1;
				return NULL;
			}
		}
		if (args->arg_count == 5) {
			int i;
			for (i = 3; i < args->arg_count; i++) {
				if (args->arg_type[i] == STRING_RESULT) {
					if (!strcasecmp(args->args[i], "DAUBECHIES")) {
						extra->wfamily = *(DAUBECHIES);
					} else if (!strcasecmp(args->args[i], "DAUBECHIES_C")) {
						extra->wfamily = *(DAUBECHIES_C);
					} else if (!strcasecmp(args->args[i], "HAAR")) {
						extra->wfamily = *(HAAR);
					} else if (!strcasecmp(args->args[i], "HAAR_C")) {
						extra->wfamily = *(HAAR_C);
					} else if (!strcasecmp(args->args[i], "BSPLINE")) {
						extra->wfamily = *(BSPLINE);
					} else if (!strcasecmp(args->args[i], "BSPLINE_C")) {
						extra->wfamily = *(BSPLINE_C);
					} else {
						pmesg(1, __FILE__, __LINE__, "Invalid wavelet family\n");
						*length = 0;
						*is_null = 1;
						*error = 1;
						return NULL;
					}
				} else if (args->arg_type[i] == INT_RESULT) {
					extra->wmember = *((long long *) args->args[i]);
				} else {
					pmesg(1, __FILE__, __LINE__, "Invalid parameter type\n");
					*length = 0;
					*is_null = 1;
					*error = 1;
					return NULL;
				}
			}
		}

		extra->w = gsl_wavelet_alloc(&(extra->wfamily), extra->wmember);
		if (!extra->w) {
			pmesg(1, __FILE__, __LINE__, "Error allocating wavelet: insufficient memory or unsupported member\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}

		extra->ws = gsl_wavelet_workspace_alloc(output->numelem);
		if (!extra->ws) {
			pmesg(1, __FILE__, __LINE__, "Error allocating workspace\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}

	}

	oph_stringPtr output = (oph_stringPtr) initid->ptr;
	oph_gsl_dwt_extraspace *extra = (oph_gsl_dwt_extraspace *) initid->extension;

	memcpy(output->content, args->args[2], *(output->length));

	if (gsl_wavelet_transform_forward(extra->w, (double *) output->content, 1, output->numelem, extra->ws)) {
		pmesg(1, __FILE__, __LINE__, "Error computing dwt\n");
		*length = 0;
		*is_null = 1;
		*error = 1;
		return NULL;
	}

	*length = *(output->length);
	*error = 0;
	*is_null = 0;

	return output->content;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
