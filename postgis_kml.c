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
 

/**********************************************************************
 *
 * This code reuse the lwgeom_in_kml
 *
 **********************************************************************/
#include "postgres.h"
 
#include "libvgeom.h"

#include <libxml/tree.h>
#include <libxml/parser.h>

static LWGEOM* parse_kml(xmlNodePtr node, bool *hasz);
#define KML_NS		((char *) "http://www.opengis.net/kml/2.2")

/**
 * Ability to parse KML geometry fragment and to return an LWGEOM
 * or an error message.
 */
extern VAGUEGEOM *vaguegeom_from_vkml(const char *vkml, uint8_t punion) {
	LWGEOM *kernel=NULL, *conjecture=NULL, *hkernel, *hconjecture;
	xmlDocPtr xmldoc;
	int xml_size;
	bool hasz=true;
	char *vg_type = NULL;
	xmlNodePtr xmlroot=NULL;
	xmlNodePtr xa;

	xml_size = strlen(vkml);

	/* Begin to Parse XML doc */
	xmlInitParser();
	xmldoc = xmlReadMemory(vkml, xml_size, NULL, NULL, XML_PARSE_SAX1);
	if (!xmldoc || (xmlroot = xmlDocGetRootElement(xmldoc)) == NULL)  {
		xmlFreeDoc(xmldoc);
		xmlCleanupParser();
		lwerror("invalid VKML representation");
	}
		
	while (xmlroot != NULL && (xmlroot->type != XML_ELEMENT_NODE)) xmlroot = xmlroot->next;

	if (xmlroot == NULL) lwerror("invalid VKML representation");
		
	vg_type = (char *) xmlroot->name;

	for (xa = xmlroot->children; xa != NULL; xa = xa->next) {
		if (xa->type != XML_ELEMENT_NODE) continue;
		if (xa->name == NULL) continue;

		if (!strcmp((char *) xa->name, "Kernel")) {
			kernel = parse_kml(xa->children, &hasz);
			if(kernel!=NULL) {
				/*check if the kernel type is compatible with the vaguegeometry type */
				if(vaguegeom_compatible_type(vg_type, kernel->type) ==LW_FALSE) {
					lwerror("The vague geometry type (%s) is not compatible with the kernel type (%s)", vg_type, lwtype_name(kernel->type));
				}
			}
		}
		if (!strcmp((char *) xa->name, "Conjecture")) {
			conjecture = parse_kml(xa->children, &hasz);
			if(conjecture!=NULL) {
				/*check if the kernel type is compatible with the vaguegeometry type */
				if(vaguegeom_compatible_type(vg_type, conjecture->type) ==LW_FALSE) {
					lwerror("The vague geometry type (%s) is not compatible with the kernel type (%s)", vg_type, lwtype_name(conjecture->type));
				}
			}
		}
	}	

	if(kernel == NULL || conjecture == NULL) {
		if(vg_type != NULL) {
			if( strcasecmp( vg_type, "VaguePoint" )==0 ) {
				if(kernel==NULL)
					kernel = lwgeom_construct_empty(POINTTYPE, SRID_UNKNOWN, 0, 0);
				if(conjecture==NULL)
					conjecture = lwgeom_construct_empty(POINTTYPE, SRID_UNKNOWN, 0, 0);
			} else if(strcasecmp(vg_type, "VagueLineString")==0) {
				if(kernel==NULL)
					kernel = lwgeom_construct_empty(LINETYPE, SRID_UNKNOWN, 0, 0);
				if(conjecture==NULL)
					conjecture = lwgeom_construct_empty(LINETYPE, SRID_UNKNOWN, 0, 0);
			} else if(strcasecmp(vg_type, "VaguePolygon")==0) {
				if(kernel==NULL)
					kernel = lwgeom_construct_empty(POLYGONTYPE, SRID_UNKNOWN, 0, 0);
				if(conjecture==NULL)
					conjecture = lwgeom_construct_empty(POLYGONTYPE, SRID_UNKNOWN, 0, 0);
			} else if(strcasecmp(vg_type, "VagueMultiPoint")==0) {
				if(kernel==NULL)
					kernel = lwgeom_construct_empty(MULTIPOINTTYPE, SRID_UNKNOWN, 0, 0);
				if(conjecture==NULL)
					conjecture = lwgeom_construct_empty(MULTIPOINTTYPE, SRID_UNKNOWN, 0, 0);
			} else if(strcasecmp(vg_type, "VagueMultiLineString")==0) {
				if(kernel==NULL)
					kernel = lwgeom_construct_empty(MULTILINETYPE, SRID_UNKNOWN, 0, 0);
				if(conjecture==NULL)
					conjecture = lwgeom_construct_empty(MULTILINETYPE, SRID_UNKNOWN, 0, 0);
			} else if(strcasecmp(vg_type, "VagueMultiPolygon") ==0) {
				if(kernel==NULL)
					kernel = lwgeom_construct_empty(MULTIPOLYGONTYPE, SRID_UNKNOWN, 0, 0);
				if(conjecture==NULL)
					conjecture = lwgeom_construct_empty(MULTIPOLYGONTYPE, SRID_UNKNOWN, 0, 0);
			}else {
				lwerror("Invalid VagueGeometry type");
			}
		}else {
			lwerror("Invalid VagueGeometry type, please a type must be setted");
		}
	}

	xmlFreeDoc(xmldoc);
	xmlCleanupParser();

	if(kernel!=NULL) {
		LWGEOM *tmp;
		/* Homogenize geometry result if needed */
		if (kernel->type == COLLECTIONTYPE) {
			hkernel = lwgeom_homogenize(kernel);
			lwgeom_release(kernel);
			kernel = hkernel;
		}
		lwgeom_add_bbox(kernel);
		tmp = lwgeom_force_2d(kernel);
		lwgeom_free(kernel);
		kernel = tmp;		
	}

	if(conjecture!=NULL) {
		LWGEOM *tmp;
		/* Homogenize geometry result if needed */
		if (conjecture->type == COLLECTIONTYPE) {
			hconjecture = lwgeom_homogenize(conjecture);
			lwgeom_release(conjecture);
			conjecture = hconjecture;
		}
		lwgeom_add_bbox(conjecture);
		tmp = lwgeom_force_2d(conjecture);
		lwgeom_free(conjecture);
		conjecture = tmp;		
	}

	return vaguegeom_construct(kernel, conjecture, punion);
}


