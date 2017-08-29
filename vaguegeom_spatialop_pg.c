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
* Basic Spatial operations of VagueGeometry, which includes
* vague geometric set operations, type-dependent operations, and general operations
*
*************************/

#include "util_pg.h"
#include "fmgr.h"
#include "utils/array.h"

#include "lwgeom_pg.h"

/* spatial operations that return vaguegeometry */

/* vague spatial set operations */
Datum VG_intersection(PG_FUNCTION_ARGS);
Datum VG_union(PG_FUNCTION_ARGS);
Datum VG_union_vgarray(PG_FUNCTION_ARGS);
Datum VG_difference(PG_FUNCTION_ARGS);

/* other spatial operations */
Datum VG_common_border(PG_FUNCTION_ARGS);
Datum VG_common_points(PG_FUNCTION_ARGS);
Datum VG_get_conjecture(PG_FUNCTION_ARGS); //projection conjecture
Datum VG_get_kernel(PG_FUNCTION_ARGS); //projection kernel
Datum VG_invert(PG_FUNCTION_ARGS);
Datum VG_same(PG_FUNCTION_ARGS);

/*conjecture versions for spatial operations */
Datum VG_conjecture_boundary(PG_FUNCTION_ARGS);
Datum VG_conjecture_convexhull(PG_FUNCTION_ARGS);
Datum VG_conjecture_interior(PG_FUNCTION_ARGS);
Datum VG_conjecture_vertices(PG_FUNCTION_ARGS);

/*kernel versions for spatial operations */
Datum VG_kernel_boundary(PG_FUNCTION_ARGS);
Datum VG_kernel_convexhull(PG_FUNCTION_ARGS);
Datum VG_kernel_interior(PG_FUNCTION_ARGS);
Datum VG_kernel_vertices(PG_FUNCTION_ARGS);


/* intersection(vg1, vg2) -> vg3 */
PG_FUNCTION_INFO_V1(VG_intersection);
Datum VG_intersection(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2, *result;
	VAGUEGEOM *vague1, *vague2, *vague3;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);
	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	vague3 = vaguegeom_intersection(vague1, vague2);

	result = vgeometry_serialize(vague3);
	valid_convertion_serialized(result);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	vaguegeom_free(vague3);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}

/* union(vg1, vg2) -> vg3 */
PG_FUNCTION_INFO_V1(VG_union);
Datum VG_union(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2, *result;
	VAGUEGEOM *vague1, *vague2, *vague3;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);
	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	vague3 = vaguegeom_union(vague1, vague2);

	result = vgeometry_serialize(vague3);
	valid_convertion_serialized(result);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	vaguegeom_free(vague3);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}

/* union([vg1,..., vgn]) -> vg3 */
PG_FUNCTION_INFO_V1(VG_union_vgarray);
Datum VG_union_vgarray(PG_FUNCTION_ARGS) {
	Datum datum;
	ArrayType *array;
	int nelems;
	VAGUEGEOMSERIALIZED *result=NULL;
	VAGUEGEOM **geoms; //array of vgeoms
	VAGUEGEOM *resultvg;
	uint32 ngeoms;
	int i;
	size_t offset;
	int srid=SRID_UNKNOWN;
	int type;

	bits8 *bitmap;
	int bitmask;
	
	/* Get input datum */
	datum = PG_GETARG_DATUM(0);

	/* Return null on null input */
	if ( (Pointer *)datum == NULL ) {
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("null input")));
		PG_RETURN_NULL();
	}

	/* Get actual ArrayType */
	array = DatumGetArrayTypeP(datum);

	/* Get number of geometries in array */
	nelems = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));

	/* Return null on 0-elements input array */
	if ( nelems == 0 ) {
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("the array doesnt has vague geometries")));
		PG_RETURN_NULL();
	}
	
	/* possibly more then required */
	geoms = (VAGUEGEOM**)palloc(sizeof(VAGUEGEOM *)*nelems);
	ngeoms = 0;
	offset = 0;
	bitmap = ARR_NULLBITMAP(array);
	bitmask = 1;
	for (i=0; i<nelems; i++){
		/* Don't do anything for NULL values */
		if ((bitmap && (*bitmap & bitmask) != 0) || !bitmap) {
			VAGUEGEOMSERIALIZED *geom = (VAGUEGEOMSERIALIZED *)(ARR_DATA_PTR(array)+offset);
			offset += INTALIGN(VARSIZE(geom));

			//validate here the differents types
			geoms[ngeoms++] = serialization_to_vaguegeom(geom);

			/* Check SRID homogeneity */
			if ( ngeoms == 1 ) {
				/* Get first geometry SRID */
				srid = vaguegeom_getsrid(geoms[ngeoms-1]);
				type = geoms[ngeoms-1]->type;
			}
			else {
				if ( vaguegeom_getsrid(geoms[ngeoms-1]) != srid ) {
					ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Operation on mixed SRID geometries")));
					PG_RETURN_NULL();
				}
				if ( (geoms[ngeoms-1])->type != type ) {
					ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Operation on mixed type geometries")));
					PG_RETURN_NULL();
				}
			}
		}

		/* Advance NULL bitmap */
		if (bitmap) {
			bitmask <<= 1;
			if (bitmask == 0x100) {
				bitmap++;
				bitmask = 1;
			}
		}
	}

	/* Return null on 0-points input array */
	if ( ngeoms == 0 ) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("the array doesnt has vague geometries")));
		PG_RETURN_NULL();
	}
	
	resultvg = vaguegeom_clone(geoms[0]);
	
	for(i=1; i<ngeoms;i++) {
		resultvg = vaguegeom_union(resultvg, geoms[i]);	
	}
	if(resultvg==NULL) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("result of aggregate union is null")));
	}

	//clean
	for(i=0; i<ngeoms;i++) {
		vaguegeom_free(geoms[i]);
	}
	result = vgeometry_serialize(resultvg);
	if(result==NULL)
		PG_RETURN_NULL();
	vaguegeom_free(resultvg);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_difference);
