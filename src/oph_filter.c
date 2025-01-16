/*
    Ophidia Primitives
    Copyright (C) 2012-2023 CMCC Foundation

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

#include "oph_filter.h"
#include <math.h>


int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_filter_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	/* oph_filter(input_OPH_TYPE, output_OPH_TYPE, measure, filter_type, cut_frequency1, cut_frequency2) */
	if (args->arg_count < 5 && args->arg_count > 6) {
		strcpy(message, "ERROR: Wrong arguments! oph_filter(input_OPH_TYPE, output_OPH_TYPE, measure, filter_type, cut_frequency1, cut_frequency2)");
		return 1;
	}

	for (i = 0; i < args->arg_count; i++) {
		if (i == 4 || i == 5) {
			if (args->arg_type[i] == STRING_RESULT) {
				strcpy(message, "ERROR: Wrong argument 'cut_frequency' to oph_filter function");
				return 1;
			}
			args->arg_type[i] = REAL_RESULT;
		} else if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_filter function");
			return 1;
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;

	return 0;
}

void oph_filter_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		if ((((oph_stringPtr) initid->ptr)[0]).extend) {
			free((((oph_stringPtr) initid->ptr)[0]).extend);
			(((oph_stringPtr) initid->ptr)[0]).extend = NULL;
		}
		if ((((oph_stringPtr) initid->ptr)[0]).content) {
			free((((oph_stringPtr) initid->ptr)[0]).content);
			(((oph_stringPtr) initid->ptr)[0]).content = NULL;
		}
		if ((((oph_stringPtr) initid->ptr)[0]).length) {
			free((((oph_stringPtr) initid->ptr)[0]).length);
			(((oph_stringPtr) initid->ptr)[0]).length = NULL;
		}
		if ((((oph_stringPtr) initid->ptr)[1]).length) {
			free((((oph_stringPtr) initid->ptr)[1]).length);
			(((oph_stringPtr) initid->ptr)[1]).length = NULL;
		}
		free(initid->ptr);
		initid->ptr = NULL;
	}
	if (initid->extension) {
		oph_gsl_fft_extraspace *extra = (oph_gsl_fft_extraspace *) initid->extension;
		if (extra->ws) {
			gsl_fft_real_workspace_free(extra->ws);
			extra->ws = NULL;
		}
		if (extra->wt) {
			gsl_fft_real_wavetable_free(extra->wt);
			extra->wt = NULL;
		}
		if (extra->hc) {
			gsl_fft_halfcomplex_wavetable_free(extra->hc);
			extra->hc = NULL;
		}
		if (extra->cws) {
			gsl_fft_complex_workspace_free(extra->cws);
			extra->cws = NULL;
		}
		if (extra->cwt) {
			gsl_fft_complex_wavetable_free(extra->cwt);
			extra->cwt = NULL;
		}
		free(initid->extension);
		initid->extension = NULL;
	}
}