/**
 * Return false if current element namespace is not a KML one
 * Return true otherwise.
 */
static bool is_kml_namespace(xmlNodePtr xnode, bool is_strict)
{
	xmlNsPtr *ns, *p;

	ns = xmlGetNsList(xnode->doc, xnode);
	/*
	 * If no namespace is available we could return true anyway
	 * (because we work only on KML fragment, we don't want to
	 *  'oblige' to add namespace on the geometry root node)
	 */
	if (ns == NULL) return !is_strict;

        for (p=ns ; *p ; p++)
        {
                if ((*p)->href == NULL || (*p)->prefix == NULL ||
                     xnode->ns == NULL || xnode->ns->prefix == NULL) continue;

                if (!xmlStrcmp(xnode->ns->prefix, (*p)->prefix))
                {
                        if (!strcmp((char *) (*p)->href, KML_NS))
                        {
                                xmlFree(ns);
                                return true;
                        } else {
                                xmlFree(ns);
                                return false;
                        }
                }
        }

	xmlFree(ns);
	return !is_strict; /* Same reason here to not return false */;
}


/* Temporarily disabling unused function. */
#if 0
/**
 * Retrieve a KML propertie from a node or NULL otherwise
 * Respect namespaces if presents in the node element
 */
static xmlChar *kmlGetProp(xmlNodePtr xnode, xmlChar *prop)
{
	xmlChar *value;

	if (!is_kml_namespace(xnode, true))
		return xmlGetProp(xnode, prop);

	value = xmlGetNsProp(xnode, prop, (xmlChar *) KML_NS);

	/* In last case try without explicit namespace */
	if (value == NULL) value = xmlGetNoNsProp(xnode, prop);

	return value;
}
#endif


/**
 * Parse a string supposed to be a double
 */
