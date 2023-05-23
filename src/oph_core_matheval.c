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

#include "oph_core_matheval.h"

int core_set_comp(oph_comp *op, char *type, unsigned long *len)
{
	if (!type) {
		*op = DEFAULT_COMP;
		return 0;
	}
	*op = core_get_comp(type, len);
	if (*op == INVALID_COMP) {
		pmesg(1, __FILE__, __LINE__, "Comparison operator not recognized\n");
		return -1;
	}
	return 0;
}

oph_comp core_get_comp(char *type, unsigned long *len)
{
	char type_buff[BUFF_LEN];
	if (core_strncpy(type_buff, type, len)) {
		pmesg(1, __FILE__, __LINE__, "Invalid operator\n");
		return INVALID_COMP;
	}
	//Case insensitive comparison
	if (!strcasecmp(type_buff, "OPH_GREATER_THAN_ZERO") || !strcasecmp(type_buff, "OPH_MORE_THAN_ZERO") || !strcasecmp(type_buff, "OPH_POSITIVE") || !strcasecmp(type_buff, ">0")
	    || !strcasecmp(type_buff, ">"))
		return OPH_GREATER_THAN_ZERO;
	if (!strcasecmp(type_buff, "OPH_LESS_THAN_ZERO") || !strcasecmp(type_buff, "OPH_NEGATIVE") || !strcasecmp(type_buff, "<0") || !strcasecmp(type_buff, "<"))
		return OPH_LESS_THAN_ZERO;
	if (!strcasecmp(type_buff, "OPH_EQUAL_TO_ZERO") || !strcasecmp(type_buff, "OPH_ZERO") || !strcasecmp(type_buff, "==0") || !strcasecmp(type_buff, "=0") || !strcasecmp(type_buff, "0"))
		return OPH_EQUAL_TO_ZERO;
	if (!strcasecmp(type_buff, "OPH_GREATER_OR_EQUAL_TO_ZERO") || !strcasecmp(type_buff, "OPH_MORE_OR_EQUAL_TO_ZERO") || !strcasecmp(type_buff, "OPH_NOT_NEGATIVE") || !strcasecmp(type_buff, ">=0")
	    || !strcasecmp(type_buff, ">="))
		return OPH_GREATER_OR_EQUAL_TO_ZERO;
	if (!strcasecmp(type_buff, "OPH_LESS_OR_EQUAL_TO_ZERO") || !strcasecmp(type_buff, "OPH_NOT_POSITIVE") || !strcasecmp(type_buff, "<=0") || !strcasecmp(type_buff, "<="))
		return OPH_LESS_OR_EQUAL_TO_ZERO;
	if (!strcasecmp(type_buff, "OPH_NOT_EQUAL_TO_ZERO") || !strcasecmp(type_buff, "OPH_NOT_ZERO") || !strcasecmp(type_buff, "!=0") || !strcasecmp(type_buff, "!=") || !strcasecmp(type_buff, "<>0")
	    || !strcasecmp(type_buff, "<>"))
		return OPH_NOT_EQUAL_TO_ZERO;
	if (!strcasecmp(type_buff, "OPH_NULL") || !strcasecmp(type_buff, "NULL") || !strcasecmp(type_buff, "NAN") || !strcasecmp(type_buff, "MISSED") || !strcasecmp(type_buff, "MISS")
	    || !strcasecmp(type_buff, "ISNULL") || !strcasecmp(type_buff, "ISNAN") || !strcasecmp(type_buff, "ISMISSED"))
		return OPH_NULL;
	pmesg(1, __FILE__, __LINE__, "Invalid operator '%s'\n", type_buff);
	return INVALID_COMP;
}
