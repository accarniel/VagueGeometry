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
 
/************************
*
* Basic functions executed by the PostgreSQL internal library to handle VagueGeometry objects.
*
*************************/

#include "util_pg.h"
#include "fmgr.h"

#include "lwgeom_pg.h" //because of convertions

/* basic functions for VAGUEGEOM */
Datum VG_setsrid(PG_FUNCTION_ARGS);
Datum VG_getsrid(PG_FUNCTION_ARGS);
Datum VG_gettype(PG_FUNCTION_ARGS);
Datum VG_pcunion(PG_FUNCTION_ARGS);
Datum VG_precompute(PG_FUNCTION_ARGS);

/* TO-DO define functions to capture the MBR of each element, the allextension inclusive */

/* set srid for kernel and conjecture */
PG_FUNCTION_INFO_V1(VG_setsrid);
Datum VG_setsrid(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vgeom_serialized = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	int32_t srid = PG_GETARG_INT32(1);
	vgserialized_set_srid(vgeom_serialized, srid);
	PG_FREE_IF_COPY(vgeom_serialized, 0);
	PG_RETURN_POINTER(vgeom_serialized);
}

/* get srid */
PG_FUNCTION_INFO_V1(VG_getsrid);
Datum VG_getsrid(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vgeom_serialized = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	int32_t srid = vgserialized_get_srid(vgeom_serialized);
	PG_FREE_IF_COPY(vgeom_serialized, 0);
	PG_RETURN_INT32(srid);
}

PG_FUNCTION_INFO_V1(VG_gettype);
Datum VG_gettype(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1;
	char *type;
	text *result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
		
	type = vaguegeom_gettype_i(vgserialized_get_type(vg1));
	
	/* Write to text and free */
	result = cstring2text(type);

	/* Return the text */
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_TEXT_P(result);
}

PG_FUNCTION_INFO_V1(VG_pcunion);
Datum VG_pcunion(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	if(HAS_PUNION(vg1->flags)) {
		PG_FREE_IF_COPY(vg1, 0);
		PG_RETURN_BOOL(TRUE);
	} else {
		PG_FREE_IF_COPY(vg1, 0);
		PG_RETURN_BOOL(FALSE);
	}
}

PG_FUNCTION_INFO_V1(VG_precompute);
Datum VG_precompute(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vgeom_serialized = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	
	/* TO-DO create specific functions in vaguegeom_serialize to do it */
	
	if(PG_GETARG_BOOL(1) && !HAS_PUNION(vgeom_serialized->flags)) {
		VAGUEGEOMSERIALIZED *result;
		VAGUEGEOM *vgeom;
		
		vgeom = serialization_to_vaguegeom(vgeom_serialized);
		valid_convertion_vaguegeom(vgeom);
		
		FLAGS_SET_PUNION(vgeom->flags, 1);	
		vgeom->allextension = lwgeom_union(vgeom->kernel, vgeom->conjecture);
		
		result = vgeometry_serialize(vgeom);
		vaguegeom_free(vgeom);
		PG_FREE_IF_COPY(vgeom_serialized, 0);
		PG_RETURN_POINTER(result);
	} else if(!PG_GETARG_BOOL(1) && HAS_PUNION(vgeom_serialized->flags)) {
		VAGUEGEOMSERIALIZED *result;
		VAGUEGEOM *vgeom;
		
		vgeom = serialization_to_vaguegeom(vgeom_serialized);
		valid_convertion_vaguegeom(vgeom);
		
		FLAGS_SET_PUNION(vgeom->flags, 0);	
		vgeom->allextension = NULL;
		
		result = vgeometry_serialize(vgeom);
		vaguegeom_free(vgeom);
		PG_FREE_IF_COPY(vgeom_serialized, 0);
		PG_RETURN_POINTER(result);
	} else {
		PG_FREE_IF_COPY(vgeom_serialized, 0);
		PG_RETURN_POINTER(vgeom_serialized);
	}
}