static double parse_kml_double(char *d, bool space_before, bool space_after)
{
	char *p;
	int st;
	enum states
	{
		INIT     	= 0,
		NEED_DIG  	= 1,
		DIG	  	= 2,
		NEED_DIG_DEC 	= 3,
		DIG_DEC 	= 4,
		EXP	 	= 5,
		NEED_DIG_EXP 	= 6,
		DIG_EXP 	= 7,
		END 		= 8
	};

	/*
	 * Double pattern
	 * [-|\+]?[0-9]+(\.)?([0-9]+)?([Ee](\+|-)?[0-9]+)?
	 * We could also meet spaces before and/or after
	 * this pattern upon parameters
	 */

	if (space_before) while (isspace(*d)) d++;
	for (st = INIT, p = d ; *p ; p++)
	{

		if (isdigit(*p))
		{
			if (st == INIT || st == NEED_DIG) 	st = DIG;
			else if (st == NEED_DIG_DEC) 			st = DIG_DEC;
			else if (st == NEED_DIG_EXP || st == EXP) 	st = DIG_EXP;
			else if (st == DIG || st == DIG_DEC || st == DIG_EXP);
			else lwerror("invalid KML representation");
		}
		else if (*p == '.')
		{
			if      (st == DIG) 				st = NEED_DIG_DEC;
			else    lwerror("invalid KML representation");
		}
		else if (*p == '-' || *p == '+')
		{
			if      (st == INIT) 				st = NEED_DIG;
			else if (st == EXP) 				st = NEED_DIG_EXP;
			else    lwerror("invalid KML representation");
		}
		else if (*p == 'e' || *p == 'E')
		{
			if      (st == DIG || st == DIG_DEC) 		st = EXP;
			else    lwerror("invalid KML representation");
		}
		else if (isspace(*p))
		{
			if (!space_after) lwerror("invalid KML representation");
			if (st == DIG || st == DIG_DEC || st == DIG_EXP)st = END;
			else if (st == NEED_DIG_DEC)			st = END;
			else if (st == END);
			else    lwerror("invalid KML representation");
		}
		else  lwerror("invalid KML representation");
	}

	if (st != DIG && st != NEED_DIG_DEC && st != DIG_DEC && st != DIG_EXP && st != END)
		lwerror("invalid KML representation");

	return atof(d);
}


/**
 * Parse kml:coordinates
 */
static POINTARRAY* parse_kml_coordinates(xmlNodePtr xnode, bool *hasz)
{
	xmlChar *kml_coord;
	bool digit, found;
	POINTARRAY *dpa;
	int kml_dims;
	char *p, *q;
	POINT4D pt;

	if (xnode == NULL) lwerror("invalid KML representation");

	for (found = false ; xnode != NULL ; xnode = xnode->next)
	{
		if (xnode->type != XML_ELEMENT_NODE) continue;
		if (!is_kml_namespace(xnode, false)) continue;
		if (strcmp((char *) xnode->name, "coordinates")) continue;

		found = true;
		break;
	}
	if (!found) lwerror("invalid KML representation");

	/* We begin to retrieve coordinates string */
	kml_coord = xmlNodeGetContent(xnode);
	p = (char *) kml_coord;

	/* KML coordinates pattern:     x1,y1 x2,y2
	 *                              x1,y1,z1 x2,y2,z2
	*/

	/* Now we create PointArray from coordinates values */
	/* HasZ, !HasM, 1pt */
	dpa = ptarray_construct_empty(1, 0, 1);

	for (q = p, kml_dims=0, digit = false ; *p ; p++)
	{

		if (isdigit(*p)) digit = true;  /* One state parser */

		/* Coordinate Separator */
		if (*p == ',')
		{
			*p = '\0';
			kml_dims++;

			if (*(p+1) == '\0') lwerror("invalid KML representation");

			if      (kml_dims == 1) pt.x = parse_kml_double(q, true, true);
			else if (kml_dims == 2) pt.y = parse_kml_double(q, true, true);
			q = p+1;

			/* Tuple Separator (or end string) */
		}
		else if (digit && (isspace(*p) || *(p+1) == '\0'))
		{
			if (isspace(*p)) *p = '\0';
			kml_dims++;

			if (kml_dims < 2 || kml_dims > 3)
				lwerror("invalid KML representation");

			if (kml_dims == 3)
				pt.z = parse_kml_double(q, true, true);
			else
			{
				pt.y = parse_kml_double(q, true, true);
				*hasz = false;
			}

			ptarray_append_point(dpa, &pt, LW_FALSE);
			digit = false;
			q = p+1;
			kml_dims = 0;

		}
	}

	xmlFree(kml_coord);

	/* TODO: we shouldn't need to clone here */
	return ptarray_clone_deep(dpa);
}


/**
 * Parse KML point
 */
static LWGEOM* parse_kml_point(xmlNodePtr xnode, bool *hasz)
{
	POINTARRAY *pa;

	if (xnode->children == NULL) lwerror("invalid KML representation");
	pa = parse_kml_coordinates(xnode->children, hasz);
	if (pa->npoints != 1) lwerror("invalid KML representation");

	return (LWGEOM *) lwpoint_construct(4326, NULL, pa);
}


/**
 * Parse KML lineString
 */
static LWGEOM* parse_kml_line(xmlNodePtr xnode, bool *hasz)
{
	POINTARRAY *pa;

	if (xnode->children == NULL) lwerror("invalid KML representation");
	pa = parse_kml_coordinates(xnode->children, hasz);
	if (pa->npoints < 2) lwerror("invalid KML representation");

	return (LWGEOM *) lwline_construct(4326, NULL, pa);
}


