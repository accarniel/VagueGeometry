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
* Serialization of VagueGeometry objects - it is based on the PostGIS implementation.
* It copies private functions of the PostGIS object serializations (from g_serialized.c file)
*
*************************/

#include "libvgeom.h"
#include "lwgeom_log.h"

int32_t vgserialized_get_srid(const VAGUEGEOMSERIALIZED *vg) {
	int32_t srid = 0;
	srid = srid | (vg->srid[0] << 16);
	srid = srid | (vg->srid[1] << 8);
	srid = srid | vg->srid[2];
	/* Only the first 21 bits are set. Slide up and back to pull
	   the negative bits down, if we need them. */
	srid = (srid<<11)>>11;
	
	/* 0 is our internal unknown value. We'll map back and forth here for now */
	if ( srid == 0 ) 
		return SRID_UNKNOWN;
	else
		return clamp_srid(srid);
}

void vgserialized_set_srid(VAGUEGEOMSERIALIZED *vg, int32_t srid) {
	srid = clamp_srid(srid);

	/* 0 is our internal unknown value.
	 * We'll map back and forth here for now */
	if ( srid == SRID_UNKNOWN )
		srid = 0;
		
	vg->srid[0] = (srid & 0x001F0000) >> 16;
	vg->srid[1] = (srid & 0x0000FF00) >> 8;
	vg->srid[2] = (srid & 0x000000FF);
}

uint8_t vgserialized_get_type(const VAGUEGEOMSERIALIZED *vg) {
	uint8_t type;
	assert(vg);
	//when there is nothing the the first value is its type
	if(HAS_NOTHING(vg->flags)) {
		uint32_t *ptr;
		ptr = (uint32_t*)(vg->data);
		return *ptr;
	} else if(HAS_KERNEL(vg->flags)) {
		uint8_t *ptr, kernelflag;
		ptr = (uint8_t*)(vg->data);

		if(HAS_BOTH(vg->flags)) {
			ptr += sizeof(uint32_t); //because of kernel size, then jump it
			if(HAS_PUNION(vg->flags))
				ptr += sizeof(uint32_t); //because of conjecture size, then jump it
			else
				ptr += sizeof(uint32_t); //padding when there is no the union
		}

		memcpy(&kernelflag, ptr, sizeof(uint8_t));
		ptr += sizeof(uint8_t);

		//we need to jump the padding
		ptr += sizeof(uint8_t)*7;
		
		if ( FLAGS_GET_BBOX(kernelflag) ) {
			ptr += (gbox_serialized_size(kernelflag));
		}

		type = lw_get_uint32_t(ptr);
	} else if(HAS_CONJECTURE(vg->flags)) {
		uint8_t *ptr, conjecflag;
		ptr = (uint8_t*)(vg->data);

		memcpy(&conjecflag, ptr, sizeof(uint8_t));
		ptr += sizeof(uint8_t);

		//we need to jump the padding
		ptr += sizeof(uint8_t)*7;
		
		if ( FLAGS_GET_BBOX(conjecflag) ) {
			ptr += (gbox_serialized_size(conjecflag));
		}

		type = lw_get_uint32_t(ptr);
	}
	
	return settype_fromlwgeom(type);
}

/***********************************************************************
* Calculate the GSERIALIZED size for an LWGEOM.
*/

/* Private functions from the PostGIS extension from g_serialized.c file */

static size_t gserialized_from_any_size(const LWGEOM *geom); /* Local prototype */

static size_t gserialized_from_lwpoint_size(const LWPOINT *point)
{
	size_t size = 4; /* Type number. */

	assert(point);

	size += 4; /* Number of points (one or zero (empty)). */
	size += point->point->npoints * FLAGS_NDIMS(point->flags) * sizeof(double);

	LWDEBUGF(3, "point size = %d", size);

	return size;
}

static size_t gserialized_from_lwline_size(const LWLINE *line)
{
	size_t size = 4; /* Type number. */

	assert(line);

	size += 4; /* Number of points (zero => empty). */
	size += line->points->npoints * FLAGS_NDIMS(line->flags) * sizeof(double);

	LWDEBUGF(3, "linestring size = %d", size);

	return size;
}

static size_t gserialized_from_lwtriangle_size(const LWTRIANGLE *triangle)
{
	size_t size = 4; /* Type number. */

	assert(triangle);

	size += 4; /* Number of points (zero => empty). */
	size += triangle->points->npoints * FLAGS_NDIMS(triangle->flags) * sizeof(double);

	LWDEBUGF(3, "triangle size = %d", size);

	return size;
}

static size_t gserialized_from_lwpoly_size(const LWPOLY *poly)
{
	size_t size = 4; /* Type number. */
	int i = 0;

	assert(poly);

	size += 4; /* Number of rings (zero => empty). */
	if ( poly->nrings % 2 )
		size += 4; /* Padding to double alignment. */

	for ( i = 0; i < poly->nrings; i++ )
	{
		size += 4; /* Number of points in ring. */
		size += poly->rings[i]->npoints * FLAGS_NDIMS(poly->flags) * sizeof(double);
	}

	LWDEBUGF(3, "polygon size = %d", size);

	return size;
}

static size_t gserialized_from_lwcircstring_size(const LWCIRCSTRING *curve)
{
	size_t size = 4; /* Type number. */

	assert(curve);

	size += 4; /* Number of points (zero => empty). */
	size += curve->points->npoints * FLAGS_NDIMS(curve->flags) * sizeof(double);

	LWDEBUGF(3, "circstring size = %d", size);

	return size;
}

static size_t gserialized_from_lwcollection_size(const LWCOLLECTION *col)
{
	size_t size = 4; /* Type number. */
	int i = 0;

	assert(col);

	size += 4; /* Number of sub-geometries (zero => empty). */

	for ( i = 0; i < col->ngeoms; i++ )
	{
		size_t subsize = gserialized_from_any_size(col->geoms[i]);
		size += subsize;
		LWDEBUGF(3, "lwcollection subgeom(%d) size = %d", i, subsize);
	}

	LWDEBUGF(3, "lwcollection size = %d", size);

	return size;
}

