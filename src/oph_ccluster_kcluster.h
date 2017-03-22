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

/*/ Standard C headers */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Core function header */
#include "oph_core.h"

/* MySQL headers  */
#include <mysql.h>		// It contains UDF-related symbols and data structures

/* The C Clustering Library */
#include "cluster.h"

#define OPH_CCLUSTER_KCLUSTER_KMEANS 0
#define OPH_CCLUSTER_KCLUSTER_KMEDIANS 1
#define OPH_CCLUSTER_KCLUSTER_CENTROIDS 0
#define OPH_CCLUSTER_KCLUSTER_LABELS 1
#define OPH_CCLUSTER_KCLUSTER_ALL 2

typedef struct {
	int k;
	int method;
	int level;
	int npass;
	oph_type type;
	int npoints;
	double **data;
	int **mask;
	int *clusterid;
	double **cdata;
	int **cmask;
} oph_ccluster_kcluster_extra;

/*------------------------------------------------------------------|
|		Functions' declarations (BEGIN)			    |
|------------------------------------------------------------------*/

/* These must be right or mysqld will not find the symbol! */
my_bool oph_ccluster_kcluster_init(UDF_INIT * initid, UDF_ARGS * args, char *message);
void oph_ccluster_kcluster_deinit(UDF_INIT * initid);
char *oph_ccluster_kcluster(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error);

/*------------------------------------------------------------------|
|               Functions' declarations (END)                       |
|------------------------------------------------------------------*/
