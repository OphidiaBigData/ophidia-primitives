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

#include "oph_debug.h"

#include <string.h>
#include <time.h>
#include <stdio.h>

#define CTIME_BUF 32

extern int msglevel;		/* the higher, the more messages... */

#if defined(NDEBUG) && defined(__GNUC__)
/* Nothing. pmesg has been "defined away" in debug.h already. */
#else
void pmesg(int level, char *source, long int line_number, char *format, ...)
{
#ifdef NDEBUG
	/* Empty body, so a good compiler will optimise calls
	   to pmesg away */
#else
	va_list args;
	char log_type[10];

	int new_msglevel = msglevel % 10;
	if (level > new_msglevel)
		return;

	switch (level) {
		case LOG_ERROR:
			sprintf(log_type, LOG_ERROR_MESSAGE);
			break;
		case LOG_INFO:
			sprintf(log_type, LOG_INFO_MESSAGE);
			break;
		case LOG_WARNING:
			sprintf(log_type, LOG_WARNING_MESSAGE);
			break;
		case LOG_DEBUG:
			sprintf(log_type, LOG_DEBUG_MESSAGE);
			break;
		default:
			sprintf(log_type, LOG_UNKNOWN_MESSAGE);
			break;
	}

	if (msglevel > 10) {
		time_t t1 = time(NULL);
		char s[CTIME_BUF];
		ctime_r(&t1, s);
		s[strlen(s) - 1] = 0;	// remove \n
		fprintf(stderr, "[%s][%s][%s][%ld]\t", s, log_type, source, line_number);
	} else {
		fprintf(stderr, "[%s][%s][%ld]\t", log_type, source, line_number);
	}

	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
#endif				/* NDEBUG */
}
#endif				/* NDEBUG && __GNUC__ */
