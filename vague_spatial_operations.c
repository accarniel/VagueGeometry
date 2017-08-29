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
 
/*************************************************
* Internal processing of basic operations that handle VagueGeometry objects (in the form of VGEOM)
**************************************************/

#include "libvgeom.h"

static uint8_t extract_super(uint8_t vasa_type) {
	switch (vasa_type)
	{
case VAGUEPOINTTYPE:
	return VAGUEMULTIPOINTTYPE;
case VAGUELINETYPE:
	return VAGUEMULTILINETYPE;
case VAGUEPOLYGONTYPE:
	return VAGUEMULTIPOLYGONTYPE;
case VAGUEMULTIPOINTTYPE:
	return VAGUEMULTIPOINTTYPE;
case VAGUEMULTILINETYPE:
	return VAGUEMULTILINETYPE;
case VAGUEMULTIPOLYGONTYPE:
	return VAGUEMULTIPOLYGONTYPE;
	default:
		lwerror("type not supported");
		return 0;
	}	
}

/* this function will to harmonize the kernel and conjecture types for ensure the same types, according with vasa_type*/
static VAGUEGEOM *vague_harmonize_types(LWGEOM *k, LWGEOM *c, uint8_t punion, uint8_t vasa_type) {
	LWGEOM *kernel=NULL, *conjecture=NULL;
	if(k->type == COLLECTIONTYPE || c->type == COLLECTIONTYPE) {
		int i, ngeoms, j;
		/*lets remove the geometries that not respect the vasa data type */
		if(k->type == COLLECTIONTYPE) {
			LWCOLLECTION *colk = (LWCOLLECTION*)k;
			uint8_t type;
			ngeoms = colk->ngeoms;
			kernel = (LWGEOM*)lwcollection_construct_empty(get_potgis_type_from_vaguegeom(vasa_type), k->srid, 0, 0);
			for(i=0;i <  ngeoms; i++) {
				type = colk->geoms[i]->type;
				if( ((type == MULTIPOINTTYPE || type == POINTTYPE) && vasa_type == VAGUEMULTIPOINTTYPE) ||
				((type == MULTILINETYPE || type == LINETYPE) && vasa_type == VAGUEMULTILINETYPE) ||
				((type == MULTIPOLYGONTYPE || type == POLYGONTYPE) && vasa_type == VAGUEMULTIPOLYGONTYPE)) {
					if(lwtype_is_collection(type)) {
						LWCOLLECTION *coll = (LWCOLLECTION*)colk->geoms[i];
						for(j=0; j < coll->ngeoms; j++) {
							lwcollection_add_lwgeom((LWCOLLECTION*)kernel, lwgeom_clone_deep(coll->geoms[j]));
						}
					} else {
						lwcollection_add_lwgeom((LWCOLLECTION*)kernel, lwgeom_clone_deep(colk->geoms[i]));
					}
				}
			}
			lwgeom_free(k);
			k = kernel;
		} 
		if(c->type == COLLECTIONTYPE) {
			LWCOLLECTION *colc = (LWCOLLECTION*)c;
			uint8_t type;
			ngeoms = colc->ngeoms;
			conjecture = (LWGEOM*)lwcollection_construct_empty(get_potgis_type_from_vaguegeom(vasa_type), c->srid, 0, 0);
			for(i=0;i <  ngeoms; i++) {
				type = colc->geoms[i]->type;
				if( ((type == MULTIPOINTTYPE || type == POINTTYPE) && vasa_type == VAGUEMULTIPOINTTYPE) ||
				((type == MULTILINETYPE || type == LINETYPE) && vasa_type == VAGUEMULTILINETYPE) ||
				((type == MULTIPOLYGONTYPE || type == POLYGONTYPE) && vasa_type == VAGUEMULTIPOLYGONTYPE)) {
					if(lwtype_is_collection(type)) {
						LWCOLLECTION *coll = (LWCOLLECTION*)colc->geoms[i];
						for(j=0; j < coll->ngeoms; j++) {
							lwcollection_add_lwgeom((LWCOLLECTION*)conjecture, lwgeom_clone_deep(coll->geoms[j]));
						}
					} else {
						lwcollection_add_lwgeom((LWCOLLECTION*)conjecture, lwgeom_clone_deep(colc->geoms[i]));
					}
				}
			}
			lwgeom_free(c);
			c = conjecture;
		}
	}
	
	if(k->type != c->type) {		
		if((k->type == MULTIPOINTTYPE && c->type == POINTTYPE) ||
			(k->type == MULTILINETYPE && c->type == LINETYPE) ||
			(k->type == MULTIPOLYGONTYPE && c->type == POLYGONTYPE)) {
			conjecture = (LWGEOM*)lwcollection_add_lwgeom(lwcollection_construct_empty(k->type, c->srid, 0, 0), c);
			kernel = k;
		} else if((c->type == MULTIPOINTTYPE && k->type == POINTTYPE) ||
			(c->type == MULTILINETYPE && k->type == LINETYPE) ||
			(c->type == MULTIPOLYGONTYPE && k->type == POLYGONTYPE)) {
			kernel = (LWGEOM*)lwcollection_add_lwgeom(lwcollection_construct_empty(c->type, k->srid, 0, 0), k);
			conjecture = c;
		} 
	}
	
	//checking the compatibility with vasa_type, if is not compatible, then lets construct empties objects
	if( !(((k->type == MULTIPOINTTYPE || k->type == POINTTYPE) && vasa_type == VAGUEMULTIPOINTTYPE) ||
				((k->type == MULTILINETYPE || k->type == LINETYPE) && vasa_type == VAGUEMULTILINETYPE) ||
				((k->type == MULTIPOLYGONTYPE || k->type == POLYGONTYPE) && vasa_type == VAGUEMULTIPOLYGONTYPE))) {
		kernel = lwgeom_construct_empty(get_potgis_type_from_vaguegeom(vasa_type), k->srid, 0, 0);	
	} 
	if( !(((c->type == MULTIPOINTTYPE || c->type == POINTTYPE) && vasa_type == VAGUEMULTIPOINTTYPE) ||
				((c->type == MULTILINETYPE || c->type == LINETYPE) && vasa_type == VAGUEMULTILINETYPE) ||
				((c->type == MULTIPOLYGONTYPE || c->type == POLYGONTYPE) && vasa_type == VAGUEMULTIPOLYGONTYPE))) {
		conjecture = lwgeom_construct_empty(get_potgis_type_from_vaguegeom(vasa_type), c->srid, 0, 0);	
	}
	
	if(kernel==NULL)
		kernel = k;
	if(conjecture==NULL)
		conjecture = c;
	return vaguegeom_construct(kernel, conjecture, punion);
}




