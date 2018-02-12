/*
    Ophidia Primitives
    Copyright (C) 2012-2018 CMCC Foundation

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

#include "oph_uncompress.h"
#include <stdint.h>

int msglevel = 1;

int core_oph_uncompress(const unsigned char *source, unsigned long source_length, unsigned long uncompressed_length, unsigned char **dest)
{
	if (uncompress((Bytef *) (*dest), (uLongf *) & uncompressed_length, (const Bytef *) source, (uLong) source_length) != Z_OK) {
		pmesg(1, __FILE__, __LINE__, "Error during uncompression phase\n");
		return -1;
	}
	return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_uncompress_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	if (args->arg_count != 3) {
		strcpy(message, "ERROR: Wrong arguments! oph_uncompress(input_OPH_TYPE, output_OPH_TYPE, measure)");
		return 1;
	}

	if (args->arg_type[2] != STRING_RESULT) {
		strcpy(message, "ERROR: Wrong arguments to oph_uncompress function");
		return 1;
	}

	initid->ptr = NULL;

	return 0;
}

void oph_uncompress_deinit(UDF_INIT * initid)
{
	//Free allocated space
	if (initid->ptr) {
		free(initid->ptr);
		initid->ptr = NULL;
	}

}

char *oph_uncompress(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	char *res = args->args[2];
	int rc;
	unsigned long new_size = 0;

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

	if (args->lengths[2] <= COMPRESS_OFFSET) {
		pmesg(1, __FILE__, __LINE__, "Data is corrupt\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	new_size = *((uint32_t *) res) & 0x3FFFFFFF;

	if (!initid->ptr) {
		initid->ptr = (char *) calloc(new_size, sizeof(char));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating uncompressed string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}
	rc = uncompress((Byte *) initid->ptr, (uLongf *) & new_size, ((const Bytef *) res) + COMPRESS_OFFSET, (uLong) args->lengths[2] - COMPRESS_OFFSET);
	if (rc != Z_OK) {
		pmesg(1, __FILE__, __LINE__, "Error on compressing string\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	*length = new_size;
	*is_null = 0;
	*error = 0;

	return initid->ptr;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
