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
* A part this file was copied from lwout_geojson.c in order to handle the geojson representation.
*
**************************************************/

#include <assert.h>

#include "postgres.h"
#include "libvgeom.h"
#include "liblwgeom.h"
#include "lwgeom_export.h"

#include <json/json.h>
#include <json/json_object_private.h>


static void geojson_lwerror(char *msg, int error_code)
{
	lwerror("%s", msg);
}

/* Prototype */
LWGEOM* parse_geojson(json_object *geojson, bool *hasz,  int *root_srid);

static json_object* findMemberByName(json_object* poObj, const char* pszName ) {
	json_object* poTmp;
	json_object_iter it;

	poTmp = poObj;

	if( NULL == pszName || NULL == poObj)
		return NULL;

	it.key = NULL;
	it.val = NULL;
	it.entry = NULL;

	if( NULL != json_object_get_object(poTmp) )
	{
		assert( NULL != json_object_get_object(poTmp)->head );

		for( it.entry = json_object_get_object(poTmp)->head;
		        ( it.entry ?
		          ( it.key = (char*)it.entry->k,
		            it.val = (json_object*)it.entry->v, it.entry) : 0);
		        it.entry = it.entry->next)
		{
			if( strcasecmp((char *)it.key, pszName )==0 )
				return it.val;
		}
	}

	return NULL;
}

static int
parse_geojson_coord(json_object *poObj, bool *hasz, POINTARRAY *pa)
{
	POINT4D pt;
	int iType = 0;
	
	if( json_type_array == json_object_get_type( poObj ) )
	{

		json_object* poObjCoord = NULL;
		const int nSize = json_object_array_length( poObj );

		// Read X coordinate
		poObjCoord = json_object_array_get_idx( poObj, 0 );
		iType = json_object_get_type(poObjCoord);
		if (iType == json_type_double)
			pt.x = json_object_get_double( poObjCoord );
		else
			pt.x = json_object_get_int( poObjCoord );

		// Read Y coordiante
		poObjCoord = json_object_array_get_idx( poObj, 1 );
		if (iType == json_type_double)
			pt.y = json_object_get_double( poObjCoord );
		else
			pt.y = json_object_get_int( poObjCoord );

		if( nSize == 3 ) /* should this be >= 3 ? */
		{
			// Read Z coordiante
			poObjCoord = json_object_array_get_idx( poObj, 2 );
			if (iType == 3)
				pt.z = json_object_get_double( poObjCoord );
			else
				pt.z = json_object_get_int( poObjCoord );
			*hasz = true;
		}
		else
		{
			*hasz = false;
			/* Initialize Z coordinate, if required */
			if ( FLAGS_GET_Z(pa->flags) ) pt.z = 0.0;
		}

		/* TODO: should we account for nSize > 3 ? */

		/* Initialize M coordinate, if required */
		if ( FLAGS_GET_M(pa->flags) ) pt.m = 0.0;

	}

	return ptarray_append_point(pa, &pt, LW_FALSE);
}

static LWGEOM*
parse_geojson_point(json_object *geojson, bool *hasz,  int *root_srid)
{
	LWGEOM *geom;
	POINTARRAY *pa;
	json_object* coords = NULL;

	coords = findMemberByName( geojson, "coordinates" );
	if ( ! coords )
		geojson_lwerror("Unable to find 'coordinates' in GeoJSON string", 4);
	
	pa = ptarray_construct_empty(1, 0, 1);
	parse_geojson_coord(coords, hasz, pa);

	geom = (LWGEOM *) lwpoint_construct(*root_srid, NULL, pa);
	return geom;
}

static LWGEOM*
parse_geojson_linestring(json_object *geojson, bool *hasz,  int *root_srid)
{
	LWGEOM *geom;
	POINTARRAY *pa;
	json_object* points = NULL;
	int i = 0;

	points = findMemberByName( geojson, "coordinates" );
	if ( ! points )
		geojson_lwerror("Unable to find 'coordinates' in GeoJSON string", 4);

	pa = ptarray_construct_empty(1, 0, 1);

	if( json_type_array == json_object_get_type( points ) )
	{
		const int nPoints = json_object_array_length( points );
		for(i = 0; i < nPoints; ++i)
		{
			json_object* coords = NULL;
			coords = json_object_array_get_idx( points, i );
			parse_geojson_coord(coords, hasz, pa);
		}
	}

	geom = (LWGEOM *) lwline_construct(*root_srid, NULL, pa);
	return geom;
}

