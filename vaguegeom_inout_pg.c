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
* Input and Output functions for VagueGeometry objects. It reuses some functions of the lwgeom_export.h of the PostGIS
*
*************************/

#include "util_pg.h"
#include "fmgr.h"
#include "utils/array.h"
#include "catalog/pg_type.h"
#include "utils/builtins.h"
#include "lib/stringinfo.h"

#include "libvgeom.h"

#include "lwgeom_pg.h"
#include "lwgeom_export.h"

#include <float.h>

/* basic functions for postgres */
Datum VG_in(PG_FUNCTION_ARGS);
Datum VG_out(PG_FUNCTION_ARGS);
Datum VG_send(PG_FUNCTION_ARGS);
Datum VG_recv(PG_FUNCTION_ARGS);

/* variable type */
Datum VG_typmod_in(PG_FUNCTION_ARGS);
Datum VG_typmod_out(PG_FUNCTION_ARGS);
Datum VG_enforce_typmod(PG_FUNCTION_ARGS);

/* basic functions for vague geometry (input and output) */
/* output */
Datum VG_astext(PG_FUNCTION_ARGS); //to vague wkt
Datum VG_asvgeojson(PG_FUNCTION_ARGS); //to vague geojson
Datum VG_asvgml(PG_FUNCTION_ARGS); //to vague gml
Datum VG_asVKML(PG_FUNCTION_ARGS); //to vague kml
Datum VG_asvwkb(PG_FUNCTION_ARGS); //to bytea

Datum VG_asevwkt(PG_FUNCTION_ARGS); //to e-vwkt
Datum VG_asevwkb(PG_FUNCTION_ARGS); //to e-wkb

/* input */
Datum VG_vgeomfromtext(PG_FUNCTION_ARGS); //from wkt of 2 postgis's object
Datum VG_vgeomfromvgeojson(PG_FUNCTION_ARGS); //from vague geojson
Datum VG_vgeomfromvgml(PG_FUNCTION_ARGS); //from vague gml
Datum VG_vgeomfromvkml(PG_FUNCTION_ARGS); //from vague kml
Datum VG_vgeomfromvwkb(PG_FUNCTION_ARGS); //from binary FORM
Datum VG_vgeomfromvwkt(PG_FUNCTION_ARGS); //from the vague wkt form (the output form of VG_astext)

Datum VG_vgeomfromevwkt(PG_FUNCTION_ARGS); //from e-vwkt
Datum VG_vgeomfromevwkb(PG_FUNCTION_ARGS); //from e-vwkb

/* receive two objects from postgis and make */
Datum VG_make(PG_FUNCTION_ARGS);
Datum VG_enforcemake(PG_FUNCTION_ARGS);

/*
auxiliary function 
*/
static void valid_typmod(const VAGUEGEOMSERIALIZED *vg, int32 typmod);
void valid_typmod(const VAGUEGEOMSERIALIZED *vg, int32 typmod) {
	int32 geom_srid = vgserialized_get_srid(vg);
	int32 geom_type = vgserialized_get_type(vg);
	int32 typmod_srid = TYPMOD_GET_SRID(typmod);
	int32 typmod_type = TYPMOD_GET_TYPE(typmod);
	
	/* No typmod (-1) => no preferences */
	if (typmod < 0) return;
	
	/* Typmod has a preference for SRID and geom has a non-default SRID? They had better match. */
	if ( typmod_srid > 0 && typmod_srid != geom_srid )	{
		ereport(ERROR, (
		            errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		            errmsg("Vague geometry SRID (%d) does not match column SRID (%d)", geom_srid, typmod_srid) ));
	}

	/* Typmod has a preference for geometry type. */
	if ( typmod_type > 0 && (typmod_type != geom_type))	{
		ereport(ERROR, (
		            errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		            errmsg("Vague geometry type (%s) does not match column type (%s)", vaguegeom_gettype_i(geom_type), vaguegeom_gettype_i(typmod_type)) ));
	}
}



/* basic functions to POSTGRES input and output and send and receive */
/* the format is VAGUEGEOMETRY(KERNEL in WKT form, CONJECTURE in WKT form)
or SRID=[numeric];VAGUEGEOMETRY(KERNEL; CONJECTURE)
accept in binary format represented hexadecimal too
*/
PG_FUNCTION_INFO_V1(VG_in);
Datum VG_in(PG_FUNCTION_ARGS) {
	char *input = PG_GETARG_CSTRING(0);
	int32 geom_typmod = -1;
	uint8_t punion=1; //pre computed, yes or not? the default is true.
	char *str = input;
	VAGUEGEOM *vg;
	int srid=0;
	VAGUEGEOMSERIALIZED *ret;
	
	if ( (PG_NARGS()>2) && (!PG_ARGISNULL(2)) ) {
		geom_typmod = PG_GETARG_INT32(2);
	}

	/* Empty string. */
	if ( str[0] == '\0' )
		ereport(ERROR,(errmsg("parse error - invalid vague geometry")));

	/* this is in the WKB form, that is, the binary form in hexadecimal representation? */
	if(str[0]=='0') {
		vg = vgeom_from_vhexwkb(str, LW_PARSER_CHECK_NONE, punion);
		if(vg==NULL)  {
			ereport(ERROR, (
		            errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		            errmsg("Convertion from hexadecimal to vague geometry failed") ));
		}
		/* If we picked up an SRID at the head of the WKB set it manually */
		if ( srid ) {
			lwgeom_set_srid(vg->kernel, srid);
			lwgeom_set_srid(vg->conjecture, srid);
		}
		/* Add a bbox if necessary */
		if ( lwgeom_needs_bbox(vg->kernel) ) lwgeom_add_bbox(vg->kernel);
		if ( lwgeom_needs_bbox(vg->conjecture) ) lwgeom_add_bbox(vg->conjecture);
	} else {
		vg = vgeom_from_vwkt(str, punion);
		if(vg==NULL)  {
			ereport(ERROR, (
		            errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		            errmsg("Convertion from vwkt to vague geometry failed") ));
		}
		/* Add a bbox if necessary */
		if ( lwgeom_needs_bbox(vg->kernel) ) lwgeom_add_bbox(vg->kernel);
		if ( lwgeom_needs_bbox(vg->conjecture) ) lwgeom_add_bbox(vg->conjecture);
	}

	if(vaguegeom_is_valid(vg) == LW_FALSE) {
		ereport(ERROR,(errmsg("parse error - invalid vague geometry - kernel and conjecture must be disjoint or meet")));
	}

	ret = vgeometry_serialize(vg);

	if ( geom_typmod >= 0 ) {
		valid_typmod(ret, geom_typmod);
		/* the typmod is consistent */
	}

	/* return the vague geometry serialized for postgres*/	
	PG_RETURN_POINTER(ret);
}

/*
* the function out return the binary format in hexadecimal
*/
PG_FUNCTION_INFO_V1(VG_out);
Datum VG_out(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg = (VAGUEGEOMSERIALIZED*)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	VAGUEGEOM *vgeom;
	char *result;
	size_t vhexwkb_size;

	vgeom = serialization_to_vaguegeom(vg);
	result = vaguegeom_to_hexvwkb(vgeom, WKB_EXTENDED, &vhexwkb_size);
	vaguegeom_free(vgeom);

	PG_RETURN_CSTRING(result);
}

