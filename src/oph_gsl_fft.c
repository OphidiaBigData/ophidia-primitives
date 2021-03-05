/*
    Ophidia Primitives
    Copyright (C) 2012-2020 CMCC Foundation

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

#include "oph_gsl_fft.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_gsl_fft_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	/* oph_gsl_fft(input_OPH_TYPE, output_OPH_TYPE, measure) */
	if (args->arg_count != 3) {
		strcpy(message, "ERROR: Wrong arguments! oph_gsl_fft(input_OPH_TYPE, output_OPH_TYPE, measure)");
		return 1;
	}

	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_gsl_fft function");
			return 1;
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;

	return 0;
}

void oph_gsl_fft_deinit(UDF_INIT * initid)
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
		oph_gsl_fft_extraspace *extra = (oph_gsl_fft_extraspace *) initid->extension;
		if (extra->ws) {
			gsl_fft_complex_workspace_free(extra->ws);
			extra->ws = NULL;
		}
		if (extra->wt) {
			gsl_fft_complex_wavetable_free(extra->wt);
			extra->wt = NULL;
		}
		free(initid->extension);
		initid->extension = NULL;
	}
}

char *oph_gsl_fft(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
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
		if (output->type != OPH_COMPLEX_DOUBLE) {
			pmesg(1, __FILE__, __LINE__, "Invalid type: oph_complex_double required\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}

		core_set_type(output, args->args[0], &(args->lengths[0]));
		if (output->type != OPH_DOUBLE && output->type != OPH_COMPLEX_DOUBLE) {
			pmesg(1, __FILE__, __LINE__, "Invalid type: oph_complex_double or oph_double required\n");
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

		switch (output->type) {
			case OPH_DOUBLE:
				*(output->length) = args->lengths[2] * 2;
				break;
			case OPH_COMPLEX_DOUBLE:
				*(output->length) = args->lengths[2];
				break;
			default:
				pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
				*length = 0;
				*is_null = 1;
				*error = 1;
				return NULL;
		}

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

		output->content = (char *) calloc(1, *(output->length));
		if (!output->content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result string\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}

		initid->extension = calloc(1, sizeof(oph_gsl_fft_extraspace));
		if (!initid->extension) {
			pmesg(1, __FILE__, __LINE__, "Error allocating extra space\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}

		oph_gsl_fft_extraspace *extra = (oph_gsl_fft_extraspace *) initid->extension;

		switch (output->type) {
			case OPH_DOUBLE:
				extra->wt = gsl_fft_complex_wavetable_alloc(output->numelem / 2);
				break;
			case OPH_COMPLEX_DOUBLE:
				extra->wt = gsl_fft_complex_wavetable_alloc(output->numelem);
				break;
			default:;
		}
		if (!extra->wt) {
			pmesg(1, __FILE__, __LINE__, "Error allocating wavetable\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}

		switch (output->type) {
			case OPH_DOUBLE:
				extra->ws = gsl_fft_complex_workspace_alloc(output->numelem / 2);
				break;
			case OPH_COMPLEX_DOUBLE:
				extra->ws = gsl_fft_complex_workspace_alloc(output->numelem);
				break;
			default:;
		}
		if (!extra->ws) {
			pmesg(1, __FILE__, __LINE__, "Error allocating workspace\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}

	}

	oph_stringPtr output = (oph_stringPtr) initid->ptr;
	oph_gsl_fft_extraspace *extra = (oph_gsl_fft_extraspace *) initid->extension;

	gsl_set_error_handler_off();

	switch (output->type) {
		case OPH_DOUBLE:
			{
				int i, j = 0;
				memset(output->content, 0, *(output->length));
				for (i = 0; i < (output->numelem) / 2; i++) {
					if (core_oph_type_cast(args->args[2] + (i * sizeof(double)), output->content + (j * sizeof(double)), OPH_DOUBLE, OPH_DOUBLE, NULL)) {
						pmesg(1, __FILE__, __LINE__, "Error casting output\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
					j += 2;
				}
				if (gsl_fft_complex_forward((gsl_complex_packed_array) output->content, 1, (output->numelem) / 2, extra->wt, extra->ws)) {
					pmesg(1, __FILE__, __LINE__, "Error computing fft\n");
					*length = 0;
					*is_null = 1;
					*error = 1;
					return NULL;
				}
				break;
			}
		case OPH_COMPLEX_DOUBLE:
			memcpy(output->content, args->args[2], *(output->length));
			if (gsl_fft_complex_forward((gsl_complex_packed_array) output->content, 1, output->numelem, extra->wt, extra->ws)) {
				pmesg(1, __FILE__, __LINE__, "Error computing fft\n");
				*length = 0;
				*is_null = 1;
				*error = 1;
				return NULL;
			}
			break;
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
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