static LWGEOM*
parse_geojson_polygon(json_object *geojson, bool *hasz,  int *root_srid)
{
	LWGEOM *geom;
	POINTARRAY **ppa;
	json_object* rings = NULL;
	int i = 0, j = 0;
	int ring = 0;

	rings = findMemberByName( geojson, "coordinates" );
	if ( ! rings )
		geojson_lwerror("Unable to find 'coordinates' in GeoJSON string", 4);

	ppa = (POINTARRAY**) lwalloc(sizeof(POINTARRAY*));

	if( json_type_array == json_object_get_type( rings ) )
	{
		int nPoints;
		json_object* points = NULL;
		ppa[0] = ptarray_construct_empty(1, 0, 1);
		ring = json_object_array_length( rings );
		points = json_object_array_get_idx( rings, 0 );
		nPoints = json_object_array_length( points );

		for (i=0; i < nPoints; i++ )
		{
			json_object* coords = NULL;
			coords = json_object_array_get_idx( points, i );
			parse_geojson_coord(coords, hasz, ppa[0]);
		}

		for(i = 1; i < ring; ++i)
		{
			int nPoints;
			ppa = (POINTARRAY**) lwrealloc((POINTARRAY *) ppa, sizeof(POINTARRAY*) * (i + 1));
			ppa[i] = ptarray_construct_empty(1, 0, 1);
			points = json_object_array_get_idx( rings, i );
			nPoints = json_object_array_length( points );
			for (j=0; j < nPoints; j++ )
			{
				json_object* coords = NULL;
				coords = json_object_array_get_idx( points, j );
				parse_geojson_coord(coords, hasz, ppa[i]);
			}
		}
	}

	geom = (LWGEOM *) lwpoly_construct(*root_srid, NULL, ring, ppa);
	return geom;
}

static LWGEOM*
parse_geojson_multipoint(json_object *geojson, bool *hasz,  int *root_srid)
{
	LWGEOM *geom;
	int i = 0;
	json_object* poObjPoints = NULL;

	if (!*root_srid)
	{
		geom = (LWGEOM *)lwcollection_construct_empty(MULTIPOINTTYPE, *root_srid, 1, 0);
	}
	else
	{
		geom = (LWGEOM *)lwcollection_construct_empty(MULTIPOINTTYPE, -1, 1, 0);
	}

	poObjPoints = findMemberByName( geojson, "coordinates" );
	if ( ! poObjPoints )
		geojson_lwerror("Unable to find 'coordinates' in GeoJSON string", 4);

	if( json_type_array == json_object_get_type( poObjPoints ) )
	{
		const int nPoints = json_object_array_length( poObjPoints );
		for( i = 0; i < nPoints; ++i)
		{
			POINTARRAY *pa;
			json_object* poObjCoords = NULL;
			poObjCoords = json_object_array_get_idx( poObjPoints, i );

			pa = ptarray_construct_empty(1, 0, 1);
			parse_geojson_coord(poObjCoords, hasz, pa);

			geom = (LWGEOM*)lwmpoint_add_lwpoint((LWMPOINT*)geom,
			                                     (LWPOINT*)lwpoint_construct(*root_srid, NULL, pa));
		}
	}

	return geom;
}