/*
* the function out return the binary format in bytea form
*/ 
PG_FUNCTION_INFO_V1(VG_send);
Datum VG_send(PG_FUNCTION_ARGS) {
	/* call the VG_asvwkb that return a bytea */
	PG_RETURN_POINTER(
	  DatumGetPointer(
	    DirectFunctionCall1(VG_asvwkb, PG_GETARG_DATUM(0))
		));
}

/* convert from binary form to internal form */
PG_FUNCTION_INFO_V1(VG_recv);
Datum VG_recv(PG_FUNCTION_ARGS) {
	StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
	int32 geom_typmod = -1;
	uint8_t punion=1; //pre computed, yes or not? the default is true.
	VAGUEGEOMSERIALIZED *vgserialized;
	VAGUEGEOM *vgeom;

	if ( (PG_NARGS()>2) && (!PG_ARGISNULL(2)) ) {
		geom_typmod = PG_GETARG_INT32(2);
	}
	
	vgeom = vgeom_from_vwkb((uint8_t*)buf->data, buf->len, LW_PARSER_CHECK_ALL, punion);
	if(vaguegeom_is_valid(vgeom) == LW_FALSE) {
		ereport(ERROR,(errmsg("parse error - invalid vague geometry - kernel and conjecture must be disjoint or meet")));
	}
	/* Add a bbox if necessary */
	if ( lwgeom_needs_bbox(vgeom->kernel) ) lwgeom_add_bbox(vgeom->kernel);
	if ( lwgeom_needs_bbox(vgeom->conjecture) ) lwgeom_add_bbox(vgeom->conjecture);

	/* Set cursor to the end of buffer (so the backend is happy) */
	buf->cursor = buf->len;

	vgserialized = vgeometry_serialize(vgeom);

	vaguegeom_free(vgeom);
	if ( geom_typmod >= 0 ) {
		valid_typmod(vgserialized, geom_typmod);
		/* the typmod is consistent */
	}
		
	PG_RETURN_POINTER(vgserialized);
}

/* variable type */
PG_FUNCTION_INFO_V1(VG_typmod_in);
Datum VG_typmod_in(PG_FUNCTION_ARGS) {
	int32 typmod; //the return must be a int32
	ArrayType *input;
	Datum *element;
	int n, i;
	/*
	* The input will be (TYPE, SRID) like (VAGUEPOINT, 4326)
	* or just (TYPE) like (VAGUEPOINT)
	*/
	input = (ArrayType*) DatumGetPointer(PG_GETARG_DATUM(0));

	if (ARR_ELEMTYPE(input) != CSTRINGOID)
		ereport(ERROR,
		        (errcode(ERRCODE_ARRAY_ELEMENT_ERROR),
		         errmsg("typmod array must be type cstring[]")));

	if (ARR_NDIM(input) != 1)
		ereport(ERROR,
		        (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
		         errmsg("typmod array must be one-dimensional")));

	if (ARR_HASNULL(input))
		ereport(ERROR,
		        (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
		         errmsg("typmod array must not contain nulls")));

	deconstruct_array(input, CSTRINGOID, -2, false, 'c', &element, NULL, &n);

	TYPMOD_SET_SRID(typmod, SRID_UNKNOWN);

	for (i = 0; i < n; i++)	{
		if ( i == 0 ) {/* TYPE */
			char *s = DatumGetCString(element[i]);
			uint8_t type = 0;

			type = vaguegeom_gettype_char(s);
			if(type==-1) {
				ereport(ERROR,
				        (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				         errmsg("Invalid vague geometry type modifier: %s", s)));
			} else {
				TYPMOD_SET_TYPE(typmod, type);
			}
		}
		if ( i == 1 ) { /* SRID */
			int srid = pg_atoi(DatumGetCString(element[i]), sizeof(int32), '\0');
			if(srid <= 0)
				srid = SRID_UNKNOWN;
			if (srid > SRID_MAXIMUM) 
				srid = SRID_USER_MAXIMUM + 1 + ( srid % ( SRID_MAXIMUM - SRID_USER_MAXIMUM - 1 ) );
			
			TYPMOD_SET_SRID(typmod, srid);
		}
	}

	pfree(element);
	PG_RETURN_INT32(typmod);
}

PG_FUNCTION_INFO_V1(VG_typmod_out);
Datum VG_typmod_out(PG_FUNCTION_ARGS) {
	char *s = (char*)palloc(70);
	uint32 typmod = PG_GETARG_INT32(0);
	uint32 srid = TYPMOD_GET_SRID(typmod);
	uint32 type = TYPMOD_GET_TYPE(typmod);
	/* No SRID or type? Then no typmod at all. Return empty string. */
	if ( ! ( srid || type ) ) {
		*s = '\0';
		PG_RETURN_CSTRING(s);
	}

	/* Opening bracket. */
	s += sprintf(s, "(");

	/* Has type? */
	if ( type )
		s += sprintf(s, "%s", vaguegeom_gettype_i(type));
	else if ( (!type) &&  ( srid ) )
		s += sprintf(s, "VagueGeometry");
	
	/* Comma? and SRID? */
	if ( srid ) {
		s += sprintf(s, ",");
		s += sprintf(s, "%d", srid);
	}

	/* Closing bracket. */
	s += sprintf(s, ")");

	PG_RETURN_CSTRING(s);
}

PG_FUNCTION_INFO_V1(VG_enforce_typmod);
Datum VG_enforce_typmod(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg;
	int32 typmod;
	
	vg = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	typmod = PG_GETARG_INT32(1);

	valid_typmod(vg, typmod);
	PG_RETURN_POINTER(vg);
}

/* from BINARY (i.e. the bytea form) form to internal form*/
PG_FUNCTION_INFO_V1(VG_vgeomfromvwkb);
Datum VG_vgeomfromvwkb(PG_FUNCTION_ARGS) {
	bytea *bytea_wkb = (bytea*)PG_GETARG_BYTEA_P(0);
	int32 srid = 0;
	uint8_t punion = 1;
	VAGUEGEOMSERIALIZED *vgserialized;
	VAGUEGEOM *vgeom;
	uint8_t *vwkb = (uint8_t*)VARDATA(bytea_wkb);
	
	if (  ( PG_NARGS()>2) && ( ! PG_ARGISNULL(2) )) {
		if(PG_GETARG_BOOL(2))
			punion =1;
		else
			punion = 0;
	}

	vgeom = vgeom_from_vwkb(vwkb, VARSIZE(bytea_wkb)-VARHDRSZ, LW_PARSER_CHECK_ALL, punion);
	if(vgeom==NULL)  {
		ereport(ERROR, (
		            errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		            errmsg("Convertion from binary to vague geometry failed") ));
	}
	if (  ( PG_NARGS()>1) && ( ! PG_ARGISNULL(1) )) {
		srid = PG_GETARG_INT32(1);
		lwgeom_set_srid(vgeom->kernel, srid);
		lwgeom_set_srid(vgeom->conjecture, srid);
	}

	if ( lwgeom_needs_bbox(vgeom->kernel) )
		lwgeom_add_bbox(vgeom->kernel);
	if ( lwgeom_needs_bbox(vgeom->conjecture) )
		lwgeom_add_bbox(vgeom->conjecture);
	if(vaguegeom_is_valid(vgeom) == LW_FALSE) {
		ereport(ERROR,(errmsg("parse error - invalid vague geometry - kernel and conjecture must be disjoint or meet")));
	}
	vgserialized = vgeometry_serialize(vgeom);
	vaguegeom_free(vgeom);
	PG_FREE_IF_COPY(bytea_wkb, 0);
	PG_RETURN_POINTER(vgserialized);
}