Datum VG_difference(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2, *result;
	VAGUEGEOM *vague1, *vague2, *vague3;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);
	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	vague3 = vaguegeom_difference(vague1, vague2);

	result = vgeometry_serialize(vague3);
	valid_convertion_serialized(result);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	vaguegeom_free(vague3);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_common_border);
Datum VG_common_border(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2, *result;
	VAGUEGEOM *vague1, *vague2, *vague3;
	int32_t type1, type2;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	/*common border (vagueline, vagueregion) or (vagueregion, vagueregion) */
	type1 = vgserialized_get_type(vg1);
	type2 = vgserialized_get_type(vg2);
	if(type1 != VAGUELINETYPE && type1 != VAGUEMULTILINETYPE && 
		type1 != VAGUEPOLYGONTYPE && type1 != VAGUEMULTIPOLYGONTYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("For common border operation, vague geometry type must be vague line or vague polygon for first parameter")));
	}

	if(type2 != VAGUELINETYPE && type2 != VAGUEMULTILINETYPE && 
		type2 != VAGUEPOLYGONTYPE && type2 != VAGUEMULTIPOLYGONTYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("For common border operation, vague geometry type must be vague polygon or vague line for second parameter")));
	}

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);
	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	vague3 = vaguegeom_common_border(vague1, vague2);

	result = vgeometry_serialize(vague3);
	valid_convertion_serialized(result);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	vaguegeom_free(vague3);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_common_points);
Datum VG_common_points(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2, *result;
	VAGUEGEOM *vague1, *vague2, *vague3;
	int32_t type1, type2;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	/*common points (vagueline, vagueline) */
	type1 = vgserialized_get_type(vg1);
	type2 = vgserialized_get_type(vg2);
	if(type1 != VAGUELINETYPE && type1 != VAGUEMULTILINETYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("For common points operation, vague geometry type must be vague line for first parameter")));
	}

	if(type2 != VAGUELINETYPE && type2 != VAGUEMULTILINETYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("For common points operation, vague geometry type must be vague line for second parameter")));
	}

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);
	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	vague3 = vaguegeom_common_points(vague1, vague2);

	result = vgeometry_serialize(vague3);
	valid_convertion_serialized(result);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	vaguegeom_free(vague3);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}


PG_FUNCTION_INFO_V1(VG_get_conjecture);
Datum VG_get_conjecture(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1;
	LWGEOM *conjecture;
	GSERIALIZED *result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	conjecture = get_conjecture_from_serialization(vg1);
	if(conjecture==NULL) {
		PG_RETURN_NULL();
	}

	//postgis function because of conjecture projection 
	result = geometry_serialize(conjecture);

	/* memory free */
	lwgeom_free(conjecture);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_get_kernel);
Datum VG_get_kernel(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1;
	LWGEOM *kernel;
	GSERIALIZED *result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	kernel = get_kernel_from_serialization(vg1);
	if(kernel==NULL) {
		PG_RETURN_NULL();
	}

	//postgis function because of kernel projection 
	result = geometry_serialize(kernel);

	/* memory free */
	lwgeom_free(kernel);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_invert);
