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
* A part this file was copied from lwout_wkb.c and lwin_wkb.c of postgis's project in order to handle the Vague Well-Known Binary representation.
*
**************************************************/

#include "libvgeom.h"
#include "lwgeom_log.h"

static uint8_t* lwgeom_to_wkb_buf(const LWGEOM *geom, uint8_t *buf, uint8_t variant);
static size_t lwgeom_to_wkb_size(const LWGEOM *geom, uint8_t variant);

/*
* Look-up table for hex writer
*/
static char *hexchr = "0123456789ABCDEF";

/*
* Optional SRID
*/
static int lwgeom_wkb_needs_srid(const LWGEOM *geom, uint8_t variant)
{
	/* Sub-components of collections inherit their SRID from the parent.
	   We force that behavior with the WKB_NO_SRID flag */
	if ( variant & WKB_NO_SRID )
		return LW_FALSE;
		
	/* We can only add an SRID if the geometry has one, and the 
	   WKB form is extended */	
	if ( (variant & WKB_EXTENDED) && lwgeom_has_srid(geom) )
		return LW_TRUE;
		
	/* Everything else doesn't get an SRID */
	return LW_FALSE;
}

/*
* GeometryType
*/
static uint32_t lwgeom_wkb_type(const LWGEOM *geom, uint8_t variant)
{
	uint32_t wkb_type = 0;

	switch ( geom->type )
	{
	case POINTTYPE:
		wkb_type = WKB_POINT_TYPE;
		break;
	case LINETYPE:
		wkb_type = WKB_LINESTRING_TYPE;
		break;
	case POLYGONTYPE:
		wkb_type = WKB_POLYGON_TYPE;
		break;
	case MULTIPOINTTYPE:
		wkb_type = WKB_MULTIPOINT_TYPE;
		break;
	case MULTILINETYPE:
		wkb_type = WKB_MULTILINESTRING_TYPE;
		break;
	case MULTIPOLYGONTYPE:
		wkb_type = WKB_MULTIPOLYGON_TYPE;
		break;
	case COLLECTIONTYPE:
		wkb_type = WKB_GEOMETRYCOLLECTION_TYPE;
		break;
	case CIRCSTRINGTYPE:
		wkb_type = WKB_CIRCULARSTRING_TYPE;
		break;
	case COMPOUNDTYPE:
		wkb_type = WKB_COMPOUNDCURVE_TYPE;
		break;
	case CURVEPOLYTYPE:
		wkb_type = WKB_CURVEPOLYGON_TYPE;
		break;
	case MULTICURVETYPE:
		wkb_type = WKB_MULTICURVE_TYPE;
		break;
	case MULTISURFACETYPE:
		wkb_type = WKB_MULTISURFACE_TYPE;
		break;
	case POLYHEDRALSURFACETYPE:
		wkb_type = WKB_POLYHEDRALSURFACE_TYPE;
		break;
	case TINTYPE:
		wkb_type = WKB_TIN_TYPE;
		break;
	case TRIANGLETYPE:
		wkb_type = WKB_TRIANGLE_TYPE;
		break;
	default:
		lwerror("Unsupported geometry type: %s [%d]",
			lwtype_name(geom->type), geom->type);
	}

	if ( variant & WKB_EXTENDED )
	{
		if ( FLAGS_GET_Z(geom->flags) )
			wkb_type |= WKBZOFFSET;
		if ( FLAGS_GET_M(geom->flags) )
			wkb_type |= WKBMOFFSET;
/*		if ( geom->srid != SRID_UNKNOWN && ! (variant & WKB_NO_SRID) ) */
		if ( lwgeom_wkb_needs_srid(geom, variant) )
			wkb_type |= WKBSRIDFLAG;
	}
	else if ( variant & WKB_ISO )
	{
		/* Z types are in the 1000 range */
		if ( FLAGS_GET_Z(geom->flags) )
			wkb_type += 1000;
		/* M types are in the 2000 range */
		if ( FLAGS_GET_M(geom->flags) )
			wkb_type += 2000;
		/* ZM types are in the 1000 + 2000 = 3000 range, see above */
	}
	return wkb_type;
}

/*
* Vague Geometry Type
*/
static uint32_t vgeom_vwkb_type(const VAGUEGEOM *vgeom, uint8_t variant)
{
	uint32_t wkb_type = 0;
	if(vgeom->type == VAGUEPOINTTYPE && !HAS_BOTH(vgeom->flags))
		wkb_type = VAGUEMULTIPOINTTYPE;
	else
		wkb_type = vgeom->type;

	if ( variant & WKB_EXTENDED ) {
		if ( FLAGS_GET_Z(vgeom->kernel->flags) )
			wkb_type |= WKBZOFFSET;
		if ( FLAGS_GET_M(vgeom->kernel->flags) )
			wkb_type |= WKBMOFFSET;
/*		if ( geom->srid != SRID_UNKNOWN && ! (variant & WKB_NO_SRID) ) */
		if ( lwgeom_wkb_needs_srid(vgeom->kernel, variant) )
			wkb_type |= WKBSRIDFLAG;
	}
	else if ( variant & WKB_ISO ) {
		/* Z types are in the 1000 range */
		if ( FLAGS_GET_Z(vgeom->kernel->flags) )
			wkb_type += 1000;
		/* M types are in the 2000 range */
		if ( FLAGS_GET_M(vgeom->kernel->flags) )
			wkb_type += 2000;
		/* ZM types are in the 1000 + 2000 = 3000 range, see above */
	}
	return wkb_type;
}

/*
* Endian
*/
static uint8_t* endian_to_wkb_buf(uint8_t *buf, uint8_t variant)
{
	if ( variant & WKB_HEX )
	{
		buf[0] = '0';
		buf[1] = ((variant & WKB_NDR) ? '1' : '0');
		return buf + 2;
	}
	else
	{
		buf[0] = ((variant & WKB_NDR) ? 1 : 0);
		return buf + 1;
	}
}

/*
* SwapBytes?
*/
static inline int wkb_swap_bytes(uint8_t variant)
{
	/* If requested variant matches machine arch, we don't have to swap! */
	if ( ((variant & WKB_NDR) && (getMachineEndian() == NDR)) ||
	     ((! (variant & WKB_NDR)) && (getMachineEndian() == XDR)) )
	{
		return LW_FALSE;
	}
	return LW_TRUE;
}

/*
* Integer32
*/
static uint8_t* integer_to_wkb_buf(const int ival, uint8_t *buf, uint8_t variant)
{
	char *iptr = (char*)(&ival);
	int i = 0;

	if ( sizeof(int) != WKB_INT_SIZE )
	{
		lwerror("Machine int size is not %d bytes!", WKB_INT_SIZE);
	}
	LWDEBUGF(4, "Writing value '%u'", ival);
	if ( variant & WKB_HEX )
	{
		int swap = wkb_swap_bytes(variant);
		/* Machine/request arch mismatch, so flip byte order */
		for ( i = 0; i < WKB_INT_SIZE; i++ )
		{
			int j = (swap ? WKB_INT_SIZE - 1 - i : i);
			uint8_t b = iptr[j];
			/* Top four bits to 0-F */
			buf[2*i] = hexchr[b >> 4];
			/* Bottom four bits to 0-F */
			buf[2*i+1] = hexchr[b & 0x0F];
		}
		return buf + (2 * WKB_INT_SIZE);
	}
	else
	{
		/* Machine/request arch mismatch, so flip byte order */
		if ( wkb_swap_bytes(variant) )
		{
			for ( i = 0; i < WKB_INT_SIZE; i++ )
			{
				buf[i] = iptr[WKB_INT_SIZE - 1 - i];
			}
		}
		/* If machine arch and requested arch match, don't flip byte order */
		else
		{
			memcpy(buf, iptr, WKB_INT_SIZE);
		}
		return buf + WKB_INT_SIZE;
	}
}

