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

#include "oph_debug.h"
#include <stdio.h>

extern int msglevel; /* the higher, the more messages... */

#if defined(NDEBUG) && defined(__GNUC__)
/* Nothing. pmesg has been "defined away" in debug.h already. */
#else
void pmesg(int level, char* source, long int line_number, char* format, ...) {
#ifdef NDEBUG
	/* Empty body, so a good compiler will optimise calls
	   to pmesg away */
#else
        va_list args;
	char log_type[10];

        if (level>msglevel)
                return;
		
	switch ( level ) {
		case 1:
		  sprintf(log_type,"ERROR");
		  break;
		case 2:
		  sprintf(log_type,"WARNING");
		  break;
		case 3:
		  sprintf(log_type,"DEBUG");
		  break;
		default:
		  sprintf(log_type,"UNKNOWN");
		  break;
		}
	
	fprintf(stderr,"[%s][%s][%ld] ",log_type, source,line_number);

        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
#endif /* NDEBUG */
}
#endif /* NDEBUG && __GNUC__ */        