/* from extended BINARY form to internal form */
PG_FUNCTION_INFO_V1(VG_vgeomfromevwkb);
Datum VG_vgeomfromevwkb(PG_FUNCTION_ARGS) {
	bytea *bytea_wkb = (bytea*)PG_GETARG_BYTEA_P(0);
	uint8_t punion = 1;
	VAGUEGEOMSERIALIZED *vgserialized;
	VAGUEGEOM *vgeom;
	uint8_t *vwkb = (uint8_t*)VARDATA(bytea_wkb);
	
	if (  ( PG_NARGS()>1) && ( ! PG_ARGISNULL(1) )) {
		if(PG_GETARG_BOOL(1))
			punion =1;
		else
			punion = 0;
	}

	vgeom = vgeom_from_vwkb(vwkb, VARSIZE(bytea_wkb)-VARHDRSZ, LW_PARSER_CHECK_ALL, punion);
	if(vgeom==NULL)  {
		ereport(ERROR, (
		            errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		            errmsg("Convertion from binary to vague geometry failed") ));
	}
	if ( lwgeom_needs_bbox(vgeom->kernel) )
		lwgeom_add_bbox(vgeom->kernel);
	if ( lwgeom_needs_bbox(vgeom->conjecture) )
		lwgeom_add_bbox(vgeom->conjecture);
	if(vaguegeom_is_valid(vgeom) == LW_FALSE) {
		ereport(ERROR,(errmsg("parse error - invalid vague geometry - kernel and conjecture must be disjoint or meet")));
	}
	vgserialized = vgeometry_serialize(vgeom);
	vaguegeom_free(vgeom);
	PG_FREE_IF_COPY(bytea_wkb, 0);
	PG_RETURN_POINTER(vgserialized);
}

/* from the vague wkt form, that is, for instance VAGUEPOINT(POINT(1 1); POINT(2 2)) */
PG_FUNCTION_INFO_V1(VG_vgeomfromvwkt);
Datum VG_vgeomfromvwkt(PG_FUNCTION_ARGS) {
	text *wkt_text = PG_GETARG_TEXT_P(0);
	int srid = PG_GETARG_INT32(1);
	char *vwkt;
	uint8_t punion =1;
	VAGUEGEOM *vgeom;
	VAGUEGEOMSERIALIZED *vg;

	/* read user-requested pre compute union */
	if ( (PG_NARGS()>2) && (!PG_ARGISNULL(2)) ) {
		if(PG_GETARG_BOOL(2))
			punion = 1;
		else
			punion = 0;
	}

	/* Unwrap the PgSQL text type into a cstring */
	vwkt = text2cstring(wkt_text); 
	vgeom = vgeom_from_vwkt(vwkt, punion);
	if(vgeom==NULL)  {
		ereport(ERROR, (
		        errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		        errmsg("Convertion from vwkt to vague geometry failed") ));
	}
	/* Add a bbox if necessary */
	if ( lwgeom_needs_bbox(vgeom->kernel) ) lwgeom_add_bbox(vgeom->kernel);
	if ( lwgeom_needs_bbox(vgeom->conjecture) ) lwgeom_add_bbox(vgeom->conjecture);

	lwgeom_set_srid(vgeom->kernel, srid);
	lwgeom_set_srid(vgeom->conjecture, srid);
	
	lwfree(vwkt);
	
	if(vaguegeom_is_valid(vgeom) == LW_FALSE) {
		ereport(ERROR,(errmsg("parse error - invalid vague geometry - kernel and conjecture must be disjoint or meet")));
	}

	vg = vgeometry_serialize(vgeom);
	
	/* return the vague geometry serialized for postgres*/	
	PG_RETURN_POINTER(vg);	
}

/* from the extended vague wkt form, for instance SRID=4326;VAGUEPOINT(POINT(1 1);POINT(2 2)) */
PG_FUNCTION_INFO_V1(VG_vgeomfromevwkt);
Datum VG_vgeomfromevwkt(PG_FUNCTION_ARGS) {
	text *wkt_text = PG_GETARG_TEXT_P(0);
	char *vwkt;
	uint8_t punion =1;
	VAGUEGEOM *vgeom;
	VAGUEGEOMSERIALIZED *vg;

	/* read user-requested pre compute union */
	if ( (PG_NARGS()>1) && (!PG_ARGISNULL(1)) ) {
		if(PG_GETARG_BOOL(1))
			punion = 1;
		else
			punion = 0;
	}

	/* Unwrap the PgSQL text type into a cstring */
	vwkt = text2cstring(wkt_text); 
	vgeom = vgeom_from_vwkt(vwkt, punion);
	if(vgeom==NULL)  {
		ereport(ERROR, (
		        errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		        errmsg("Convertion from Evwkt to vague geometry failed") ));
	}
	/* Add a bbox if necessary */
	if ( lwgeom_needs_bbox(vgeom->kernel) ) lwgeom_add_bbox(vgeom->kernel);
	if ( lwgeom_needs_bbox(vgeom->conjecture) ) lwgeom_add_bbox(vgeom->conjecture);
		
	lwfree(vwkt);
	
	if(vaguegeom_is_valid(vgeom) == LW_FALSE) {
		ereport(ERROR,(errmsg("parse error - invalid vague geometry - kernel and conjecture must be disjoint or meet")));
	}

	vg = vgeometry_serialize(vgeom);
	
	/* return the vague geometry serialized for postgres*/	
	PG_RETURN_POINTER(vg);	
}