/*
* Float64
*/
static uint8_t* double_to_wkb_buf(const double d, uint8_t *buf, uint8_t variant)
{
	char *dptr = (char*)(&d);
	int i = 0;

	if ( sizeof(double) != WKB_DOUBLE_SIZE )
	{
		lwerror("Machine double size is not %d bytes!", WKB_DOUBLE_SIZE);
	}

	if ( variant & WKB_HEX )
	{
		int swap =  wkb_swap_bytes(variant);
		/* Machine/request arch mismatch, so flip byte order */
		for ( i = 0; i < WKB_DOUBLE_SIZE; i++ )
		{
			int j = (swap ? WKB_DOUBLE_SIZE - 1 - i : i);
			uint8_t b = dptr[j];
			/* Top four bits to 0-F */
			buf[2*i] = hexchr[b >> 4];
			/* Bottom four bits to 0-F */
			buf[2*i+1] = hexchr[b & 0x0F];
		}
		return buf + (2 * WKB_DOUBLE_SIZE);
	}
	else
	{
		/* Machine/request arch mismatch, so flip byte order */
		if ( wkb_swap_bytes(variant) )
		{
			for ( i = 0; i < WKB_DOUBLE_SIZE; i++ )
			{
				buf[i] = dptr[WKB_DOUBLE_SIZE - 1 - i];
			}
		}
		/* If machine arch and requested arch match, don't flip byte order */
		else
		{
			memcpy(buf, dptr, WKB_DOUBLE_SIZE);
		}
		return buf + WKB_DOUBLE_SIZE;
	}
}


/*
* Empty
*/
static size_t empty_to_wkb_size(const LWGEOM *geom, uint8_t variant)
{
	size_t size = WKB_BYTE_SIZE + WKB_INT_SIZE + WKB_INT_SIZE;

	if ( lwgeom_wkb_needs_srid(geom, variant) )
		size += WKB_INT_SIZE;

	return size;
}

static uint8_t* empty_to_wkb_buf(const LWGEOM *geom, uint8_t *buf, uint8_t variant)
{
	uint32_t wkb_type = lwgeom_wkb_type(geom, variant);

	if ( geom->type == POINTTYPE )
	{
		/* Change POINT to MULTIPOINT */
		wkb_type &= ~WKB_POINT_TYPE;     /* clear POINT flag */
		wkb_type |= WKB_MULTIPOINT_TYPE; /* set MULTIPOINT flag */
	}

	/* Set the endian flag */
	buf = endian_to_wkb_buf(buf, variant);

	/* Set the geometry type */
	buf = integer_to_wkb_buf(wkb_type, buf, variant);

	/* Set the SRID if necessary */
	if ( lwgeom_wkb_needs_srid(geom, variant) )
		buf = integer_to_wkb_buf(geom->srid, buf, variant);

	/* Set nrings/npoints/ngeoms to zero */
	buf = integer_to_wkb_buf(0, buf, variant);
	return buf;
}

/*
* POINTARRAY
*/
static size_t ptarray_to_wkb_size(const POINTARRAY *pa, uint8_t variant)
{
	int dims = 2;
	size_t size = 0;

	if ( variant & (WKB_ISO | WKB_EXTENDED) )
		dims = FLAGS_NDIMS(pa->flags);

	/* Include the npoints if it's not a POINT type) */
	if ( ! ( variant & WKB_NO_NPOINTS ) )
		size += WKB_INT_SIZE;

	/* size of the double list */
	size += pa->npoints * dims * WKB_DOUBLE_SIZE;

	return size;
}

static uint8_t* ptarray_to_wkb_buf(const POINTARRAY *pa, uint8_t *buf, uint8_t variant)
{
	int dims = 2;
	int i, j;
	double *dbl_ptr;

	/* SFSQL is always 2-d. Extended and ISO use all available dimensions */
	if ( (variant & WKB_ISO) || (variant & WKB_EXTENDED) )
		dims = FLAGS_NDIMS(pa->flags);

	/* Set the number of points (if it's not a POINT type) */
	if ( ! ( variant & WKB_NO_NPOINTS ) )
		buf = integer_to_wkb_buf(pa->npoints, buf, variant);

	/* Set the ordinates. */
	/* TODO: Ensure that getPoint_internal is always aligned so
	         this doesn't fail on RiSC architectures */
	/* TODO: Make this faster by bulk copying the coordinates when
	         the output endian/dims match the internal endian/dims */
	for ( i = 0; i < pa->npoints; i++ )
	{
		LWDEBUGF(4, "Writing point #%d", i);
		dbl_ptr = (double*)getPoint_internal(pa, i);
		for ( j = 0; j < dims; j++ )
		{
			LWDEBUGF(4, "Writing dimension #%d (buf = %p)", j, buf);
			buf = double_to_wkb_buf(dbl_ptr[j], buf, variant);
		}
	}
	LWDEBUGF(4, "Done (buf = %p)", buf);
	return buf;
}

/*
* POINT
*/
static size_t lwpoint_to_wkb_size(const LWPOINT *pt, uint8_t variant)
{
	/* Endian flag + type number */
	size_t size = WKB_BYTE_SIZE + WKB_INT_SIZE;

	/* Extended WKB needs space for optional SRID integer */
	if ( lwgeom_wkb_needs_srid((LWGEOM*)pt, variant) )
		size += WKB_INT_SIZE;

	/* Points */
	size += ptarray_to_wkb_size(pt->point, variant | WKB_NO_NPOINTS);
	return size;
}

static uint8_t* lwpoint_to_wkb_buf(const LWPOINT *pt, uint8_t *buf, uint8_t variant)
{
	/* Set the endian flag */
	LWDEBUGF(4, "Entering function, buf = %p", buf);
	buf = endian_to_wkb_buf(buf, variant);
	LWDEBUGF(4, "Endian set, buf = %p", buf);
	/* Set the geometry type */
	buf = integer_to_wkb_buf(lwgeom_wkb_type((LWGEOM*)pt, variant), buf, variant);
	LWDEBUGF(4, "Type set, buf = %p", buf);
	/* Set the optional SRID for extended variant */
	if ( lwgeom_wkb_needs_srid((LWGEOM*)pt, variant) )
	{
		buf = integer_to_wkb_buf(pt->srid, buf, variant);
		LWDEBUGF(4, "SRID set, buf = %p", buf);
	}
	/* Set the coordinates */
	buf = ptarray_to_wkb_buf(pt->point, buf, variant | WKB_NO_NPOINTS);
	LWDEBUGF(4, "Pointarray set, buf = %p", buf);
	return buf;
}