static LWGEOM*
parse_geojson_multilinestring(json_object *geojson, bool *hasz,  int *root_srid)
{
	LWGEOM *geom = NULL;
	int i, j;
	json_object* poObjLines = NULL;

	if (!*root_srid)
	{
		geom = (LWGEOM *)lwcollection_construct_empty(MULTILINETYPE, *root_srid, 1, 0);
	}
	else
	{
		geom = (LWGEOM *)lwcollection_construct_empty(MULTILINETYPE, -1, 1, 0);
	}

	poObjLines = findMemberByName( geojson, "coordinates" );
	if ( ! poObjLines )
		geojson_lwerror("Unable to find 'coordinates' in GeoJSON string", 4);

	if( json_type_array == json_object_get_type( poObjLines ) )
	{
		const int nLines = json_object_array_length( poObjLines );
		for( i = 0; i < nLines; ++i)
		{
			POINTARRAY *pa = NULL;
			json_object* poObjLine = NULL;
			poObjLine = json_object_array_get_idx( poObjLines, i );
			pa = ptarray_construct_empty(1, 0, 1);

			if( json_type_array == json_object_get_type( poObjLine ) )
			{
				const int nPoints = json_object_array_length( poObjLine );
				for(j = 0; j < nPoints; ++j)
				{
					json_object* coords = NULL;
					coords = json_object_array_get_idx( poObjLine, j );
					parse_geojson_coord(coords, hasz, pa);
				}

				geom = (LWGEOM*)lwmline_add_lwline((LWMLINE*)geom,
				                                   (LWLINE*)lwline_construct(*root_srid, NULL, pa));
			}
		}
	}

	return geom;
}

static LWGEOM*
parse_geojson_multipolygon(json_object *geojson, bool *hasz,  int *root_srid)
{
	LWGEOM *geom = NULL;
	int i, j, k;
	json_object* poObjPolys = NULL;

	if (!*root_srid)
	{
		geom = (LWGEOM *)lwcollection_construct_empty(MULTIPOLYGONTYPE, *root_srid, 1, 0);
	}
	else
	{
		geom = (LWGEOM *)lwcollection_construct_empty(MULTIPOLYGONTYPE, -1, 1, 0);
	}

	poObjPolys = findMemberByName( geojson, "coordinates" );
	if ( ! poObjPolys )
		geojson_lwerror("Unable to find 'coordinates' in GeoJSON string", 4);

	if( json_type_array == json_object_get_type( poObjPolys ) )
	{
		const int nPolys = json_object_array_length( poObjPolys );

		for(i = 0; i < nPolys; ++i)
		{
			POINTARRAY **ppa;
			json_object* poObjPoly = NULL;
			poObjPoly = json_object_array_get_idx( poObjPolys, i );

			ppa = (POINTARRAY**) lwalloc(sizeof(POINTARRAY*));

			if( json_type_array == json_object_get_type( poObjPoly ) )
			{
				int nPoints;
				json_object* points = NULL;
				int ring = json_object_array_length( poObjPoly );
				ppa[0] = ptarray_construct_empty(1, 0, 1);

				points = json_object_array_get_idx( poObjPoly, 0 );
				nPoints = json_object_array_length( points );

				for (j=0; j < nPoints; j++ )
				{
					json_object* coords = NULL;
					coords = json_object_array_get_idx( points, j );
					parse_geojson_coord(coords, hasz, ppa[0]);
				}

				for(j = 1; j < ring; ++j)
				{
					int nPoints;
					ppa = (POINTARRAY**) lwrealloc((POINTARRAY *) ppa, sizeof(POINTARRAY*) * (j + 1));
					ppa[j] = ptarray_construct_empty(1, 0, 1);
					points = json_object_array_get_idx( poObjPoly, j );

					nPoints = json_object_array_length( points );
					for (k=0; k < nPoints; k++ )
					{
						json_object* coords = NULL;
						coords = json_object_array_get_idx( points, k );
						parse_geojson_coord(coords, hasz, ppa[i]);
					}
				}

				geom = (LWGEOM*)lwmpoly_add_lwpoly((LWMPOLY*)geom,
				                                   (LWPOLY*)lwpoly_construct(*root_srid, NULL, ring, ppa));
			}
		}
	}

	return geom;
}