/* make a vague geometry from 2 lwgeoms in text format (postgis geometry textual form)
kernel and conjecture can NOT be NULL
*/
PG_FUNCTION_INFO_V1(VG_vgeomfromtext);
Datum VG_vgeomfromtext(PG_FUNCTION_ARGS) {
	text *kernel_wkttext = PG_GETARG_TEXT_P(0);
	text *conjecture_wkttext = PG_GETARG_TEXT_P(1);

	char *k_wkt = text2cstring(kernel_wkttext);
	char *c_wkt = text2cstring(conjecture_wkttext);

	LWGEOM_PARSER_RESULT k_lwg_parser_result;
	LWGEOM_PARSER_RESULT c_lwg_parser_result;

	VAGUEGEOMSERIALIZED *vgeom_result = NULL;
	VAGUEGEOM *vgeom;
	LWGEOM *kernel_lwgeom, *conjecture_lwgeom;

	uint8_t punion = 1; //default is precomputed
	
	/* kernel first */
	if (lwgeom_parse_wkt(&k_lwg_parser_result, k_wkt, LW_PARSER_CHECK_ALL) == LW_FAILURE)
		PG_PARSER_ERROR(k_lwg_parser_result);

	/* then conjecture */
	if (lwgeom_parse_wkt(&c_lwg_parser_result, c_wkt, LW_PARSER_CHECK_ALL) == LW_FAILURE) {
		lwgeom_parser_result_free(&k_lwg_parser_result);
		PG_PARSER_ERROR(c_lwg_parser_result);
	}

	//verifies type and srid
	if(k_lwg_parser_result.geom->type != c_lwg_parser_result.geom->type) {
		ereport(ERROR,
			(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
	         errmsg("they kernel and conjecture must be of the same types")));
	}
	if(k_lwg_parser_result.geom->srid != c_lwg_parser_result.geom->srid) {
		ereport(ERROR,
			(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
	         errmsg("they kernel and conjecture must be of the same SRID")));
	}
		
	kernel_lwgeom = lwgeom_clone_deep(k_lwg_parser_result.geom);
	conjecture_lwgeom = lwgeom_clone_deep(c_lwg_parser_result.geom);

	/* read user-requested SRID if any */
	if ( PG_NARGS() > 2 ) { 
		lwgeom_set_srid(kernel_lwgeom, PG_GETARG_INT32(2));
		lwgeom_set_srid(conjecture_lwgeom, PG_GETARG_INT32(2));
	}
	/* read user-requested pre compute union */
	if ( (PG_NARGS()>3) && (!PG_ARGISNULL(3)) ) {
		if(PG_GETARG_BOOL(3))
			punion = 1;
		else
			punion = 0;
	}

	vgeom = vaguegeom_construct(kernel_lwgeom, conjecture_lwgeom, punion);	
	
	if(vaguegeom_is_valid(vgeom) == LW_FALSE) {
		ereport(ERROR,(errmsg("parse error - invalid vague geometry - kernel and conjecture must be disjoint or meet")));
	}
	vgeom_result = vgeometry_serialize(vgeom);

	lwgeom_parser_result_free(&k_lwg_parser_result);
	lwgeom_parser_result_free(&c_lwg_parser_result);
	vaguegeom_free(vgeom);

	PG_RETURN_POINTER(vgeom_result);
}

//{\"type\": \"%s\", \"kernel\":%s, \"conjecture\":%s}
//{\"type\": \"%s\", \"crs\": %s, \"kernel\":%s, \"conjecture\":%s}
/* make a vaguegeometry from vgeojson (vaguegeojson, punion) */
PG_FUNCTION_INFO_V1(VG_vgeomfromvgeojson);
Datum VG_vgeomfromvgeojson(PG_FUNCTION_ARGS) {
	VAGUEGEOM *vgeom;
	VAGUEGEOMSERIALIZED *result;
	uint8_t punion = 1;
	char *vgeojson;
	text *input;

	if(PG_ARGISNULL(0)) PG_RETURN_NULL();
	if (PG_NARGS() >1 && !PG_ARGISNULL(1)) {
		if(PG_GETARG_BOOL(1))
			punion = 1;
		else
			punion = 0;
	}
	input = PG_GETARG_TEXT_P(0);
	vgeojson = text2cstring(input);
	vgeom = vgeojson_to_vaguegeom(vgeojson, punion);
	if(vaguegeom_is_valid(vgeom) == LW_FALSE) {
		ereport(ERROR,(errmsg("parse error - invalid vague geometry - kernel and conjecture must be disjoint or meet")));
	}
	result = vgeometry_serialize(vgeom);
	valid_convertion_serialized(result);
	vaguegeom_free(vgeom);
	PG_RETURN_POINTER(result);
}

/* make a vaguegeometry from vgml (vgml, srid, punion) */
PG_FUNCTION_INFO_V1(VG_vgeomfromvgml);
Datum VG_vgeomfromvgml(PG_FUNCTION_ARGS) {
	VAGUEGEOM *vgeom;
	VAGUEGEOMSERIALIZED *result;
	uint8_t punion = 1;
	char *vgml;
	text *input;
	int srid = 0;

	if(PG_ARGISNULL(0)) PG_RETURN_NULL();

	if (PG_NARGS() >2 && !PG_ARGISNULL(2)) {
		if(PG_GETARG_BOOL(2))
			punion = 1;
		else
			punion = 0;
	}

	if (PG_NARGS() >1 && !PG_ARGISNULL(1)) {
		srid = PG_GETARG_INT32(1);
	}
	input = PG_GETARG_TEXT_P(0);
	vgml = text2cstring(input);
	vgeom = vaguegeom_from_vgml(vgml, &srid, punion);
	if(vaguegeom_is_valid(vgeom) == LW_FALSE) {
		ereport(ERROR,(errmsg("parse error - invalid vague geometry - kernel and conjecture must be disjoint or meet")));
	}
	result = vgeometry_serialize(vgeom);
	valid_convertion_serialized(result);
	vaguegeom_free(vgeom);
	PG_RETURN_POINTER(result);
}

/* make a vaguegeometry from vkml (vkml, srid, punion) */
PG_FUNCTION_INFO_V1(VG_vgeomfromvkml);
Datum VG_vgeomfromvkml(PG_FUNCTION_ARGS) {
	VAGUEGEOM *vgeom;
	VAGUEGEOMSERIALIZED *result;
	uint8_t punion = 1;
	char *vkml;
	text *input;
	int srid = 0;

	if(PG_ARGISNULL(0)) PG_RETURN_NULL();

	if (PG_NARGS() >2 && !PG_ARGISNULL(2)) {
		if(PG_GETARG_BOOL(2))
			punion = 1;
		else
			punion = 0;
	}

	if (PG_NARGS() >1 && !PG_ARGISNULL(1)) {
		srid = PG_GETARG_INT32(1);
	}
	input = PG_GETARG_TEXT_P(0);
	vkml = text2cstring(input);
	vgeom = vaguegeom_from_vkml(vkml, punion);
	lwgeom_set_srid(vgeom->kernel, srid);
	lwgeom_set_srid(vgeom->conjecture, srid);
	if(vaguegeom_is_valid(vgeom) == LW_FALSE) {
		ereport(ERROR,(errmsg("parse error - invalid vague geometry - kernel and conjecture must be disjoint or meet")));
	}
	result = vgeometry_serialize(vgeom);
	valid_convertion_serialized(result);
	vaguegeom_free(vgeom);
	PG_RETURN_POINTER(result);
}

/******************************************************************************/
/* output to strings */
/******************************************************************************/

/* show in the textual form a vague geometry passed by parameter */
PG_FUNCTION_INFO_V1(VG_astext);
Datum VG_astext(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vgeom_serialized;
	VAGUEGEOM *vgeom;
	char *vg_ewkt;

	text *result;
	uint8_t simplified = LW_FALSE; 

	vgeom_serialized = (VAGUEGEOMSERIALIZED*)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	vgeom = serialization_to_vaguegeom(vgeom_serialized);
	valid_convertion_vaguegeom(vgeom);

	if (PG_NARGS() >1 && !PG_ARGISNULL(1)) {
		if(PG_GETARG_BOOL(1))
			simplified = 1;
		else
			simplified = 0;
	}
	vg_ewkt = vgeom_to_vwkt(vgeom, simplified, LW_FALSE);

	/* Write to text and free the WKT */
	result = cstring2text(vg_ewkt);

	lwfree(vg_ewkt);

	/* Return the text */
	PG_FREE_IF_COPY(vgeom_serialized, 0);
	PG_RETURN_TEXT_P(result);
}

