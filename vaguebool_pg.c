/**********************************************************************
 *
 * VagueGeometry - Vague Spatial Objects for PostgreSQL
 * http://gbd.dc.ufscar.br/vaguegeometry/
 *
 * Copyright 2013-2015 Anderson Chaves Carniel <accarniel@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU General Public Licence. See the COPYING file.
 *
 * Fully developed by Anderson Chaves Carniel
 *
 **********************************************************************/
 
/*************************************************
* Handling the VagueBool objects.
*
**************************************************/

#include "postgres.h"
#include "fmgr.h"
#include "lib/stringinfo.h"
#include "libpq/pqformat.h"
#include "libvgeom.h"

/* basic functions for postgres */
Datum VB_in(PG_FUNCTION_ARGS);
Datum VB_out(PG_FUNCTION_ARGS);
Datum VB_send(PG_FUNCTION_ARGS);
Datum VB_recv(PG_FUNCTION_ARGS);

/* cast required */
Datum VB_enforce_cast(PG_FUNCTION_ARGS);

/*
* OPERATORS FOR VAGUEBOOL
*/

/* left operators */
Datum VB_not_op(PG_FUNCTION_ARGS);
Datum VB_and_VB(PG_FUNCTION_ARGS);
Datum VB_or_VB(PG_FUNCTION_ARGS);
Datum VB_maybe_op(PG_FUNCTION_ARGS);
Datum VB_true_op(PG_FUNCTION_ARGS);
Datum VB_false_op(PG_FUNCTION_ARGS);
Datum VB_maybe_maybe_op(PG_FUNCTION_ARGS);

/*
* TRUE is 1
* FALSE is 0
* MAYBE is 2
*/
PG_FUNCTION_INFO_V1(VB_in);
Datum VB_in(PG_FUNCTION_ARGS) {
	char *input = PG_GETARG_CSTRING(0);
	VAGUEBOOL *result;

	if(strcasecmp(input, "TRUE")==0) {
		result = (VAGUEBOOL*) palloc(sizeof(VAGUEBOOL));
		result->vbool = VG_TRUE;
		PG_RETURN_POINTER(result);
	} else if(strcasecmp(input, "FALSE")==0) {
		result = (VAGUEBOOL*) palloc(sizeof(VAGUEBOOL));
		result->vbool = VG_FALSE;
		PG_RETURN_POINTER(result);
	} else if(strcasecmp(input, "MAYBE") ==0 ) {
		result = (VAGUEBOOL*) palloc(sizeof(VAGUEBOOL));
		result->vbool = VG_MAYBE;
		PG_RETURN_POINTER(result);
	} else { //error..
		ereport(ERROR,
			(errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
			 errmsg("unexpected value for vague bool - \"%s\"", input)));
	}
}