char *oph_filter(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
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

	filter_oper oper = INVALID_FILTER_OPER;
	oph_stringPtr output;
	oph_stringPtr measure;

	if (!initid->ptr) {
		initid->ptr = malloc(2 * sizeof(oph_string));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating ptr\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}
		output = &(((oph_stringPtr) initid->ptr)[0]);
		measure = &(((oph_stringPtr) initid->ptr)[1]);


		core_set_type(measure, args->args[0], &(args->lengths[0]));
		if (measure->type != OPH_INT && measure->type != OPH_LONG && measure->type != OPH_FLOAT && measure->type != OPH_DOUBLE && measure->type != OPH_COMPLEX_INT
		    && measure->type != OPH_COMPLEX_LONG && measure->type != OPH_COMPLEX_FLOAT && measure->type != OPH_COMPLEX_DOUBLE) {
			pmesg(1, __FILE__, __LINE__, "Invalid input type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		core_set_type(output, args->args[1], &(args->lengths[1]));
		if (output->type != OPH_DOUBLE && output->type != OPH_COMPLEX_DOUBLE) {
			pmesg(1, __FILE__, __LINE__, "Invalid output type: oph_double or oph_complex_double required\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		measure->length = malloc(sizeof(unsigned long));
		if (!measure->length) {
			pmesg(1, __FILE__, __LINE__, "Error allocating length\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		output->length = malloc(sizeof(unsigned long));
		if (!output->length) {
			pmesg(1, __FILE__, __LINE__, "Error allocating length\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		*(measure->length) = args->lengths[2];

		if (core_set_elemsize(measure)) {
			pmesg(1, __FILE__, __LINE__, "Error on setting elements size\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (core_set_numelem(measure)) {
			pmesg(1, __FILE__, __LINE__, "Error on counting elements\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		if (core_set_elemsize(output)) {
			pmesg(1, __FILE__, __LINE__, "Error on setting elements size\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		*(output->length) = measure->numelem * output->elemsize;

		if (core_set_numelem(output)) {
			pmesg(1, __FILE__, __LINE__, "Error on counting elements\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		output->extend = (void *) calloc(1, *(output->length));
		if (!output->extend) {
			pmesg(1, __FILE__, __LINE__, "Error allocating extend array\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		output->content = (char *) calloc(1, *(output->length));
		if (!output->content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating output array\n");
			*length = 0;
			*is_null = 0;
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
		unsigned long long kmin = 0;
		unsigned long long kmax = 0;
		double n1 = 0;
		double x = 0;

		if (args->arg_count < 6) {
			if (*((double *) (args->args[4])) < 0 || *((double *) (args->args[4])) > 0.5) {
				pmesg(1, __FILE__, __LINE__, "Invalid frequency value\n");
				*length = 0;
				*is_null = 1;
				*error = 1;
				return NULL;
			}
			if (!strcasecmp(args->args[3], "LPF"))
				oper = LPF;
			else if (!strcasecmp(args->args[3], "HPF"))
				oper = HPF;
			else {
				oper = INVALID_FILTER_OPER;
				pmesg(1, __FILE__, __LINE__, "Invalid filter operator\n");
			}
		} else {
			if ((*((double *) (args->args[4])) < 0 || *((double *) (args->args[4])) > 0.5) || (*((double *) (args->args[5])) < 0 || *((double *) (args->args[5])) > 0.5)
			    || (*((double *) (args->args[4])) > *((double *) (args->args[5])))) {
				pmesg(1, __FILE__, __LINE__, "Invalid frequency values\n");
				*length = 0;
				*is_null = 1;
				*error = 1;
				return NULL;
			}
			if (!strcasecmp(args->args[3], "BPF"))
				oper = BPF;
			else if (!strcasecmp(args->args[3], "BSF"))
				oper = BSF;
			else {
				oper = INVALID_FILTER_OPER;
				pmesg(1, __FILE__, __LINE__, "Invalid filter operator\n");
			}
		}
		switch (output->type) {
			case OPH_DOUBLE:
				extra->n = (output->numelem % 2 == 0 ? output->numelem - 1 : output->numelem);
				x = (output->numelem % 2 == 0 ? 2 : 1);
				n1 = ((output->numelem - x) * 2) + x;
				switch (oper) {
					case LPF:
						//lowpass filter
						kmin = ceil(n1 * (*((double *) (args->args[4]))));
						extra->fmin = (kmin % 2 == 0 ? kmin + 1 : kmin + 2);
						extra->fmax = output->numelem;
						break;
					case HPF:
						//highpass filter
						kmax = floor(n1 * (*((double *) (args->args[4]))));
						extra->fmin = 0;
						extra->fmax = (kmax % 2 == 0 && kmax != 0 ? kmax - 1 : kmax);
						break;
					case BPF:
						//bandpass filter
						kmin = floor(n1 * (*((double *) (args->args[4]))));
						kmax = ceil(n1 * (*((double *) (args->args[5]))));
						extra->fmin = (kmin % 2 == 0 && kmin != 0 ? kmin - 1 : kmin);
						extra->fmax = (kmax % 2 == 0 ? kmax + 1 : kmax + 2);
						break;
					case BSF:
						//inverse bandpass filter
						kmin = ceil(n1 * (*((double *) (args->args[4]))));
						kmax = floor(n1 * (*((double *) (args->args[5]))));
						extra->fmin = (kmin % 2 == 0 ? kmin + 1 : kmin + 2);
						extra->fmax = (kmax % 2 == 0 ? kmax - 1 : kmax);
						if (extra->fmin > extra->fmax)
							extra->fmax = extra->fmin;
						break;
					default:
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
				}
				break;
			case OPH_COMPLEX_DOUBLE:
				switch (oper) {
					case LPF:
						//lowpass filter
						kmin = ceil(output->numelem * (*((double *) (args->args[4])))) + 1;
						extra->fmin = (*((double *) (args->args[4])) == 0 ? kmin * 2 : (kmin - 1) * 2);
						extra->fmax = (output->numelem % 2 == 0 ? output->numelem + 2 : output->numelem + 1);
						break;
					case HPF:
						//highpass filter
						kmax = floor(output->numelem * (*((double *) (args->args[4])))) + 1;
						extra->fmin = 0;
						extra->fmax = (kmax - 1) * 2;
						break;
					case BPF:
						//bandpass filter
						kmin = floor(output->numelem * (*((double *) (args->args[4])))) + 1;
						kmax = ceil(output->numelem * (*((double *) (args->args[5])))) + 1;
						extra->fmin = (kmin - 1) * 2;
						extra->fmax = (kmax - 1) * 2;
						extra->n = (output->numelem % 2 == 0 ? output->numelem + 2 : output->numelem + 1);
						break;
					case BSF:
						//inverse bandpass filter
						kmin = ceil(output->numelem * (*((double *) (args->args[4])))) + 1;
						kmax = floor(output->numelem * (*((double *) (args->args[5])))) + 1;
						extra->fmin = (kmin - 1) * 2;
						extra->fmax = (kmax - 1) * 2;
						if (extra->fmin > extra->fmax)
							extra->fmax = extra->fmin;
						break;
					default:
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
				}
				break;
			default:;
		}

		switch (output->type) {
			case OPH_DOUBLE:
				extra->wt = gsl_fft_real_wavetable_alloc(output->numelem);
				extra->hc = gsl_fft_halfcomplex_wavetable_alloc(output->numelem);
				if (!extra->wt) {
					pmesg(1, __FILE__, __LINE__, "Error allocating wavetable\n");
					*length = 0;
					*is_null = 1;
					*error = 1;
					return NULL;
				}
				if (!extra->hc) {
					pmesg(1, __FILE__, __LINE__, "Error allocating halfcomplex wavetable\n");
					*length = 0;
					*is_null = 1;
					*error = 1;
					return NULL;
				}
				break;
			case OPH_COMPLEX_DOUBLE:
				extra->cwt = gsl_fft_complex_wavetable_alloc(output->numelem);
				if (!extra->cwt) {
					pmesg(1, __FILE__, __LINE__, "Error allocating complex wavetable\n");
					*length = 0;
					*is_null = 1;
					*error = 1;
					return NULL;
				}
				break;
			default:;
		}

		switch (output->type) {
			case OPH_DOUBLE:
				extra->ws = gsl_fft_real_workspace_alloc(output->numelem);
				if (!extra->ws) {
					pmesg(1, __FILE__, __LINE__, "Error allocating workspace\n");
					*length = 0;
					*is_null = 1;
					*error = 1;
					return NULL;
				}
				break;
			case OPH_COMPLEX_DOUBLE:
				extra->cws = gsl_fft_complex_workspace_alloc(output->numelem);
				if (!extra->cws) {
					pmesg(1, __FILE__, __LINE__, "Error allocating complex workspace\n");
					*length = 0;
					*is_null = 1;
					*error = 1;
					return NULL;
				}

				break;
			default:;
		}
	}

	output = &(((oph_stringPtr) initid->ptr)[0]);
	measure = &(((oph_stringPtr) initid->ptr)[1]);

	measure->content = args->args[2];

	oph_gsl_fft_extraspace *extra = (oph_gsl_fft_extraspace *) initid->extension;


	gsl_set_error_handler_off();

	switch (output->type) {
		case OPH_DOUBLE:
			{

				int i;
				for (i = 0; i < measure->numelem; i++) {
					if (core_oph_type_cast(measure->content + (i * measure->elemsize), output->extend + (i * output->elemsize), measure->type, output->type, NULL)) {
						pmesg(1, __FILE__, __LINE__, "Error casting output\n");
						*length = 0;
						*is_null = 0;
						*error = 1;
						return NULL;
					}
				}
				if (gsl_fft_real_transform((double *) output->extend, 1, output->numelem, extra->wt, extra->ws)) {
					pmesg(1, __FILE__, __LINE__, "Error computing fft\n");
					*length = 0;
					*is_null = 1;
					*error = 1;
					return NULL;
				}
				switch (oper) {
					case HPF:
						if (extra->fmin < extra->fmax)
							((double *) (output->extend))[0] = 0.0;
						if (extra->fmax == -1)
							extra->fmax = 1;
						for (i = extra->fmin + 1; i < extra->fmax; i = i + 2) {
							printf("%d", i);
							((double *) (output->extend))[i] = 0.0;
							if (i != 0 || i != extra->n)
								((double *) (output->extend))[i + 1] = 0.0;
						}
						break;
					case LPF:
					case BSF:
						for (i = extra->fmin; i < extra->fmax; i = i + 2) {
							((double *) (output->extend))[i] = 0.0;
							if (i != 0 || i != extra->n)
								((double *) (output->extend))[i + 1] = 0.0;
						}
						break;
					case BPF:
						for (i = 0; i < extra->fmin; i++) {
							((double *) (output->extend))[i] = 0.0;
						}
						for (i = extra->fmax; i < output->numelem; i++) {
							((double *) (output->extend))[i] = 0.0;
						}
						break;
					default:;
				}
				memcpy(output->content, output->extend, *(output->length));
				if (gsl_fft_halfcomplex_inverse((double *) output->content, 1, output->numelem, extra->hc, extra->ws)) {
					pmesg(1, __FILE__, __LINE__, "Error computing ifft\n");
					*length = 0;
					*is_null = 1;
					*error = 1;
					return NULL;
				}

				break;
			}
		case OPH_COMPLEX_DOUBLE:
			{
				int i;
				switch (measure->type) {
					case OPH_COMPLEX_INT:
						for (i = 0; i < measure->numelem * 2; i++) {
							((double *) (output->extend))[i] = (double) ((int *) (measure->content))[i];
						}
						break;
					case OPH_COMPLEX_LONG:
						for (i = 0; i < measure->numelem * 2; i++) {
							((double *) (output->extend))[i] = (double) ((long long *) (measure->content))[i];
						}
						break;
					case OPH_COMPLEX_FLOAT:
						for (i = 0; i < measure->numelem * 2; i++) {
							((double *) (output->extend))[i] = (double) ((float *) (measure->content))[i];
						}
						break;

					default:
						memcpy(output->extend, measure->content, *(output->length));
				}

				if (gsl_fft_complex_forward((gsl_complex_packed_array) output->extend, 1, output->numelem, extra->cwt, extra->cws)) {
					pmesg(1, __FILE__, __LINE__, "Error computing fft\n");
					*length = 0;
					*is_null = 1;
					*error = 1;
					return NULL;
				}
				switch (oper) {
					case LPF:
					case HPF:
					case BSF:
						for (i = extra->fmin; i < extra->fmax; i = i + 2) {
							((gsl_complex_packed_array) (output->extend))[i] = 0.0;
							((gsl_complex_packed_array) (output->extend))[i + 1] = 0.0;
							if (i != 0) {
								((gsl_complex_packed_array) (output->extend))[(output->numelem * 2) - i] = 0.0;
								((gsl_complex_packed_array) (output->extend))[(output->numelem * 2) - i + 1] = 0.0;
							}
						}
						break;
					case BPF:
						for (i = 0; i < extra->fmin; i = i + 2) {
							((gsl_complex_packed_array) (output->extend))[i] = 0.0;
							((gsl_complex_packed_array) (output->extend))[i + 1] = 0.0;
							if (i != 0) {
								((gsl_complex_packed_array) (output->extend))[(output->numelem * 2) - i] = 0.0;
								((gsl_complex_packed_array) (output->extend))[(output->numelem * 2) - i + 1] = 0.0;
							}
						}
						for (i = extra->fmax; i < extra->n; i = i + 2) {
							((gsl_complex_packed_array) (output->extend))[i] = 0.0;
							((gsl_complex_packed_array) (output->extend))[i + 1] = 0.0;
							if (i != 0) {
								((gsl_complex_packed_array) (output->extend))[(output->numelem * 2) - i] = 0.0;
								((gsl_complex_packed_array) (output->extend))[(output->numelem * 2) - i + 1] = 0.0;
							}
						}
						break;
					default:;
				}
				memcpy(output->content, output->extend, *(output->length));
				if (gsl_fft_complex_inverse((gsl_complex_packed_array) output->content, 1, output->numelem, extra->cwt, extra->cws)) {
					pmesg(1, __FILE__, __LINE__, "Error computing ifft\n");
					*length = 0;
					*is_null = 1;
					*error = 1;
					return NULL;
				}
				break;
			}
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