static size_t gserialized_from_any_size(const LWGEOM *geom)
{
	LWDEBUGF(2, "Input type: %s", lwtype_name(geom->type));

	switch (geom->type)
	{
	case POINTTYPE:
		return gserialized_from_lwpoint_size((LWPOINT *)geom);
	case LINETYPE:
		return gserialized_from_lwline_size((LWLINE *)geom);
	case POLYGONTYPE:
		return gserialized_from_lwpoly_size((LWPOLY *)geom);
	case TRIANGLETYPE:
		return gserialized_from_lwtriangle_size((LWTRIANGLE *)geom);
	case CIRCSTRINGTYPE:
		return gserialized_from_lwcircstring_size((LWCIRCSTRING *)geom);
	case CURVEPOLYTYPE:
	case COMPOUNDTYPE:
	case MULTIPOINTTYPE:
	case MULTILINETYPE:
	case MULTICURVETYPE:
	case MULTIPOLYGONTYPE:
	case MULTISURFACETYPE:
	case POLYHEDRALSURFACETYPE:
	case TINTYPE:
	case COLLECTIONTYPE:
		return gserialized_from_lwcollection_size((LWCOLLECTION *)geom);
	default:
		lwerror("Unknown geometry type: %d - %s", geom->type, lwtype_name(geom->type));
		return 0;
	}
}


/* Public function */

size_t gserialized_from_lwgeom_size(const LWGEOM *geom)
{
	size_t size = 8; /* Header overhead. */
	assert(geom);
	
	if ( geom->bbox )
		size += gbox_serialized_size(geom->flags);	
		
	size += gserialized_from_any_size(geom);
	LWDEBUGF(3, "g_serialize size = %d", size);
	
	return size;
}

static size_t vgserialized_from_vgeom_size(const VAGUEGEOM *vgeom);
size_t vgserialized_from_vgeom_size(const VAGUEGEOM *vgeom) {
	size_t size = 0;
	assert(vgeom);
	
	//situation 1: an empty vgeom object
	if(HAS_NOTHING(vgeom->flags)) {
		//type + padding
		size = sizeof(uint32_t) + sizeof(uint32_t);
	}
	
	if(HAS_KERNEL(vgeom->flags)) {
		size += sizeof(uint8_t); //one byte because of its flag		
		size += sizeof(uint8_t); //padding
		size += sizeof(uint8_t); //padding
		size += sizeof(uint8_t); //padding		
		size += sizeof(uint32_t); //padding
		
		size += (gserialized_from_lwgeom_size(vgeom->kernel)-8);
	}
	
	if(HAS_CONJECTURE(vgeom->flags)) {
		size += sizeof(uint8_t); //one byte because of its flag
		size += sizeof(uint8_t); //padding
		size += sizeof(uint8_t); //padding
		size += sizeof(uint8_t); //padding
		size += sizeof(uint32_t); //padding

		size += (gserialized_from_lwgeom_size(vgeom->conjecture)-8);
	}
	
	if(HAS_BOTH(vgeom->flags)) {
		size += sizeof(uint32_t); //stores the length of the kernel (to jump to de conjecture directly)
		
		size += sizeof(uint32_t); //this is padding when there is no union or the length of the conjecture (to jump to the union directly)
		
		if(HAS_PUNION(vgeom->flags)) {
			size += sizeof(uint8_t); //one byte because of its flag union			
			size += sizeof(uint8_t); //padding
			size += sizeof(uint8_t); //padding
			size += sizeof(uint8_t); //padding
			size += sizeof(uint32_t); //padding
			
			size += (gserialized_from_lwgeom_size(vgeom->allextension)-8);
		}
	}
	
	return size;	
}




/***********************************************************************
* Serialize an LWGEOM into GSERIALIZED.
*/

/* Private functions */

static size_t gserialized_from_lwgeom_any(const LWGEOM *geom, uint8_t *buf);