/*
* LINESTRING, CIRCULARSTRING
*/
static size_t lwline_to_wkb_size(const LWLINE *line, uint8_t variant)
{
	/* Endian flag + type number */
	size_t size = WKB_BYTE_SIZE + WKB_INT_SIZE;

	/* Extended WKB needs space for optional SRID integer */
	if ( lwgeom_wkb_needs_srid((LWGEOM*)line, variant) )
		size += WKB_INT_SIZE;

	/* Size of point array */
	size += ptarray_to_wkb_size(line->points, variant);
	return size;
}

static uint8_t* lwline_to_wkb_buf(const LWLINE *line, uint8_t *buf, uint8_t variant)
{
	/* Set the endian flag */
	buf = endian_to_wkb_buf(buf, variant);
	/* Set the geometry type */
	buf = integer_to_wkb_buf(lwgeom_wkb_type((LWGEOM*)line, variant), buf, variant);
	/* Set the optional SRID for extended variant */
	if ( lwgeom_wkb_needs_srid((LWGEOM*)line, variant) )
		buf = integer_to_wkb_buf(line->srid, buf, variant);
	/* Set the coordinates */
	buf = ptarray_to_wkb_buf(line->points, buf, variant);
	return buf;
}

/*
* TRIANGLE
*/
static size_t lwtriangle_to_wkb_size(const LWTRIANGLE *tri, uint8_t variant)
{
	/* endian flag + type number + number of rings */
	size_t size = WKB_BYTE_SIZE + WKB_INT_SIZE + WKB_INT_SIZE;

	/* Extended WKB needs space for optional SRID integer */
	if ( lwgeom_wkb_needs_srid((LWGEOM*)tri, variant) )
		size += WKB_INT_SIZE;

	/* How big is this point array? */
	size += ptarray_to_wkb_size(tri->points, variant);

	return size;
}

static uint8_t* lwtriangle_to_wkb_buf(const LWTRIANGLE *tri, uint8_t *buf, uint8_t variant)
{
	/* Set the endian flag */
	buf = endian_to_wkb_buf(buf, variant);
	
	/* Set the geometry type */
	buf = integer_to_wkb_buf(lwgeom_wkb_type((LWGEOM*)tri, variant), buf, variant);
	
	/* Set the optional SRID for extended variant */
	if ( lwgeom_wkb_needs_srid((LWGEOM*)tri, variant) )
		buf = integer_to_wkb_buf(tri->srid, buf, variant);

	/* Set the number of rings (only one, it's a triangle, buddy) */
	buf = integer_to_wkb_buf(1, buf, variant);
	
	/* Write that ring */
	buf = ptarray_to_wkb_buf(tri->points, buf, variant);

	return buf;
}

/*
* POLYGON
*/
static size_t lwpoly_to_wkb_size(const LWPOLY *poly, uint8_t variant)
{
	/* endian flag + type number + number of rings */
	size_t size = WKB_BYTE_SIZE + WKB_INT_SIZE + WKB_INT_SIZE;
	int i = 0;

	/* Extended WKB needs space for optional SRID integer */
	if ( lwgeom_wkb_needs_srid((LWGEOM*)poly, variant) )
		size += WKB_INT_SIZE;

	for ( i = 0; i < poly->nrings; i++ )
	{
		/* Size of ring point array */
		size += ptarray_to_wkb_size(poly->rings[i], variant);
	}

	return size;
}

static uint8_t* lwpoly_to_wkb_buf(const LWPOLY *poly, uint8_t *buf, uint8_t variant)
{
	int i;

	/* Set the endian flag */
	buf = endian_to_wkb_buf(buf, variant);
	/* Set the geometry type */
	buf = integer_to_wkb_buf(lwgeom_wkb_type((LWGEOM*)poly, variant), buf, variant);
	/* Set the optional SRID for extended variant */
	if ( lwgeom_wkb_needs_srid((LWGEOM*)poly, variant) )
		buf = integer_to_wkb_buf(poly->srid, buf, variant);
	/* Set the number of rings */
	buf = integer_to_wkb_buf(poly->nrings, buf, variant);

	for ( i = 0; i < poly->nrings; i++ )
	{
		buf = ptarray_to_wkb_buf(poly->rings[i], buf, variant);
	}

	return buf;
}


/*
* MULTIPOINT, MULTILINESTRING, MULTIPOLYGON, GEOMETRYCOLLECTION
* MULTICURVE, COMPOUNDCURVE, MULTISURFACE, CURVEPOLYGON, TIN, 
* POLYHEDRALSURFACE
*/
static size_t lwcollection_to_wkb_size(const LWCOLLECTION *col, uint8_t variant)
{
	/* Endian flag + type number + number of subgeoms */
	size_t size = WKB_BYTE_SIZE + WKB_INT_SIZE + WKB_INT_SIZE;
	int i = 0;

	/* Extended WKB needs space for optional SRID integer */
	if ( lwgeom_wkb_needs_srid((LWGEOM*)col, variant) )
		size += WKB_INT_SIZE;

	for ( i = 0; i < col->ngeoms; i++ )
	{
		/* size of subgeom */
		size += lwgeom_to_wkb_size((LWGEOM*)col->geoms[i], variant | WKB_NO_SRID);
	}

	return size;
}

static uint8_t* lwcollection_to_wkb_buf(const LWCOLLECTION *col, uint8_t *buf, uint8_t variant)
{
	int i;

	/* Set the endian flag */
	buf = endian_to_wkb_buf(buf, variant);
	/* Set the geometry type */
	buf = integer_to_wkb_buf(lwgeom_wkb_type((LWGEOM*)col, variant), buf, variant);
	/* Set the optional SRID for extended variant */
	if ( lwgeom_wkb_needs_srid((LWGEOM*)col, variant) )
		buf = integer_to_wkb_buf(col->srid, buf, variant);
	/* Set the number of sub-geometries */
	buf = integer_to_wkb_buf(col->ngeoms, buf, variant);

	/* Write the sub-geometries. Sub-geometries do not get SRIDs, they
	   inherit from their parents. */
	for ( i = 0; i < col->ngeoms; i++ )
	{
		buf = lwgeom_to_wkb_buf(col->geoms[i], buf, variant | WKB_NO_SRID);
	}

	return buf;
}