/*
* SPATIAL OPERATIONS (UNION, INTERSECTION AND DIFFERENCE)
*/
VAGUEGEOM *vaguegeom_union(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	LWGEOM *kernel, *conjecture;
	GEOSGeometry *geos_kernel, *geos_conjecture, *geos_temp;
	GEOSGeometry *geos_k1, *geos_k2, *geos_c1, *geos_c2;
	//LWGEOM *temp;
	uint8_t punion =0;
	
	if(HAS_PUNION(vgeom1->flags) && HAS_PUNION(vgeom2->flags))
		punion = 1;

	initGEOS(lwnotice, lwgeom_geos_error);

#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
	geos_k1 = LWGEOM2GEOS(vgeom1->kernel, 0);
	geos_c1 = LWGEOM2GEOS(vgeom1->conjecture, 0);
	
	geos_k2 = LWGEOM2GEOS(vgeom2->kernel, 0);
	geos_c2 = LWGEOM2GEOS(vgeom2->conjecture, 0);
#else
	geos_k1 = LWGEOM2GEOS(vgeom1->kernel);
	geos_c1 = LWGEOM2GEOS(vgeom1->conjecture);
	
	geos_k2 = LWGEOM2GEOS(vgeom2->kernel);
	geos_c2 = LWGEOM2GEOS(vgeom2->conjecture);
#endif
	
	geos_kernel = GEOSUnion(geos_k1, geos_k2);
	//kernel = lwgeom_union(vgeom1->kernel, vgeom2->kernel);

	geos_temp = GEOSUnion(geos_c1, geos_c2);
	geos_conjecture = GEOSDifference(geos_temp, geos_kernel);
	//temp = lwgeom_union(vgeom1->conjecture, vgeom2->conjecture);
	//conjecture = lwgeom_difference(temp, kernel);

	conjecture = GEOS2LWGEOM(geos_conjecture, 1);
	conjecture->srid = vgeom1->conjecture->srid;
	kernel = GEOS2LWGEOM(geos_kernel, 1);
	kernel->srid = vgeom1->kernel->srid;
	
	GEOSGeom_destroy(geos_kernel);
	GEOSGeom_destroy(geos_conjecture);
	GEOSGeom_destroy(geos_k1);
	GEOSGeom_destroy(geos_k2);
	GEOSGeom_destroy(geos_c1);
	GEOSGeom_destroy(geos_c2);
	GEOSGeom_destroy(geos_temp);
	
	//lwgeom_free(temp);

	return vague_harmonize_types(kernel, conjecture, punion, extract_super(vgeom1->type));
}

VAGUEGEOM *vaguegeom_intersection(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	LWGEOM *kernel, *conjecture;
	GEOSGeometry *allUnion, *geosTemp1, *geosTemp2, *geosTemp3;
	GEOSGeometry *g1;
	GEOSGeometry **geoms = NULL;
	GEOSGeometry *geos_kernel, *geos_conjecture;
	GEOSGeometry *geos_k1, *geos_k2, *geos_c1, *geos_c2;
	uint8_t punion =0;
	
	if(HAS_PUNION(vgeom1->flags) && HAS_PUNION(vgeom2->flags))
		punion = 1;
		
	initGEOS(lwnotice, lwgeom_geos_error);

#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
	geos_k1 = LWGEOM2GEOS(vgeom1->kernel, 0);
	geos_c1 = LWGEOM2GEOS(vgeom1->conjecture, 0);
	
	geos_k2 = LWGEOM2GEOS(vgeom2->kernel, 0);
	geos_c2 = LWGEOM2GEOS(vgeom2->conjecture, 0);
#else
	geos_k1 = LWGEOM2GEOS(vgeom1->kernel);
	geos_c1 = LWGEOM2GEOS(vgeom1->conjecture);
	
	geos_k2 = LWGEOM2GEOS(vgeom2->kernel);
	geos_c2 = LWGEOM2GEOS(vgeom2->conjecture);
#endif
	
	geoms = (GEOSGeometry**) lwalloc( sizeof(GEOSGeometry *) * 3 );

	geos_kernel = GEOSIntersection(geos_k1, geos_k2);
	//kernel = lwgeom_intersection(vgeom1->kernel, vgeom2->kernel);

	geosTemp1 = GEOSIntersection(geos_c1, geos_c2);
	geosTemp2 = GEOSIntersection(geos_k1, geos_c2);
	geosTemp3 = GEOSIntersection(geos_c1, geos_k2);	
	//temp1 = lwgeom_intersection(vgeom1->conjecture, vgeom2->conjecture);
	//temp2 = lwgeom_intersection(vgeom1->kernel, vgeom2->conjecture);
	//temp3 = lwgeom_intersection(vgeom1->conjecture, vgeom2->kernel);
	
	/* convertions */
	geoms[0] = geosTemp1;
	geoms[1] = geosTemp2;
	geoms[2] = geosTemp3;
	g1 = GEOSGeom_createCollection(GEOS_GEOMETRYCOLLECTION, geoms, 3);
	allUnion = GEOSUnaryUnion(g1);
	GEOSGeom_destroy(g1);
	
	geos_conjecture = GEOSDifference(allUnion, geos_kernel);
	GEOSGeom_destroy(allUnion);

	conjecture = GEOS2LWGEOM(geos_conjecture, 1);
	conjecture->srid = vgeom1->conjecture->srid;
	kernel = GEOS2LWGEOM(geos_kernel, 1);
	kernel->srid = vgeom1->kernel->srid;
	
	GEOSGeom_destroy(geos_kernel);
	GEOSGeom_destroy(geos_conjecture);
	GEOSGeom_destroy(geos_k1);
	GEOSGeom_destroy(geos_k2);
	GEOSGeom_destroy(geos_c1);
	GEOSGeom_destroy(geos_c2);
	
	return vague_harmonize_types(kernel, conjecture, punion, (extract_super(vgeom1->type) < extract_super(vgeom2->type) ? extract_super(vgeom1->type) : extract_super(vgeom2->type)));
}

VAGUEGEOM *vaguegeom_difference(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	LWGEOM *kernel, *conjecture;
	GEOSGeometry *geos_kernel, *geos_conjecture;
	GEOSGeometry *geos_k1, *geos_k2, *geos_c1, *geos_c2;
	GEOSGeometry *temp1, *temp2, *temp3, *union_vg2;
	GEOSGeometry **geoms = NULL;
	GEOSGeometry *g1;
	//LWGEOM *temp1, *temp2, *temp3, *temp4; 
	//LWGEOM *union_vg2;
	uint8_t punion =0;
	
	if(HAS_PUNION(vgeom1->flags) && HAS_PUNION(vgeom2->flags))
		punion = 1;

	initGEOS(lwnotice, lwgeom_geos_error);

	geoms = (GEOSGeometry**) lwalloc( sizeof(GEOSGeometry *) * 3 );
	

#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
	geos_k1 = LWGEOM2GEOS(vgeom1->kernel, 0);
	geos_c1 = LWGEOM2GEOS(vgeom1->conjecture, 0);
	
	geos_c2 = LWGEOM2GEOS(vgeom2->conjecture, 0);
#else
	geos_k1 = LWGEOM2GEOS(vgeom1->kernel);
	geos_c1 = LWGEOM2GEOS(vgeom1->conjecture);
	
	geos_c2 = LWGEOM2GEOS(vgeom2->conjecture);
#endif
	
	if(HAS_PUNION(vgeom2->flags)) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
		union_vg2 = LWGEOM2GEOS(vgeom2->allextension, 0);
#else
		union_vg2 = LWGEOM2GEOS(vgeom2->allextension);
#endif
	} else {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
		geos_k2 = LWGEOM2GEOS(vgeom2->kernel, 0);
#else
		geos_k2 = LWGEOM2GEOS(vgeom2->kernel);
#endif
		union_vg2 = GEOSUnion(geos_k2, geos_c2);
		GEOSGeom_destroy(geos_k2);
	}		
		
	geos_kernel = GEOSDifference(geos_k1, union_vg2);
	//kernel = lwgeom_difference(vgeom1->kernel, union_vg2);

	temp1 = GEOSIntersection(geos_c1, geos_c2);
	temp2 = GEOSIntersection(geos_k1, geos_c2);
	temp3 = GEOSDifference(geos_c1, union_vg2);	
	//temp1 = lwgeom_intersection(vgeom1->conjecture, vgeom2->conjecture);
	//temp2 = lwgeom_intersection(vgeom1->kernel, vgeom2->conjecture);
	//temp3 = lwgeom_difference(vgeom1->conjecture, union_vg2);
		
	/* convertions */
	geoms[0] = temp1;
	geoms[1] = temp2;
	geoms[2] = temp3;
	g1 = GEOSGeom_createCollection(GEOS_GEOMETRYCOLLECTION, geoms, 3);
	geos_conjecture = GEOSUnaryUnion(g1);
	GEOSGeom_destroy(g1);	
	
	conjecture = GEOS2LWGEOM(geos_conjecture, 1);
	conjecture->srid = vgeom1->conjecture->srid;
	kernel = GEOS2LWGEOM(geos_kernel, 1);
	kernel->srid = vgeom1->kernel->srid;	

	GEOSGeom_destroy(geos_kernel);
	GEOSGeom_destroy(geos_conjecture);
	GEOSGeom_destroy(geos_k1);
	GEOSGeom_destroy(geos_c1);
	GEOSGeom_destroy(geos_c2);
	return vague_harmonize_types(kernel, conjecture, punion, extract_super(vgeom1->type));
}

