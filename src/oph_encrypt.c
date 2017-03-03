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

#include "oph_encrypt.h"

#include <openssl/md5.h>
#include <openssl/sha.h>

#define OPH_ENCRYPT_MD5 "md5"
#define OPH_ENCRYPT_SHA256 "sha256"
#define OPH_ENCRYPT_SHA512 "sha512"

#define OPH_ENCRYPT_MD5_SIZE 3
#define OPH_ENCRYPT_SHA256_SIZE 6
#define OPH_ENCRYPT_SHA512_SIZE 6

int msglevel = 1;

unsigned long core_encrypted_size(const int encryption_algorithm)
{
	switch (encryption_algorithm) {
		case 1:
			return MD5_DIGEST_LENGTH;
		case 5:
			return SHA256_DIGEST_LENGTH;
		case 6:
			return SHA512_DIGEST_LENGTH;
		default:
			pmesg(1, __FILE__, __LINE__, "Unknown encryption algorithm\n");
	}
	return 0;
}

int core_oph_encrypt(const char *src, const unsigned long numelem, const size_t blocksize, const int encryption_algorithm, char *dest)
{
	if (!src) {
		pmesg(1, __FILE__, __LINE__, "Input buffer is empty\n");
		return -1;
	}
	if (!dest) {
		pmesg(1, __FILE__, __LINE__, "Ouput buffer is not given\n");
		return -2;
	}
	unsigned long i;
	switch (encryption_algorithm) {
		case 1:
			for (i = 0; i < numelem; ++i, src += blocksize) {
				MD5(src, blocksize, dest);
				dest += MD5_DIGEST_LENGTH;
			}
			break;
		case 5:
			for (i = 0; i < numelem; ++i, src += blocksize) {
				SHA256(src, blocksize, dest);
				dest += SHA256_DIGEST_LENGTH;
			}
			break;
		case 6:
			for (i = 0; i < numelem; ++i, src += blocksize) {
				SHA512(src, blocksize, dest);
				dest += SHA512_DIGEST_LENGTH;
			}
			break;
		default:
			pmesg(1, __FILE__, __LINE__, "Unknown encryption algorithm\n");
			return -3;
	}
	return 0;
}

/*------------------------------------------------------------------|
|               Functions' implementation (BEGIN)                   |
|------------------------------------------------------------------*/
my_bool oph_encrypt_init(UDF_INIT * initid, UDF_ARGS * args, char *message)
{
	if ((args->arg_count < 3) || (args->arg_count > 4)) {
		strcpy(message, "ERROR: Wrong arguments! oph_encrypt(input_OPH_TYPE, output_OPH_TYPE, measure, [algorithm])");
		return 1;
	}

	if (args->arg_type[2] != STRING_RESULT) {
		strcpy(message, "ERROR: Wrong arguments to oph_encrypt function");
		return 1;
	}
	if ((args->arg_count > 3) && (args->arg_type[3] != STRING_RESULT)) {
		strcpy(message, "ERROR: Wrong arguments to oph_encrypt function");
		return 1;
	}

	initid->ptr = NULL;
	initid->extension = NULL;

	return 0;
}

void oph_encrypt_deinit(UDF_INIT * initid)
{
	if (initid->ptr) {
		free(initid->ptr);
		initid->ptr = NULL;
	}
	if (initid->extension) {
		free_oph_multistring((oph_multistring *) (initid->extension));
		initid->extension = NULL;
	}
}

char *oph_encrypt(UDF_INIT * initid, UDF_ARGS * args, char *result, unsigned long *length, char *is_null, char *error)
{
	if (*error) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	if (*is_null || !args->args[2] || !args->lengths[2]) {
		*length = 0;
		*is_null = 1;
		*error = 0;
		return NULL;
	}

	if (!initid->extension) {
		if (core_set_oph_multistring((oph_multistring **) (&(initid->extension)), args->args[0], &(args->lengths[0]))) {
			pmesg(1, __FILE__, __LINE__, "Error setting measure structure\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}
	oph_multistring *multim = (oph_multistring *) (initid->extension);
	multim->length = args->lengths[2];

	//Check
	if (multim->length % multim->blocksize) {
		pmesg(1, __FILE__, __LINE__, "Wrong input type or data corrupted\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}
	multim->numelem = multim->length / multim->blocksize;

	int encryption_algorithm = 1;
	if (args->arg_count > 3) {
		if (!strncasecmp(args->args[3], OPH_ENCRYPT_SHA256, OPH_ENCRYPT_SHA256_SIZE))
			encryption_algorithm = 5;
		else if (!strncasecmp(args->args[3], OPH_ENCRYPT_SHA512, OPH_ENCRYPT_SHA512_SIZE))
			encryption_algorithm = 6;
		else if (strncasecmp(args->args[3], OPH_ENCRYPT_MD5, OPH_ENCRYPT_MD5_SIZE)) {
			pmesg(1, __FILE__, __LINE__, "Unable to read hash algorithm\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	unsigned long buffer_length = multim->numelem * core_encrypted_size(encryption_algorithm);
	if (!buffer_length) {
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	if (!initid->ptr) {
		initid->ptr = (char *) calloc(buffer_length, sizeof(char));
		if (!initid->ptr) {
			pmesg(1, __FILE__, __LINE__, "Error allocating encrypted string\n");
			*length = 0;
			*is_null = 0;
			*error = 1;
			return NULL;
		}
	}

	if (core_oph_encrypt(args->args[2], multim->numelem, multim->blocksize, encryption_algorithm, initid->ptr)) {
		pmesg(1, __FILE__, __LINE__, "Encryption error\n");
		*length = 0;
		*is_null = 0;
		*error = 1;
		return NULL;
	}

	*length = buffer_length;
	return initid->ptr;
}

/*------------------------------------------------------------------|
|               Functions' implementation (END)                     |
|------------------------------------------------------------------*/