static LWGEOM*
parse_geojson_geometrycollection(json_object *geojson, bool *hasz,  int *root_srid)
{
	LWGEOM *geom = NULL;
	int i;
	json_object* poObjGeoms = NULL;

	if (!*root_srid)
	{
		geom = (LWGEOM *)lwcollection_construct_empty(COLLECTIONTYPE, *root_srid, 1, 0);
	}
	else
	{
		geom = (LWGEOM *)lwcollection_construct_empty(COLLECTIONTYPE, -1, 1, 0);
	}

	poObjGeoms = findMemberByName( geojson, "geometries" );
	if ( ! poObjGeoms )
		geojson_lwerror("Unable to find 'geometries' in GeoJSON string", 4);

	if( json_type_array == json_object_get_type( poObjGeoms ) )
	{
		const int nGeoms = json_object_array_length( poObjGeoms );
		json_object* poObjGeom = NULL;
		for(i = 0; i < nGeoms; ++i )
		{
			poObjGeom = json_object_array_get_idx( poObjGeoms, i );
			geom = (LWGEOM*)lwcollection_add_lwgeom((LWCOLLECTION *)geom,
			                                        parse_geojson(poObjGeom, hasz, root_srid));
		}
	}

	return geom;
}

LWGEOM*
parse_geojson(json_object *geojson, bool *hasz,  int *root_srid)
{
	json_object* type = NULL;
	const char* name;

	if( NULL == geojson )
		geojson_lwerror("invalid GeoJSON representation", 2);

	type = findMemberByName( geojson, "type" );
	if( NULL == type )
		geojson_lwerror("unknown GeoJSON type", 3);

	name = json_object_get_string( type );

	if( strcasecmp( name, "Point" )==0 )
		return parse_geojson_point(geojson, hasz, root_srid);

	if( strcasecmp( name, "LineString" )==0 )
		return parse_geojson_linestring(geojson, hasz, root_srid);

	if( strcasecmp( name, "Polygon" )==0 )
		return parse_geojson_polygon(geojson, hasz, root_srid);

	if( strcasecmp( name, "MultiPoint" )==0 )
		return parse_geojson_multipoint(geojson, hasz, root_srid);

	if( strcasecmp( name, "MultiLineString" )==0 )
		return parse_geojson_multilinestring(geojson, hasz, root_srid);

	if( strcasecmp( name, "MultiPolygon" )==0 )
		return parse_geojson_multipolygon(geojson, hasz, root_srid);

	if( strcasecmp( name, "GeometryCollection" )==0 )
		return parse_geojson_geometrycollection(geojson, hasz, root_srid);

	lwerror("invalid GeoJson representation");
	return NULL; /* Never reach */
}