/*
*  GENERIC OPERATIONS 
*/

VAGUEGEOM *vaguegeom_kernel(const VAGUEGEOM *vgeom) {
	if(HAS_PUNION(vgeom->flags))
		return vaguegeom_construct(vgeom->kernel, NULL, 1);
	else
		return vaguegeom_construct(vgeom->kernel, NULL, 0);
}

VAGUEGEOM *vaguegeom_conjecture(const VAGUEGEOM *vgeom) {
	if(HAS_PUNION(vgeom->flags))
		return vaguegeom_construct(NULL, vgeom->conjecture, 1);
	else
		return vaguegeom_construct(NULL, vgeom->conjecture, 0);
}

VAGUEGEOM *vaguegeom_invert(const VAGUEGEOM *vgeom) {
	uint8_t punion = 0;
	if(HAS_PUNION(vgeom->flags))
		punion = 1;
	return vaguegeom_construct(lwgeom_clone_deep(vgeom->conjecture), lwgeom_clone_deep(vgeom->kernel), punion);
}

LWGEOM *vaguegeom_kernel_projection(const VAGUEGEOM *vgeom) {
	return vgeom->kernel;
}

LWGEOM *vaguegeom_conjecture_projection(const VAGUEGEOM *vgeom) {
	return vgeom->conjecture;
}

uint8_t vaguegeom_same(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	if(lwgeom_same(vgeom1->kernel, vgeom2->kernel)==LW_FALSE)
		return LW_FALSE;
	if(lwgeom_same(vgeom1->conjecture, vgeom2->conjecture)==LW_FALSE)
		return LW_FALSE;
	return LW_TRUE;	
}

/*
* Type-dependent vague spatial operations
*/

static LWGEOM *extract_vertices_from_line(LWLINE *line) {	
	LWMPOINT *res = lwmpoint_construct_empty(line->srid, 0, 0);
	if(line->points) {
		if(lwline_is_closed(line)) {
			LWPOINT *end;		
			end = lwline_get_lwpoint(line, 0);
			lwmpoint_add_lwpoint(res, end);
		} else {
			LWPOINT *end1, *end2;		
			end1 = lwline_get_lwpoint(line, line->points->npoints-1);
			end2 = lwline_get_lwpoint(line, 0);
			lwmpoint_add_lwpoint(res, end1);
			lwmpoint_add_lwpoint(res, end2);
		}
	}
	return (LWGEOM*) res;
}

static LWGEOM *extract_vertices_from_multiline(LWMLINE *mline) {	
	LWMPOINT *res = lwmpoint_construct_empty(mline->srid, 0, 0);
	LWGEOM *ret;
	LWLINE *line;
	LWPOINT *end, *end2;
	int i;

	for(i=0; i < mline->ngeoms;i++) {
		line = mline->geoms[i];
		if(line->points) {
			end = lwline_get_lwpoint(line, line->points->npoints-1);
			lwmpoint_add_lwpoint(res, end);
			if(!lwline_is_closed(line)) {
				end2 = lwline_get_lwpoint(line, 0);				
				lwmpoint_add_lwpoint(res, end2);
			}
		}
	}
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
	ret = lwmpoint_remove_repeated_points(res, 0);
#else
	ret = lwmpoint_remove_repeated_points(res);
#endif
	lwfree(res); /* note: only frees the wrapper, not the content */
	return ret;
}

/* kernel and conjectures versions for vertices of VAGUELINES and VAGUEREGIONS */
VAGUEGEOM *vaguegeom_kernel_vertices(const VAGUEGEOM *vgeom) {
	LWGEOM *kernel=NULL, *conjecture=NULL;
	uint8_t punion = 0;
	if(HAS_PUNION(vgeom->flags))
		punion = 1;
	assert(vgeom->type == VAGUELINETYPE || vgeom->type == VAGUEPOLYGONTYPE ||
		vgeom->type == VAGUEMULTILINETYPE || vgeom->type == VAGUEMULTIPOLYGONTYPE);
	
	switch (vgeom->type)
	{
	case VAGUELINETYPE:
		if(HAS_KERNEL(vgeom->flags)) {
			kernel = extract_vertices_from_line((LWLINE*)vgeom->kernel);
		} else {
			kernel = lwgeom_construct_empty(MULTIPOINTTYPE, vgeom->kernel->srid, FLAGS_GET_Z(vgeom->kernel->flags), FLAGS_GET_M(vgeom->kernel->flags));
		}
		if(HAS_CONJECTURE(vgeom->flags)) {
			LWGEOM *aux;
			aux = extract_vertices_from_line((LWLINE*)vgeom->conjecture);
			if(HAS_KERNEL(vgeom->flags)) {
				conjecture = lwgeom_difference(aux, kernel);
				lwgeom_free(aux);				
			} else {
				conjecture = aux;
			}	
		} else {
			conjecture = lwgeom_construct_empty(MULTIPOINTTYPE, vgeom->conjecture->srid, FLAGS_GET_Z(vgeom->conjecture->flags), FLAGS_GET_M(vgeom->conjecture->flags));
		}
		return vague_harmonize_types(kernel, conjecture, punion, VAGUEMULTIPOINTTYPE);
	case VAGUEMULTILINETYPE:
		if(HAS_KERNEL(vgeom->flags)) {
			kernel = extract_vertices_from_multiline((LWMLINE*)vgeom->kernel);
		} else {
			kernel = lwgeom_construct_empty(MULTIPOINTTYPE, vgeom->kernel->srid, FLAGS_GET_Z(vgeom->kernel->flags), FLAGS_GET_M(vgeom->kernel->flags));
		}
		if(HAS_CONJECTURE(vgeom->flags)) {
			LWGEOM *aux = extract_vertices_from_multiline((LWMLINE*)vgeom->conjecture);			
			if(HAS_KERNEL(vgeom->flags)) {
				conjecture = lwgeom_difference(aux, kernel);
				lwgeom_free(aux);
			} else {
				conjecture = aux;
			}			
		} else {
			conjecture = lwgeom_construct_empty(MULTIPOINTTYPE, vgeom->conjecture->srid, FLAGS_GET_Z(vgeom->conjecture->flags), FLAGS_GET_M(vgeom->conjecture->flags));
		}
		return vague_harmonize_types(kernel, conjecture, punion, VAGUEMULTIPOINTTYPE);
	case VAGUEPOLYGONTYPE:
	case VAGUEMULTIPOLYGONTYPE: {
		VAGUEGEOM *kboundary, *ret;
		kboundary = vaguegeom_kernel_boundary(vgeom);
		ret = vaguegeom_kernel_vertices(kboundary);
		vaguegeom_free(kboundary);
		return ret;
	}
	default:
		return NULL;
	}
}