/*
* 1 is 'TRUE'
* 0 is 'FALSE'
* 2 is 'MAYBE'
*/
PG_FUNCTION_INFO_V1(VB_out);
Datum VB_out(PG_FUNCTION_ARGS) {
	VAGUEBOOL *vb = (VAGUEBOOL*) PG_GETARG_POINTER(0);
	char *result;

	result = (char*) palloc(5);

	if(vb->vbool==VG_TRUE) {
		snprintf(result, 5, "TRUE");
	} else if(vb->vbool==VG_FALSE) {
		snprintf(result, 6, "FALSE");
	} else if(vb->vbool==VG_MAYBE) {
		snprintf(result, 6, "MAYBE");
	}

	PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(VB_recv);
Datum VB_recv(PG_FUNCTION_ARGS) {
	StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
	VAGUEBOOL *result;

	result = (VAGUEBOOL*) palloc(sizeof(VAGUEBOOL));
	result->vbool = pq_getmsgbyte(buf);

	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VB_send);
Datum VB_send(PG_FUNCTION_ARGS) {
	VAGUEBOOL *vb = (VAGUEBOOL*) PG_GETARG_POINTER(0);
	StringInfoData buf;

	pq_begintypsend(&buf);
	pq_sendbyte(&buf, vb->vbool);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/* required casting */
PG_FUNCTION_INFO_V1(VB_enforce_cast);
Datum VB_enforce_cast(PG_FUNCTION_ARGS) {
	VAGUEBOOL *vb;
		
	vb = (VAGUEBOOL*) PG_GETARG_POINTER(0);
	
	if(vb->vbool==VG_TRUE)
		PG_RETURN_BOOL(TRUE);
	else if(vb->vbool==VG_FALSE)
		PG_RETURN_BOOL(FALSE);
	else 
		PG_RETURN_NULL();
}

/* LEFT OPERATORS */

//like the truth table
PG_FUNCTION_INFO_V1(VB_not_op);
Datum VB_not_op(PG_FUNCTION_ARGS) {
	VAGUEBOOL *vb;
	VAGUEBOOL *result;
	vb = (VAGUEBOOL*) PG_GETARG_POINTER(0);
	result = (VAGUEBOOL*) palloc(sizeof(VAGUEBOOL));
	if(vb->vbool==VG_TRUE)
		result->vbool = VG_FALSE;
	else if(vb->vbool == VG_FALSE)
		result->vbool = VG_TRUE;
	else
		result->vbool = VG_MAYBE;
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VB_and_VB);
Datum VB_and_VB(PG_FUNCTION_ARGS) {
	VAGUEBOOL *vb1 = (VAGUEBOOL*) PG_GETARG_POINTER(0);
	VAGUEBOOL *vb2 = (VAGUEBOOL*) PG_GETARG_POINTER(1);
	VAGUEBOOL *result;
	
	result = (VAGUEBOOL*) palloc(sizeof(VAGUEBOOL));
	if(vb1->vbool==VG_TRUE && vb2->vbool==VG_TRUE)
		result->vbool = VG_TRUE;
	else if(vb1->vbool==VG_TRUE && vb2->vbool==VG_MAYBE)
		result->vbool = VG_MAYBE;
	else if(vb1->vbool==VG_TRUE && vb2->vbool==VG_FALSE)
		result->vbool = VG_FALSE;
	else if((vb1->vbool==VG_MAYBE) && (vb2->vbool == VG_MAYBE || vb2->vbool==VG_TRUE))
		result->vbool = VG_MAYBE;
	else if(vb1->vbool == VG_MAYBE && vb2->vbool == VG_FALSE)
		result->vbool = VG_FALSE;
	else
		result->vbool = VG_FALSE;
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VB_or_VB);
Datum VB_or_VB(PG_FUNCTION_ARGS) {
	VAGUEBOOL *vb1 = (VAGUEBOOL*) PG_GETARG_POINTER(0);
	VAGUEBOOL *vb2 = (VAGUEBOOL*) PG_GETARG_POINTER(1);
	VAGUEBOOL *result;
	
	result = (VAGUEBOOL*) palloc(sizeof(VAGUEBOOL));
	if(vb1->vbool==VG_TRUE || vb2->vbool == VG_TRUE)
		result->vbool = VG_TRUE;
	else if(vb1->vbool==VG_MAYBE || vb2->vbool == VG_MAYBE)
		result->vbool = VG_MAYBE;
	else if(vb1->vbool == VG_FALSE && vb2->vbool == VG_FALSE)
		result->vbool = VG_FALSE;
	PG_RETURN_POINTER(result);
}

//only true if, only if vague bool is really true
PG_FUNCTION_INFO_V1(VB_true_op);
Datum VB_true_op(PG_FUNCTION_ARGS) {
	VAGUEBOOL *vb;
		
	vb = (VAGUEBOOL*) PG_GETARG_POINTER(0);
	
	if(vb->vbool==VG_TRUE)
		PG_RETURN_BOOL(TRUE);
	else
		PG_RETURN_BOOL(FALSE);
}

//only true if, only if vague bool is really false
PG_FUNCTION_INFO_V1(VB_false_op);
Datum VB_false_op(PG_FUNCTION_ARGS) {
	VAGUEBOOL *vb;
		
	vb = (VAGUEBOOL*) PG_GETARG_POINTER(0);
	
	if(vb->vbool==VG_FALSE)
		PG_RETURN_BOOL(TRUE);
	else
		PG_RETURN_BOOL(FALSE);
}

//only true if, only if vague bool is maybe or true
PG_FUNCTION_INFO_V1(VB_maybe_op);
Datum VB_maybe_op(PG_FUNCTION_ARGS) {
	VAGUEBOOL *vb;
		
	vb = (VAGUEBOOL*) PG_GETARG_POINTER(0);
	
	if(vb->vbool==VG_TRUE || vb->vbool==VG_MAYBE)
		PG_RETURN_BOOL(TRUE);
	else
		PG_RETURN_BOOL(FALSE);
}

//only true if, only if vague bool is maybe
PG_FUNCTION_INFO_V1(VB_maybe_maybe_op);
Datum VB_maybe_maybe_op(PG_FUNCTION_ARGS) {
	VAGUEBOOL *vb;
		
	vb = (VAGUEBOOL*) PG_GETARG_POINTER(0);
	
	if(vb->vbool==VG_MAYBE)
		PG_RETURN_BOOL(TRUE);
	else
		PG_RETURN_BOOL(FALSE);
}