/*
* GEOMETRY
*/
static size_t lwgeom_to_wkb_size(const LWGEOM *geom, uint8_t variant)
{
	size_t size = 0;

	if ( geom == NULL )
		return 0;

	/* Short circuit out empty geometries */
	if ( lwgeom_is_empty(geom) )
	{
		return empty_to_wkb_size(geom, variant);
	}

	switch ( geom->type )
	{
		case POINTTYPE:
			size += lwpoint_to_wkb_size((LWPOINT*)geom, variant);
			break;

		/* LineString and CircularString both have points elements */
		case CIRCSTRINGTYPE:
		case LINETYPE:
			size += lwline_to_wkb_size((LWLINE*)geom, variant);
			break;

		/* Polygon has nrings and rings elements */
		case POLYGONTYPE:
			size += lwpoly_to_wkb_size((LWPOLY*)geom, variant);
			break;

		/* Triangle has one ring of three points */
		case TRIANGLETYPE:
			size += lwtriangle_to_wkb_size((LWTRIANGLE*)geom, variant);
			break;

		/* All these Collection types have ngeoms and geoms elements */
		case MULTIPOINTTYPE:
		case MULTILINETYPE:
		case MULTIPOLYGONTYPE:
		case COMPOUNDTYPE:
		case CURVEPOLYTYPE:
		case MULTICURVETYPE:
		case MULTISURFACETYPE:
		case COLLECTIONTYPE:
		case POLYHEDRALSURFACETYPE:
		case TINTYPE:
			size += lwcollection_to_wkb_size((LWCOLLECTION*)geom, variant);
			break;

		/* Unknown type! */
		default:
			lwerror("Unsupported geometry type: %s [%d]", lwtype_name(geom->type), geom->type);
	}

	return size;
}

/* TODO handle the TRIANGLE type properly */

static uint8_t* lwgeom_to_wkb_buf(const LWGEOM *geom, uint8_t *buf, uint8_t variant)
{

	if ( lwgeom_is_empty(geom) )
		return empty_to_wkb_buf(geom, buf, variant);

	switch ( geom->type )
	{
		case POINTTYPE:
			return lwpoint_to_wkb_buf((LWPOINT*)geom, buf, variant);

		/* LineString and CircularString both have 'points' elements */
		case CIRCSTRINGTYPE:
		case LINETYPE:
			return lwline_to_wkb_buf((LWLINE*)geom, buf, variant);

		/* Polygon has 'nrings' and 'rings' elements */
		case POLYGONTYPE:
			return lwpoly_to_wkb_buf((LWPOLY*)geom, buf, variant);

		/* Triangle has one ring of three points */
		case TRIANGLETYPE:
			return lwtriangle_to_wkb_buf((LWTRIANGLE*)geom, buf, variant);

		/* All these Collection types have 'ngeoms' and 'geoms' elements */
		case MULTIPOINTTYPE:
		case MULTILINETYPE:
		case MULTIPOLYGONTYPE:
		case COMPOUNDTYPE:
		case CURVEPOLYTYPE:
		case MULTICURVETYPE:
		case MULTISURFACETYPE:
		case COLLECTIONTYPE:
		case POLYHEDRALSURFACETYPE:
		case TINTYPE:
			return lwcollection_to_wkb_buf((LWCOLLECTION*)geom, buf, variant);

		/* Unknown type! */
		default:
			lwerror("Unsupported geometry type: %s [%d]", lwtype_name(geom->type), geom->type);
	}
	/* Return value to keep compiler happy. */
	return 0;
}

static size_t vgeom_to_wkb_size(const VAGUEGEOM *vgeom, uint8_t variant) {
	size_t size = 0;

	if ( vgeom == NULL )
		return 0;
	/* Endian flag + type number */
	size = WKB_BYTE_SIZE + WKB_INT_SIZE;

	/* Extended WKB needs space for optional SRID integer */
	if ( lwgeom_wkb_needs_srid(vgeom->kernel, variant) )
		size += WKB_INT_SIZE;
	size += lwgeom_to_wkb_size(vgeom->kernel, variant | WKB_NO_SRID);
	size += lwgeom_to_wkb_size(vgeom->conjecture, variant | WKB_NO_SRID);
	return size;

}

static uint8_t* vgeom_to_wkb_buf(const VAGUEGEOM *vgeom, uint8_t *buf, uint8_t variant) {
	/* Set the endian flag */
	buf = endian_to_wkb_buf(buf, variant);
	/* Set the vague geometry type that will be vague multipoint type because the kernel and conjecture can be empty*/
	buf = integer_to_wkb_buf(vgeom_vwkb_type(vgeom, variant), buf, variant);
	/* Set the optional SRID for extended variant */
	if ( lwgeom_wkb_needs_srid(vgeom->kernel, variant) )
		buf = integer_to_wkb_buf(vgeom->kernel->srid, buf, variant);
	switch ( vgeom->type ) {
		case VAGUEPOINTTYPE: {			
			buf = lwpoint_to_wkb_buf((LWPOINT*)vgeom->kernel, buf, variant | WKB_NO_SRID);
			buf = lwpoint_to_wkb_buf((LWPOINT*)vgeom->conjecture, buf, variant | WKB_NO_SRID);
			return buf;
		}
		case VAGUELINETYPE: {
			buf = lwline_to_wkb_buf((LWLINE*)vgeom->kernel, buf, variant | WKB_NO_SRID);
			buf = lwline_to_wkb_buf((LWLINE*)vgeom->conjecture, buf, variant | WKB_NO_SRID);
			return buf;
		}
		case VAGUEPOLYGONTYPE: {
			buf = lwpoly_to_wkb_buf((LWPOLY*)vgeom->kernel, buf, variant | WKB_NO_SRID);
			buf = lwpoly_to_wkb_buf((LWPOLY*)vgeom->conjecture, buf, variant | WKB_NO_SRID);
			return buf;
		}
		case VAGUEMULTIPOINTTYPE: {
			buf = lwcollection_to_wkb_buf((LWCOLLECTION*)vgeom->kernel, buf, variant | WKB_NO_SRID);
			buf = lwcollection_to_wkb_buf((LWCOLLECTION*)vgeom->conjecture, buf, variant | WKB_NO_SRID);
			return buf;
		}
		case VAGUEMULTILINETYPE: {
			buf = lwcollection_to_wkb_buf((LWCOLLECTION*)vgeom->kernel, buf, variant | WKB_NO_SRID);
			buf = lwcollection_to_wkb_buf((LWCOLLECTION*)vgeom->conjecture, buf, variant | WKB_NO_SRID);
			return buf;
		}
		case VAGUEMULTIPOLYGONTYPE: {
			buf = lwcollection_to_wkb_buf((LWCOLLECTION*)vgeom->kernel, buf, variant | WKB_NO_SRID);
			buf = lwcollection_to_wkb_buf((LWCOLLECTION*)vgeom->conjecture, buf, variant | WKB_NO_SRID);
			return buf;
		}
		/* Unknown type! */
		default:
			lwerror("Unsupported vague geometry type: %s [%d]", vaguegeom_gettype(vgeom), vgeom->type);
	}
	/* Return value to keep compiler happy. */
	return 0;
}

/**
* Used for passing the parse state between the parsing functions.
*/
typedef struct 
{
	const uint8_t *wkb; /* Points to start of WKB */
	size_t wkb_size; /* Expected size of WKB */
	int swap_bytes; /* Do an endian flip? */
	int check; /* Simple validity checks on geometries */
	uint32_t lwtype; /* Current type we are handling */
	uint32_t srid; /* Current SRID we are handling */
	int has_z; /* Z? */
	int has_m; /* M? */
	int has_srid; /* SRID? */
	const uint8_t *pos; /* Current parse position */
} wkb_parse_state;


/**
* Internal function declarations.
*/
LWGEOM* lwgeom_from_wkb_state(wkb_parse_state *s);