VAGUEGEOM *vaguegeom_conjecture_vertices(const VAGUEGEOM *vgeom) {
	LWGEOM *kernel=NULL, *conjecture=NULL;
	uint8_t punion = 0;
	if(HAS_PUNION(vgeom->flags))
		punion = 1;
	assert(vgeom->type == VAGUELINETYPE || vgeom->type == VAGUEPOLYGONTYPE ||
		vgeom->type == VAGUEMULTILINETYPE || vgeom->type == VAGUEMULTIPOLYGONTYPE);
	
	switch (vgeom->type)
	{
	case VAGUELINETYPE:
		if(HAS_CONJECTURE(vgeom->flags)) {
			conjecture = extract_vertices_from_line((LWLINE*)vgeom->conjecture);
		} else {
			conjecture = lwgeom_construct_empty(MULTIPOINTTYPE, vgeom->conjecture->srid, FLAGS_GET_Z(vgeom->conjecture->flags), FLAGS_GET_M(vgeom->conjecture->flags));
		}
		if(HAS_KERNEL(vgeom->flags)) {
			LWGEOM *aux;
			aux = extract_vertices_from_line((LWLINE*)vgeom->kernel);
			if(HAS_CONJECTURE(vgeom->flags)) {
				kernel = lwgeom_difference(aux, conjecture);
				lwgeom_free(aux);				
			} else {
				kernel = aux;
			}	
		} else {
			kernel = lwgeom_construct_empty(MULTIPOINTTYPE, vgeom->kernel->srid, FLAGS_GET_Z(vgeom->kernel->flags), FLAGS_GET_M(vgeom->kernel->flags));
		}
		return vague_harmonize_types(kernel, conjecture, punion, VAGUEMULTIPOINTTYPE);
	case VAGUEMULTILINETYPE:
		if(HAS_CONJECTURE(vgeom->flags)) {
			conjecture = extract_vertices_from_multiline((LWMLINE*)vgeom->conjecture);
		} else {
			conjecture = lwgeom_construct_empty(MULTIPOINTTYPE, vgeom->conjecture->srid, FLAGS_GET_Z(vgeom->conjecture->flags), FLAGS_GET_M(vgeom->conjecture->flags));
		}
		if(HAS_KERNEL(vgeom->flags)) {
			LWGEOM *aux = extract_vertices_from_multiline((LWMLINE*)vgeom->kernel);			
			if(HAS_CONJECTURE(vgeom->flags)) {
				kernel = lwgeom_difference(aux, conjecture);
				lwgeom_free(aux);
			} else {
				kernel = aux;
			}			
		} else {
			kernel = lwgeom_construct_empty(MULTIPOINTTYPE, vgeom->kernel->srid, FLAGS_GET_Z(vgeom->kernel->flags), FLAGS_GET_M(vgeom->kernel->flags));
		}
		return vague_harmonize_types(kernel, conjecture, punion, VAGUEMULTIPOINTTYPE);
	case VAGUEPOLYGONTYPE:
	case VAGUEMULTIPOLYGONTYPE: {
		VAGUEGEOM *cboundary, *ret;
		cboundary = vaguegeom_conjecture_boundary(vgeom);
		ret = vaguegeom_conjecture_vertices(cboundary);
		vaguegeom_free(cboundary);
		return ret;
	}
	default:
		return NULL;
	}
}

/* kernel and conjectures versions for boundary of VAGUEREGIONS only */
VAGUEGEOM *vaguegeom_kernel_boundary(const VAGUEGEOM *vgeom) {
	LWGEOM *kernel=NULL, *conjecture=NULL;
	uint8_t punion = 0;
	if(HAS_PUNION(vgeom->flags))
		punion = 1;
	assert(vgeom->type==VAGUEPOLYGONTYPE || vgeom->type==VAGUEMULTIPOLYGONTYPE);

	if(HAS_KERNEL(vgeom->flags))
		kernel = boundary(vgeom->kernel);
	else
		kernel = lwgeom_construct_empty(MULTILINETYPE, vgeom->kernel->srid, FLAGS_GET_Z(vgeom->kernel->flags), FLAGS_GET_M(vgeom->kernel->flags));

	if(HAS_CONJECTURE(vgeom->flags)) {
		if(kernel == NULL || lwgeom_is_empty(kernel)==LW_TRUE)
			conjecture = boundary(vgeom->conjecture);
		else {
			LWGEOM *temp1;
			temp1 = boundary(vgeom->conjecture);
			conjecture = lwgeom_difference(temp1, kernel);
			lwgeom_free(temp1);
		}
	} else {
		conjecture = lwgeom_construct_empty(MULTILINETYPE, vgeom->conjecture->srid, FLAGS_GET_Z(vgeom->conjecture->flags), FLAGS_GET_M(vgeom->conjecture->flags));
	}
	return vaguegeom_construct(kernel, conjecture, punion);
}

VAGUEGEOM *vaguegeom_conjecture_boundary(const VAGUEGEOM *vgeom) {
	LWGEOM *kernel=NULL, *conjecture=NULL;
	uint8_t punion = 0;
	if(HAS_PUNION(vgeom->flags))
		punion = 1;
	assert(vgeom->type==VAGUEPOLYGONTYPE || vgeom->type==VAGUEMULTIPOLYGONTYPE);

	if(HAS_CONJECTURE(vgeom->flags)) {
		conjecture = boundary(vgeom->conjecture);
	} else {
		conjecture = lwgeom_construct_empty(MULTILINETYPE, vgeom->conjecture->srid, FLAGS_GET_Z(vgeom->conjecture->flags), FLAGS_GET_M(vgeom->conjecture->flags));
	}
	if(HAS_KERNEL(vgeom->flags)) {
		if(conjecture == NULL || lwgeom_is_empty(conjecture)==LW_TRUE)
			kernel = boundary(vgeom->kernel);
		else {
			LWGEOM *temp1;
			temp1 = boundary(vgeom->kernel);
			kernel = lwgeom_difference(temp1, conjecture);
			lwgeom_free(temp1);
		}
	} else {
		kernel = lwgeom_construct_empty(MULTILINETYPE, vgeom->kernel->srid, FLAGS_GET_Z(vgeom->kernel->flags), FLAGS_GET_M(vgeom->kernel->flags));
	}
	return vaguegeom_construct(kernel, conjecture, punion);
}