/* shows in the extended VWKT form */
PG_FUNCTION_INFO_V1(VG_asevwkt);
Datum VG_asevwkt(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vgeom_serialized;
	VAGUEGEOM *vgeom;
	char *vg_ewkt;

	text *result;
	uint8_t simplified = LW_FALSE; 

	vgeom_serialized = (VAGUEGEOMSERIALIZED*)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	vgeom = serialization_to_vaguegeom(vgeom_serialized);
	valid_convertion_vaguegeom(vgeom);

	if (PG_NARGS() >1 && !PG_ARGISNULL(1)) {
		if(PG_GETARG_BOOL(1))
			simplified = 1;
		else
			simplified = 0;
	}
	vg_ewkt = vgeom_to_vwkt(vgeom, simplified, LW_TRUE);

	/* Write to text and free the WKT */
	result = cstring2text(vg_ewkt);

	lwfree(vg_ewkt);

	/* Return the text */
	PG_FREE_IF_COPY(vgeom_serialized, 0);
	PG_RETURN_TEXT_P(result);
}

/* show in the vague geojson form 
* (version, vaguegeometry, precision default 15, option)
*/
PG_FUNCTION_INFO_V1(VG_asvgeojson);
Datum VG_asvgeojson(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vgeom_seri;
	VAGUEGEOM *vgeom;
	char *geojson_k=NULL, *geojson_c=NULL, *vgeojson;
	text *result;
	int srid;
	int version;
	int option = 0;
	int has_bbox = 0;
	int precision = OUT_MAX_DOUBLE_PRECISION;
	char *srs = NULL;

	/* Get the version */
	version = PG_GETARG_INT32(0);
	if ( version != 1) {
		elog(ERROR, "Only GeoJSON 1 is supported for VagueGeoJSON type");
		PG_RETURN_NULL();
	}

	/* Get the vague geometry */
	if (PG_ARGISNULL(1) ) PG_RETURN_NULL();
	vgeom_seri = (VAGUEGEOMSERIALIZED *)PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	/* Retrieve precision if any (default is max) */
	if (PG_NARGS() >2 && !PG_ARGISNULL(2)) {
		precision = PG_GETARG_INT32(2);
		if ( precision > OUT_MAX_DOUBLE_PRECISION )
			precision = OUT_MAX_DOUBLE_PRECISION;
		else if ( precision < 0 ) precision = 0;
	}

	/* Retrieve output option
	 * 0 = without option (default)
	 * 1 = bbox
	 * 2 = short crs
	 * 4 = long crs
	 */
	if (PG_NARGS() >3 && !PG_ARGISNULL(3))
		option = PG_GETARG_INT32(3);

	if (option & 2 || option & 4) {
		srid = vgserialized_get_srid(vgeom_seri);
		if ( srid != SRID_UNKNOWN ) {
			if (option & 2) srs = getSRSbySRID(srid, true);
			if (option & 4) srs = getSRSbySRID(srid, false);
			if (!srs) {
				elog(	ERROR,
				      "SRID %i unknown in spatial_ref_sys table",
				      srid);
				PG_RETURN_NULL();
			}
		}
	}

	if (option & 1) has_bbox = 1;

	vgeom = serialization_to_vaguegeom(vgeom_seri);
	valid_convertion_vaguegeom(vgeom);
	/*fix if there is a polygon empty */
	if(HAS_KERNEL(vgeom->flags))
		geojson_k = lwgeom_to_geojson(vgeom->kernel, NULL, precision, has_bbox);
	if(HAS_CONJECTURE(vgeom->flags))
		geojson_c = lwgeom_to_geojson(vgeom->conjecture, NULL, precision, has_bbox);

	if(srs!=NULL) {
		if(HAS_BOTH(vgeom->flags)) {
			vgeojson = (char*)lwalloc(strlen(geojson_k)+strlen(geojson_c)+strlen(srs)+150);
			snprintf(vgeojson, strlen(geojson_k)+strlen(geojson_c)+strlen(srs)+150, "{\"type\": \"%s\", \"crs\":{\"type\":\"name\",\"properties\":{\"name\":\"%s\"}}, \"kernel\":%s, \"conjecture\":%s}", 
			vaguegeom_gettype_i_notcaps(vgeom->type), srs, geojson_k, geojson_c);	
		} else if(HAS_KERNEL(vgeom->flags)) {
			vgeojson = (char*)lwalloc(strlen(geojson_k)+strlen(srs)+150);
			snprintf(vgeojson, strlen(geojson_k)+strlen(srs)+150, "{\"type\": \"%s\", \"crs\":{\"type\":\"name\",\"properties\":{\"name\":\"%s\"}}, \"kernel\":%s}", 
			vaguegeom_gettype_i_notcaps(vgeom->type), srs, geojson_k);
		} else if(HAS_CONJECTURE(vgeom->flags)) {
			vgeojson = (char*)lwalloc(strlen(geojson_c)+strlen(srs)+150);
			snprintf(vgeojson, strlen(geojson_c)+strlen(srs)+150, "{\"type\": \"%s\", \"crs\":{\"type\":\"name\",\"properties\":{\"name\":\"%s\"}}, \"conjecture\":%s}", 
			vaguegeom_gettype_i_notcaps(vgeom->type), srs, geojson_c);
		} else { //empty
			vgeojson = (char*)lwalloc(strlen(srs)+150);
			snprintf(vgeojson, strlen(srs)+150, "{\"type\": \"%s\", \"crs\":{\"type\":\"name\",\"properties\":{\"name\":\"%s\"}}}", 
			vaguegeom_gettype_i_notcaps(vgeom->type), srs);
		}
	} else {
		if(HAS_BOTH(vgeom->flags)) {
			vgeojson = (char*)lwalloc(strlen(geojson_k)+strlen(geojson_c)+100);
			snprintf(vgeojson, strlen(geojson_k)+strlen(geojson_c)+100, "{\"type\": \"%s\", \"kernel\":%s, \"conjecture\":%s}", 
			vaguegeom_gettype_i_notcaps(vgeom->type), geojson_k, geojson_c);	
		} else if(HAS_KERNEL(vgeom->flags)) {
			vgeojson = (char*)lwalloc(strlen(geojson_k)+100);
			snprintf(vgeojson, strlen(geojson_k)+100, "{\"type\": \"%s\", \"kernel\":%s}", 
			vaguegeom_gettype_i_notcaps(vgeom->type), geojson_k);
		} else if(HAS_CONJECTURE(vgeom->flags)) {
			vgeojson = (char*)lwalloc(strlen(geojson_c)+100);
			snprintf(vgeojson, strlen(geojson_c)+100, "{\"type\": \"%s\", \"conjecture\":%s}", 
			vaguegeom_gettype_i_notcaps(vgeom->type), geojson_c);
		} else { //empty
			vgeojson = (char*)lwalloc(100);
			snprintf(vgeojson, 100, "{\"type\": \"%s\"}", 
			vaguegeom_gettype_i_notcaps(vgeom->type));
		}
	}
	vaguegeom_free(vgeom);

	PG_FREE_IF_COPY(vgeom_seri, 1);
	if (srs) pfree(srs);

	if(geojson_k) pfree(geojson_k);
	if(geojson_c) pfree(geojson_c);

	result = cstring2text(vgeojson);

	lwfree(vgeojson);

	PG_RETURN_TEXT_P(result);
}