/**
 * Parse KML Polygon
 */
static LWGEOM* parse_kml_polygon(xmlNodePtr xnode, bool *hasz)
{
	int ring;
	xmlNodePtr xa, xb;
	POINTARRAY **ppa = NULL;

	for (xa = xnode->children ; xa != NULL ; xa = xa->next)
	{

		/* Polygon/outerBoundaryIs */
		if (xa->type != XML_ELEMENT_NODE) continue;
		if (!is_kml_namespace(xa, false)) continue;
		if (strcmp((char *) xa->name, "outerBoundaryIs")) continue;

		for (xb = xa->children ; xb != NULL ; xb = xb->next)
		{

			if (xb->type != XML_ELEMENT_NODE) continue;
			if (!is_kml_namespace(xb, false)) continue;
			if (strcmp((char *) xb->name, "LinearRing")) continue;

			ppa = (POINTARRAY**) lwalloc(sizeof(POINTARRAY*));
			ppa[0] = parse_kml_coordinates(xb->children, hasz);

			if (ppa[0]->npoints < 4
			        || (!*hasz && !ptarray_is_closed_2d(ppa[0]))
			        ||  (*hasz && !ptarray_is_closed_3d(ppa[0])))
				lwerror("invalid KML representation");
		}
	}

	for (ring=1, xa = xnode->children ; xa != NULL ; xa = xa->next)
	{

		/* Polygon/innerBoundaryIs */
		if (xa->type != XML_ELEMENT_NODE) continue;
		if (!is_kml_namespace(xa, false)) continue;
		if (strcmp((char *) xa->name, "innerBoundaryIs")) continue;

		for (xb = xa->children ; xb != NULL ; xb = xb->next)
		{

			if (xb->type != XML_ELEMENT_NODE) continue;
			if (!is_kml_namespace(xb, false)) continue;
			if (strcmp((char *) xb->name, "LinearRing")) continue;

			ppa = (POINTARRAY**) lwrealloc((POINTARRAY *) ppa,
			                               sizeof(POINTARRAY*) * (ring + 1));
			ppa[ring] = parse_kml_coordinates(xb->children, hasz);

			if (ppa[ring]->npoints < 4
			        || (!*hasz && !ptarray_is_closed_2d(ppa[ring]))
			        ||  (*hasz && !ptarray_is_closed_3d(ppa[ring])))
				lwerror("invalid KML representation");

			ring++;
		}
	}

	/* Exterior Ring is mandatory */
	if (ppa == NULL || ppa[0] == NULL) lwerror("invalid KML representation");

	return (LWGEOM *) lwpoly_construct(4326, NULL, ring, ppa);
}


/**
 * Parse KML MultiGeometry
 */
static LWGEOM* parse_kml_multi(xmlNodePtr xnode, bool *hasz)
{
	LWGEOM *geom;
	xmlNodePtr xa;

	geom = (LWGEOM *)lwcollection_construct_empty(COLLECTIONTYPE, 4326, 1, 0);

	for (xa = xnode->children ; xa != NULL ; xa = xa->next)
	{

		if (xa->type != XML_ELEMENT_NODE) continue;
		if (!is_kml_namespace(xa, false)) continue;

		if (	   !strcmp((char *) xa->name, "Point")
		        || !strcmp((char *) xa->name, "LineString")
		        || !strcmp((char *) xa->name, "Polygon")
		        || !strcmp((char *) xa->name, "MultiGeometry"))
		{

			if (xa->children == NULL) break;
			geom = (LWGEOM*)lwcollection_add_lwgeom((LWCOLLECTION*)geom, parse_kml(xa, hasz));
		}
	}

	return geom;
}


/**
 * Parse KML
 */
static LWGEOM* parse_kml(xmlNodePtr xnode, bool *hasz) {
	xmlNodePtr xa = xnode;

	while (xa != NULL && (xa->type != XML_ELEMENT_NODE
	                      || !is_kml_namespace(xa, false))) xa = xa->next;

	if (xa == NULL) lwerror("invalid KML representation");

	if (!strcmp((char *) xa->name, "Point"))
		return parse_kml_point(xa, hasz);

	if (!strcmp((char *) xa->name, "LineString"))
		return parse_kml_line(xa, hasz);

	if (!strcmp((char *) xa->name, "Polygon"))
		return parse_kml_polygon(xa, hasz);

	if (!strcmp((char *) xa->name, "MultiGeometry"))
		return parse_kml_multi(xa, hasz);

	lwerror("invalid KML representation");
	return NULL; /* Never reach */
}