/**********************************************************************/


/**
* Check that we are not about to read off the end of the WKB 
* array.
*/
static inline void wkb_parse_state_check(wkb_parse_state *s, size_t next)
{
	if( (s->pos + next) > (s->wkb + s->wkb_size) )
		lwerror("WKB structure does not match expected size!");
} 

/**
* Take in an unknown kind of wkb type number and ensure it comes out
* as an extended WKB type number (with Z/M/SRID flags masked onto the 
* high bits).
*/
static void lwtype_from_wkb_state(wkb_parse_state *s, uint32_t wkb_type)
{
	uint32_t wkb_simple_type;
	
	LWDEBUG(4, "Entered function");
	
	s->has_z = LW_FALSE;
	s->has_m = LW_FALSE;
	s->has_srid = LW_FALSE;

	/* If any of the higher bits are set, this is probably an extended type. */
	if( wkb_type & 0xF0000000 )
	{
		if( wkb_type & WKBZOFFSET ) s->has_z = LW_TRUE;
		if( wkb_type & WKBMOFFSET ) s->has_m = LW_TRUE;
		if( wkb_type & WKBSRIDFLAG ) s->has_srid = LW_TRUE;
		LWDEBUGF(4, "Extended type: has_z=%d has_m=%d has_srid=%d", s->has_z, s->has_m, s->has_srid);
	}
	
	/* Mask off the flags */
	wkb_type = wkb_type & 0x0FFFFFFF;
	/* Strip out just the type number (1-12) from the ISO number (eg 3001-3012) */
	wkb_simple_type = wkb_type % 1000;
	
	/* Extract the Z/M information from ISO style numbers */
	if( wkb_type >= 3000 && wkb_type < 4000 )
	{
		s->has_z = LW_TRUE;
		s->has_m = LW_TRUE;
	}
	else if ( wkb_type >= 2000 && wkb_type < 3000 )
	{
		s->has_m = LW_TRUE;
	}
	else if ( wkb_type >= 1000 && wkb_type < 2000 )
	{
		s->has_z = LW_TRUE;
	}

	switch (wkb_simple_type)
	{
		case WKB_POINT_TYPE: 
			s->lwtype = POINTTYPE;
			break;
		case WKB_LINESTRING_TYPE: 
			s->lwtype = LINETYPE;
			break;
		case WKB_POLYGON_TYPE:
			s->lwtype = POLYGONTYPE;
			break;
		case WKB_MULTIPOINT_TYPE:
			s->lwtype = MULTIPOINTTYPE;
			break;
		case WKB_MULTILINESTRING_TYPE:
			s->lwtype = MULTILINETYPE;
			break;
		case WKB_MULTIPOLYGON_TYPE:
			s->lwtype = MULTIPOLYGONTYPE;
			break;
		case WKB_GEOMETRYCOLLECTION_TYPE: 
			s->lwtype = COLLECTIONTYPE;
			break;
		case WKB_CIRCULARSTRING_TYPE:
			s->lwtype = CIRCSTRINGTYPE;
			break;
		case WKB_COMPOUNDCURVE_TYPE:
			s->lwtype = COMPOUNDTYPE;
			break;
		case WKB_CURVEPOLYGON_TYPE:
			s->lwtype = CURVEPOLYTYPE;
			break;
		case WKB_MULTICURVE_TYPE:
			s->lwtype = MULTICURVETYPE;
			break;
		case WKB_MULTISURFACE_TYPE: 
			s->lwtype = MULTISURFACETYPE;
			break;
		case WKB_POLYHEDRALSURFACE_TYPE:
			s->lwtype = POLYHEDRALSURFACETYPE;
			break;
		case WKB_TIN_TYPE:
			s->lwtype = TINTYPE;
			break;
		case WKB_TRIANGLE_TYPE:
			s->lwtype = TRIANGLETYPE;
			break;
		
		/* PostGIS 1.5 emits 13, 14 for CurvePolygon, MultiCurve */
		/* These numbers aren't SQL/MM (numbers currently only */
		/* go up to 12. We can handle the old data here (for now??) */
		/* converting them into the lwtypes that are intended. */
		case WKB_CURVE_TYPE:
			s->lwtype = CURVEPOLYTYPE;
			break;
		case WKB_SURFACE_TYPE:
			s->lwtype = MULTICURVETYPE;
			break;
		
		default: /* Error! */
			lwerror("Unknown WKB type (%d)! Full WKB type number was (%d).", wkb_simple_type, wkb_type);
			break;	
	}

	LWDEBUGF(4,"Got lwtype %s (%u)", lwtype_name(s->lwtype), s->lwtype);

	return;
}

/**
* Byte
* Read a byte and advance the parse state forward.
*/
static char byte_from_wkb_state(wkb_parse_state *s)
{
	char char_value = 0;
	LWDEBUG(4, "Entered function");

	wkb_parse_state_check(s, WKB_BYTE_SIZE);
	LWDEBUG(4, "Passed state check");
	
	char_value = s->pos[0];
	LWDEBUGF(4, "Read byte value: %x", char_value);
	s->pos += WKB_BYTE_SIZE;
	
	return char_value;
}

/**
* Int32
* Read 4-byte integer and advance the parse state forward.
*/
static uint32_t integer_from_wkb_state(wkb_parse_state *s)
{
	uint32_t i = 0;

	wkb_parse_state_check(s, WKB_INT_SIZE);
	
	memcpy(&i, s->pos, WKB_INT_SIZE);
	
	/* Swap? Copy into a stack-allocated integer. */
	if( s->swap_bytes )
	{
		int j = 0;
		uint8_t tmp;
		
		for( j = 0; j < WKB_INT_SIZE/2; j++ )
		{
			tmp = ((uint8_t*)(&i))[j];
			((uint8_t*)(&i))[j] = ((uint8_t*)(&i))[WKB_INT_SIZE - j - 1];
			((uint8_t*)(&i))[WKB_INT_SIZE - j - 1] = tmp;
		}
	}

	s->pos += WKB_INT_SIZE;
	return i;
}

/**
* Double
* Read an 8-byte double and advance the parse state forward.
*/
static double double_from_wkb_state(wkb_parse_state *s)
{
	double d = 0;

	wkb_parse_state_check(s, WKB_DOUBLE_SIZE);

	memcpy(&d, s->pos, WKB_DOUBLE_SIZE);

	/* Swap? Copy into a stack-allocated integer. */
	if( s->swap_bytes )
	{
		int i = 0;
		uint8_t tmp;
		
		for( i = 0; i < WKB_DOUBLE_SIZE/2; i++ )
		{
			tmp = ((uint8_t*)(&d))[i];
			((uint8_t*)(&d))[i] = ((uint8_t*)(&d))[WKB_DOUBLE_SIZE - i - 1];
			((uint8_t*)(&d))[WKB_DOUBLE_SIZE - i - 1] = tmp;
		}

	}

	s->pos += WKB_DOUBLE_SIZE;
	return d;
}

