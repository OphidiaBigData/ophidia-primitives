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

#include "oph_sum_array_r.h"

int msglevel = 1;

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_sum_array_r_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	/* oph_sum_array_r(measureA, measureB, OPH_TYPEA, OPH_TYPEB) */
	int i = 0;
	if (args->arg_count < 2 || args->arg_count > 4) {
		strcpy(message, "ERROR: oph_sum_array_r(measure_a, measure_b, [OPH_TYPE_a], [OPH_TYPE_b])");
		return 1;
	}

	if (args->arg_type[0] != STRING_RESULT || args->arg_type[1] != STRING_RESULT) {
		strcpy(message, "ERROR: Wrong arguments to oph_sum_array_r function");
		return 1;
	}

	if (args->arg_count > 2) {
		if (args->arg_type[2] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_sum_array_r function");
			return 1;
		}
	}

	if (args->arg_count > 3) {
		if (args->arg_type[3] != STRING_RESULT) {
			strcpy(message, "ERROR: Wrong arguments to oph_sum_array_r function");
			return 1;
		}
	}

	initid->ptr = NULL;
	initid->extension = NULL;

	/* Plugin specific initializations */
	th_data *data_r = malloc(sizeof(th_data));
	data_r->curr_args = (void *) args;	// Used in thread function
	data_r->exit_flag = 0;
	if (barrier_init(&(data_r->barr_start), NTHREAD + 1)) {
		strcpy(message, "Could not create a barrier");
		return 1;
	}
	if (barrier_init(&(data_r->barr_end), NTHREAD + 1)) {
		strcpy(message, "Could not create a barrier");
		return 1;
	}
	initid->extension = (char *) data_r;
	for (i = 0; i < NTHREAD; i++) {
		pthread_create(&(data_r->thread[i]), NULL, sum_array_r, (void *) (initid));
	}


	return 0;
}

void oph_sum_array_r_deinit(UDF_INIT * initid)
{
	int i;
	((th_data *) (initid->extension))->exit_flag = 1;
	barrier_wait(&(((th_data *) (initid->extension))->barr_start));

	for (i = 0; i < NTHREAD; i++)
		pthread_join((((th_data *) (initid->extension))->thread[i]), NULL);

	barrier_destroy(&(((th_data *) (initid->extension))->barr_start));
	barrier_destroy(&(((th_data *) (initid->extension))->barr_end));

	//Free allocated space
	if (initid->ptr) {
		free(initid->ptr);
		initid->ptr = NULL;
	}
	if (initid->extension) {
		free(initid->extension);
		initid->extension = NULL;
	}
}

void *sum_array_r(void *initid_r)
{

	// Plugin thread function
	int thid;

	// Each thread has a subset of elements 
	unsigned long *th_array_dim = malloc(sizeof(unsigned long));

	for (thid = 0; thid < NTHREAD; thid++) {
		if (pthread_equal(((th_data *) (((UDF_INIT *) (initid_r))->extension))->thread[thid], pthread_self()))
			break;
	}
	for (;;) {
		barrier_wait(&(((th_data *) (((UDF_INIT *) (initid_r))->extension))->barr_start));
		if (((th_data *) (((UDF_INIT *) (initid_r))->extension))->exit_flag == 1)
			break;

		oph_string measurea;
		oph_string measureb;

		int res = 0;

		if (((UDF_ARGS *) ((th_data *) (((UDF_INIT *) (initid_r))->extension))->curr_args)->arg_count < 3) {
			core_set_type(&(measurea), NULL, 0);
			core_set_type(&(measureb), NULL, 0);
		} else if (((UDF_ARGS *) ((th_data *) (((UDF_INIT *) (initid_r))->extension))->curr_args)->arg_count == 3) {
			core_set_type(&(measurea), ((UDF_ARGS *) ((th_data *) (((UDF_INIT *) (initid_r))->extension))->curr_args)->args[2],
				      &(((UDF_ARGS *) ((th_data *) (((UDF_INIT *) (initid_r))->extension))->curr_args)->lengths[2]));
			core_set_type(&(measureb), NULL, 0);
		} else {
			core_set_type(&(measurea), ((UDF_ARGS *) ((th_data *) (((UDF_INIT *) (initid_r))->extension))->curr_args)->args[2],
				      &(((UDF_ARGS *) ((th_data *) (((UDF_INIT *) (initid_r))->extension))->curr_args)->lengths[2]));
			core_set_type(&(measureb), ((UDF_ARGS *) ((th_data *) (((UDF_INIT *) (initid_r))->extension))->curr_args)->args[3],
				      &(((UDF_ARGS *) ((th_data *) (((UDF_INIT *) (initid_r))->extension))->curr_args)->lengths[3]));
		}



		if (measurea.type != measureb.type) {
			pmesg(1, __FILE__, __LINE__, "Type are different; unable to sum values\n");
		}

		*th_array_dim = (((UDF_ARGS *) ((th_data *) (((UDF_INIT *) (initid_r))->extension))->curr_args)->lengths[0]) / NTHREAD;

		measurea.content = ((UDF_ARGS *) ((th_data *) (((UDF_INIT *) (initid_r))->extension))->curr_args)->args[0] + ((*th_array_dim) * thid);
		measurea.length = th_array_dim;
		measureb.content = ((UDF_ARGS *) ((th_data *) (((UDF_INIT *) (initid_r))->extension))->curr_args)->args[1] + ((*th_array_dim) * thid);
		measureb.length = th_array_dim;

		core_set_elemsize(&(measurea));

		if (core_set_numelem(&(measurea))) {
			pmesg(1, __FILE__, __LINE__, "Error on counting elements\n");
		}

		core_set_elemsize(&(measureb));

		if (core_set_numelem(&(measureb))) {
			pmesg(1, __FILE__, __LINE__, "Error on counting elements\n");
		}
		//Exit if array series have different number of elements
		if (measurea.numelem != measureb.numelem) {
			pmesg(1, __FILE__, __LINE__, "Number of array elements are different; unable to sum values\n");
		}


		res = core_oph_sum_array(&measurea, &measureb, (((UDF_INIT *) (initid_r))->ptr) + ((*th_array_dim) * thid));
		if (res) {
			pmesg(1, __FILE__, __LINE__, "Unable to compute result\n");
		}
		barrier_wait(&(((th_data *) (((UDF_INIT *) (initid_r))->extension))->barr_end));
	}			// end for
	free(th_array_dim);
	th_array_dim = NULL;
}

char *oph_sum_array_r(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{

	// Allocate space for the result_array; result_array[i] = measurea[i] + measureb[i]
	//initid->ptr=(char *)malloc(*(measure.length) * (core_sizeof(measure.type))); 
	if (!initid->ptr) {
		initid->ptr = (char *) malloc(args->lengths[0]);
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating result string\n");
			*length = 0;
			*is_null = 1;
			*error = 1;
			return NULL;
		}
	}

	barrier_wait(&(((th_data *) (initid->extension))->barr_start));
	barrier_wait(&(((th_data *) (initid->extension))->barr_end));

	*length = args->lengths[0];
	*error = 0;
	*is_null = 0;
	return initid->ptr;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