/* show in the vague gml form, that can be gml2 or gml3
* (version of gml, vaguegeometry, precision default 15, option)
*/
PG_FUNCTION_INFO_V1(VG_asvgml);
Datum VG_asvgml(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vgs;
	VAGUEGEOM *vgeom;
	char *vgml = NULL;
	text *result;
	int version;
	char *srs;
	int srid;
	int option = 0;
	int lwopts = LW_GML_IS_DIMS;
	int precision = OUT_MAX_DOUBLE_PRECISION;
	const char* prefix = "gml:"; /* default prefix */
	char *gml_kernel=NULL, *gml_conjecture=NULL;

	/* Get the version */
	version = PG_GETARG_INT32(0);
	if ( version != 2 && version != 3 ) {
		elog(ERROR, "Only GML 2 and GML 3 are supported");
		PG_RETURN_NULL();
	}
	
	/* Get the vague geometry */
	if ( PG_ARGISNULL(1) ) PG_RETURN_NULL();
	vgs = (VAGUEGEOMSERIALIZED *)PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	if(HAS_NOTHING(vgs->flags)) {
		result = cstring2text("");
		PG_RETURN_TEXT_P(result);
	}
	
	/* Retrieve precision if any (default is max) */
	if (PG_NARGS() >2 && !PG_ARGISNULL(2)) {
		precision = PG_GETARG_INT32(2);
		if ( precision > OUT_MAX_DOUBLE_PRECISION )
			precision = OUT_MAX_DOUBLE_PRECISION;
		else if ( precision < 0 ) precision = 0;
	}

	/* retrieve option */
	if (PG_NARGS() >3 && !PG_ARGISNULL(3))
		option = PG_GETARG_INT32(3);
	
	srid = vgserialized_get_srid(vgs);
	if (srid == SRID_UNKNOWN)      srs = NULL;
	else if (option & 1) srs = getSRSbySRID(srid, false);
	else                 srs = getSRSbySRID(srid, true);

	if (option & 2)  lwopts &= ~LW_GML_IS_DIMS; 
	if (option & 4)  lwopts |= LW_GML_SHORTLINE;
	if (option & 16) lwopts |= LW_GML_IS_DEGREE;
    //if (option & 32) lwopts |= LW_GML_EXTENT; because this show only the envelop (this is not valid yet...)

	vgeom = serialization_to_vaguegeom(vgs);
	valid_convertion_vaguegeom(vgeom);
	
	if (version == 2) {
		if(HAS_KERNEL(vgeom->flags)) {
			gml_kernel = lwgeom_to_gml2(vgeom->kernel, NULL, precision, prefix);
		}
		if(HAS_CONJECTURE(vgeom->flags)) {
			gml_conjecture = lwgeom_to_gml2(vgeom->conjecture, NULL, precision, prefix);
		}
	} else if (version == 3) {
		if(HAS_KERNEL(vgeom->flags)) {
			gml_kernel = lwgeom_to_gml3(vgeom->kernel, NULL, precision, lwopts, prefix, NULL);		
		}
		if(HAS_CONJECTURE(vgeom->flags)) {
			gml_conjecture = lwgeom_to_gml3(vgeom->conjecture, NULL, precision, lwopts, prefix, NULL);		
		}
	}
	
	if(srs!=NULL) {
		if(HAS_BOTH(vgeom->flags)) {
			vgml = (char*)lwalloc(strlen(gml_kernel)+strlen(gml_conjecture)+strlen(srs)+200);
			snprintf(vgml, strlen(gml_kernel)+strlen(gml_conjecture)+strlen(srs)+200, 
				"<vgml:%s srsName=\"%s\"><vgml:Kernel>%s</vgml:Kernel><vgml:Conjecture>%s</vgml:Conjecture></vgml:%s>", 
				vaguegeom_gettype_i_notcaps(vgeom->type), srs, gml_kernel, gml_conjecture, vaguegeom_gettype_i_notcaps(vgeom->type));	
		} else if(HAS_KERNEL(vgeom->flags)) {
			vgml = (char*)lwalloc(strlen(gml_kernel)+strlen(srs)+200);
			snprintf(vgml, strlen(gml_kernel)+strlen(srs)+200, 
				"<vgml:%s srsName=\"%s\"><vgml:Kernel>%s</vgml:Kernel></vgml:%s>", 
				vaguegeom_gettype_i_notcaps(vgeom->type), srs, gml_kernel, vaguegeom_gettype_i_notcaps(vgeom->type));	
		} else if(HAS_CONJECTURE(vgeom->flags)) {
			vgml = (char*)lwalloc(strlen(gml_conjecture)+strlen(srs)+200);
			snprintf(vgml, strlen(gml_conjecture)+strlen(srs)+200, 
				"<vgml:%s srsName=\"%s\"><vgml:Conjecture>%s</vgml:Conjecture></vgml:%s>", 
				vaguegeom_gettype_i_notcaps(vgeom->type), srs, gml_conjecture, vaguegeom_gettype_i_notcaps(vgeom->type));	
		}
	} else {
		if(HAS_BOTH(vgeom->flags)) {
			vgml = (char*)lwalloc(strlen(gml_kernel)+strlen(gml_conjecture)+200);
			snprintf(vgml, strlen(gml_kernel)+strlen(gml_conjecture)+200, 
				"<vgml:%s><vgml:Kernel>%s</vgml:Kernel><vgml:Conjecture>%s</vgml:Conjecture></vgml:%s>", 
				vaguegeom_gettype_i_notcaps(vgeom->type), gml_kernel, gml_conjecture, vaguegeom_gettype_i_notcaps(vgeom->type));	
		} else if(HAS_KERNEL(vgeom->flags)) {
			vgml = (char*)lwalloc(strlen(gml_kernel)+200);
			snprintf(vgml, strlen(gml_kernel)+200, 
				"<vgml:%s><vgml:Kernel>%s</vgml:Kernel></vgml:%s>", 
				vaguegeom_gettype_i_notcaps(vgeom->type), gml_kernel, vaguegeom_gettype_i_notcaps(vgeom->type));	
		} else if(HAS_CONJECTURE(vgeom->flags)) {
			vgml = (char*)lwalloc(strlen(gml_conjecture)+200);
			snprintf(vgml, strlen(gml_conjecture)+200, 
				"<vgml:%s><vgml:Conjecture>%s</vgml:Conjecture></vgml:%s>", 
				vaguegeom_gettype_i_notcaps(vgeom->type), gml_conjecture, vaguegeom_gettype_i_notcaps(vgeom->type));	
		}
	}
		
	vaguegeom_free(vgeom);
	PG_FREE_IF_COPY(vgs, 0);

	if(gml_kernel!=NULL)
		lwfree(gml_kernel);
	if(gml_conjecture!=NULL)
		lwfree(gml_conjecture);

	/* Return null on null */
	if ( ! vgml )
		PG_RETURN_NULL();
	
	if (srs) pfree(srs);

	result = cstring2text(vgml);
	lwfree(vgml);
	PG_RETURN_TEXT_P(result);
}

