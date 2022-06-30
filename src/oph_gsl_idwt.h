/*
    Ophidia Primitives
    Copyright (C) 2012-2022 CMCC Foundation

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

/* Standard C headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

/* Core function header */
#include "oph_core_gsl.h"

/* MySQL headers  */
#include <mysql.h>		// It contains UDF-related symbols and data structures

/* GSL DWT headers  */
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_wavelet.h>

// The following wavelet types are implemented:
//
// - DAUBECHIES (default)
// - DAUBECHIES_C
//   This is the Daubechies wavelet family of maximum phase with k/2 vanishing moments.
//   The implemented wavelets are k=4, 6, ..., 20, with k even.
//
// - HAAR
// - HAAR_C
//   This is the Haar wavelet. The only valid choice of k for the Haar wavelet is k=2.
//
// - BSPLINE
// - BSPLINE_C
//   This is the biorthogonal B-spline wavelet family of order (i,j).
//   The implemented values of k = 100*i + j are 103, 105, 202, 204, 206, 208, 301, 303, 305 307, 309.
//
// The centered forms of the wavelets align the coefficients of the various sub-bands on edges.
// Thus the resulting visualization of the coefficients of the wavelet transform in the phase plane
// is easier to understand.

#define DEFAULT_WAVELET_FAMILY DAUBECHIES
#define DEFAULT_WAVELET_MEMBER 4

#define DAUBECHIES      gsl_wavelet_daubechies
#define DAUBECHIES_C    gsl_wavelet_daubechies_centered
#define HAAR            gsl_wavelet_haar
#define HAAR_C          gsl_wavelet_haar_centered
#define BSPLINE         gsl_wavelet_bspline
#define BSPLINE_C       gsl_wavelet_bspline_centered


/* Struct to contain wavelet and workspace*/
typedef struct {
	gsl_wavelet *w;
	gsl_wavelet_workspace *ws;
	gsl_wavelet_type wfamily;
	size_t wmember;
} oph_gsl_dwt_extraspace;

/*------------------------------------------------------------------|
|       Functions' declarations (BEGIN)             |
|------------------------------------------------------------------*/

/* These must be right or mysqld will not find the symbol! */
my_bool oph_gsl_idwt_init(UDF_INIT * initid, UDF_ARGS * args, char *message);
void oph_gsl_idwt_deinit(UDF_INIT * initid);
char *oph_gsl_idwt(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error);

/*------------------------------------------------------------------|
|               Functions' declarations (END)                       |
|------------------------------------------------------------------*/