/**
* POINTARRAY
* Read a dynamically sized point array and advance the parse state forward.
* First read the number of points, then read the points.
*/
static POINTARRAY* ptarray_from_wkb_state(wkb_parse_state *s)
{
	POINTARRAY *pa = NULL;
	size_t pa_size;
	uint32_t ndims = 2;
	uint32_t npoints = 0;

	/* Calculate the size of this point array. */
	npoints = integer_from_wkb_state(s);
	if( s->has_z ) ndims++;
	if( s->has_m ) ndims++;
	pa_size = npoints * ndims * WKB_DOUBLE_SIZE;

	/* Empty! */
	if( npoints == 0 )
		return ptarray_construct(s->has_z, s->has_m, npoints);

	/* Does the data we want to read exist? */
	wkb_parse_state_check(s, pa_size);
	
	/* If we're in a native endianness, we can just copy the data directly! */
	if( ! s->swap_bytes )
	{
		pa = ptarray_construct_copy_data(s->has_z, s->has_m, npoints, (uint8_t*)s->pos);
		s->pos += pa_size;
	}
	/* Otherwise we have to read each double, separately. */
	else
	{
		int i = 0;
		double *dlist;
		pa = ptarray_construct(s->has_z, s->has_m, npoints);
		dlist = (double*)(pa->serialized_pointlist);
		for( i = 0; i < npoints * ndims; i++ )
		{
			dlist[i] = double_from_wkb_state(s);
		}
	}

	return pa;
}

/**
* POINT
* Read a WKB point, starting just after the endian byte, 
* type number and optional srid number.
* Advance the parse state forward appropriately.
* WKB point has just a set of doubles, with the quantity depending on the 
* dimension of the point, so this looks like a special case of the above
* with only one point.
*/
static LWPOINT* lwpoint_from_wkb_state(wkb_parse_state *s)
{
	static uint32_t npoints = 1;
	POINTARRAY *pa = NULL;
	size_t pa_size;
	uint32_t ndims = 2;

	/* Count the dimensions. */
	if( s->has_z ) ndims++;
	if( s->has_m ) ndims++;
	pa_size = ndims * WKB_DOUBLE_SIZE;

	/* Does the data we want to read exist? */
	wkb_parse_state_check(s, pa_size);

	/* If we're in a native endianness, we can just copy the data directly! */
	if( ! s->swap_bytes )
	{
		pa = ptarray_construct_copy_data(s->has_z, s->has_m, npoints, (uint8_t*)s->pos);
		s->pos += pa_size;
	}
	/* Otherwise we have to read each double, separately */
	else
	{
		int i = 0;
		double *dlist;
		pa = ptarray_construct(s->has_z, s->has_m, npoints);
		dlist = (double*)(pa->serialized_pointlist);
		for( i = 0; i < ndims; i++ )
		{
			dlist[i] = double_from_wkb_state(s);
		}
	}
	
	return lwpoint_construct(s->srid, NULL, pa);
}

/**
* LINESTRING
* Read a WKB linestring, starting just after the endian byte, 
* type number and optional srid number. Advance the parse state 
* forward appropriately. 
* There is only one pointarray in a linestring. Optionally
* check for minimal following of rules (two point minimum).
*/
static LWLINE* lwline_from_wkb_state(wkb_parse_state *s)
{
	POINTARRAY *pa = ptarray_from_wkb_state(s);

	if( pa == NULL || pa->npoints == 0 )
		return lwline_construct_empty(s->srid, s->has_z, s->has_m);

	if( s->check & LW_PARSER_CHECK_MINPOINTS && pa->npoints < 2 )
	{
		lwerror("%s must have at least two points", lwtype_name(s->lwtype));
		return NULL;
	}

	return lwline_construct(s->srid, NULL, pa);
}

/**
* CIRCULARSTRING
* Read a WKB circularstring, starting just after the endian byte, 
* type number and optional srid number. Advance the parse state 
* forward appropriately. 
* There is only one pointarray in a linestring. Optionally
* check for minimal following of rules (three point minimum,
* odd number of points).
*/
static LWCIRCSTRING* lwcircstring_from_wkb_state(wkb_parse_state *s)
{
	POINTARRAY *pa = ptarray_from_wkb_state(s);

	if( pa == NULL || pa->npoints == 0 )
		return lwcircstring_construct_empty(s->srid, s->has_z, s->has_m);

	if( s->check & LW_PARSER_CHECK_MINPOINTS && pa->npoints < 3 )
	{
		lwerror("%s must have at least three points", lwtype_name(s->lwtype));
		return NULL;
	}

	if( s->check & LW_PARSER_CHECK_ODD && ! (pa->npoints % 2) )
	{
		lwerror("%s must have an odd number of points", lwtype_name(s->lwtype));
		return NULL;
	}

	return lwcircstring_construct(s->srid, NULL, pa);	
}

/**
* POLYGON
* Read a WKB polygon, starting just after the endian byte, 
* type number and optional srid number. Advance the parse state 
* forward appropriately. 
* First read the number of rings, then read each ring
* (which are structured as point arrays)
*/
static LWPOLY* lwpoly_from_wkb_state(wkb_parse_state *s)
{
	uint32_t nrings = integer_from_wkb_state(s);
	int i = 0;
	LWPOLY *poly = lwpoly_construct_empty(s->srid, s->has_z, s->has_m);
	
	/* Empty polygon? */
	if( nrings == 0 )
		return poly;

	for( i = 0; i < nrings; i++ )
	{
		POINTARRAY *pa = ptarray_from_wkb_state(s);
		if( pa == NULL )
			continue;

		/* Check for at least four points. */
		if( s->check & LW_PARSER_CHECK_MINPOINTS && pa->npoints < 4 )
		{
			LWDEBUGF(2, "%s must have at least four points in each ring", lwtype_name(s->lwtype));
			lwerror("%s must have at least four points in each ring", lwtype_name(s->lwtype));
			return NULL;
		}

		/* Check that first and last points are the same. */
		if( s->check & LW_PARSER_CHECK_CLOSURE && ! ptarray_is_closed_2d(pa) )
		{
			LWDEBUGF(2, "%s must have closed rings", lwtype_name(s->lwtype));
			lwerror("%s must have closed rings", lwtype_name(s->lwtype));
			return NULL;
		}
		
		/* Add ring to polygon */
		if ( lwpoly_add_ring(poly, pa) == LW_FAILURE )
		{
			LWDEBUG(2, "Unable to add ring to polygon");
			lwerror("Unable to add ring to polygon");
		}

	}
	return poly;
}

