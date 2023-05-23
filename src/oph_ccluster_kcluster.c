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

#include "oph_ccluster_kcluster.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_ccluster_kcluster_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	if (args->arg_count < 4 || args->arg_count > 7) {
		strcpy(message, "ERROR: Wrong arguments! oph_ccluster_kcluster(input_OPH_TYPE, output_OPH_TYPE, measure,k,[KMEANS|KMEDIANS],[CENTROIDS|LABELS|ALL],[npass])");
		return 1;
	}

	int i;
	for (i = 0; i < 3; i++) {
		if (args->arg_type[i] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_ccluster_kcluster function");
			return 1;
		}
	}

	if (args->arg_type[3] != INT_RESULT) {
		strcpy(message, "ERROR: Wrong arguments to oph_ccluster_kcluster function");
		return 1;
	}

	for (i = 4; i < args->arg_count; i++) {
		if (i < 6) {
			if (args->arg_type[i] != STRING_RESULT) {
				strcpy(message, "ERROR: Wrong arguments to oph_ccluster_kcluster function");
				return 1;
			}
		} else {
			if (args->arg_type[i] != INT_RESULT) {
				strcpy(message, "ERROR: Wrong arguments to oph_ccluster_kcluster function");
				return 1;
			}
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;
	return 0;
}

void oph_ccluster_kcluster_deinit(UDF_INIT *initid)
{
	//Free allocated space
	oph_stringPtr output = (oph_stringPtr) initid->ptr;
	oph_ccluster_kcluster_extra *extra = (oph_ccluster_kcluster_extra *) initid->extension;

	if (output->content) {
		free(output->content);
		output->content = NULL;
	}
	if (output->length) {
		free(output->length);
		output->length = NULL;
	}
	if (initid->ptr) {
		free(initid->ptr);
		initid->ptr = NULL;
	}

	if (extra->data) {
		int i;
		for (i = 0; i < extra->npoints; i++) {
			if (extra->data[i]) {
				free(extra->data[i]);
				extra->data[i] = NULL;
			}
		}
		free(extra->data);
		extra->data = NULL;
	}
	if (extra->mask) {
		int i;
		for (i = 0; i < extra->npoints; i++) {
			if (extra->mask[i]) {
				free(extra->mask[i]);
				extra->mask[i] = NULL;
			}
		}
		free(extra->mask);
		extra->mask = NULL;
	}
	if (extra->clusterid) {
		free(extra->clusterid);
		extra->clusterid = NULL;
	}
	if (extra->cdata) {
		int i;
		for (i = 0; i < extra->k; i++) {
			if (extra->cdata[i]) {
				free(extra->cdata[i]);
				extra->cdata[i] = NULL;
			}
		}
		free(extra->cdata);
		extra->cdata = NULL;
	}
	if (extra->cmask) {
		int i;
		for (i = 0; i < extra->k; i++) {
			if (extra->cmask[i]) {
				free(extra->cmask[i]);
				extra->cmask[i] = NULL;
			}
		}
		free(extra->cmask);
		extra->cmask = NULL;
	}
	if (initid->extension) {
		free(initid->extension);
		initid->extension = NULL;
	}
}

char *oph_ccluster_kcluster(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
	if (!initid->ptr) {
		*error = 0;
		*is_null = 0;

		initid->ptr = (char *) calloc(1, sizeof(oph_string));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}
		initid->extension = calloc(1, sizeof(oph_ccluster_kcluster_extra));
		if (!initid->extension) {
			pmesg(1, __FILE__, __LINE__, "Error allocating extension\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}

		oph_ccluster_kcluster_extra *extra = (oph_ccluster_kcluster_extra *) initid->extension;

		extra->k = (int) *((long long *) args->args[3]);	// set cluster number
		extra->method = OPH_CCLUSTER_KCLUSTER_KMEANS;	// default method
		extra->level = OPH_CCLUSTER_KCLUSTER_ALL;	// default level
		extra->npass = 1;	// default npass
		extra->type = OPH_DOUBLE;	// default input type

		if (!strncasecmp(args->args[0], "OPH_INT", args->lengths[0]))
			extra->type = OPH_INT;
		else if (!strncasecmp(args->args[0], "OPH_SHORT", args->lengths[0]))
			extra->type = OPH_SHORT;
		else if (!strncasecmp(args->args[0], "OPH_BYTE", args->lengths[0]))
			extra->type = OPH_BYTE;
		else if (!strncasecmp(args->args[0], "OPH_LONG", args->lengths[0]))
			extra->type = OPH_LONG;
		else if (!strncasecmp(args->args[0], "OPH_FLOAT", args->lengths[0]))
			extra->type = OPH_FLOAT;
		else if (!strncasecmp(args->args[0], "OPH_DOUBLE", args->lengths[0]))
			extra->type = OPH_DOUBLE;
		else {
			pmesg(1, __FILE__, __LINE__, "Invalid input data type!\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		int i;
		for (i = 4; i < args->arg_count; i++) {
			if (args->arg_type[i] == INT_RESULT) {	// npass
				extra->npass = (int) *((long long *) args->args[i]);
				if (extra->npass < 1) {
					pmesg(1, __FILE__, __LINE__, "npass must be >= 1!\n");
					*length = 0;
					*is_null = 0;
					*error = 1;
					return NULL;
				}
			} else if (args->arg_type[i] == STRING_RESULT) {
				if (!strncasecmp(args->args[i], "KMEANS", args->lengths[i])) {
					extra->method = OPH_CCLUSTER_KCLUSTER_KMEANS;
				} else if (!strncasecmp(args->args[i], "KMEDIANS", args->lengths[i])) {
					extra->method = OPH_CCLUSTER_KCLUSTER_KMEDIANS;
				} else if (!strncasecmp(args->args[i], "CENTROIDS", args->lengths[i])) {
					extra->level = OPH_CCLUSTER_KCLUSTER_CENTROIDS;
				} else if (!strncasecmp(args->args[i], "LABELS", args->lengths[i])) {
					extra->level = OPH_CCLUSTER_KCLUSTER_LABELS;
				} else if (!strncasecmp(args->args[i], "ALL", args->lengths[i])) {
					extra->level = OPH_CCLUSTER_KCLUSTER_ALL;
				} else {
					pmesg(1, __FILE__, __LINE__, "invalid argument %d!\n", i);
					*length = 0;
					*is_null = 0;
					*error = 1;
					return NULL;
				}
			} else {
				pmesg(1, __FILE__, __LINE__, "wrong type for argument %d!\n", i);
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
		}

		extra->npoints = args->lengths[2] / core_sizeof(extra->type);

		extra->data = (double **) malloc(sizeof(double *) * extra->npoints);
		if (!extra->data) {
			pmesg(1, __FILE__, __LINE__, "Error allocating memory\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		for (i = 0; i < extra->npoints; i++) {
			extra->data[i] = (double *) malloc(sizeof(double));
			if (!extra->data[i]) {
				pmesg(1, __FILE__, __LINE__, "Error allocating memory\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
		}

		extra->mask = (int **) malloc(sizeof(int *) * extra->npoints);
		if (!extra->mask) {
			pmesg(1, __FILE__, __LINE__, "Error allocating memory\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		for (i = 0; i < extra->npoints; i++) {
			extra->mask[i] = (int *) malloc(sizeof(int));
			if (!extra->mask[i]) {
				pmesg(1, __FILE__, __LINE__, "Error allocating memory\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
		}

		extra->clusterid = (int *) malloc(sizeof(int) * extra->npoints);
		if (!extra->clusterid) {
			pmesg(1, __FILE__, __LINE__, "Error allocating memory\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}

		oph_stringPtr output = (oph_stringPtr) initid->ptr;

		if (extra->level == OPH_CCLUSTER_KCLUSTER_CENTROIDS) {
			extra->cdata = (double **) malloc(sizeof(double *) * extra->k);
			if (!extra->cdata) {
				pmesg(1, __FILE__, __LINE__, "Error allocating memory\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			for (i = 0; i < extra->k; i++) {
				extra->cdata[i] = (double *) malloc(sizeof(double));
				if (!extra->cdata[i]) {
					pmesg(1, __FILE__, __LINE__, "Error allocating memory\n");
					*length = 0;
					*is_null = 0;
					*error = 1;
					return NULL;
				}
			}

			extra->cmask = (int **) malloc(sizeof(int *) * extra->k);
			if (!extra->cmask) {
				pmesg(1, __FILE__, __LINE__, "Error allocating memory\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			for (i = 0; i < extra->k; i++) {
				extra->cmask[i] = (int *) malloc(sizeof(int));
				if (!extra->cmask[i]) {
					pmesg(1, __FILE__, __LINE__, "Error allocating memory\n");
					*length = 0;
					*is_null = 0;
					*error = 1;
					return NULL;
				}
			}

			unsigned long len = (unsigned long) strlen("OPH_DOUBLE");
			core_set_type(output, "OPH_DOUBLE", &len);
			if (core_set_elemsize(output)) {
				pmesg(1, __FILE__, __LINE__, "Error on setting result elements size\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}

			output->length = (unsigned long *) malloc(sizeof(unsigned long));
			if (!output->length) {
				pmesg(1, __FILE__, __LINE__, "Error allocating length\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			*(output->length) = (unsigned long) extra->k * output->elemsize;

			output->content = (char *) malloc(*(output->length));
			if (!output->content) {
				pmesg(1, __FILE__, __LINE__, "Error allocating output\n");
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
		} else if (extra->level == OPH_CCLUSTER_KCLUSTER_ALL) {
			extra->cdata = (double **) malloc(sizeof(double *) * extra->k);
			if (!extra->cdata) {
				pmesg(1, __FILE__, __LINE__, "Error allocating memory\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			for (i = 0; i < extra->k; i++) {
				extra->cdata[i] = (double *) malloc(sizeof(double));
				if (!extra->cdata[i]) {
					pmesg(1, __FILE__, __LINE__, "Error allocating memory\n");
					*length = 0;
					*is_null = 0;
					*error = 1;
					return NULL;
				}
			}

			extra->cmask = (int **) malloc(sizeof(int *) * extra->k);
			if (!extra->cmask) {
				pmesg(1, __FILE__, __LINE__, "Error allocating memory\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			for (i = 0; i < extra->k; i++) {
				extra->cmask[i] = (int *) malloc(sizeof(int));
				if (!extra->cmask[i]) {
					pmesg(1, __FILE__, __LINE__, "Error allocating memory\n");
					*length = 0;
					*is_null = 0;
					*error = 1;
					return NULL;
				}
			}

			unsigned long len = (unsigned long) strlen("OPH_DOUBLE");
			core_set_type(output, "OPH_DOUBLE", &len);
			if (core_set_elemsize(output)) {
				pmesg(1, __FILE__, __LINE__, "Error on setting result elements size\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}

			output->length = (unsigned long *) malloc(sizeof(unsigned long));
			if (!output->length) {
				pmesg(1, __FILE__, __LINE__, "Error allocating length\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			*(output->length) = (unsigned long) extra->npoints * output->elemsize;

			output->content = (char *) malloc(*(output->length));
			if (!output->content) {
				pmesg(1, __FILE__, __LINE__, "Error allocating output\n");
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
		} else {
			unsigned long len = (unsigned long) strlen("OPH_INT");
			core_set_type(output, "OPH_INT", &len);
			if (core_set_elemsize(output)) {
				pmesg(1, __FILE__, __LINE__, "Error on setting result elements size\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}

			output->length = (unsigned long *) malloc(sizeof(unsigned long));
			if (!output->length) {
				pmesg(1, __FILE__, __LINE__, "Error allocating length\n");
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			}
			*(output->length) = (unsigned long) extra->npoints * output->elemsize;

			output->content = (char *) malloc(*(output->length));
			if (!output->content) {
				pmesg(1, __FILE__, __LINE__, "Error allocating output\n");
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
		}
	}

	if (*error) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	if (*is_null) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
	}

	oph_stringPtr output = (oph_stringPtr) initid->ptr;
	oph_ccluster_kcluster_extra *extra = (oph_ccluster_kcluster_extra *) initid->extension;

	double weight[1] = { 1.0 };
	char method = (extra->method == OPH_CCLUSTER_KCLUSTER_KMEANS) ? 'a' : 'm';
	double err;
	int ifound;
	int n;

	switch (extra->type) {
		case OPH_INT:
			for (n = 0; n < extra->npoints; n++) {
				extra->data[n][0] = (double) ((int *) (args->args[2]))[n];
				extra->mask[n][0] = 1;
			}
			break;
		case OPH_SHORT:
			for (n = 0; n < extra->npoints; n++) {
				extra->data[n][0] = (double) ((short *) (args->args[2]))[n];
				extra->mask[n][0] = 1;
			}
			break;
		case OPH_BYTE:
			for (n = 0; n < extra->npoints; n++) {
				extra->data[n][0] = (double) ((char *) (args->args[2]))[n];
				extra->mask[n][0] = 1;
			}
			break;
		case OPH_LONG:
			for (n = 0; n < extra->npoints; n++) {
				extra->data[n][0] = (double) ((long long *) (args->args[2]))[n];
				extra->mask[n][0] = 1;
			}
			break;
		case OPH_FLOAT:
			for (n = 0; n < extra->npoints; n++) {
				extra->data[n][0] = (double) ((float *) (args->args[2]))[n];
				if (isnan(extra->data[n][0])) {
					extra->mask[n][0] = 0;
				} else {
					extra->mask[n][0] = 1;
				}
			}
			break;
		case OPH_DOUBLE:
			for (n = 0; n < extra->npoints; n++) {
				extra->data[n][0] = ((double *) (args->args[2]))[n];
				if (isnan(extra->data[n][0])) {
					extra->mask[n][0] = 0;
				} else {
					extra->mask[n][0] = 1;
				}
			}
			break;
		default:
			pmesg(1, __FILE__, __LINE__, "Invalid input type\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
	}

	kcluster(extra->k, extra->npoints, 1, extra->data, extra->mask, weight, 0, extra->npass, method, 'e', extra->clusterid, &err, &ifound);

	if (ifound == 0) {
		pmesg(1, __FILE__, __LINE__, "k > number of points!\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	if (ifound == -1) {
		pmesg(1, __FILE__, __LINE__, "Error allocating memory for clustering\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	if (extra->level == OPH_CCLUSTER_KCLUSTER_CENTROIDS) {
		if (!getclustercentroids(extra->k, extra->npoints, 1, extra->data, extra->mask, extra->clusterid, extra->cdata, extra->cmask, 0, method)) {
			pmesg(1, __FILE__, __LINE__, "Error allocating memory for centroids retrieval\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		for (n = 0; n < extra->k; n++) {
			if (extra->cmask[n][0] == 0) {
				pmesg(1, __FILE__, __LINE__, "All cluster members are missing for cluster %d\n", n);
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			} else {
				((double *) (output->content))[n] = extra->cdata[n][0];
			}
		}
	} else if (extra->level == OPH_CCLUSTER_KCLUSTER_ALL) {
		if (!getclustercentroids(extra->k, extra->npoints, 1, extra->data, extra->mask, extra->clusterid, extra->cdata, extra->cmask, 0, method)) {
			pmesg(1, __FILE__, __LINE__, "Error allocating memory for centroids retrieval\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
		for (n = 0; n < extra->npoints; n++) {
			if (extra->cmask[extra->clusterid[n]][0] == 0) {
				pmesg(1, __FILE__, __LINE__, "All cluster members are missing for cluster %d\n", extra->clusterid[n]);
				*length = 0;
				*is_null = 0;
				*error = 1;
				return NULL;
			} else {
				((double *) (output->content))[n] = extra->cdata[extra->clusterid[n]][0];
			}
		}
	} else {
		memcpy(output->content, extra->clusterid, *(output->length));
	}

	*length = *(output->length);
	return output->content;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
