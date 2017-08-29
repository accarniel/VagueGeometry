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
 
/************************
*
* All functions that handles VagueNumeric objects.
*
*************************/

#include "util_pg.h"
#include "fmgr.h"

/*distance and measure functions */
Datum VG_farthest_max_distance(PG_FUNCTION_ARGS);
Datum VG_farthest_min_distance(PG_FUNCTION_ARGS);
Datum VG_nearest_max_distance(PG_FUNCTION_ARGS);
Datum VG_nearest_min_distance(PG_FUNCTION_ARGS);
Datum VG_max_area(PG_FUNCTION_ARGS);
Datum VG_min_area(PG_FUNCTION_ARGS);
Datum VG_min_length(PG_FUNCTION_ARGS);
Datum VG_max_length(PG_FUNCTION_ARGS);
Datum VG_min_ncomp(PG_FUNCTION_ARGS);
Datum VG_max_ncomp(PG_FUNCTION_ARGS);

/* these functions will return a VAGUENUMERIC */
Datum VG_farthest_distance(PG_FUNCTION_ARGS);
Datum VG_nearest_distance(PG_FUNCTION_ARGS);
Datum VG_area(PG_FUNCTION_ARGS);
Datum VG_length(PG_FUNCTION_ARGS);
Datum VG_ncomp(PG_FUNCTION_ARGS);

/* these functions will return number of a VAGUENUMERIC */
Datum VG_GetMIN(PG_FUNCTION_ARGS);
Datum VG_GetMAX(PG_FUNCTION_ARGS);