/**
* TRIANGLE
* Read a WKB triangle, starting just after the endian byte, 
* type number and optional srid number. Advance the parse state 
* forward appropriately. 
* Triangles are encoded like polygons in WKB, but more like linestrings
* as lwgeometries.
*/
static LWTRIANGLE* lwtriangle_from_wkb_state(wkb_parse_state *s)
{
	uint32_t nrings = integer_from_wkb_state(s);
	LWTRIANGLE *tri = lwtriangle_construct_empty(s->srid, s->has_z, s->has_m);
	POINTARRAY *pa = NULL;

	/* Empty triangle? */
	if( nrings == 0 )
		return tri;

	/* Should be only one ring. */
	if ( nrings != 1 )
		lwerror("Triangle has wrong number of rings: %d", nrings);

	/* There's only one ring, we hope? */	
	pa = ptarray_from_wkb_state(s);

	/* If there's no points, return an empty triangle. */
	if( pa == NULL )
		return tri;

	/* Check for at least four points. */
	if( s->check & LW_PARSER_CHECK_MINPOINTS && pa->npoints < 4 )
	{
		LWDEBUGF(2, "%s must have at least four points", lwtype_name(s->lwtype));
		lwerror("%s must have at least four points", lwtype_name(s->lwtype));
		return NULL;
	}

	if( s->check & LW_PARSER_CHECK_CLOSURE && ! ptarray_is_closed(pa) )
	{
		lwerror("%s must have closed rings", lwtype_name(s->lwtype));
		return NULL;
	}

	if( s->check & LW_PARSER_CHECK_ZCLOSURE && ! ptarray_is_closed_z(pa) )
	{
		lwerror("%s must have closed rings", lwtype_name(s->lwtype));
		return NULL;
	}

	tri->points = pa;	
	return tri;
}

/**
* CURVEPOLYTYPE
*/
static LWCURVEPOLY* lwcurvepoly_from_wkb_state(wkb_parse_state *s)
{
	uint32_t ngeoms = integer_from_wkb_state(s);
	LWCURVEPOLY *cp = lwcurvepoly_construct_empty(s->srid, s->has_z, s->has_m);
	LWGEOM *geom = NULL;
	int i;
	
	/* Empty collection? */
	if ( ngeoms == 0 )
		return cp;

	for ( i = 0; i < ngeoms; i++ )
	{
		geom = lwgeom_from_wkb_state(s);
		if ( lwcurvepoly_add_ring(cp, geom) == LW_FAILURE )
			lwerror("Unable to add geometry (%p) to curvepoly (%p)", geom, cp);
	}
	
	return cp;
}

/**
* POLYHEDRALSURFACETYPE
*/

/**
* COLLECTION, MULTIPOINTTYPE, MULTILINETYPE, MULTIPOLYGONTYPE, COMPOUNDTYPE,
* MULTICURVETYPE, MULTISURFACETYPE, 
* TINTYPE
*/
static LWCOLLECTION* lwcollection_from_wkb_state(wkb_parse_state *s)
{
	uint32_t ngeoms = integer_from_wkb_state(s);
	LWCOLLECTION *col = lwcollection_construct_empty(s->lwtype, s->srid, s->has_z, s->has_m);
	LWGEOM *geom = NULL;
	int i;
	
	/* Empty collection? */
	if ( ngeoms == 0 )
		return col;

	/* Be strict in polyhedral surface closures */
	if ( s->lwtype == POLYHEDRALSURFACETYPE )
		s->check |= LW_PARSER_CHECK_ZCLOSURE;

	for ( i = 0; i < ngeoms; i++ )
	{
		geom = lwgeom_from_wkb_state(s);
		if ( lwcollection_add_lwgeom(col, geom) == NULL )
		{
			lwerror("Unable to add geometry (%p) to collection (%p)", geom, col);
			return NULL;
		}
	}
	
	return col;
}


/**
* GEOMETRY
* Generic handling for WKB geometries. The front of every WKB geometry
* (including those embedded in collections) is an endian byte, a type
* number and an optional srid number. We handle all those here, then pass
* to the appropriate handler for the specific type.
*/
LWGEOM* lwgeom_from_wkb_state(wkb_parse_state *s)
{
	char wkb_little_endian;
	uint32_t wkb_type;
	
	LWDEBUG(4,"Entered function");
	
	/* Fail when handed incorrect starting byte */
	wkb_little_endian = byte_from_wkb_state(s);
	if( wkb_little_endian != 1 && wkb_little_endian != 0 )
	{
		LWDEBUG(4,"Leaving due to bad first byte!");
		lwerror("Invalid endian flag value encountered.");
		return NULL;
	}

	/* Check the endianness of our input  */
	s->swap_bytes = LW_FALSE;
	if( getMachineEndian() == NDR ) /* Machine arch is little */
	{
		if ( ! wkb_little_endian )    /* Data is big! */
			s->swap_bytes = LW_TRUE;
	}
	else                              /* Machine arch is big */
	{
		if ( wkb_little_endian )      /* Data is little! */
			s->swap_bytes = LW_TRUE;
	}

	/* Read the type number */
	wkb_type = integer_from_wkb_state(s);
	LWDEBUGF(4,"Got WKB type number: 0x%X", wkb_type);
	lwtype_from_wkb_state(s, wkb_type);
	
	/* Read the SRID, if necessary */
	//if( s->has_srid )
	//{
	//	s->srid = clamp_srid(integer_from_wkb_state(s));
	//	/* TODO: warn on explicit UNKNOWN srid ? */
	//	LWDEBUGF(4,"Got SRID: %u", s->srid);
	//}
	
	/* Do the right thing */
	switch( s->lwtype )
	{
		case POINTTYPE:
			return (LWGEOM*)lwpoint_from_wkb_state(s);
			break;
		case LINETYPE:
			return (LWGEOM*)lwline_from_wkb_state(s);
			break;
		case CIRCSTRINGTYPE:
			return (LWGEOM*)lwcircstring_from_wkb_state(s);
			break;
		case POLYGONTYPE:
			return (LWGEOM*)lwpoly_from_wkb_state(s);
			break;
		case TRIANGLETYPE:
			return (LWGEOM*)lwtriangle_from_wkb_state(s);
			break;
		case CURVEPOLYTYPE:
			return (LWGEOM*)lwcurvepoly_from_wkb_state(s);
			break;
		case MULTIPOINTTYPE:
		case MULTILINETYPE:
		case MULTIPOLYGONTYPE:
		case COMPOUNDTYPE:
		case MULTICURVETYPE:
		case MULTISURFACETYPE:
		case POLYHEDRALSURFACETYPE:
		case TINTYPE:
		case COLLECTIONTYPE:
			return (LWGEOM*)lwcollection_from_wkb_state(s);
			break;

		/* Unknown type! */
		default:
			lwerror("Unsupported geometry type: %s [%d]", lwtype_name(s->lwtype), s->lwtype);
	}

	/* Return value to keep compiler happy. */
	return NULL;
	
}

