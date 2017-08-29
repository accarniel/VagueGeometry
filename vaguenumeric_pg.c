/**********************************************************************
 *
 * VagueGeometry - Vague Spatial Objects for PostgreSQL
 * http://gbd.dc.ufscar.br/vaguegeometry/
 *
 * Copyright 2013-2016 Anderson Chaves Carniel <accarniel@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public Licence. See the COPYING file.
 *
 * Fully developed by Anderson Chaves Carniel
 *
 **********************************************************************/
 
 /**************************
 *
 * Handling of VagueNumeric data type
 ***************************/

#include "postgres.h"
#include "fmgr.h"
#include "lib/stringinfo.h"
#include "libpq/pqformat.h"
#include "libvgeom.h"

#include <math.h>

/* basic functions for postgres */
Datum VN_in(PG_FUNCTION_ARGS);
Datum VN_out(PG_FUNCTION_ARGS);
Datum VN_send(PG_FUNCTION_ARGS);
Datum VN_recv(PG_FUNCTION_ARGS);

/* operators */
Datum VN_maybe_op(PG_FUNCTION_ARGS);
Datum VN_equals_op(PG_FUNCTION_ARGS);
Datum VN_nequals_op(PG_FUNCTION_ARGS);

/*
* The format is: [0-99]*;[0-99]*
* where the first value is the minimum value and the second value is the maximum value
*/
PG_FUNCTION_INFO_V1(VN_in);
Datum VN_in(PG_FUNCTION_ARGS) {
	char *s = PG_GETARG_CSTRING(0);
	VAGUENUMERIC *result;
	char *s2;
	double min, max;

	while (*s != '\0' && isspace(*s))
		s++;

	if (*s == '\0')
		ereport(ERROR,
			(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
			 errmsg("unexpected empty value")));
	
	min = strtod(s, &s2);
	if (s == s2)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("expected numeric value for first number of type vague numeric: \"%s\"", PG_GETARG_CSTRING(0))));
	s = s2;

	while (*s != '\0' && isspace(*s))
		s++;

	if (*s == '\0')
		ereport(ERROR,
			(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
			 errmsg("unexpected empty value")));

	if(*s == ';')
		s++;
	else 
		ereport(ERROR,
			(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
			 errmsg("unexpected value - %s", s)));
	
	max = strtod(s, &s2);
	if (s == s2)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
				 errmsg("expected numeric value for first number of type vague numeric: \"%s\"", PG_GETARG_CSTRING(0))));
	s = s2;

	while (*s != '\0' && isspace(*s))
		s++;

	if (*s == '\0') {
		result = (VAGUENUMERIC*) palloc(sizeof(VAGUENUMERIC));
		result->max = max;
		result->min = min;
		PG_RETURN_POINTER(result);
	} else {
		ereport(ERROR,
			(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
			 errmsg("unexpected value - %s", s)));
	}
	
}

/*
* Format is MIN;MAX
*/
PG_FUNCTION_INFO_V1(VN_out);
Datum VN_out(PG_FUNCTION_ARGS) {
	VAGUENUMERIC *vn = (VAGUENUMERIC*) PG_GETARG_POINTER(0);
	char *result;

	result = (char*) palloc(60);

	snprintf(result, 60, "%f;%f", vn->min, vn->max);

	PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(VN_recv);
Datum VN_recv(PG_FUNCTION_ARGS) {
	StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
	VAGUENUMERIC *result;

	result = (VAGUENUMERIC*) palloc(sizeof(VAGUENUMERIC));
	
	result->min = pq_getmsgfloat8(buf);
	result->max = pq_getmsgfloat8(buf);
	
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VN_send);
Datum VN_send(PG_FUNCTION_ARGS) {
	VAGUENUMERIC *vn = (VAGUENUMERIC*) PG_GETARG_POINTER(0);
	StringInfoData buf;

	pq_begintypsend(&buf);
	pq_sendfloat8(&buf, vn->min);
	pq_sendfloat8(&buf, vn->max);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/* operators */
PG_FUNCTION_INFO_V1(VN_maybe_op);
Datum VN_maybe_op(PG_FUNCTION_ARGS) {
	VAGUENUMERIC *vn = (VAGUENUMERIC*) PG_GETARG_POINTER(0);
	double num = PG_GETARG_FLOAT8(1);

	PG_RETURN_BOOL(FP_LTEQ(num, vn->max) && FP_GTEQ(num, vn->min));
}

PG_FUNCTION_INFO_V1(VN_equals_op);
Datum VN_equals_op(PG_FUNCTION_ARGS) {
	VAGUENUMERIC *vn = (VAGUENUMERIC*) PG_GETARG_POINTER(0);
	double num = PG_GETARG_FLOAT8(1);

	PG_RETURN_BOOL(FP_EQUALS(num, vn->min));
}

PG_FUNCTION_INFO_V1(VN_nequals_op);
Datum VN_nequals_op(PG_FUNCTION_ARGS) {
	VAGUENUMERIC *vn = (VAGUENUMERIC*) PG_GETARG_POINTER(0);
	double num = PG_GETARG_FLOAT8(1);

	PG_RETURN_BOOL(FP_NEQUALS(num, vn->min));
}