Datum VG_MakeVagueNumeric(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(VG_GetMIN);
Datum VG_GetMIN(PG_FUNCTION_ARGS) {
	VAGUENUMERIC *vn = (VAGUENUMERIC*) PG_GETARG_POINTER(0);
	PG_RETURN_FLOAT8(vn->min);
}

PG_FUNCTION_INFO_V1(VG_GetMAX);
Datum VG_GetMAX(PG_FUNCTION_ARGS) {
	VAGUENUMERIC *vn = (VAGUENUMERIC*) PG_GETARG_POINTER(0);
	PG_RETURN_FLOAT8(vn->max);
}

PG_FUNCTION_INFO_V1(VG_MakeVagueNumeric);
Datum VG_MakeVagueNumeric(PG_FUNCTION_ARGS) {
	double min = PG_GETARG_FLOAT8(0);
	double max = PG_GETARG_FLOAT8(1);
	VAGUENUMERIC *result;

	result = (VAGUENUMERIC*) palloc(sizeof(VAGUENUMERIC));
	
	result->min = min;
	result->max = max;
	
	PG_RETURN_POINTER(result);
}

/* 
*  Distance function returning max or min distances
*/
PG_FUNCTION_INFO_V1(VG_farthest_max_distance);
Datum VG_farthest_max_distance(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	double result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	if (vague1->kernel->srid != vague2->kernel->srid)	{
		ereport(ERROR, (
		            errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		            errmsg("Operation on two VAGUEGEOMETRIES with different SRIDs") ));
		PG_RETURN_NULL();
	}
	
	result = vaguegeom_farthest_max_distance(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(VG_farthest_min_distance);
Datum VG_farthest_min_distance(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	double result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);
	if (vague1->kernel->srid != vague2->kernel->srid)	{
		ereport(ERROR, (
		            errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		            errmsg("Operation on two VAGUEGEOMETRIES with different SRIDs") ));
		PG_RETURN_NULL();
	}
	result = vaguegeom_farthest_min_distance(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(VG_nearest_max_distance);
Datum VG_nearest_max_distance(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	double result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);
	if (vague1->kernel->srid != vague2->kernel->srid)	{
		ereport(ERROR, (
		            errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		            errmsg("Operation on two VAGUEGEOMETRIES with different SRIDs") ));
		PG_RETURN_NULL();
	}
	result = vaguegeom_nearest_max_distance(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(VG_nearest_min_distance);
Datum VG_nearest_min_distance(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	double result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);
	if (vague1->kernel->srid != vague2->kernel->srid)	{
		ereport(ERROR, (
		            errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		            errmsg("Operation on two VAGUEGEOMETRIES with different SRIDs") ));
		PG_RETURN_NULL();
	}
	result = vaguegeom_nearest_min_distance(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_FLOAT8(result);
}

/* 
* area, length and ncomp functions
*/
PG_FUNCTION_INFO_V1(VG_max_area);
Datum VG_max_area(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1;
	VAGUEGEOM *vague1;
	double result;
	uint8_t type;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	type = vgserialized_get_type(vg1);

	if(type != VAGUEPOLYGONTYPE && type != VAGUEMULTIPOLYGONTYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Area operation only for vague polygon")));
	}

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	result = vaguegeom_max_area(vague1);

	/* memory free */
	vaguegeom_free(vague1);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(VG_min_area);
Datum VG_min_area(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1;
	VAGUEGEOM *vague1;
	double result;
	uint8_t type;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	type = vgserialized_get_type(vg1);
	if(type != VAGUEPOLYGONTYPE && type != VAGUEMULTIPOLYGONTYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Area operation only for vague polygon")));
	}

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	result = vaguegeom_min_area(vague1);

	/* memory free */
	vaguegeom_free(vague1);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(VG_min_length);
Datum VG_min_length(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1;
	VAGUEGEOM *vague1;
	double result;
	uint8_t type;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	type = vgserialized_get_type(vg1);
	if(type != VAGUELINETYPE && type != VAGUEMULTILINETYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Length operation only for vague line")));
	}

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	result = vaguegeom_min_length(vague1);

	/* memory free */
	vaguegeom_free(vague1);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(VG_max_length);
Datum VG_max_length(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1;
	VAGUEGEOM *vague1;
	double result;
	uint8_t type;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	type = vgserialized_get_type(vg1);
	if(type != VAGUELINETYPE && type != VAGUEMULTILINETYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Length operation only for vague line")));
	}

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	result = vaguegeom_max_length(vague1);

	/* memory free */
	vaguegeom_free(vague1);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(VG_min_ncomp);
Datum VG_min_ncomp(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1;
	VAGUEGEOM *vague1;
	double result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	result = vaguegeom_min_ncomp(vague1);

	/* memory free */
	vaguegeom_free(vague1);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(VG_max_ncomp);
Datum VG_max_ncomp(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1;
	VAGUEGEOM *vague1;
	double result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	result = vaguegeom_max_ncomp(vague1);

	/* memory free */
	vaguegeom_free(vague1);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_FLOAT8(result);
}

/* these functions will return a VAGUENUMERIC */
PG_FUNCTION_INFO_V1(VG_farthest_distance);
Datum VG_farthest_distance(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	VAGUENUMERIC *result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);
	if (vague1->kernel->srid != vague2->kernel->srid)	{
		ereport(ERROR, (
		            errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		            errmsg("Operation on two VAGUEGEOMETRIES with different SRIDs") ));
		PG_RETURN_NULL();
	}
	result = vaguegeom_farthest_distance(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_nearest_distance);
Datum VG_nearest_distance(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	VAGUENUMERIC *result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);
	if (vague1->kernel->srid != vague2->kernel->srid)	{
		ereport(ERROR, (
		            errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		            errmsg("Operation on two VAGUEGEOMETRIES with different SRIDs") ));
		PG_RETURN_NULL();
	}
	result = vaguegeom_nearest_distance(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_area);
Datum VG_area(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1;
	VAGUEGEOM *vague1;
	VAGUENUMERIC *result;
	uint8_t type;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	type = vgserialized_get_type(vg1);
	if(type != VAGUEPOLYGONTYPE && type != VAGUEMULTIPOLYGONTYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Area operation only for vague polygon")));
	}

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	result = vaguegeom_area(vague1);

	/* memory free */
	vaguegeom_free(vague1);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_length);
Datum VG_length(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1;
	VAGUEGEOM *vague1;
	VAGUENUMERIC *result;
	uint8_t type;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	type = vgserialized_get_type(vg1);
	if(type != VAGUELINETYPE && type != VAGUEMULTILINETYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Length operation only for vague polygon")));
	}

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	result = vaguegeom_length(vague1);

	/* memory free */
	vaguegeom_free(vague1);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_ncomp);
Datum VG_ncomp(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1;
	VAGUEGEOM *vague1;
	VAGUENUMERIC *result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	result = vaguegeom_ncomp(vague1);

	/* memory free */
	vaguegeom_free(vague1);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_POINTER(result);
}