/* show in the vague kml form
* (vaguegeometry, precision default 15, namespace [prefix] is optional)
*/
PG_FUNCTION_INFO_V1(VG_asVKML);
Datum VG_asVKML(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vgserialize;
	VAGUEGEOM *vgeom;
	char *kml_kernel = NULL, *kml_conjecture = NULL, *vkml=NULL;
	text *result;
	int precision = OUT_MAX_DOUBLE_PRECISION;
	static const char* default_prefix = ""; /* default prefix */
	char *prefixbuf;
	const char* prefix = default_prefix;
	text *prefix_text;
	
	/* Get the geometry */
	if ( PG_ARGISNULL(0) ) PG_RETURN_NULL();
	vgserialize = (VAGUEGEOMSERIALIZED *)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	if(HAS_NOTHING(vgserialize->flags)) {
		result = cstring2text("");
		PG_RETURN_TEXT_P(result);
	}

	/* Retrieve precision if any (default is max) */
	if (PG_NARGS() >1 && !PG_ARGISNULL(1)) {
		precision = PG_GETARG_INT32(1);
		if ( precision > OUT_MAX_DOUBLE_PRECISION )
			precision = OUT_MAX_DOUBLE_PRECISION;
		else if ( precision < 0 ) precision = 0;
	}

	/* retrieve prefix */
	if (PG_NARGS() >2 && !PG_ARGISNULL(2)) {
		prefix_text = PG_GETARG_TEXT_P(2);
		if ( VARSIZE(prefix_text)-VARHDRSZ == 0 ) {
			prefix = "";
		}
		else {
			/* +2 is one for the ':' and one for term null */
			prefixbuf = (char*)palloc(VARSIZE(prefix_text)-VARHDRSZ+2);
			memcpy(prefixbuf, VARDATA(prefix_text),
			       VARSIZE(prefix_text)-VARHDRSZ);
			/* add colon and null terminate */
			prefixbuf[VARSIZE(prefix_text)-VARHDRSZ] = ':';
			prefixbuf[VARSIZE(prefix_text)-VARHDRSZ+1] = '\0';
			prefix = prefixbuf;
		}
	}
	
	vgeom = serialization_to_vaguegeom(vgserialize);
	valid_convertion_vaguegeom(vgeom);

	if(HAS_KERNEL(vgeom->flags)) {
		kml_kernel = lwgeom_to_kml2(vgeom->kernel, precision, prefix);
	}
	if(HAS_CONJECTURE(vgeom->flags)) {
		kml_conjecture = lwgeom_to_kml2(vgeom->conjecture, precision, prefix);
	}
	
	if(HAS_BOTH(vgeom->flags)) {
		vkml = (char*)lwalloc(strlen(kml_kernel)+strlen(kml_conjecture)+200);
		snprintf(vkml, strlen(kml_kernel)+strlen(kml_conjecture)+200, 
			"<vkml:%s><vkml:Kernel>%s</vkml:Kernel><vkml:Conjecture>%s</vkml:Conjecture></vkml:%s>", 
			vaguegeom_gettype_i_notcaps(vgeom->type), kml_kernel, kml_conjecture, vaguegeom_gettype_i_notcaps(vgeom->type));
	} else if(HAS_KERNEL(vgeom->flags)) {
		vkml = (char*)lwalloc(strlen(kml_kernel)+200);
		snprintf(vkml, strlen(kml_kernel)+200, 
			"<vkml:%s><vkml:Kernel>%s</vkml:Kernel></vkml:%s>", 
			vaguegeom_gettype_i_notcaps(vgeom->type), kml_kernel, vaguegeom_gettype_i_notcaps(vgeom->type));
	} else if(HAS_CONJECTURE(vgeom->flags)) {
		vkml = (char*)lwalloc(strlen(kml_conjecture)+200);
		snprintf(vkml, strlen(kml_conjecture)+200, 
			"<vkml:%s><vkml:Conjecture>%s</vkml:Conjecture></vkml:%s>", 
			vaguegeom_gettype_i_notcaps(vgeom->type), kml_conjecture, vaguegeom_gettype_i_notcaps(vgeom->type));
	}
		
	vaguegeom_free(vgeom);
	PG_FREE_IF_COPY(vgserialize, 0);

	if(kml_kernel!=NULL)
		lwfree(kml_kernel);
	if(kml_conjecture!=NULL)
		lwfree(kml_conjecture);

	/* Return null on null */
	if ( ! vkml )
		PG_RETURN_NULL();

	result = cstring2text(vkml);
	lwfree(vkml);
	PG_RETURN_TEXT_P(result);
}

/* show the vague geometry in binary format, that is, bytea */
PG_FUNCTION_INFO_V1(VG_asvwkb);
Datum VG_asvwkb(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *geom = (VAGUEGEOMSERIALIZED*)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	VAGUEGEOM *vgeom;
	uint8_t *wkb;
	size_t wkb_size;
	uint8_t variant = 0;
 	bytea *result;
	text *type;

	/* If user specified endianness, respect it */
	if ( (PG_NARGS()>1) && (!PG_ARGISNULL(1)) ) {
		type = PG_GETARG_TEXT_P(1);

		if  ( ! strncmp(VARDATA(type), "xdr", 3) ||
		      ! strncmp(VARDATA(type), "XDR", 3) )
		{
			variant = variant | WKB_XDR;
		}
		else
		{
			variant = variant | WKB_NDR;
		}
	}

	/* Create VWKB */
	vgeom = serialization_to_vaguegeom(geom); 
	wkb = vaguegeom_to_vwkb(vgeom, variant | WKB_NO_SRID , &wkb_size);
	vaguegeom_free(vgeom);
	
	result = uint_8_to_bytea(wkb, wkb_size);
	
	/* Clean up and return */
	pfree(wkb);
	PG_FREE_IF_COPY(geom, 0);
	PG_RETURN_BYTEA_P(result);
}

/* show the vague geometry in extended binary format, that is, bytea */
PG_FUNCTION_INFO_V1(VG_asevwkb);
Datum VG_asevwkb(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *geom = (VAGUEGEOMSERIALIZED*)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	VAGUEGEOM *vgeom;
	uint8_t *wkb;
	size_t wkb_size;
	uint8_t variant = 0;
 	bytea *result;
	text *type;

	/* If user specified endianness, respect it */
	if ( (PG_NARGS()>1) && (!PG_ARGISNULL(1)) ) {
		type = PG_GETARG_TEXT_P(1);

		if  ( ! strncmp(VARDATA(type), "xdr", 3) ||
		      ! strncmp(VARDATA(type), "XDR", 3) )
		{
			variant = variant | WKB_XDR;
		}
		else
		{
			variant = variant | WKB_NDR;
		}
	}

	/* Create VWKB */
	vgeom = serialization_to_vaguegeom(geom); 
	wkb = vaguegeom_to_vwkb(vgeom, variant | WKB_EXTENDED , &wkb_size);
	vaguegeom_free(vgeom);
	
	result = uint_8_to_bytea(wkb, wkb_size);
	
	/* Clean up and return */
	pfree(wkb);
	PG_FREE_IF_COPY(geom, 0);
	PG_RETURN_BYTEA_P(result);
}