Datum VG_invert(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *result;
	VAGUEGEOM *vague1, *vague2;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = vaguegeom_invert(vague1);

	result = vgeometry_serialize(vague2);
	valid_convertion_serialized(result);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_same);
Datum VG_same(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	uint8_t result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	result = vaguegeom_same(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_BOOL(result);
}

/*conjecture versions for operations */
PG_FUNCTION_INFO_V1(VG_conjecture_boundary);
Datum VG_conjecture_boundary(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *result;
	VAGUEGEOM *vague1, *vague2;
	uint8_t type;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	type = vgserialized_get_type(vg1);
	if(type != VAGUEPOLYGONTYPE && type != VAGUEMULTIPOLYGONTYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Conjecture-boundary operation only for vague polygon")));
	}

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = vaguegeom_conjecture_boundary(vague1);
	result = vgeometry_serialize(vague2);
	valid_convertion_serialized(result);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_conjecture_convexhull);
Datum VG_conjecture_convexhull(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *result;
	VAGUEGEOM *vague1, *vague2;
	uint8_t type;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	type = vgserialized_get_type(vg1);
	if(type != VAGUEPOINTTYPE && type != VAGUEMULTIPOINTTYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Conjecture-convexhull operation only for vague points")));
	}

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = vaguegeom_conjecture_convexhull(vague1);
	result = vgeometry_serialize(vague2);
	valid_convertion_serialized(result);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_conjecture_interior);
Datum VG_conjecture_interior(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *result;
	VAGUEGEOM *vague1, *vague2;
	uint8_t type;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	type = vgserialized_get_type(vg1);
	if(type != VAGUELINETYPE && type != VAGUEMULTILINETYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Conjecture-interior operation only for vague line")));
	}

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = vaguegeom_conjecture_interior(vague1);
	result = vgeometry_serialize(vague2);
	valid_convertion_serialized(result);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_conjecture_vertices);
Datum VG_conjecture_vertices(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *result;
	VAGUEGEOM *vague1, *vague2;
	uint8_t type;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	type = vgserialized_get_type(vg1);
	if(type != VAGUELINETYPE && type != VAGUEMULTILINETYPE && type != VAGUEPOLYGONTYPE && type != VAGUEMULTIPOLYGONTYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Conjecture-vertices operation only for vague polygon or vague line")));
	}

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = vaguegeom_conjecture_vertices(vague1);
	result = vgeometry_serialize(vague2);
	valid_convertion_serialized(result);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_POINTER(result);
}

/*kernel versions for operations */
PG_FUNCTION_INFO_V1(VG_kernel_boundary);
Datum VG_kernel_boundary(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *result;
	VAGUEGEOM *vague1, *vague2;
	uint8_t type;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	type = vgserialized_get_type(vg1);
	if(type != VAGUEPOLYGONTYPE && type != VAGUEMULTIPOLYGONTYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Kernel-boundary operation only for vague polygon")));
	}

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = vaguegeom_kernel_boundary(vague1);
	result = vgeometry_serialize(vague2);
	valid_convertion_serialized(result);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_kernel_convexhull);
Datum VG_kernel_convexhull(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *result;
	VAGUEGEOM *vague1, *vague2;
	uint8_t type;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	type = vgserialized_get_type(vg1);
	if(type != VAGUEPOINTTYPE && type != VAGUEMULTIPOINTTYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Kernel-convexhull operation only for vague points")));
	}

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = vaguegeom_kernel_convexhull(vague1);
	result = vgeometry_serialize(vague2);
	valid_convertion_serialized(result);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_kernel_interior);
Datum VG_kernel_interior(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *result;
	VAGUEGEOM *vague1, *vague2;
	uint8_t type;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	type = vgserialized_get_type(vg1);
	if(type != VAGUELINETYPE && type != VAGUEMULTILINETYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Kernel-interior operation only for vague line")));
	}

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = vaguegeom_kernel_interior(vague1);
	result = vgeometry_serialize(vague2);
	valid_convertion_serialized(result);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_kernel_vertices);
Datum VG_kernel_vertices(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *result;
	VAGUEGEOM *vague1, *vague2;
	uint8_t type;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));

	type = vgserialized_get_type(vg1);
	if(type != VAGUELINETYPE && type != VAGUEMULTILINETYPE && type != VAGUEPOLYGONTYPE && type != VAGUEMULTIPOLYGONTYPE) {
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Kernel-vertices operation only for vague polygon or vague line")));
	}

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = vaguegeom_kernel_vertices(vague1);
	result = vgeometry_serialize(vague2);
	valid_convertion_serialized(result);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_RETURN_POINTER(result);
}