/*kernel and conjectures versions for interior of VAGUELINES only */
VAGUEGEOM *vaguegeom_kernel_interior(const VAGUEGEOM *vgeom) {
	LWGEOM *kernel=NULL, *conjecture=NULL;
	uint8_t punion = 0;
	if(HAS_PUNION(vgeom->flags))
		punion = 1;
	assert(vgeom->type==VAGUELINETYPE || vgeom->type==VAGUEMULTILINETYPE);
	switch (vgeom->type)
	{
	case VAGUELINETYPE:
		if(HAS_KERNEL(vgeom->flags)) {
			if(lwline_is_closed((LWLINE*)vgeom->kernel))
				kernel = (LWGEOM*)lwpoly_from_lwlines((LWLINE*)vgeom->kernel, 0, NULL);			
		}
		if(HAS_CONJECTURE(vgeom->flags)) {
			if(lwline_is_closed((LWLINE*)vgeom->conjecture)) {				
				if(HAS_KERNEL(vgeom->flags) && kernel != NULL) {
					LWGEOM *temp1;
					temp1 = (LWGEOM*)lwpoly_from_lwlines((LWLINE*)vgeom->conjecture, 0, NULL);
					conjecture = lwgeom_difference(temp1, kernel);
					lwgeom_free(temp1);
				} else {
					conjecture = (LWGEOM*)lwpoly_from_lwlines((LWLINE*)vgeom->conjecture, 0, NULL);	
				}
			}
		}
		break;
	case VAGUEMULTILINETYPE: {
		GEOSGeometry *geos_kernel=NULL; 
		if(HAS_KERNEL(vgeom->flags)) {
			int i;
			LWMLINE *p = (LWMLINE*)vgeom->kernel;
			GEOSGeometry **geoms = (GEOSGeometry**)lwalloc(sizeof(GEOSGeometry*)*p->ngeoms);
			int curgeom=0;
			GEOSGeometry *g;
			LWGEOM *aux;
			
			initGEOS(lwnotice, lwgeom_geos_error);
			
			for(i=0; i < p->ngeoms; i++) {
				if(lwline_is_closed((LWLINE*)p->geoms[i])) {			
					aux = (LWGEOM*)lwpoly_from_lwlines((LWLINE*)p->geoms[i], 0, NULL);
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
					geoms[curgeom] = LWGEOM2GEOS(aux, 0);
#else
					geoms[curgeom] = LWGEOM2GEOS(aux);
#endif
					curgeom++;
					lwgeom_free(aux);
				}
			}
			g = GEOSGeom_createCollection(GEOS_GEOMETRYCOLLECTION, geoms, curgeom);
			geos_kernel = GEOSUnaryUnion(g);
			GEOSGeom_destroy(g);
			
			kernel = GEOS2LWGEOM(geos_kernel, 1);
		}
		if(HAS_CONJECTURE(vgeom->flags)) {
			int i;
			LWMLINE *p = (LWMLINE*)vgeom->conjecture;
			GEOSGeometry **geoms = (GEOSGeometry**)lwalloc(sizeof(GEOSGeometry*)*p->ngeoms);
			int curgeom=0;
			GEOSGeometry *g, *geos_temp;
			LWGEOM *aux;
			
			initGEOS(lwnotice, lwgeom_geos_error);
			
			for(i=0; i < p->ngeoms; i++) {
				if(lwline_is_closed((LWLINE*)p->geoms[i])) {			
					aux = (LWGEOM*)lwpoly_from_lwlines((LWLINE*)p->geoms[i], 0, NULL);
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
					geoms[curgeom] = LWGEOM2GEOS(aux, 0);
#else
					geoms[curgeom] = LWGEOM2GEOS(aux);
#endif
					curgeom++;
					lwgeom_free(aux);
				}
			}
			g = GEOSGeom_createCollection(GEOS_GEOMETRYCOLLECTION, geoms, curgeom);
			geos_temp = GEOSUnaryUnion(g);
			GEOSGeom_destroy(g);

			if(HAS_KERNEL(vgeom->flags)) {
				if(!GEOSisEmpty(geos_temp) && kernel != NULL) {
					GEOSGeometry *dif;
					dif = GEOSDifference(geos_temp, geos_kernel);
					conjecture = GEOS2LWGEOM(dif, 1);
					GEOSGeom_destroy(dif);
				}
			} else {
				conjecture = GEOS2LWGEOM(geos_temp, 1);
			}
			GEOSGeom_destroy(geos_temp);
		}
		if(HAS_KERNEL(vgeom->flags))
			GEOSGeom_destroy(geos_kernel);
		break;
		}
	}
	if(kernel==NULL)
		kernel = lwgeom_construct_empty(POLYGONTYPE, vgeom->kernel->srid, 0, 0);
	if(conjecture==NULL)
		conjecture = lwgeom_construct_empty(POLYGONTYPE, vgeom->conjecture->srid, 0, 0);
	return vague_harmonize_types(kernel, conjecture, punion, VAGUEMULTIPOLYGONTYPE);
}