extern VAGUEGEOM *vgeojson_to_vaguegeom(char *geojson, uint8_t punion) {
	LWGEOM *kernel=NULL, *conjecture=NULL;
	int root_srid=0;
	bool hasz=true;
	json_tokener* jstok = NULL;
	json_object* poObj = NULL;
	json_object* poObjSrs = NULL;
	json_object* poType = NULL;
	json_object* poKernel = NULL;
	json_object* poConjecture = NULL;
	const char* name;
	
	/* Begin to Parse json */
	jstok = json_tokener_new();
	poObj = json_tokener_parse_ex(jstok, geojson, -1);
	if( jstok->err != json_tokener_success) {
		char err[256];
		snprintf(err, 256, "%s (at offset %d)", json_tokener_errors[jstok->err], jstok->char_offset);
		json_tokener_free(jstok);
		geojson_lwerror(err, 1);
	}
	json_tokener_free(jstok);

	poObjSrs = findMemberByName( poObj, "crs" );
	if (poObjSrs != NULL) {
		json_object* poObjSrsType = findMemberByName( poObjSrs, "type" );
		if (poObjSrsType != NULL) {
			json_object* poObjSrsProps = findMemberByName( poObjSrs, "properties" );
			json_object* poNameURL = findMemberByName( poObjSrsProps, "name" );
			const char* pszName = json_object_get_string( poNameURL );
			root_srid = getSRIDbySRS(pszName);
		}
	}
	poType = findMemberByName(poObj, "type");
	name = json_object_get_string( poType );

	poKernel = findMemberByName(poObj, "kernel");
	poConjecture = findMemberByName(poObj, "conjecture");
	if(poKernel != NULL) {
		kernel = parse_geojson(poKernel, &hasz, &root_srid);
		/*check if the kernel type is compatible with the vaguegeometry type */
		if(vaguegeom_compatible_type(name, kernel->type) ==LW_FALSE) {
			lwerror("The vague geometry type (%s) is not compatible with the kernel type (%s)", name, lwtype_name(kernel->type));
		}
		lwgeom_add_bbox(kernel);
		if (root_srid && kernel->srid == -1) kernel->srid = root_srid;
	} 
	
	if(poConjecture != NULL) {
		conjecture = parse_geojson(poConjecture, &hasz, &root_srid);
		/*check if the kernel type is compatible with the vaguegeometry type */
		if(vaguegeom_compatible_type(name, conjecture->type) ==LW_FALSE) {
			lwerror("The vague geometry type (%s) is not compatible with the conjecture type (%s)", name, lwtype_name(conjecture->type));
		}
		lwgeom_add_bbox(conjecture);
		if (root_srid && conjecture->srid == -1) conjecture->srid = root_srid;
	}

	if (kernel == NULL || conjecture == NULL) {
		if(poType != NULL) {
			if( strcasecmp( name, "VaguePoint" )==0 ) {
				if(kernel == NULL)
					kernel = lwgeom_construct_empty(POINTTYPE, root_srid, 0, 0);
				if(conjecture == NULL)
					conjecture = lwgeom_construct_empty(POINTTYPE, root_srid, 0, 0);
			} else if(strcasecmp(name, "VagueLineString")==0) {
				if(kernel == NULL)
					kernel = lwgeom_construct_empty(LINETYPE, root_srid, 0, 0);
				if(conjecture == NULL)
					conjecture = lwgeom_construct_empty(LINETYPE, root_srid, 0, 0);
			} else if(strcasecmp(name, "VaguePolygon")==0) {
				if(kernel == NULL)
					kernel = lwgeom_construct_empty(POLYGONTYPE, root_srid, 0, 0);
				if(conjecture == NULL)
					conjecture = lwgeom_construct_empty(POLYGONTYPE, root_srid, 0, 0);
			} else if(strcasecmp(name, "VagueMultiPoint")==0) {
				if(kernel == NULL)
					kernel = lwgeom_construct_empty(MULTIPOINTTYPE, root_srid, 0, 0);
				if(conjecture == NULL)
					conjecture = lwgeom_construct_empty(MULTIPOINTTYPE, root_srid, 0, 0);
			} else if(strcasecmp(name, "VagueMultiLineString")==0) {
				if(kernel == NULL)
					kernel = lwgeom_construct_empty(MULTILINETYPE, root_srid, 0, 0);
				if(conjecture == NULL)
					conjecture = lwgeom_construct_empty(MULTILINETYPE, root_srid, 0, 0);
			} else if(strcasecmp(name, "VagueMultiPolygon") ==0) {
				if(kernel == NULL)
					kernel = lwgeom_construct_empty(MULTIPOLYGONTYPE, root_srid, 0, 0);
				if(conjecture == NULL)
					conjecture = lwgeom_construct_empty(MULTIPOLYGONTYPE, root_srid, 0, 0);
			}else {
				lwerror("Invalid VagueGeometry type (%s)", name);
			}
		} else {
			lwerror("The vague geometry object must has a type");
		}
	}
	
	if (!hasz) {
		LWGEOM *tmpK = lwgeom_force_2d(kernel);
		LWGEOM *tmpC = lwgeom_force_2d(conjecture);
		lwgeom_free(kernel);
		lwgeom_free(conjecture);
		kernel = tmpK;
		conjecture = tmpC;
	}

	if(kernel!=NULL && conjecture !=NULL) {
		if(kernel->type != conjecture->type) {
			geojson_lwerror("Kernel and conjecture must be same types", 4);
		}
	}
	return vaguegeom_construct(kernel, conjecture, punion);
}