static size_t gserialized_from_lwpoint(const LWPOINT *point, uint8_t *buf)
{
	uint8_t *loc;
	int ptsize = ptarray_point_size(point->point);
	int type = POINTTYPE;

	assert(point);
	assert(buf);

	if ( FLAGS_GET_ZM(point->flags) != FLAGS_GET_ZM(point->point->flags) )
		lwerror("Dimensions mismatch in lwpoint");

	LWDEBUGF(2, "lwpoint_to_gserialized(%p, %p) called", point, buf);

	loc = buf;

	/* Write in the type. */
	memcpy(loc, &type, sizeof(uint32_t));
	loc += sizeof(uint32_t);
	/* Write in the number of points (0 => empty). */
	memcpy(loc, &(point->point->npoints), sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Copy in the ordinates. */
	if ( point->point->npoints > 0 )
	{
		memcpy(loc, getPoint_internal(point->point, 0), ptsize);
		loc += ptsize;
	}

	return (size_t)(loc - buf);
}

static size_t gserialized_from_lwline(const LWLINE *line, uint8_t *buf)
{
	uint8_t *loc;
	int ptsize;
	size_t size;
	int type = LINETYPE;

	assert(line);
	assert(buf);

	LWDEBUGF(2, "lwline_to_gserialized(%p, %p) called", line, buf);

	if ( FLAGS_GET_Z(line->flags) != FLAGS_GET_Z(line->points->flags) )
		lwerror("Dimensions mismatch in lwline");

	ptsize = ptarray_point_size(line->points);

	loc = buf;

	/* Write in the type. */
	memcpy(loc, &type, sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Write in the npoints. */
	memcpy(loc, &(line->points->npoints), sizeof(uint32_t));
	loc += sizeof(uint32_t);

	LWDEBUGF(3, "lwline_to_gserialized added npoints (%d)", line->points->npoints);

	/* Copy in the ordinates. */
	if ( line->points->npoints > 0 )
	{
		size = line->points->npoints * ptsize;
		memcpy(loc, getPoint_internal(line->points, 0), size);
		loc += size;
	}
	LWDEBUGF(3, "lwline_to_gserialized copied serialized_pointlist (%d bytes)", ptsize * line->points->npoints);

	return (size_t)(loc - buf);
}

static size_t gserialized_from_lwpoly(const LWPOLY *poly, uint8_t *buf)
{
	int i;
	uint8_t *loc;
	int ptsize;
	int type = POLYGONTYPE;

	assert(poly);
	assert(buf);

	LWDEBUG(2, "lwpoly_to_gserialized called");

	ptsize = sizeof(double) * FLAGS_NDIMS(poly->flags);
	loc = buf;

	/* Write in the type. */
	memcpy(loc, &type, sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Write in the nrings. */
	memcpy(loc, &(poly->nrings), sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Write in the npoints per ring. */
	for ( i = 0; i < poly->nrings; i++ )
	{
		memcpy(loc, &(poly->rings[i]->npoints), sizeof(uint32_t));
		loc += sizeof(uint32_t);
	}

	/* Add in padding if necessary to remain double aligned. */
	if ( poly->nrings % 2 )
	{
		memset(loc, 0, sizeof(uint32_t));
		loc += sizeof(uint32_t);
	}

	/* Copy in the ordinates. */
	for ( i = 0; i < poly->nrings; i++ )
	{
		POINTARRAY *pa = poly->rings[i];
		size_t pasize;

		if ( FLAGS_GET_ZM(poly->flags) != FLAGS_GET_ZM(pa->flags) )
			lwerror("Dimensions mismatch in lwpoly");

		pasize = pa->npoints * ptsize;
		memcpy(loc, getPoint_internal(pa, 0), pasize);
		loc += pasize;
	}
	return (size_t)(loc - buf);
}

static size_t gserialized_from_lwtriangle(const LWTRIANGLE *triangle, uint8_t *buf)
{
	uint8_t *loc;
	int ptsize;
	size_t size;
	int type = TRIANGLETYPE;

	assert(triangle);
	assert(buf);

	LWDEBUGF(2, "lwtriangle_to_gserialized(%p, %p) called", triangle, buf);

	if ( FLAGS_GET_ZM(triangle->flags) != FLAGS_GET_ZM(triangle->points->flags) )
		lwerror("Dimensions mismatch in lwtriangle");

	ptsize = ptarray_point_size(triangle->points);

	loc = buf;

	/* Write in the type. */
	memcpy(loc, &type, sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Write in the npoints. */
	memcpy(loc, &(triangle->points->npoints), sizeof(uint32_t));
	loc += sizeof(uint32_t);

	LWDEBUGF(3, "lwtriangle_to_gserialized added npoints (%d)", triangle->points->npoints);

	/* Copy in the ordinates. */
	if ( triangle->points->npoints > 0 )
	{
		size = triangle->points->npoints * ptsize;
		memcpy(loc, getPoint_internal(triangle->points, 0), size);
		loc += size;
	}
	LWDEBUGF(3, "lwtriangle_to_gserialized copied serialized_pointlist (%d bytes)", ptsize * triangle->points->npoints);

	return (size_t)(loc - buf);
}

static size_t gserialized_from_lwcircstring(const LWCIRCSTRING *curve, uint8_t *buf)
{
	uint8_t *loc;
	int ptsize;
	size_t size;
	int type = CIRCSTRINGTYPE;

	assert(curve);
	assert(buf);

	if (FLAGS_GET_ZM(curve->flags) != FLAGS_GET_ZM(curve->points->flags))
		lwerror("Dimensions mismatch in lwcircstring");


	ptsize = ptarray_point_size(curve->points);
	loc = buf;

	/* Write in the type. */
	memcpy(loc, &type, sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Write in the npoints. */
	memcpy(loc, &curve->points->npoints, sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Copy in the ordinates. */
	if ( curve->points->npoints > 0 )
	{
		size = curve->points->npoints * ptsize;
		memcpy(loc, getPoint_internal(curve->points, 0), size);
		loc += size;
	}

	return (size_t)(loc - buf);
}

static size_t gserialized_from_lwcollection(const LWCOLLECTION *coll, uint8_t *buf)
{
	size_t subsize = 0;
	uint8_t *loc;
	int i;
	int type;

	assert(coll);
	assert(buf);

	type = coll->type;
	loc = buf;

	/* Write in the type. */
	memcpy(loc, &type, sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Write in the number of subgeoms. */
	memcpy(loc, &coll->ngeoms, sizeof(uint32_t));
	loc += sizeof(uint32_t);

	/* Serialize subgeoms. */
	for ( i=0; i<coll->ngeoms; i++ )
	{
		if (FLAGS_GET_ZM(coll->flags) != FLAGS_GET_ZM(coll->geoms[i]->flags))
			lwerror("Dimensions mismatch in lwcollection");
		subsize = gserialized_from_lwgeom_any(coll->geoms[i], loc);
		loc += subsize;
	}

	return (size_t)(loc - buf);
}

static size_t gserialized_from_lwgeom_any(const LWGEOM *geom, uint8_t *buf)
{
	assert(geom);
	assert(buf);

	LWDEBUGF(2, "Input type (%d) %s, hasz: %d hasm: %d",
		geom->type, lwtype_name(geom->type),
		FLAGS_GET_Z(geom->flags), FLAGS_GET_M(geom->flags));
	LWDEBUGF(2, "LWGEOM(%p) uint8_t(%p)", geom, buf);

	switch (geom->type)
	{
	case POINTTYPE:
		return gserialized_from_lwpoint((LWPOINT *)geom, buf);
	case LINETYPE:
		return gserialized_from_lwline((LWLINE *)geom, buf);
	case POLYGONTYPE:
		return gserialized_from_lwpoly((LWPOLY *)geom, buf);
	case TRIANGLETYPE:
		return gserialized_from_lwtriangle((LWTRIANGLE *)geom, buf);
	case CIRCSTRINGTYPE:
		return gserialized_from_lwcircstring((LWCIRCSTRING *)geom, buf);
	case CURVEPOLYTYPE:
	case COMPOUNDTYPE:
	case MULTIPOINTTYPE:
	case MULTILINETYPE:
	case MULTICURVETYPE:
	case MULTIPOLYGONTYPE:
	case MULTISURFACETYPE:
	case POLYHEDRALSURFACETYPE:
	case TINTYPE:
	case COLLECTIONTYPE:
		return gserialized_from_lwcollection((LWCOLLECTION *)geom, buf);
	default:
		lwerror("Unknown geometry type: %d - %s", geom->type, lwtype_name(geom->type));
		return 0;
	}
	return 0;
}

static size_t gserialized_from_gbox(const GBOX *gbox, uint8_t *buf)
{
	uint8_t *loc = buf;
	float f;
	size_t return_size;

	assert(buf);

	f = next_float_down(gbox->xmin);
	memcpy(loc, &f, sizeof(float));
	loc += sizeof(float);

	f = next_float_up(gbox->xmax);
	memcpy(loc, &f, sizeof(float));
	loc += sizeof(float);

	f = next_float_down(gbox->ymin);
	memcpy(loc, &f, sizeof(float));
	loc += sizeof(float);

	f = next_float_up(gbox->ymax);
	memcpy(loc, &f, sizeof(float));
	loc += sizeof(float);

	if ( FLAGS_GET_GEODETIC(gbox->flags) )
	{
		f = next_float_down(gbox->zmin);
		memcpy(loc, &f, sizeof(float));
		loc += sizeof(float);

		f = next_float_up(gbox->zmax);
		memcpy(loc, &f, sizeof(float));
		loc += sizeof(float);

		return_size = (size_t)(loc - buf);
		LWDEBUGF(4, "returning size %d", return_size);
		return return_size;
	}

	if ( FLAGS_GET_Z(gbox->flags) )
	{
		f = next_float_down(gbox->zmin);
		memcpy(loc, &f, sizeof(float));
		loc += sizeof(float);

		f = next_float_up(gbox->zmax);
		memcpy(loc, &f, sizeof(float));
		loc += sizeof(float);

	}

	if ( FLAGS_GET_M(gbox->flags) )
	{
		f = next_float_down(gbox->mmin);
		memcpy(loc, &f, sizeof(float));
		loc += sizeof(float);

		f = next_float_up(gbox->mmax);
		memcpy(loc, &f, sizeof(float));
		loc += sizeof(float);
	}
	return_size = (size_t)(loc - buf);
	LWDEBUGF(4, "returning size %d", return_size);
	return return_size;
}


static size_t vgserialized_from_vgeom_component(const LWGEOM *comp, uint8_t *buf) {
	uint8_t *loc = buf;
	
	//save the postgis flag
	memcpy(loc, &comp->flags, sizeof(uint8_t));
	loc += sizeof(uint8_t);
	
	//padding of 7 bytes (3 uint8_t plus 1 uint32_t) to complete 8 bytes (1 byte of the flag + 7 of padding)
	memset(loc, 0, sizeof(uint8_t));
	loc += sizeof(uint8_t);
	
	memset(loc, 0, sizeof(uint8_t));
	loc += sizeof(uint8_t);
	
	memset(loc, 0, sizeof(uint8_t));
	loc += sizeof(uint8_t);
	
	memset(loc, 0, sizeof(uint32_t));
	loc += sizeof(uint32_t);
	
	/* Write in the serialized form of the gbox, if necessary. */
	if ( comp->bbox )
		loc += gserialized_from_gbox(comp->bbox, loc);

	/* Write in the serialized form of the geometry. */
	loc += gserialized_from_lwgeom_any(comp, loc);	
	
	return (size_t)(loc - buf);
}

static size_t vgserialized_from_vgeom(const VAGUEGEOM *vgeom, uint8_t *buf) {
	uint8_t *loc = buf;
	size_t return_size, subsize;
	
	/*
	* The strategy of the serialization is: 
	* 1 - has the kernel part? then serialize it
	* 2 - has the conjecture part too? store the size of the kernel part and then serialize it
	* 3 - has the kernel and conjecture? then store its precomputed union
	* The BBOX is serialized before of the each part.
	* but before, we check if the vgeom object is empty
	*/
	if(HAS_NOTHING(vgeom->flags)) {		
		//save the vaguegeometry type
		memcpy(loc, &vgeom->type, sizeof(uint32_t));
		loc += sizeof(uint32_t);
		
		//padding
		memset(loc, 0, sizeof(uint32_t));
		loc += sizeof(uint32_t);
		
		return_size = (size_t)(loc - buf);
		return return_size;
	}
	
	/* First step, if there are a conjecture and kernel, then serialize the kernel_size and conjecture_size */
	if(HAS_BOTH(vgeom->flags)) {
		size_t kernel_size, conjecture_size;
	
		//get the size of kernel
		kernel_size = (gserialized_from_lwgeom_size(vgeom->kernel)-8);
		kernel_size += sizeof(uint8_t); //one byte because of its flag
		kernel_size += 7; //padding
		
		//save the kernel size
		memcpy(loc, &kernel_size, sizeof(uint32_t));
		loc += sizeof(uint32_t);

		if(HAS_PUNION(vgeom->flags)) {
			//get the size of conjecture
			conjecture_size = (gserialized_from_lwgeom_size(vgeom->conjecture)-8);
			conjecture_size += sizeof(uint8_t); //one byte because of its flag
			conjecture_size += 7; //padding
		
			//save the conjecture size
			memcpy(loc, &conjecture_size, sizeof(uint32_t));
			loc += sizeof(uint32_t);
		} else {
			memset(loc, 0, sizeof(uint32_t));
			loc += sizeof(uint32_t);
		}
	}
	
	if(HAS_KERNEL(vgeom->flags)) {
		subsize = vgserialized_from_vgeom_component(vgeom->kernel, loc);	
		loc += subsize;
	}	

	if(HAS_CONJECTURE(vgeom->flags)) {
		subsize = vgserialized_from_vgeom_component(vgeom->conjecture, loc);	
		loc += subsize;		
	}		

	/* 4th step is store the allextension if there are the kernel and conjecture */
	if(HAS_BOTH(vgeom->flags) && HAS_PUNION(vgeom->flags)) {
		subsize = vgserialized_from_vgeom_component(vgeom->allextension, loc);	
		loc += subsize;	
	}
	
	return_size = (size_t)(loc - buf);
	return return_size;
}


static void vgserialized_harmonize_bbox_component(LWGEOM *comp) {
	/* See if we need a bounding box, add one if we don't have one. */
	if ( (! comp->bbox) && lwgeom_needs_bbox(comp) && (!lwgeom_is_empty(comp)) ) {
		lwgeom_add_bbox(comp);
	}

	/* Harmonize the flags to the state of the lwgeom */
	if ( comp->bbox )
		FLAGS_SET_BBOX(comp->flags, 1);
}

VAGUEGEOMSERIALIZED *vaguegeom_to_serialization(VAGUEGEOM *vgeom, size_t *size) {
	uint8_t *serialized, *ptr;
	size_t expected_size = 0, return_size;
	VAGUEGEOMSERIALIZED *ret;

	if(HAS_KERNEL(vgeom->flags)) {
		vgserialized_harmonize_bbox_component(vgeom->kernel);
	}
	
	if(HAS_CONJECTURE(vgeom->flags)) {
		vgserialized_harmonize_bbox_component(vgeom->conjecture);
	}
	
	if(HAS_BOTH(vgeom->flags) && HAS_PUNION(vgeom->flags)) {
		vgserialized_harmonize_bbox_component(vgeom->allextension);
	}
	
	expected_size = 8; //header of the vaguegeomserialized	
	expected_size += vgserialized_from_vgeom_size(vgeom);

	serialized = (uint8_t*)lwalloc(expected_size);
	ptr = serialized;

	/* move past size, srid and flag */
	ptr += 8;
	
	/* serialization process */
	ptr += vgserialized_from_vgeom(vgeom, ptr);
	
	/* Calculate size as returned by data processing functions. */
	return_size = ptr - serialized;

	if ( expected_size != return_size ) /* Uh oh! */{
		lwerror("Return size (%d) not equal to expected size (%d)!!!", return_size, expected_size);
		return NULL;
	}

	if ( size ) /* Return the output size to the caller if necessary. */
		*size = return_size;

	ret = (VAGUEGEOMSERIALIZED*)serialized;

	ret->size = return_size << 2;
	vgserialized_set_srid(ret, vgeom->kernel->srid);
	ret->flags = vgeom->flags;

	return ret;
}

static LWGEOM* lwgeom_from_gserialized_buffer(uint8_t *data_ptr, uint8_t g_flags, size_t *g_size);

static LWPOINT* lwpoint_from_gserialized_buffer(uint8_t *data_ptr, uint8_t g_flags, size_t *g_size)
{
	uint8_t *start_ptr = data_ptr;
	LWPOINT *point;
	uint32_t npoints = 0;

	assert(data_ptr);

	point = (LWPOINT*)lwalloc(sizeof(LWPOINT));
	point->srid = SRID_UNKNOWN; /* Default */
	point->bbox = NULL;
	point->type = POINTTYPE;
	point->flags = g_flags;

	data_ptr += 4; /* Skip past the type. */
	npoints = lw_get_uint32_t(data_ptr); /* Zero => empty geometry */
	data_ptr += 4; /* Skip past the npoints. */

	if ( npoints > 0 )
		point->point = ptarray_construct_reference_data(FLAGS_GET_Z(g_flags), FLAGS_GET_M(g_flags), 1, data_ptr);
	else
		point->point = ptarray_construct(FLAGS_GET_Z(g_flags), FLAGS_GET_M(g_flags), 0); /* Empty point */

	data_ptr += npoints * FLAGS_NDIMS(g_flags) * sizeof(double);

	if ( g_size )
		*g_size = data_ptr - start_ptr;

	return point;
}

static LWLINE* lwline_from_gserialized_buffer(uint8_t *data_ptr, uint8_t g_flags, size_t *g_size)
{
	uint8_t *start_ptr = data_ptr;
	LWLINE *line;
	uint32_t npoints = 0;

	assert(data_ptr);

	line = (LWLINE*)lwalloc(sizeof(LWLINE));
	line->srid = SRID_UNKNOWN; /* Default */
	line->bbox = NULL;
	line->type = LINETYPE;
	line->flags = g_flags;

	data_ptr += 4; /* Skip past the type. */
	npoints = lw_get_uint32_t(data_ptr); /* Zero => empty geometry */
	data_ptr += 4; /* Skip past the npoints. */

	if ( npoints > 0 )
		line->points = ptarray_construct_reference_data(FLAGS_GET_Z(g_flags), FLAGS_GET_M(g_flags), npoints, data_ptr);
		
	else
		line->points = ptarray_construct(FLAGS_GET_Z(g_flags), FLAGS_GET_M(g_flags), 0); /* Empty linestring */

	data_ptr += FLAGS_NDIMS(g_flags) * npoints * sizeof(double);

	if ( g_size )
		*g_size = data_ptr - start_ptr;

	return line;
}

static LWPOLY* lwpoly_from_gserialized_buffer(uint8_t *data_ptr, uint8_t g_flags, size_t *g_size)
{
	uint8_t *start_ptr = data_ptr;
	LWPOLY *poly;
	uint8_t *ordinate_ptr;
	uint32_t nrings = 0;
	int i = 0;

	assert(data_ptr);

	poly = (LWPOLY*)lwalloc(sizeof(LWPOLY));
	poly->srid = SRID_UNKNOWN; /* Default */
	poly->bbox = NULL;
	poly->type = POLYGONTYPE;
	poly->flags = g_flags;

	data_ptr += 4; /* Skip past the polygontype. */
	nrings = lw_get_uint32_t(data_ptr); /* Zero => empty geometry */
	poly->nrings = nrings;
	data_ptr += 4; /* Skip past the nrings. */

	ordinate_ptr = data_ptr; /* Start the ordinate pointer. */
	if ( nrings > 0)
	{
		poly->rings = (POINTARRAY**)lwalloc( sizeof(POINTARRAY*) * nrings );
		ordinate_ptr += nrings * 4; /* Move past all the npoints values. */
		if ( nrings % 2 ) /* If there is padding, move past that too. */
			ordinate_ptr += 4;
	}
	else /* Empty polygon */
	{
		poly->rings = NULL;
	}

	for ( i = 0; i < nrings; i++ )
	{
		uint32_t npoints = 0;

		/* Read in the number of points. */
		npoints = lw_get_uint32_t(data_ptr);
		data_ptr += 4;

		/* Make a point array for the ring, and move the ordinate pointer past the ring ordinates. */
		poly->rings[i] = ptarray_construct_reference_data(FLAGS_GET_Z(g_flags), FLAGS_GET_M(g_flags), npoints, ordinate_ptr);
		
		ordinate_ptr += sizeof(double) * FLAGS_NDIMS(g_flags) * npoints;
	}

	if ( g_size )
		*g_size = ordinate_ptr - start_ptr;

	return poly;
}

static LWTRIANGLE* lwtriangle_from_gserialized_buffer(uint8_t *data_ptr, uint8_t g_flags, size_t *g_size)
{
	uint8_t *start_ptr = data_ptr;
	LWTRIANGLE *triangle;
	uint32_t npoints = 0;

	assert(data_ptr);

	triangle = (LWTRIANGLE*)lwalloc(sizeof(LWTRIANGLE));
	triangle->srid = SRID_UNKNOWN; /* Default */
	triangle->bbox = NULL;
	triangle->type = TRIANGLETYPE;
	triangle->flags = g_flags;

	data_ptr += 4; /* Skip past the type. */
	npoints = lw_get_uint32_t(data_ptr); /* Zero => empty geometry */
	data_ptr += 4; /* Skip past the npoints. */

	if ( npoints > 0 )
		triangle->points = ptarray_construct_reference_data(FLAGS_GET_Z(g_flags), FLAGS_GET_M(g_flags), npoints, data_ptr);		
	else
		triangle->points = ptarray_construct(FLAGS_GET_Z(g_flags), FLAGS_GET_M(g_flags), 0); /* Empty triangle */

	data_ptr += FLAGS_NDIMS(g_flags) * npoints * sizeof(double);

	if ( g_size )
		*g_size = data_ptr - start_ptr;

	return triangle;
}

static LWCIRCSTRING* lwcircstring_from_gserialized_buffer(uint8_t *data_ptr, uint8_t g_flags, size_t *g_size)
{
	uint8_t *start_ptr = data_ptr;
	LWCIRCSTRING *circstring;
	uint32_t npoints = 0;

	assert(data_ptr);

	circstring = (LWCIRCSTRING*)lwalloc(sizeof(LWCIRCSTRING));
	circstring->srid = SRID_UNKNOWN; /* Default */
	circstring->bbox = NULL;
	circstring->type = CIRCSTRINGTYPE;
	circstring->flags = g_flags;

	data_ptr += 4; /* Skip past the circstringtype. */
	npoints = lw_get_uint32_t(data_ptr); /* Zero => empty geometry */
	data_ptr += 4; /* Skip past the npoints. */

	if ( npoints > 0 )
		circstring->points = ptarray_construct_reference_data(FLAGS_GET_Z(g_flags), FLAGS_GET_M(g_flags), npoints, data_ptr);		
	else
		circstring->points = ptarray_construct(FLAGS_GET_Z(g_flags), FLAGS_GET_M(g_flags), 0); /* Empty circularstring */

	data_ptr += FLAGS_NDIMS(g_flags) * npoints * sizeof(double);

	if ( g_size )
		*g_size = data_ptr - start_ptr;

	return circstring;
}

static LWCOLLECTION* lwcollection_from_gserialized_buffer(uint8_t *data_ptr, uint8_t g_flags, size_t *g_size)
{
	uint32_t type;
	uint8_t *start_ptr = data_ptr;
	LWCOLLECTION *collection;
	uint32_t ngeoms = 0;
	int i = 0;

	assert(data_ptr);

	type = lw_get_uint32_t(data_ptr);
	data_ptr += 4; /* Skip past the type. */

	collection = (LWCOLLECTION*)lwalloc(sizeof(LWCOLLECTION));
	collection->srid = SRID_UNKNOWN; /* Default */
	collection->bbox = NULL;
	collection->type = type;
	collection->flags = g_flags;

	ngeoms = lw_get_uint32_t(data_ptr);
	collection->ngeoms = ngeoms; /* Zero => empty geometry */
	data_ptr += 4; /* Skip past the ngeoms. */

	if ( ngeoms > 0 )
		collection->geoms = lwalloc(sizeof(LWGEOM*) * ngeoms);
	else
		collection->geoms = NULL;

	/* Sub-geometries are never de-serialized with boxes (#1254) */
	FLAGS_SET_BBOX(g_flags, 0);

	for ( i = 0; i < ngeoms; i++ )
	{
		uint32_t subtype = lw_get_uint32_t(data_ptr);
		size_t subsize = 0;

		if ( ! lwcollection_allows_subtype(type, subtype) )
		{
			lwerror("Invalid subtype (%s) for collection type (%s)", lwtype_name(subtype), lwtype_name(type));
			lwfree(collection);
			return NULL;
		}
		collection->geoms[i] = lwgeom_from_gserialized_buffer(data_ptr, g_flags, &subsize);
		data_ptr += subsize;
	}

	if ( g_size )
		*g_size = data_ptr - start_ptr;

	return collection;
}

LWGEOM* lwgeom_from_gserialized_buffer(uint8_t *data_ptr, uint8_t g_flags, size_t *g_size) {
	uint32_t type;

	assert(data_ptr);

	type = lw_get_uint32_t(data_ptr);

	switch (type)
	{
	case POINTTYPE:
		return (LWGEOM *)lwpoint_from_gserialized_buffer(data_ptr, g_flags, g_size);
	case LINETYPE:
		return (LWGEOM *)lwline_from_gserialized_buffer(data_ptr, g_flags, g_size);
	case CIRCSTRINGTYPE:
		return (LWGEOM *)lwcircstring_from_gserialized_buffer(data_ptr, g_flags, g_size);
	case POLYGONTYPE:
		return (LWGEOM *)lwpoly_from_gserialized_buffer(data_ptr, g_flags, g_size);
	case TRIANGLETYPE:
		return (LWGEOM *)lwtriangle_from_gserialized_buffer(data_ptr, g_flags, g_size);
	case MULTIPOINTTYPE:
	case MULTILINETYPE:
	case MULTIPOLYGONTYPE:
	case COMPOUNDTYPE:
	case CURVEPOLYTYPE:
	case MULTICURVETYPE:
	case MULTISURFACETYPE:
	case POLYHEDRALSURFACETYPE:
	case TINTYPE:
	case COLLECTIONTYPE:
		return (LWGEOM *)lwcollection_from_gserialized_buffer(data_ptr, g_flags, g_size);
	default:
		lwerror("Unknown geometry type: %d - %s", type, lwtype_name(type));
		return NULL;
	}
}

int gserialized_read_gbox_p_vaguegeom(const VAGUEGEOMSERIALIZED *vg, GBOX *gbox, uint8_t which) {
	uint8_t *data = (uint8_t*)vg->data;
	size_t size_kernel=0, size_conjecture=0;
	uint8_t flags, type;
	/* Null input! */
	if ( ! ( data && gbox ) ) return LW_FAILURE;
	
	if(HAS_BOTH(vg->flags)) {
		size_kernel = lw_get_uint32_t(data);
		data += sizeof(uint32_t);
		if(HAS_PUNION(vg->flags)) {
			size_conjecture = lw_get_uint32_t(data);
			data += sizeof(uint32_t);
		} else {
			//jump the padding
			data += sizeof(uint32_t);
		}
	}	

	//get the bbox of kernel
	if(which == 1) {
		memcpy(&flags, data, sizeof(uint8_t));
		data += sizeof(uint8_t); //jump the postgis flag
		data += 7; //jump the padding
	} 
	//get the bbox of conjecture
	else if(which == 2) {
		if(HAS_KERNEL(vg->flags))
			data += size_kernel;
		memcpy(&flags, data, sizeof(uint8_t));
		data += sizeof(uint8_t); //jump the postgis flag
		data += 7; //jump the padding
	} //get the bbox of all extension (the union between conjecture and kernel) 
	else {
		if(HAS_KERNEL(vg->flags))
			data += size_kernel;
		if(HAS_CONJECTURE(vg->flags))
			data += size_conjecture;
		memcpy(&flags, data, sizeof(uint8_t));
		data += sizeof(uint8_t); //jump the postgis flag
		data += 7; //jump the padding
	}
	
	if ( FLAGS_GET_BBOX(flags) ) {
		type = lw_get_uint32_t((data + gbox_serialized_size(flags)));
	} else {
		type = lw_get_uint32_t(data);
	}
	
	/* Initialize the flags on the box */
	gbox->flags = flags;

	/* Has pre-calculated box */
	if ( FLAGS_GET_BBOX(flags) )
	{
		int i = 0;
		float *fbox = (float*)(data);
		gbox->xmin = fbox[i++];
		gbox->xmax = fbox[i++];
		gbox->ymin = fbox[i++];
		gbox->ymax = fbox[i++];

		/* Geodetic? Read next dimension (geocentric Z) and return */
		if ( FLAGS_GET_GEODETIC(flags) )
		{
			gbox->zmin = fbox[i++];
			gbox->zmax = fbox[i++];
			return LW_SUCCESS;
		}
		/* Cartesian? Read extra dimensions (if there) and return */
		if ( FLAGS_GET_Z(flags) )
		{
			gbox->zmin = fbox[i++];
			gbox->zmax = fbox[i++];
		}
		if ( FLAGS_GET_M(flags) )
		{
			gbox->mmin = fbox[i++];
			gbox->mmax = fbox[i++];
		}
		return LW_SUCCESS;
	}

	/* No pre-calculated box, but for cartesian entries we can do some magic */
	if ( ! FLAGS_GET_GEODETIC(flags) )
	{
		/* Boxes of points are easy peasy */
		if ( type == POINTTYPE )
		{
			int i = 1; /* Start past <pointtype><padding> */
			double *dptr = (double*)(data);

			/* Read the empty flag */
			int *iptr = (int*)(data);
			int isempty = (iptr[1] == 0);

			/* EMPTY point has no box */
			if ( isempty ) return LW_FAILURE;

			gbox->xmin = gbox->xmax = dptr[i++];
			gbox->ymin = gbox->ymax = dptr[i++];
			if ( FLAGS_GET_Z(flags) )
			{
				gbox->zmin = gbox->zmax = dptr[i++];
			}
			if ( FLAGS_GET_M(flags) )
			{
				gbox->mmin = gbox->mmax = dptr[i++];
			}
			gbox_float_round(gbox);
			return LW_SUCCESS;
		}
		/* We can calculate the box of a two-point cartesian line trivially */
		else if ( type == LINETYPE )
		{
			int ndims = FLAGS_NDIMS(flags);
			int i = 0; /* Start past <linetype><npoints> */
			double *dptr = (double*)(data);
			int *iptr = (int*)(data);
			int npoints = iptr[1]; /* Read the npoints */
			
			/* This only works with 2-point lines */
			if ( npoints != 2 )
				return LW_FAILURE;
				
			/* Advance to X */
			i++;
			gbox->xmin = FP_MIN(dptr[i], dptr[i+ndims]);
			gbox->xmax = FP_MAX(dptr[i], dptr[i+ndims]);
			
			/* Advance to Y */
			i++;
			gbox->ymin = FP_MIN(dptr[i], dptr[i+ndims]);
			gbox->ymax = FP_MAX(dptr[i], dptr[i+ndims]);
			
			if ( FLAGS_GET_Z(flags) )
			{
				/* Advance to Z */
				i++;
				gbox->zmin = FP_MIN(dptr[i], dptr[i+ndims]);
				gbox->zmax = FP_MAX(dptr[i], dptr[i+ndims]);
			}
			if ( FLAGS_GET_M(flags) )
			{
				/* Advance to M */
				i++;
				gbox->mmin = FP_MIN(dptr[i], dptr[i+ndims]);
				gbox->mmax = FP_MAX(dptr[i], dptr[i+ndims]);
			}
			gbox_float_round(gbox);
			return LW_SUCCESS;
		}
		/* We could also do single-entry multi-points */
		else if ( type == MULTIPOINTTYPE )
		{
			/* TODO: Make this actually happen */
			return LW_FAILURE;
		}
	}
	return LW_FAILURE;
}

static uint8_t vaguegeomtype_to_crisptype(const uint8_t type) {
	switch (type)	{
		case VAGUEPOINTTYPE:
			return POINTTYPE;
		case VAGUELINETYPE:
			return LINETYPE;
		case VAGUEPOLYGONTYPE:
			return POLYGONTYPE;
		case VAGUEMULTIPOINTTYPE:
			return MULTIPOINTTYPE;
		case VAGUEMULTILINETYPE:
			return MULTILINETYPE;
		case VAGUEMULTIPOLYGONTYPE:
			return MULTIPOLYGONTYPE;
		case VAGUECOLLECTIONTYPE:
			return COLLECTIONTYPE;
		default:
			lwerror("type %d of vgeom is not valid", type);
			assert(0); //error
	}	
}


static LWGEOM *serialization_to_comp(uint8_t *data_ptr, int32_t srid, uint8_t which, size_t *size, const VAGUEGEOMSERIALIZED *vg) {
	size_t subsize;
	LWGEOM *lwgeom;
	uint8_t flag;
	uint8_t *start_ptr = data_ptr;
	uint32_t type;
	GBOX bbox;
	
	//first get the postgis flag
	memcpy(&flag, data_ptr, sizeof(uint8_t));
	data_ptr += sizeof(uint8_t); /* Skip past the postgis flag. */
	
	data_ptr += 7; //skip the padding 

	if ( FLAGS_GET_BBOX(flag) ) {
		data_ptr += gbox_serialized_size(flag);
	}

	type = lw_get_uint32_t(data_ptr);
	
	lwgeom = lwgeom_from_gserialized_buffer(data_ptr, flag, &subsize);
	lwgeom->flags = flag;
	lwgeom_set_srid(lwgeom, srid);
	lwgeom->type = type;

	data_ptr += subsize;
	
	//1 is kernel, 2 is conjecture and 3 for the allextension
	if ( gserialized_read_gbox_p_vaguegeom(vg, &bbox, which) == LW_SUCCESS ){
		lwgeom->bbox = gbox_copy(&bbox);
	} else if ( lwgeom_needs_bbox(lwgeom) && (lwgeom_calculate_gbox(lwgeom, &bbox) == LW_SUCCESS) )	{
		lwgeom->bbox = gbox_copy(&bbox);
	} else {
		lwgeom->bbox = NULL;
	}
	
	if ( size )
		*size = data_ptr - start_ptr;
		
	return lwgeom;
}

VAGUEGEOM *serialization_to_vaguegeom(const VAGUEGEOMSERIALIZED *vg) {
	LWGEOM *lw_kernel=NULL, *lw_conjecture=NULL;
	uint8_t *vg_serialized;
	size_t size;
	int32_t srid = 0;
	uint32_t crisptype, vgtype;
	
	srid = vgserialized_get_srid(vg);
	vgtype = vgserialized_get_type(vg);
	crisptype = vaguegeomtype_to_crisptype(vgtype);

	vg_serialized = (uint8_t*)vg->data;

	/* the best case is if there are not kernel and conjecture */
	if(HAS_NOTHING(vg->flags)) {
		lw_kernel = lwgeom_construct_empty(crisptype, srid, 0, 0);
		lw_conjecture = lwgeom_construct_empty(crisptype, srid, 0, 0);
		return vaguegeom_construct(lw_kernel, lw_conjecture, HAS_PUNION(vg->flags));
	}
	/*
	* The strategy of the deserialization is: 
	* 1 - has the kernel part? then serialize it
	* 2 - has the conjecture part too? store the size of the kernel part and then serialize it
	* The BBOX is serialized before of the each part.
	*/

	/* first step is: when there are kernel and conjecture, get the kernel size and conjecture size */
	if(HAS_BOTH(vg->flags)) {
		vg_serialized += sizeof(uint32_t); //jump the size of kernel
		vg_serialized += sizeof(uint32_t); //jump the size of conjecture/padding
	}	

	/*second step is about the kernel, if there is a kernel part*/
	if(HAS_KERNEL(vg->flags)) {
		lw_kernel = serialization_to_comp(vg_serialized, srid, 1, &size, vg);
		
		/* if there is only a kernel, then can return the vaguegeometry object */
		if(!HAS_CONJECTURE(vg->flags)) {	
			lw_conjecture = lwgeom_construct_empty(lw_kernel->type, lw_kernel->srid, FLAGS_GET_Z(lw_kernel->flags), FLAGS_GET_M(lw_kernel->flags));
			return vaguegeom_construct(lw_kernel, lw_conjecture, HAS_PUNION(vg->flags));
		}
	}		

	/* if there is a kernel and conjecture, then jump the size of kernel, which already did read */
	if(HAS_BOTH(vg->flags)) {
		vg_serialized += size;
	}
	
	/* if there is a conjecture then read it */
	if(HAS_CONJECTURE(vg->flags)) {
		lw_conjecture = serialization_to_comp(vg_serialized, srid, 2, &size, vg);

		/* if there is only a conjecture, then can return the vaguegeometry object */
		if(!HAS_KERNEL(vg->flags)) {	
			lw_kernel = lwgeom_construct_empty(lw_conjecture->type, lw_conjecture->srid, FLAGS_GET_Z(lw_conjecture->flags), FLAGS_GET_M(lw_conjecture->flags));
			return vaguegeom_construct(lw_kernel, lw_conjecture, HAS_PUNION(vg->flags));
		}
	}

	/* if there are both, then read the precomputed union when exists it */
	if(HAS_BOTH(vg->flags) && HAS_PUNION(vg->flags)) {
		LWGEOM *allextension;
		
		vg_serialized += size;
		allextension = serialization_to_comp(vg_serialized, srid, 3, &size, vg);

		return vaguegeom_construct_precompunion(lw_kernel, lw_conjecture, allextension);
	}
		
	return vaguegeom_construct(lw_kernel, lw_conjecture, HAS_PUNION(vg->flags));
}

LWGEOM *get_kernel_from_serialization(const VAGUEGEOMSERIALIZED *vg) {
	LWGEOM *lw_kernel = NULL;
	uint8_t *vg_serialized;
	int32_t srid;
	uint32_t crisptype, vgtype;
	
	srid = vgserialized_get_srid(vg);
	vgtype = vgserialized_get_type(vg);
	crisptype = vaguegeomtype_to_crisptype(vgtype);

	vg_serialized = (uint8_t*)vg->data;

	/* the best case is if there are not kernel, then return a empty geometry */
	if(HAS_NOTHING(vg->flags) || (!HAS_KERNEL(vg->flags)))
		return lwgeom_construct_empty(crisptype, srid, 0, 0);

	if(HAS_BOTH(vg->flags)) {
		vg_serialized += sizeof(uint32_t);
		vg_serialized += sizeof(uint32_t);
	}	

	/*second step is about the kernel, if there is a kernel part*/
	if(HAS_KERNEL(vg->flags)) {
		lw_kernel = serialization_to_comp(vg_serialized, srid, 1, NULL, vg);
	}	
	return lw_kernel;
}

LWGEOM *get_conjecture_from_serialization(const VAGUEGEOMSERIALIZED *vg) {
	LWGEOM *lw_conjecture = NULL;
	uint8_t *vg_serialized;
	size_t size_kernel;
	int32_t srid;
	uint32_t crisptype, vgtype;
	
	srid = vgserialized_get_srid(vg);
	vgtype = vgserialized_get_type(vg);
	crisptype = vaguegeomtype_to_crisptype(vgtype);

	vg_serialized = (uint8_t*)vg->data;

	/* the best case is if there are not conjecture, then return a empty geometry */
	if(HAS_NOTHING(vg->flags) || !HAS_CONJECTURE(vg->flags)) 
		return lwgeom_construct_empty(crisptype, srid, 0, 0);

	if(HAS_BOTH(vg->flags)) {
		size_kernel = lw_get_uint32_t(vg_serialized);
		vg_serialized += sizeof(uint32_t);
		
		vg_serialized += sizeof(uint32_t);
		
		vg_serialized += size_kernel; //jump the kernel part
	}	
	
	/* if there is a conjecture then read it */
	if(HAS_CONJECTURE(vg->flags)) {
		lw_conjecture = serialization_to_comp(vg_serialized, srid, 2, NULL, vg);
	}

	return lw_conjecture;
}

/* TODO LWGEOM *get_union_from_serialization(const VAGUEGEOMSERIALIZED *vg) */