VAGUEGEOM *vaguegeom_conjecture_interior(const VAGUEGEOM *vgeom) {
	LWGEOM *kernel=NULL, *conjecture=NULL;
	uint8_t punion = 0;
	if(HAS_PUNION(vgeom->flags))
		punion = 1;
	assert(vgeom->type==VAGUELINETYPE || vgeom->type==VAGUEMULTILINETYPE);
	switch (vgeom->type)
	{
	case VAGUELINETYPE:		
		if(HAS_CONJECTURE(vgeom->flags)) {
			if(lwline_is_closed((LWLINE*)vgeom->conjecture)) {
				conjecture = (LWGEOM*)lwpoly_from_lwlines((LWLINE*)vgeom->conjecture, 0, NULL);					
			}
		}
		if(HAS_KERNEL(vgeom->flags)) {
			if(lwline_is_closed((LWLINE*)vgeom->kernel)) {
				if(HAS_CONJECTURE(vgeom->flags) && conjecture != NULL) {
					LWGEOM *temp;
					temp = (LWGEOM*)lwpoly_from_lwlines((LWLINE*)vgeom->kernel, 0, NULL);
					kernel = lwgeom_difference(temp, conjecture);
					lwgeom_free(temp);
				}
				else
					kernel = (LWGEOM*)lwpoly_from_lwlines((LWLINE*)vgeom->kernel, 0, NULL);	
			}
		}
		break;
	case VAGUEMULTILINETYPE: {
		GEOSGeometry *geos_conjecture=NULL; 
		if(HAS_CONJECTURE(vgeom->flags)) {
			int i;
			LWMLINE *p = (LWMLINE*)vgeom->conjecture;
			GEOSGeometry **geoms = (GEOSGeometry**)lwalloc(sizeof(GEOSGeometry*)*p->ngeoms);
			int curgeom=0;
			GEOSGeometry *g;
			LWGEOM *aux;
			
			initGEOS(lwnotice, lwgeom_geos_error);
			
			for(i=0; i < p->ngeoms; i++) {
				if(lwline_is_closed((LWLINE*)p->geoms[i])) {			
					aux = (LWGEOM*)lwpoly_from_lwlines((LWLINE*)p->geoms[i], 0, NULL);
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
					geoms[curgeom] = LWGEOM2GEOS(aux, 0);
#else
					geoms[curgeom] = LWGEOM2GEOS(aux);
#endif

					curgeom++;
					lwgeom_free(aux);
				}
			}
			g = GEOSGeom_createCollection(GEOS_GEOMETRYCOLLECTION, geoms, curgeom);
			geos_conjecture = GEOSUnaryUnion(g);
			GEOSGeom_destroy(g);
			
			conjecture = GEOS2LWGEOM(geos_conjecture, 1);
		}
		if(HAS_KERNEL(vgeom->flags)) {
			int i;
			LWMLINE *p = (LWMLINE*)vgeom->kernel;
			GEOSGeometry **geoms = (GEOSGeometry**)lwalloc(sizeof(GEOSGeometry*)*p->ngeoms);
			int curgeom=0;
			GEOSGeometry *g, *geos_temp;
			LWGEOM *aux;
			
			initGEOS(lwnotice, lwgeom_geos_error);
			
			for(i=0; i < p->ngeoms; i++) {
				if(lwline_is_closed((LWLINE*)p->geoms[i])) {			
					aux = (LWGEOM*)lwpoly_from_lwlines((LWLINE*)p->geoms[i], 0, NULL);
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
					geoms[curgeom] = LWGEOM2GEOS(aux, 0);
#else
					geoms[curgeom] = LWGEOM2GEOS(aux);
#endif
					curgeom++;
					lwgeom_free(aux);
				}
			}
			g = GEOSGeom_createCollection(GEOS_GEOMETRYCOLLECTION, geoms, curgeom);
			geos_temp = GEOSUnaryUnion(g);
			GEOSGeom_destroy(g);

			if(HAS_CONJECTURE(vgeom->flags)) {
				if(!GEOSisEmpty(geos_temp) && conjecture != NULL) {
					GEOSGeometry *dif;
					dif = GEOSDifference(geos_temp, geos_conjecture);
					kernel = GEOS2LWGEOM(dif, 1);
					GEOSGeom_destroy(dif);
				}
			} else {
				kernel = GEOS2LWGEOM(geos_temp, 1);
			}
			GEOSGeom_destroy(geos_temp);
		}
		if(HAS_CONJECTURE(vgeom->flags))
			GEOSGeom_destroy(geos_conjecture);
		break;
		}
	}
	if(kernel==NULL)
		kernel = lwgeom_construct_empty(POLYGONTYPE, vgeom->kernel->srid, 0, 0);
	if(conjecture==NULL)
		conjecture = lwgeom_construct_empty(POLYGONTYPE, vgeom->conjecture->srid, 0, 0);
	return vague_harmonize_types(kernel, conjecture, punion, VAGUEMULTIPOLYGONTYPE);
}

/*kernel and conjectures versions for convex-hull of VAGUEPOINTS only */
VAGUEGEOM *vaguegeom_kernel_convexhull(const VAGUEGEOM *vgeom) {
	LWGEOM *kernel=NULL, *conjecture=NULL;
	GEOSGeometry *geos_kernel, *geos_cv_kernel=NULL;
	uint8_t punion = 0;
	
	if(HAS_PUNION(vgeom->flags))
		punion = 1;
	
	assert(vgeom->type==VAGUEPOINTTYPE||vgeom->type==VAGUEMULTIPOINTTYPE);
	
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
	geos_kernel = LWGEOM2GEOS(vgeom->kernel, 0);
#else
	geos_kernel = LWGEOM2GEOS(vgeom->kernel);
#endif

	if(HAS_KERNEL(vgeom->flags)) {
		geos_cv_kernel = (GEOSGeometry *)GEOSConvexHull(geos_kernel);
		kernel = GEOS2LWGEOM(geos_cv_kernel, 1);
	} else {
		kernel = lwgeom_construct_empty(POLYGONTYPE, vgeom->kernel->srid, 0, 0);
	}
	if(HAS_CONJECTURE(vgeom->flags)) {
		GEOSGeometry *u, *temp;
		if(HAS_PUNION(vgeom->flags)){

#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			u = LWGEOM2GEOS(vgeom->allextension, 0);
#else
			u = LWGEOM2GEOS(vgeom->allextension);
#endif

		} else {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			GEOSGeometry *conj = LWGEOM2GEOS(vgeom->conjecture, 0);
#else
			GEOSGeometry *conj = LWGEOM2GEOS(vgeom->conjecture);
#endif
			u = GEOSUnion(geos_kernel, conj);
			GEOSGeom_destroy(conj);
		}
		temp = (GEOSGeometry *)GEOSConvexHull(u);
		GEOSGeom_destroy(u);
		
		if(HAS_KERNEL(vgeom->flags)) {
			GEOSGeometry *dif = GEOSDifference(temp, geos_cv_kernel);
			conjecture = GEOS2LWGEOM(dif, 1);
			
			GEOSGeom_destroy(dif);
		} else {
			conjecture = GEOS2LWGEOM(temp, 1);
		}
		GEOSGeom_destroy(temp);
	} else {
		conjecture = lwgeom_construct_empty(POLYGONTYPE, vgeom->conjecture->srid, 0, 0);
	}
	
	GEOSGeom_destroy(geos_kernel);
	if(HAS_KERNEL(vgeom->flags))
		GEOSGeom_destroy(geos_cv_kernel);
		
	return vague_harmonize_types(kernel, conjecture, punion, VAGUEMULTIPOLYGONTYPE);
}

VAGUEGEOM *vaguegeom_conjecture_convexhull(const VAGUEGEOM *vgeom) {
	LWGEOM *kernel=NULL, *conjecture=NULL;
	GEOSGeometry *geos_conjecture, *geos_cv_conjecture=NULL;
	uint8_t punion = 0;
	
	if(HAS_PUNION(vgeom->flags))
		punion = 1;
	
	assert(vgeom->type==VAGUEPOINTTYPE||vgeom->type==VAGUEMULTIPOINTTYPE);
	
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
	geos_conjecture = LWGEOM2GEOS(vgeom->conjecture, 0);
#else
	geos_conjecture = LWGEOM2GEOS(vgeom->conjecture);
#endif
	
	if(HAS_CONJECTURE(vgeom->flags)) {
		geos_cv_conjecture = (GEOSGeometry *)GEOSConvexHull(geos_conjecture);
		conjecture = GEOS2LWGEOM(geos_cv_conjecture, 1);
	} else {
		conjecture = lwgeom_construct_empty(POLYGONTYPE, vgeom->conjecture->srid, 0, 0);
	}
		
	if(HAS_KERNEL(vgeom->flags)) {
		GEOSGeometry *u, *temp;
		if(HAS_PUNION(vgeom->flags)) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			u = LWGEOM2GEOS(vgeom->allextension, 0);
#else
			u = LWGEOM2GEOS(vgeom->allextension);
#endif
		} else {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			GEOSGeometry *kern = LWGEOM2GEOS(vgeom->kernel, 0);
#else
			GEOSGeometry *kern = LWGEOM2GEOS(vgeom->kernel);
#endif
			u = GEOSUnion(kern, geos_conjecture);
			GEOSGeom_destroy(kern);
		}
		temp = (GEOSGeometry *)GEOSConvexHull(u);
		GEOSGeom_destroy(u);
		
		if(HAS_CONJECTURE(vgeom->flags)) {
			GEOSGeometry *dif = GEOSDifference(temp, geos_cv_conjecture);
			kernel = GEOS2LWGEOM(dif, 1);
			
			GEOSGeom_destroy(dif);
		} else {
			kernel = GEOS2LWGEOM(temp, 1);
		}
		GEOSGeom_destroy(temp);
	} else {
		kernel = lwgeom_construct_empty(POLYGONTYPE, vgeom->kernel->srid, 0, 0);
	}
	
	GEOSGeom_destroy(geos_conjecture);
	if(HAS_CONJECTURE(vgeom->flags))
		GEOSGeom_destroy(geos_cv_conjecture);
		
	return vague_harmonize_types(kernel, conjecture, punion, VAGUEMULTIPOLYGONTYPE);
}