static VAGUEGEOM *vgeom_from_vwkb_state(wkb_parse_state *s, uint8_t punion) {
	char little_endian;
	uint32_t vg_type;
	LWGEOM *kernel=NULL, *conjecture=NULL;
	
	/* Fail when handed incorrect starting byte */
	little_endian = byte_from_wkb_state(s);
	if(little_endian != 1 && little_endian != 0 ) {
		lwerror("Invalid endian flag value encountered.");
		return NULL;
	}

	/* Check the endianness of our input  */
	s->swap_bytes = LW_FALSE;
	if( getMachineEndian() == NDR ) { /* Machine arch is little */
		if ( ! little_endian )    /* Data is big! */
			s->swap_bytes = LW_TRUE;
	} else {                              /* Machine arch is big */
		if ( little_endian )      /* Data is little! */
			s->swap_bytes = LW_TRUE;
	}

	/* Read the type number */
	vg_type = integer_from_wkb_state(s); 
	if( vg_type & 0xF0000000 ) {
		if( vg_type & WKBSRIDFLAG ) s->has_srid = LW_TRUE;
	}

	/* Read the SRID, if necessary */
	if( s->has_srid ) {
		s->srid = clamp_srid(integer_from_wkb_state(s));
		/* TODO: warn on explicit UNKNOWN srid ? */
	}
	/* if there is a problem use the follow function: lwgeom_from_wkb */
	kernel = lwgeom_from_wkb_state(s);
	conjecture = lwgeom_from_wkb_state(s);
	if(kernel != NULL) {
		/*check if the kernel type is compatible with the vaguegeometry type */
		if(vaguegeom_compatible_type(vaguegeom_gettype_i(vg_type), kernel->type) ==LW_FALSE) {
			lwerror("The vague geometry type (%s) is not compatible with the kernel type (%s)", vaguegeom_gettype_i(vg_type), lwtype_name(kernel->type));
		}
	}
	if(conjecture != NULL) {
		/*check if the conjecture type is compatible with the vaguegeometry type */
		if(vaguegeom_compatible_type(vaguegeom_gettype_i(vg_type), conjecture->type) ==LW_FALSE) {
			lwerror("The vague geometry type (%s) is not compatible with the conjecture type (%s)", vaguegeom_gettype_i(vg_type), lwtype_name(conjecture->type));
		}
	}
	if(kernel==NULL && conjecture ==NULL) {
		lwerror("Error to parse the vwkb, kernel and conjecture are NULL");
		return NULL;
	}
	return vaguegeom_construct(kernel, conjecture, punion);
}


extern VAGUEGEOM *vgeom_from_vwkb(const uint8_t *vwkb, const size_t size, const char check, uint8_t punion) {
	wkb_parse_state s;
	
	/* Initialize the state appropriately */
	s.wkb = vwkb;
	s.wkb_size = size;
	s.swap_bytes = LW_FALSE;
	s.check = check;
	s.lwtype = 0;
	s.srid = SRID_UNKNOWN;
	s.has_z = LW_FALSE;
	s.has_m = LW_FALSE;
	s.has_srid = LW_FALSE;
	s.pos = vwkb;
	
	/* Hand the check catch-all values */
	if ( check & LW_PARSER_CHECK_NONE ) 
		s.check = 0;
	else
		s.check = check;
		
	return vgeom_from_vwkb_state(&s, punion);
}

extern VAGUEGEOM *vgeom_from_vhexwkb(const char *hexwkb, const char check, uint8_t punion) {
	int vhexwkb_len;
	uint8_t *vwkb;
	VAGUEGEOM *vgeom;
	
	if ( ! hexwkb )	{
		lwerror("lwgeom_from_hexwkb: null input");
		return NULL;
	}
	
	vhexwkb_len = strlen(hexwkb);
	vwkb = bytes_from_hexbytes(hexwkb, vhexwkb_len);
	vgeom = vgeom_from_vwkb(vwkb, vhexwkb_len/2, check, punion);
	lwfree(vwkb);
	return vgeom;	
}

extern uint8_t *vaguegeom_to_vwkb(const VAGUEGEOM *vgeom, uint8_t variant, size_t *size_out) {
	/*
	* the format is: fist byte is the endianness, second is the vaguegeom type, srid, kernel and conjecture
	*/
	size_t buf_size;
	uint8_t *buf = NULL;
	uint8_t *vwkb_out = NULL;
	uint8_t isvpoint = LW_FALSE;
	VAGUEGEOM *vgeom_vp=NULL;

	/* Initialize output size */
	if ( size_out ) *size_out = 0;

	if ( vgeom == NULL ) {
		lwerror("Cannot convert NULL into VWKB.");
		return NULL;
	}

	/* if the vaguegeom type is a vaguepoint and dont have a kernel or a conjecture then transform it to a vaguemultipoint because of postgis */
	if(vgeom->type == VAGUEPOINTTYPE && !HAS_BOTH(vgeom->flags)) {
		LWMPOINT *mpk, *mpc;
		isvpoint = LW_TRUE;
		
		mpk = lwmpoint_construct_empty(vgeom->kernel->srid, 0, 0);
		if(HAS_KERNEL(vgeom->flags))
			mpk = lwmpoint_add_lwpoint(mpk, (LWPOINT*)lwgeom_clone_deep(vgeom->kernel));
		
		mpc = lwmpoint_construct_empty(vgeom->conjecture->srid, 0, 0);
		if(HAS_CONJECTURE(vgeom->flags))
			mpc = lwmpoint_add_lwpoint(mpc, (LWPOINT*)lwgeom_clone_deep(vgeom->conjecture));
		
		vgeom_vp = vaguegeom_construct((LWGEOM*)mpk, (LWGEOM*)mpc, 0);
	}
	
	/* Calculate the required size of the output buffer */
	if(isvpoint==LW_TRUE)
		buf_size = vgeom_to_wkb_size(vgeom_vp, variant);
	else
		buf_size = vgeom_to_wkb_size(vgeom, variant);
	if ( buf_size == 0 ) {
		lwerror("Error calculating output VWKB buffer size.");
		return NULL;
	}

	/* Hex string takes twice as much space as binary + a null character */
	if ( variant & WKB_HEX ) {
		buf_size = 2 * buf_size + 1;
	}

	/* If neither or both variants are specified, choose the native order */
	if ( ! (variant & WKB_NDR || variant & WKB_XDR) ||
	       (variant & WKB_NDR && variant & WKB_XDR) ) {
		if ( getMachineEndian() == NDR ) 
			variant = variant | WKB_NDR;
		else
			variant = variant | WKB_XDR;
	}

	/* Allocate the buffer */
	buf = (uint8_t*)lwalloc(buf_size);

	if ( buf == NULL ) {
		lwerror("Unable to allocate %d bytes for WKB output buffer.", buf_size);
		return NULL;
	}

	/* Retain a pointer to the front of the buffer for later */
	vwkb_out = buf;

	/* Write the WKB into the output buffer */
	if(isvpoint==LW_TRUE)
		buf = vgeom_to_wkb_buf(vgeom_vp, buf, variant);
	else
		buf = vgeom_to_wkb_buf(vgeom, buf, variant);

	/* Null the last byte if this is a hex output */
	if ( variant & WKB_HEX ) {
		*buf = '\0';
		buf++;
	}

	/* The buffer pointer should now land at the end of the allocated buffer space. Let's check. */
	if ( buf_size != (buf - vwkb_out) ) {
		lwerror("Output VWKB is not the same size as the allocated buffer.");
		lwfree(vwkb_out);
		return NULL;
	}

	/* Report output size */
	if ( size_out ) *size_out = buf_size;

	if(isvpoint==LW_TRUE) {
		vaguegeom_free(vgeom_vp);
	}
	
	return vwkb_out;
}

extern char* vaguegeom_to_hexvwkb(const VAGUEGEOM *vgeom, uint8_t variant, size_t *size_out) {
	return (char*)vaguegeom_to_vwkb(vgeom, variant | WKB_HEX, size_out);
}


