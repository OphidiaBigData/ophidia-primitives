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

#include "oph_gsl_complex_to_rect.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_gsl_complex_to_rect_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	int i = 0;
	/* oph_gsl_complex_to_rect(input_OPH_TYPE, output_OPH_TYPE, measure) */
	if (args->arg_count != 3) {
		strcpy(message, "ERROR: Wrong arguments! oph_gsl_complex_to_rect(input_OPH_TYPE, output_OPH_TYPE, measure)");
		return 1;
	}

	for (i = 0; i < args->arg_count; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_gsl_complex_to_rect function");
			return 1;
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;
	return 0;
}

void oph_gsl_complex_to_rect_deinit(UDF_INIT * initid)
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
		free(initid->extension);
		initid->extension = NULL;
	}
}

char *oph_gsl_complex_to_rect(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
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
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		oph_stringPtr output = (oph_stringPtr) initid->ptr;

		initid->extension = (char *) calloc(1, sizeof(oph_string));
		if (!initid->extension) {
			pmesg(1, __FILE__, __LINE__, "Error allocating measure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		oph_stringPtr measure = (oph_stringPtr) initid->extension;

		core_set_type(measure, args->args[0], &(args->lengths[0]));
		if (measure->type != OPH_COMPLEX_INT && measure->type != OPH_COMPLEX_LONG && measure->type != OPH_COMPLEX_FLOAT && measure->type != OPH_COMPLEX_DOUBLE) {
			pmesg(1, __FILE__, __LINE__, "Invalid input type: complex required\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		measure->length = &(args->lengths[2]);

		if (core_set_elemsize(measure)) {
			pmesg(1, __FILE__, __LINE__, "Error on setting element size\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		if (core_set_numelem(measure)) {
			pmesg(1, __FILE__, __LINE__, "Error on counting result elements\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		core_set_type(output, args->args[1], &(args->lengths[1]));
		if (output->type != OPH_COMPLEX_INT && output->type != OPH_COMPLEX_LONG && output->type != OPH_COMPLEX_FLOAT && output->type != OPH_COMPLEX_DOUBLE) {
			pmesg(1, __FILE__, __LINE__, "Invalid output type: complex required\n");
			*length = 0;
			*is_null = 0;
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

		output->length = (unsigned long *) calloc(1, sizeof(unsigned long));
		if (!output->length) {
			pmesg(1, __FILE__, __LINE__, "Error allocating length\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		*(output->length) = measure->numelem * output->elemsize;
		output->numelem = measure->numelem;

		output->content = (char *) calloc(1, *(output->length));
		if (!output->content) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	oph_stringPtr output = (oph_stringPtr) initid->ptr;
	oph_stringPtr measure = (oph_stringPtr) initid->extension;
	measure->content = args->args[2];

	int i, j = 0;
	gsl_complex z;
	double rho, theta;
	switch (measure->type) {
		case OPH_COMPLEX_INT:
			switch (output->type) {
				case OPH_COMPLEX_INT:
					for (i = 0; i < output->numelem * 2; i += 2) {
						rho = (double) ((int *) (args->args[2]))[i];	//real part
						theta = (double) ((int *) (args->args[2]))[i + 1];	//imag part
						z = gsl_complex_polar(rho, theta);
						if (core_oph_type_cast(&(z.dat[0]), output->content + (j * sizeof(int)), OPH_DOUBLE, OPH_INT, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						if (core_oph_type_cast(&(z.dat[1]), output->content + ((j + 1) * sizeof(int)), OPH_DOUBLE, OPH_INT, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						j += 2;
					}
					break;
				case OPH_COMPLEX_LONG:
					for (i = 0; i < output->numelem * 2; i += 2) {
						rho = (double) ((int *) (args->args[2]))[i];	//real part
						theta = (double) ((int *) (args->args[2]))[i + 1];	//imag part
						z = gsl_complex_polar(rho, theta);
						if (core_oph_type_cast(&(z.dat[0]), output->content + (j * sizeof(long long)), OPH_DOUBLE, OPH_LONG, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						if (core_oph_type_cast(&(z.dat[1]), output->content + ((j + 1) * sizeof(long long)), OPH_DOUBLE, OPH_LONG, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						j += 2;
					}
					break;
				case OPH_COMPLEX_FLOAT:
					for (i = 0; i < output->numelem * 2; i += 2) {
						rho = (double) ((int *) (args->args[2]))[i];	//real part
						theta = (double) ((int *) (args->args[2]))[i + 1];	//imag part
						z = gsl_complex_polar(rho, theta);
						if (core_oph_type_cast(&(z.dat[0]), output->content + (j * sizeof(float)), OPH_DOUBLE, OPH_FLOAT, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						if (core_oph_type_cast(&(z.dat[1]), output->content + ((j + 1) * sizeof(float)), OPH_DOUBLE, OPH_FLOAT, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						j += 2;
					}
					break;
				case OPH_COMPLEX_DOUBLE:
					for (i = 0; i < output->numelem * 2; i += 2) {
						rho = (double) ((int *) (args->args[2]))[i];	//real part
						theta = (double) ((int *) (args->args[2]))[i + 1];	//imag part
						z = gsl_complex_polar(rho, theta);
						if (core_oph_type_cast(&(z.dat[0]), output->content + (j * sizeof(double)), OPH_DOUBLE, OPH_DOUBLE, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						if (core_oph_type_cast(&(z.dat[1]), output->content + ((j + 1) * sizeof(double)), OPH_DOUBLE, OPH_DOUBLE, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						j += 2;
					}
					break;
				default:
					pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
					*length = 0;
					*is_null = 0;
					*error = 1;
					return NULL;
			}
			break;
		case OPH_COMPLEX_LONG:
			switch (output->type) {
				case OPH_COMPLEX_INT:
					for (i = 0; i < output->numelem * 2; i += 2) {
						rho = (double) ((long long *) (args->args[2]))[i];	//real part
						theta = (double) ((long long *) (args->args[2]))[i + 1];	//imag part
						z = gsl_complex_polar(rho, theta);
						if (core_oph_type_cast(&(z.dat[0]), output->content + (j * sizeof(int)), OPH_DOUBLE, OPH_INT, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						if (core_oph_type_cast(&(z.dat[1]), output->content + ((j + 1) * sizeof(int)), OPH_DOUBLE, OPH_INT, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						j += 2;
					}
					break;
				case OPH_COMPLEX_LONG:
					for (i = 0; i < output->numelem * 2; i += 2) {
						rho = (double) ((long long *) (args->args[2]))[i];	//real part
						theta = (double) ((long long *) (args->args[2]))[i + 1];	//imag part
						z = gsl_complex_polar(rho, theta);
						if (core_oph_type_cast(&(z.dat[0]), output->content + (j * sizeof(long long)), OPH_DOUBLE, OPH_LONG, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						if (core_oph_type_cast(&(z.dat[1]), output->content + ((j + 1) * sizeof(long long)), OPH_DOUBLE, OPH_LONG, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						j += 2;
					}
					break;
				case OPH_COMPLEX_FLOAT:
					for (i = 0; i < output->numelem * 2; i += 2) {
						rho = (double) ((long long *) (args->args[2]))[i];	//real part
						theta = (double) ((long long *) (args->args[2]))[i + 1];	//imag part
						z = gsl_complex_polar(rho, theta);
						if (core_oph_type_cast(&(z.dat[0]), output->content + (j * sizeof(float)), OPH_DOUBLE, OPH_FLOAT, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						if (core_oph_type_cast(&(z.dat[1]), output->content + ((j + 1) * sizeof(float)), OPH_DOUBLE, OPH_FLOAT, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						j += 2;
					}
					break;
				case OPH_COMPLEX_DOUBLE:
					for (i = 0; i < output->numelem * 2; i += 2) {
						rho = (double) ((long long *) (args->args[2]))[i];	//real part
						theta = (double) ((long long *) (args->args[2]))[i + 1];	//imag part
						z = gsl_complex_polar(rho, theta);
						if (core_oph_type_cast(&(z.dat[0]), output->content + (j * sizeof(double)), OPH_DOUBLE, OPH_DOUBLE, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						if (core_oph_type_cast(&(z.dat[1]), output->content + ((j + 1) * sizeof(double)), OPH_DOUBLE, OPH_DOUBLE, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						j += 2;
					}
					break;
				default:
					pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
					*length = 0;
					*is_null = 0;
					*error = 1;
					return NULL;
			}
			break;
		case OPH_COMPLEX_FLOAT:
			switch (output->type) {
				case OPH_COMPLEX_INT:
					for (i = 0; i < output->numelem * 2; i += 2) {
						rho = (double) ((float *) (args->args[2]))[i];	//real part
						theta = (double) ((float *) (args->args[2]))[i + 1];	//imag part
						z = gsl_complex_polar(rho, theta);
						if (core_oph_type_cast(&(z.dat[0]), output->content + (j * sizeof(int)), OPH_DOUBLE, OPH_INT, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						if (core_oph_type_cast(&(z.dat[1]), output->content + ((j + 1) * sizeof(int)), OPH_DOUBLE, OPH_INT, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						j += 2;
					}
					break;
				case OPH_COMPLEX_LONG:
					for (i = 0; i < output->numelem * 2; i += 2) {
						rho = (double) ((float *) (args->args[2]))[i];	//real part
						theta = (double) ((float *) (args->args[2]))[i + 1];	//imag part
						z = gsl_complex_polar(rho, theta);
						if (core_oph_type_cast(&(z.dat[0]), output->content + (j * sizeof(long long)), OPH_DOUBLE, OPH_LONG, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						if (core_oph_type_cast(&(z.dat[1]), output->content + ((j + 1) * sizeof(long long)), OPH_DOUBLE, OPH_LONG, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						j += 2;
					}
					break;
				case OPH_COMPLEX_FLOAT:
					for (i = 0; i < output->numelem * 2; i += 2) {
						rho = (double) ((float *) (args->args[2]))[i];	//real part
						theta = (double) ((float *) (args->args[2]))[i + 1];	//imag part
						z = gsl_complex_polar(rho, theta);
						if (core_oph_type_cast(&(z.dat[0]), output->content + (j * sizeof(float)), OPH_DOUBLE, OPH_FLOAT, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						if (core_oph_type_cast(&(z.dat[1]), output->content + ((j + 1) * sizeof(float)), OPH_DOUBLE, OPH_FLOAT, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						j += 2;
					}
					break;
				case OPH_COMPLEX_DOUBLE:
					for (i = 0; i < output->numelem * 2; i += 2) {
						rho = (double) ((float *) (args->args[2]))[i];	//real part
						theta = (double) ((float *) (args->args[2]))[i + 1];	//imag part
						z = gsl_complex_polar(rho, theta);
						if (core_oph_type_cast(&(z.dat[0]), output->content + (j * sizeof(double)), OPH_DOUBLE, OPH_DOUBLE, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						if (core_oph_type_cast(&(z.dat[1]), output->content + ((j + 1) * sizeof(double)), OPH_DOUBLE, OPH_DOUBLE, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						j += 2;
					}
					break;
				default:
					pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
					*length = 0;
					*is_null = 0;
					*error = 1;
					return NULL;
			}
			break;
		case OPH_COMPLEX_DOUBLE:
			switch (output->type) {
				case OPH_COMPLEX_INT:
					for (i = 0; i < output->numelem * 2; i += 2) {
						rho = ((double *) (args->args[2]))[i];	//real part
						theta = ((double *) (args->args[2]))[i + 1];	//imag part
						z = gsl_complex_polar(rho, theta);
						if (core_oph_type_cast(&(z.dat[0]), output->content + (j * sizeof(int)), OPH_DOUBLE, OPH_INT, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						if (core_oph_type_cast(&(z.dat[1]), output->content + ((j + 1) * sizeof(int)), OPH_DOUBLE, OPH_INT, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						j += 2;
					}
					break;
				case OPH_COMPLEX_LONG:
					for (i = 0; i < output->numelem * 2; i += 2) {
						rho = ((double *) (args->args[2]))[i];	//real part
						theta = ((double *) (args->args[2]))[i + 1];	//imag part
						z = gsl_complex_polar(rho, theta);
						if (core_oph_type_cast(&(z.dat[0]), output->content + (j * sizeof(long long)), OPH_DOUBLE, OPH_LONG, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						if (core_oph_type_cast(&(z.dat[1]), output->content + ((j + 1) * sizeof(long long)), OPH_DOUBLE, OPH_LONG, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						j += 2;
					}
					break;
				case OPH_COMPLEX_FLOAT:
					for (i = 0; i < output->numelem * 2; i += 2) {
						rho = ((double *) (args->args[2]))[i];	//real part
						theta = ((double *) (args->args[2]))[i + 1];	//imag part
						z = gsl_complex_polar(rho, theta);
						if (core_oph_type_cast(&(z.dat[0]), output->content + (j * sizeof(float)), OPH_DOUBLE, OPH_FLOAT, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						if (core_oph_type_cast(&(z.dat[1]), output->content + ((j + 1) * sizeof(float)), OPH_DOUBLE, OPH_FLOAT, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						j += 2;
					}
					break;
				case OPH_COMPLEX_DOUBLE:
					for (i = 0; i < output->numelem * 2; i += 2) {
						rho = ((double *) (args->args[2]))[i];	//real part
						theta = ((double *) (args->args[2]))[i + 1];	//imag part
						z = gsl_complex_polar(rho, theta);
						if (core_oph_type_cast(&(z.dat[0]), output->content + (j * sizeof(double)), OPH_DOUBLE, OPH_DOUBLE, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						if (core_oph_type_cast(&(z.dat[1]), output->content + ((j + 1) * sizeof(double)), OPH_DOUBLE, OPH_DOUBLE, NULL)) {
							pmesg(1, __FILE__, __LINE__, "Error casting output\n");
							*length = 0;
							*is_null = 0;
							*error = 1;
							return NULL;
						}
						j += 2;
					}
					break;
				default:
					pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
					*length = 0;
					*is_null = 0;
					*error = 1;
					return NULL;
			}
			break;
		default:
			pmesg(1, __FILE__, __LINE__, "Type not recognized\n");
			*length = 0;
			*is_null = 0;
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