/*common points functions (for intersections)*/

/* vagueline, vagueline */
VAGUEGEOM *vaguegeom_common_points(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	LWGEOM *kernel, *conjecture;
	GEOSGeometry *allUnion, *geosTemp1, *geosTemp2, *geosTemp3;
	GEOSGeometry *g1;
	GEOSGeometry **geoms = NULL;
	GEOSGeometry *geos_kernel, *geos_conjecture;
	GEOSGeometry *geos_k1, *geos_k2, *geos_c1, *geos_c2;
	uint8_t punion =0;
	
	if(HAS_PUNION(vgeom1->flags) && HAS_PUNION(vgeom2->flags))
		punion = 1;
		
	initGEOS(lwnotice, lwgeom_geos_error);

#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
	geos_k1 = LWGEOM2GEOS(vgeom1->kernel, 0);
	geos_c1 = LWGEOM2GEOS(vgeom1->conjecture, 0);
	
	geos_k2 = LWGEOM2GEOS(vgeom2->kernel, 0);
	geos_c2 = LWGEOM2GEOS(vgeom2->conjecture, 0);
#else
	geos_k1 = LWGEOM2GEOS(vgeom1->kernel);
	geos_c1 = LWGEOM2GEOS(vgeom1->conjecture);
	
	geos_k2 = LWGEOM2GEOS(vgeom2->kernel);
	geos_c2 = LWGEOM2GEOS(vgeom2->conjecture);
#endif	
	
	geoms = (GEOSGeometry**) lwalloc( sizeof(GEOSGeometry *) * 3 );

	geos_kernel = GEOSIntersection(geos_k1, geos_k2);
	//kernel = lwgeom_intersection(vgeom1->kernel, vgeom2->kernel);

	geosTemp1 = GEOSIntersection(geos_c1, geos_c2);
	geosTemp2 = GEOSIntersection(geos_k1, geos_c2);
	geosTemp3 = GEOSIntersection(geos_c1, geos_k2);	
	//temp1 = lwgeom_intersection(vgeom1->conjecture, vgeom2->conjecture);
	//temp2 = lwgeom_intersection(vgeom1->kernel, vgeom2->conjecture);
	//temp3 = lwgeom_intersection(vgeom1->conjecture, vgeom2->kernel);
	
	/* convertions */
	geoms[0] = geosTemp1;
	geoms[1] = geosTemp2;
	geoms[2] = geosTemp3;
	g1 = GEOSGeom_createCollection(GEOS_GEOMETRYCOLLECTION, geoms, 3);
	allUnion = GEOSUnaryUnion(g1);
	GEOSGeom_destroy(g1);
	
	geos_conjecture = GEOSDifference(allUnion, geos_kernel);
	GEOSGeom_destroy(allUnion);

	conjecture = GEOS2LWGEOM(geos_conjecture, 1);
	conjecture->srid = vgeom1->conjecture->srid;
	kernel = GEOS2LWGEOM(geos_kernel, 1);
	kernel->srid = vgeom1->kernel->srid;
	
	GEOSGeom_destroy(geos_kernel);
	GEOSGeom_destroy(geos_conjecture);
	GEOSGeom_destroy(geos_k1);
	GEOSGeom_destroy(geos_k2);
	GEOSGeom_destroy(geos_c1);
	GEOSGeom_destroy(geos_c2);
	
	return vague_harmonize_types(kernel, conjecture, punion, VAGUEMULTIPOINTTYPE);
}

/*common border (vagueline, vagueregion) or (vagueregion, vagueregion) */
VAGUEGEOM *vaguegeom_common_border(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	VAGUEGEOM *result=NULL;		
	assert((vgeom1->type==VAGUELINETYPE||vgeom1->type==VAGUEMULTILINETYPE||vgeom1->type==VAGUEPOLYGONTYPE||vgeom1->type==VAGUEMULTIPOLYGONTYPE)
		&&(vgeom2->type==VAGUELINETYPE||vgeom2->type==VAGUEMULTILINETYPE||vgeom2->type==VAGUEPOLYGONTYPE||vgeom2->type==VAGUEMULTIPOLYGONTYPE));

	if(vgeom1->type == VAGUELINETYPE || vgeom1->type == VAGUEMULTILINETYPE) {
		if(vgeom2->type == VAGUEPOLYGONTYPE || vgeom2->type == VAGUEMULTIPOLYGONTYPE) {
			VAGUEGEOM *bound;
			bound = vaguegeom_kernel_boundary(vgeom2);
			result = vaguegeom_intersection(vgeom1, bound);
			vaguegeom_free(bound);
		} else { //then the common border is a simple intersection between vague lines
			result = vaguegeom_intersection(vgeom1, vgeom2);
		}
	} else { //then is can be vague polygon or multipolygon
		/* vague polygon - vague line */
		if(vgeom2->type == VAGUELINETYPE || vgeom2->type == VAGUEMULTILINETYPE) {
			VAGUEGEOM *bound;
			bound = vaguegeom_kernel_boundary(vgeom1);
			result = vaguegeom_intersection(vgeom2, bound);
			vaguegeom_free(bound);
		} else { /* vague polygon - vague POLYGON */
			VAGUEGEOM *bound1, *bound2;
			bound1 = vaguegeom_kernel_boundary(vgeom1);
			bound2 = vaguegeom_kernel_boundary(vgeom2);
			result = vaguegeom_intersection(bound1, bound2);
			vaguegeom_free(bound1);
			vaguegeom_free(bound2);
		}
	}

	return result;
}

/*
* VAGUE NUMERIC OPERATIONS
*/

/*length of vague line */
double vaguegeom_min_length(const VAGUEGEOM *vgeom) {
	assert(vgeom->type == VAGUELINETYPE || vgeom->type == VAGUEMULTILINETYPE);
	return lwgeom_length_2d(vgeom->kernel);
}

/*max length of vague line */
double vaguegeom_max_length(const VAGUEGEOM *vgeom) {
	double result=0;
	assert(vgeom->type == VAGUELINETYPE || vgeom->type == VAGUEMULTILINETYPE);
	if(HAS_PUNION(vgeom->flags)) {
		result = lwgeom_length_2d(vgeom->allextension);
	} else {
		LWGEOM *union1;
		union1 = lwgeom_union(vgeom->kernel, vgeom->conjecture);
		result = lwgeom_length_2d(union1);
		lwgeom_free(union1);
	}
	return result;
}