/* make a vague geometry from 2 lwgeoms (postgis geometry form)
kernel and conjecture can be NULL
*/
PG_FUNCTION_INFO_V1(VG_make);
Datum VG_make(PG_FUNCTION_ARGS) {
	GSERIALIZED *kernel=NULL, *conjecture=NULL;
	LWGEOM *lwkernel=NULL, *lwconjecture=NULL;
	VAGUEGEOM *vague;
	VAGUEGEOMSERIALIZED *result;
	uint8_t punion = 1; //precomputed union default is true

	if(!PG_ARGISNULL(0))
		kernel = (GSERIALIZED*)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	if(!PG_ARGISNULL(1))
		conjecture = (GSERIALIZED*)PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	if(kernel != NULL && conjecture != NULL) {
		//verifies type and srid
		if(gserialized_get_type(kernel) != gserialized_get_type(conjecture)) {
			ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		         errmsg("kernel and conjecture must be of the same types")));
		}
		if(gserialized_get_srid(kernel) != gserialized_get_srid(conjecture)) {
			ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		         errmsg("kernel and conjecture must be of the same SRID")));
		}
	}

	if(kernel == NULL && conjecture == NULL) {
		ereport(ERROR,
				(errcode(ERRCODE_INTERNAL_ERROR),
		         errmsg("vague geometry must have a kernel or conjecture")));
	}
	
	if(kernel != NULL) {
		lwkernel = lwgeom_from_gserialized(kernel);
		if(lwkernel==NULL) {
			ereport(ERROR,
				(errcode(ERRCODE_INTERNAL_ERROR),
		         errmsg("convertion from gserialized to lwgeom failed")));
		}
		if(conjecture==NULL)
			lwconjecture = lwgeom_construct_empty(lwkernel->type, lwkernel->srid, FLAGS_GET_Z(lwkernel->flags), FLAGS_GET_M(lwkernel->flags));
	}
	if(conjecture != NULL) {
		lwconjecture = lwgeom_from_gserialized(conjecture);
		if(lwconjecture==NULL) {
			ereport(ERROR,
				(errcode(ERRCODE_INTERNAL_ERROR),
		         errmsg("convertion from gserialized to lwgeom failed")));
		}
		if(kernel==NULL)
			lwkernel = lwgeom_construct_empty(lwconjecture->type, lwconjecture->srid, FLAGS_GET_Z(lwconjecture->flags), FLAGS_GET_M(lwconjecture->flags));
	}

	if ( (PG_NARGS()>2) && (!PG_ARGISNULL(2)) ) {
		if(PG_GETARG_BOOL(2))
			punion = 1;
		else
			punion = 0;
	}

	vague = vaguegeom_construct(lwkernel, lwconjecture, punion);
	if(vaguegeom_is_valid(vague) == LW_FALSE) {
		ereport(ERROR,(errmsg("parse error - invalid vague geometry - kernel and conjecture must be disjoint or meet")));
	}
	result = vgeometry_serialize(vague);

	//free memory
	vaguegeom_free(vague);

	if(!PG_ARGISNULL(0))
		PG_FREE_IF_COPY(kernel, 0);
	if(!PG_ARGISNULL(1))
		PG_FREE_IF_COPY(conjecture, 1);
	PG_RETURN_POINTER(result);
}

/* make a vague geometry from 2 lwgeoms (postgis geometry form)
kernel and conjecture can be NULL
performing the difference between kernel and conjecture
*/
PG_FUNCTION_INFO_V1(VG_enforcemake);
Datum VG_enforcemake(PG_FUNCTION_ARGS) {
	GSERIALIZED *kernel=NULL, *conjecture=NULL;
	LWGEOM *lwkernel=NULL, *lwconjecture=NULL;
	VAGUEGEOM *vague;
	VAGUEGEOMSERIALIZED *result;
	uint8_t punion = 1; //precomputed union default is true

	if(!PG_ARGISNULL(0))
		kernel = (GSERIALIZED*)PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	if(!PG_ARGISNULL(1))
		conjecture = (GSERIALIZED*)PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
	if(kernel == NULL && conjecture == NULL) {
		ereport(ERROR,
				(errcode(ERRCODE_INTERNAL_ERROR),
		         errmsg("vague geometry must have a kernel or conjecture")));
	}
	if(kernel != NULL && conjecture != NULL) {
		//verifies type and srid
		if(gserialized_get_type(kernel) != gserialized_get_type(conjecture)) {
			ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		         errmsg("they kernel and conjecture must be of the same types")));
		}
		if(gserialized_get_srid(kernel) != gserialized_get_srid(conjecture)) {
			ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		         errmsg("they kernel and conjecture must be of the same SRID")));
		}
	}

	if(kernel != NULL) {
		lwkernel = lwgeom_from_gserialized(kernel);
		if(lwkernel==NULL) {
			ereport(ERROR,
				(errcode(ERRCODE_INTERNAL_ERROR),
		         errmsg("convertion from gserialized to lwgeom failed")));
		}
		if(conjecture==NULL)
			lwconjecture = lwgeom_construct_empty(lwkernel->type, lwkernel->srid, FLAGS_GET_Z(lwkernel->flags), FLAGS_GET_M(lwkernel->flags));
	}
	if(conjecture != NULL) {
		lwconjecture = lwgeom_from_gserialized(conjecture);
		if(lwconjecture==NULL) {
			ereport(ERROR,
				(errcode(ERRCODE_INTERNAL_ERROR),
		         errmsg("convertion from gserialized to lwgeom failed")));
		}
		if(kernel==NULL)
			lwkernel = lwgeom_construct_empty(lwconjecture->type, lwconjecture->srid, FLAGS_GET_Z(lwconjecture->flags), FLAGS_GET_M(lwconjecture->flags));
	}

	if ( (PG_NARGS()>2) && (!PG_ARGISNULL(2)) ) {
		if(PG_GETARG_BOOL(2))
			punion = 1;
		else
			punion = 0;
	}
	if(lwkernel != NULL && lwconjecture != NULL) {
		lwkernel = lwgeom_difference(lwkernel, lwconjecture);
		//this can occurs when are the same geometries
		if(lwkernel->type==COLLECTIONTYPE && lwgeom_is_empty(lwkernel)) {
			lwgeom_free(lwkernel);
			lwkernel = lwgeom_construct_empty(lwconjecture->type, lwconjecture->srid, FLAGS_GET_Z(lwconjecture->flags), FLAGS_GET_M(lwconjecture->flags));
		}
	}
		
	vague = vaguegeom_construct(lwkernel, lwconjecture, punion);
	if(vaguegeom_is_valid(vague) == LW_FALSE) {
		ereport(ERROR,(errmsg("parse error - invalid vague geometry - kernel and conjecture must be disjoint or meet")));
	}
	result = vgeometry_serialize(vague);

	//free memory
	vaguegeom_free(vague);

	if(!PG_ARGISNULL(0))
		PG_FREE_IF_COPY(kernel, 0);
	if(!PG_ARGISNULL(1))
		PG_FREE_IF_COPY(conjecture, 1);
	PG_RETURN_POINTER(result);
}
