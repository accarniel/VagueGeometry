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
* Processing of some topological operations by using GEOS
*
*************************/

#include "libvgeom.h"
/* perfomed by GEOS module */

LWGEOM *boundary(const LWGEOM *geom) {
	GEOSGeometry *g1, *g3;
	LWGEOM *result;
	
	if(geom==NULL)
		return NULL;
	
	initGEOS(lwnotice, lwgeom_geos_error);
	
//check POSTGIS version for the LWGEOM2GEOS function
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
		g1 = (GEOSGeometry *)LWGEOM2GEOS(geom, 0);
#else 
		g1 = (GEOSGeometry *)LWGEOM2GEOS(geom);
#endif

	if ( 0 == g1 ) {  /* exception thrown at construction */
		lwerror("First argument geometry could not be converted to GEOS: %s", lwgeom_geos_errmsg);
	}

	g3 = (GEOSGeometry *)GEOSBoundary(g1);

	if (g3 == NULL)	{
		GEOSGeom_destroy(g1);
		lwerror("GEOSBoundary: %s", lwgeom_geos_errmsg);
		assert(0);
	}

	GEOSSetSRID(g3, geom->srid);

	result = GEOS2LWGEOM(g3, 0);

	if (result == NULL)	{
		GEOSGeom_destroy(g1);
		GEOSGeom_destroy(g3);
		assert(0);
	}

	GEOSGeom_destroy(g1);
	GEOSGeom_destroy(g3);
	return result;
}

LWGEOM *convexhull(const LWGEOM *geom) {
	LWGEOM *result;
	GEOSGeometry *g1, *g3;
	initGEOS(lwnotice, lwgeom_geos_error);

//check POSTGIS version for the LWGEOM2GEOS function
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
		g1 = (GEOSGeometry *)LWGEOM2GEOS(geom, 0);
#else 
		g1 = (GEOSGeometry *)LWGEOM2GEOS(geom);
#endif

	if ( 0 == g1 ) {  /* exception thrown at construction */
		lwerror("First argument geometry could not be converted to GEOS: %s", lwgeom_geos_errmsg);
		return NULL;
	}

	g3 = (GEOSGeometry *)GEOSConvexHull(g1);
	GEOSGeom_destroy(g1);

	if (g3 == NULL)	{
		lwerror("GEOSConvexHull: %s", lwgeom_geos_errmsg);
		return NULL; /* never get here */
	}

	GEOSSetSRID(g3, geom->srid);

	result = GEOS2LWGEOM(g3, 0);
	GEOSGeom_destroy(g3);

	if (result == NULL) {
		return NULL;
	}

	return result;
}