VAGUENUMERIC *vaguegeom_length(const VAGUEGEOM *vgeom) {
	VAGUENUMERIC *result;
	assert(vgeom->type == VAGUELINETYPE || vgeom->type == VAGUEMULTILINETYPE);
	result = (VAGUENUMERIC*)lwalloc(sizeof(VAGUENUMERIC));
	result->max = vaguegeom_max_length(vgeom);
	result->min = vaguegeom_min_length(vgeom);
	return result;
}

/*area of vague region */
double vaguegeom_min_area(const VAGUEGEOM *vgeom) {
	assert(vgeom->type == VAGUEPOLYGONTYPE || vgeom->type == VAGUEMULTIPOLYGONTYPE);
	return lwgeom_area(vgeom->kernel);
}

/*max area of vague region */
double vaguegeom_max_area(const VAGUEGEOM *vgeom) {
	assert(vgeom->type == VAGUEPOLYGONTYPE || vgeom->type == VAGUEMULTIPOLYGONTYPE);
	if(HAS_PUNION(vgeom->flags)) {
		return lwgeom_area(vgeom->allextension);
	} else {
		return lwgeom_area(vgeom->kernel) + lwgeom_area(vgeom->conjecture);
	}	
}

VAGUENUMERIC *vaguegeom_area(const VAGUEGEOM *vgeom) {
	VAGUENUMERIC *result;
	assert(vgeom->type == VAGUEPOLYGONTYPE || vgeom->type == VAGUEMULTIPOLYGONTYPE);
	result = (VAGUENUMERIC*)lwalloc(sizeof(VAGUENUMERIC));
	result->max = vaguegeom_max_area(vgeom);
	result->min = vaguegeom_min_area(vgeom);
	return result;
}

/*diameter of vague region, line and point */
double vaguegeom_min_diameter(const VAGUEGEOM *vgeom) {
	return 0.0;//to-do
}

double vaguegeom_max_diameter(const VAGUEGEOM *vgeom) {
	return 0.0;//to-do
}

/*number of components of vague point */
double vaguegeom_min_ncomp(const VAGUEGEOM *vgeom) {
	if(HAS_KERNEL(vgeom->flags)) {
		if(lwgeom_is_collection(vgeom->kernel))
			return ((LWCOLLECTION*)vgeom->kernel)->ngeoms;
		return 1.0;
	}
	return 0.0;
}

double vaguegeom_max_ncomp(const VAGUEGEOM *vgeom) {
	double aux=0.0;
	if(HAS_CONJECTURE(vgeom->flags)) {
		if(lwgeom_is_collection(vgeom->conjecture))
			aux = ((LWCOLLECTION*)vgeom->conjecture)->ngeoms;
		aux = 1.0;
	}
	return aux + vaguegeom_min_ncomp(vgeom);
}

VAGUENUMERIC *vaguegeom_ncomp(const VAGUEGEOM *vgeom) {
	VAGUENUMERIC *result;
	result = (VAGUENUMERIC*)lwalloc(sizeof(VAGUENUMERIC));
	result->max = vaguegeom_max_ncomp(vgeom);
	result->min = vaguegeom_min_ncomp(vgeom);
	return result;
}

/*
* VAGUE NUMERIC OPERATIONS (DISTANCE)
*/

/*nearest distance of vague geoms */
double vaguegeom_nearest_min_distance(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	double result;
		
	if(HAS_PUNION(vgeom1->flags) && HAS_PUNION(vgeom2->flags)) {
		return lwgeom_mindistance2d(vgeom1->allextension, vgeom2->allextension);
	} else if(!HAS_PUNION(vgeom1->flags) && HAS_PUNION(vgeom2->flags)) {
		LWGEOM *union1;
		union1 = lwgeom_union(vgeom1->kernel, vgeom1->conjecture);
		result = lwgeom_mindistance2d(union1, vgeom2->allextension);
		lwgeom_free(union1);
	} else if(HAS_PUNION(vgeom1->flags) && !HAS_PUNION(vgeom2->flags)) {
		LWGEOM *union2;
		union2 = lwgeom_union(vgeom2->kernel, vgeom2->conjecture);
		result = lwgeom_mindistance2d(vgeom1->allextension, union2);
		lwgeom_free(union2);
	} else {
		LWGEOM *union2, *union1;
		union1 = lwgeom_union(vgeom1->kernel, vgeom1->conjecture);
		union2 = lwgeom_union(vgeom2->kernel, vgeom2->conjecture);
		result = lwgeom_mindistance2d(union1, union2);
		lwgeom_free(union1);
		lwgeom_free(union2);
	}
	return result;
}

double vaguegeom_nearest_max_distance(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	return lwgeom_mindistance2d(vgeom1->kernel, vgeom2->kernel);
}

VAGUENUMERIC *vaguegeom_nearest_distance(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	VAGUENUMERIC *result;
	result = (VAGUENUMERIC*)lwalloc(sizeof(VAGUENUMERIC));
	result->max = vaguegeom_nearest_max_distance(vgeom1, vgeom2);
	result->min = vaguegeom_nearest_min_distance(vgeom1, vgeom2);
	return result;
}

/*farthest distance of vague geoms */
double vaguegeom_farthest_min_distance(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	return lwgeom_maxdistance2d(vgeom1->kernel, vgeom2->kernel);
}

double vaguegeom_farthest_max_distance(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	double result;
	if(HAS_PUNION(vgeom1->flags) && HAS_PUNION(vgeom2->flags)) {
		return lwgeom_maxdistance2d(vgeom1->allextension, vgeom2->allextension);
	} else if(!HAS_PUNION(vgeom1->flags) && HAS_PUNION(vgeom2->flags)) {
		LWGEOM *union1;
		union1 = lwgeom_union(vgeom1->kernel, vgeom1->conjecture);
		result = lwgeom_maxdistance2d(union1, vgeom2->allextension);
		lwgeom_free(union1);
	} else if(HAS_PUNION(vgeom1->flags) && !HAS_PUNION(vgeom2->flags)) {
		LWGEOM *union2;
		union2 = lwgeom_union(vgeom2->kernel, vgeom2->conjecture);
		result = lwgeom_maxdistance2d(vgeom1->allextension, union2);
		lwgeom_free(union2);
	} else {
		LWGEOM *union2, *union1;
		union1 = lwgeom_union(vgeom1->kernel, vgeom1->conjecture);
		union2 = lwgeom_union(vgeom2->kernel, vgeom2->conjecture);
		result = lwgeom_maxdistance2d(union1, union2);
		lwgeom_free(union1);
		lwgeom_free(union2);
	}
	return result;
}

VAGUENUMERIC *vaguegeom_farthest_distance(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	VAGUENUMERIC *result;
	result = (VAGUENUMERIC*)lwalloc(sizeof(VAGUENUMERIC));
	result->max = vaguegeom_farthest_max_distance(vgeom1, vgeom2);
	result->min = vaguegeom_farthest_min_distance(vgeom1, vgeom2);
	return result;
}
