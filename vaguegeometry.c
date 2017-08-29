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
* Basic VagueGeometry functions 
*
*************************/
 
#include "libvgeom.h"

/* operations commons of vague geometry */
VAGUEGEOM *vaguegeom_construct_empty(uint8_t punion) {
	VAGUEGEOM *vgeom = (VAGUEGEOM*)lwalloc(sizeof(VAGUEGEOM));
	vgeom->conjecture = lwgeom_construct_empty(COLLECTIONTYPE, SRID_DEFAULT, 0, 0);
	vgeom->kernel = lwgeom_construct_empty(COLLECTIONTYPE, SRID_DEFAULT, 0, 0);
	FLAGS_SET_KERNEL(vgeom->flags, 0);
	FLAGS_SET_CONJECTURE(vgeom->flags, 0);
	if(punion>0)
		FLAGS_SET_PUNION(vgeom->flags, 1);
	else
		FLAGS_SET_PUNION(vgeom->flags, 0);
	vgeom->type = VAGUECOLLECTIONTYPE;
	return vgeom;
}

uint8_t settype_fromlwgeom(uint8_t type) {
	switch (type)
	{
	case POINTTYPE:
		return VAGUEPOINTTYPE;
	case LINETYPE:
		return VAGUELINETYPE;
	case POLYGONTYPE:
		return VAGUEPOLYGONTYPE;
	case MULTIPOINTTYPE:
		return VAGUEMULTIPOINTTYPE;
	case MULTILINETYPE:
		return VAGUEMULTILINETYPE;
	case MULTIPOLYGONTYPE:
		return VAGUEMULTIPOLYGONTYPE;
	//case COLLECTIONTYPE:
	//	return VAGUECOLLECTIONTYPE;
	default:
		assert(0);
	}
}

VAGUEGEOM *vaguegeom_construct(LWGEOM *kernel, LWGEOM *conjecture, uint8_t punion) {
	VAGUEGEOM *vgeom = (VAGUEGEOM*)lwalloc(sizeof(VAGUEGEOM));
	
	if(kernel==NULL && conjecture==NULL) {
		assert(0);
		return NULL;
		//return vaguegeom_construct_empty(punion);
	} else {
		if(conjecture==NULL)
			vgeom->conjecture = lwgeom_construct_empty(kernel->type, kernel->srid, FLAGS_GET_Z(kernel->flags), FLAGS_GET_M(kernel->flags));
		else
			vgeom->conjecture = conjecture;
		
		if(kernel==NULL)
			vgeom->kernel = lwgeom_construct_empty(conjecture->type, conjecture->srid, 0, 0);
		else 
			vgeom->kernel = kernel;
		
		assert(kernel->type==conjecture->type); //die if kernel and conjecture are the different types
		
		if(lwgeom_is_empty(vgeom->kernel))
			FLAGS_SET_KERNEL(vgeom->flags, 0);
		else
			FLAGS_SET_KERNEL(vgeom->flags, 1);
		if(lwgeom_is_empty(vgeom->conjecture))
			FLAGS_SET_CONJECTURE(vgeom->flags, 0);
		else
			FLAGS_SET_CONJECTURE(vgeom->flags, 1);
		if(punion>0)
			FLAGS_SET_PUNION(vgeom->flags, 1);
		else
			FLAGS_SET_PUNION(vgeom->flags, 0);
		vgeom->type = settype_fromlwgeom(vgeom->kernel->type);

		//pre compute its union, after when necessary, stores if the union is precomputed or not
		if(HAS_PUNION(vgeom->flags)) {			
			vgeom->allextension = lwgeom_union(vgeom->kernel, vgeom->conjecture);
		} else {
			vgeom->allextension = NULL;
		}
		return vgeom;
	}
}

VAGUEGEOM *vaguegeom_construct_precompunion(LWGEOM *kernel, LWGEOM *conjecture, LWGEOM *allextension) {
	if(kernel == NULL || conjecture == NULL) {
		return vaguegeom_construct(kernel, conjecture, 1);
	} else if (lwgeom_is_empty(kernel) || lwgeom_is_empty(conjecture)) {
		return vaguegeom_construct(kernel, conjecture, 1);
	} else {
		VAGUEGEOM *vgeom = (VAGUEGEOM*)lwalloc(sizeof(VAGUEGEOM));
		assert(kernel->type==conjecture->type); //die if kernel and conjecture are the different types
		FLAGS_SET_KERNEL(vgeom->flags, 1);
		FLAGS_SET_CONJECTURE(vgeom->flags, 1);
		FLAGS_SET_PUNION(vgeom->flags, 1);
		vgeom->type = settype_fromlwgeom(kernel->type);
		vgeom->kernel = kernel;
		vgeom->conjecture = conjecture;
		vgeom->allextension = allextension;
		return vgeom;
	}
}
//TO-DO be careful about lines...
static uint8_t kernel_and_conjecture_is_valid(LWGEOM *kernel, LWGEOM *conjecture) {
	GEOSGeometry *k, *c;
	uint8_t result;
	initGEOS(lwnotice, lwgeom_geos_error);
	if(kernel==NULL || conjecture==NULL)
		return LW_FALSE;
	if(lwgeom_is_empty(kernel) || lwgeom_is_empty(conjecture))
		return LW_TRUE;
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
	k = LWGEOM2GEOS(kernel, 0);
	c = LWGEOM2GEOS(conjecture, 0);
#else
	k = LWGEOM2GEOS(kernel);
	c = LWGEOM2GEOS(conjecture);
#endif
	if(GEOSDisjoint(k, c) || GEOSTouches(k, c))
		result = LW_TRUE;
	else if((kernel->type == MULTILINETYPE || kernel->type == LINETYPE) && GEOSCrosses(k, c))
		//perform the difference between conjecture and kernel here?
		result = LW_TRUE;
	else
		result = LW_FALSE;
	GEOSGeom_destroy(k);
	GEOSGeom_destroy(c);
	return result;
}

uint8_t vaguegeom_is_valid(const VAGUEGEOM *vgeom) {
	return kernel_and_conjecture_is_valid(vgeom->kernel, vgeom->conjecture);
}

/* the vaguegeometry type in text format and the crisp postgis type */
uint8_t vaguegeom_compatible_type(const char* type, uint8_t crisp_type) {
	if((strcasecmp(type, "VAGUEPOINT")==0 || strcasecmp(type, "VaguePoint")==0)
		&& crisp_type == POINTTYPE) {
			return LW_TRUE;
	} else if ((strcasecmp(type, "VAGUELINESTRING")==0 || strcasecmp(type, "VagueLineString")==0)
		&& crisp_type == LINETYPE) {
			return LW_TRUE;
	} else if ((strcasecmp(type, "VAGUEPOLYGON")==0 || strcasecmp(type, "VaguePolygon")==0)
		&& crisp_type == POLYGONTYPE) {
			return LW_TRUE;
	} else if((strcasecmp(type, "VAGUEMULTIPOINT")==0 || strcasecmp(type, "VagueMultiPoint")==0)
		&& crisp_type == MULTIPOINTTYPE) {
			return LW_TRUE;
	} else if ((strcasecmp(type, "VAGUEMULTILINESTRING")==0 || strcasecmp(type, "VagueMultiLineString")==0)
		&& crisp_type == MULTILINETYPE) {
			return LW_TRUE;
	} else if ((strcasecmp(type, "VAGUEMULTIPOLYGON")==0 || strcasecmp(type, "VagueMultiPolygon")==0)
		&& crisp_type == MULTIPOLYGONTYPE) {
			return LW_TRUE;
	} else if ((strcasecmp(type, "VAGUEGEOMETRYCOLLECTION")==0 || strcasecmp(type, "VagueGeometryCollection")==0)
		&& crisp_type == COLLECTIONTYPE) {
			return LW_TRUE;
	}
	return LW_FALSE;
}


LWGEOM *vaguegeom_getkernel(const VAGUEGEOM *vgeom) {
	if(HAS_KERNEL(vgeom->flags))
		return vgeom->kernel;
	else
		return NULL;
}

LWGEOM *vaguegeom_getconjecture(const VAGUEGEOM *vgeom) {
	if(HAS_CONJECTURE(vgeom->flags))
		return vgeom->kernel;
	else
		return NULL;
}

/*set kernel */
void vaguegeom_setkernel(VAGUEGEOM *vgeom, LWGEOM *kernel) {
	if(kernel == NULL) {
		if(!HAS_CONJECTURE(vgeom->flags)) {
			assert(0);
		} 
	} else {		
		if(HAS_CONJECTURE(vgeom->flags))  {
			if(vgeom->conjecture->type != kernel->type)
				assert(0);
		}
		if(HAS_NOTHING(vgeom->flags))
			vgeom->type = settype_fromlwgeom(kernel->type);
	}
	if(HAS_KERNEL(vgeom->flags))
		lwgeom_free(vgeom->kernel);	
	if(kernel ==NULL)
		vgeom->kernel = lwgeom_construct_empty(vgeom->conjecture->type, vgeom->conjecture->srid, 
				FLAGS_GET_Z(vgeom->conjecture->flags), FLAGS_GET_M(vgeom->conjecture->flags));
	else
		vgeom->kernel = kernel;

	if(HAS_PUNION(vgeom->flags)) {
		vgeom->allextension = lwgeom_union(vgeom->kernel, vgeom->conjecture);
	}
	if(lwgeom_is_empty(vgeom->kernel))
		FLAGS_SET_KERNEL(vgeom->flags, 0);
	else
		FLAGS_SET_KERNEL(vgeom->flags, 1);
	vgeom->kernel = kernel;
}

/*set conjecture */
void vaguegeom_setconjecture(VAGUEGEOM *vgeom, LWGEOM *conjecture) {
	if(conjecture == NULL) {
		if(!HAS_KERNEL(vgeom->flags)) {
			assert(0);
		} 
	} else {		
		if(HAS_KERNEL(vgeom->flags))  {
			if(vgeom->kernel->type != conjecture->type)
				assert(0);
		}
		if(HAS_NOTHING(vgeom->flags))
			vgeom->type = settype_fromlwgeom(conjecture->type);
	}
	if(HAS_CONJECTURE(vgeom->flags))
		lwgeom_free(vgeom->conjecture);	
	if(conjecture ==NULL)
		vgeom->conjecture = lwgeom_construct_empty(vgeom->kernel->type, vgeom->kernel->srid, 
			FLAGS_GET_Z(vgeom->kernel->flags), FLAGS_GET_M(vgeom->kernel->flags));
	else
		vgeom->conjecture = conjecture;

	if(HAS_PUNION(vgeom->flags)) {
		vgeom->allextension = lwgeom_union(vgeom->kernel, vgeom->conjecture);
	}
	if(lwgeom_is_empty(vgeom->conjecture))
		FLAGS_SET_CONJECTURE(vgeom->flags, 0);
	else
		FLAGS_SET_CONJECTURE(vgeom->flags, 1);
	vgeom->conjecture = conjecture;
}

int vaguegeom_getsrid(const VAGUEGEOM *vg) {
	if(HAS_KERNEL(vg->flags))
		return vg->kernel->srid;
	else if(HAS_CONJECTURE(vg->flags))
		return vg->conjecture->srid;
	else
		return SRID_UNKNOWN;
}

VAGUEGEOM *vaguegeom_clone(const VAGUEGEOM *vg) {
	VAGUEGEOM *vgeom = (VAGUEGEOM*)lwalloc(sizeof(VAGUEGEOM));
	vgeom->flags = vg->flags;
	vgeom->type = vg->type;
	vgeom->conjecture = lwgeom_clone_deep(vg->conjecture);
	vgeom->kernel = lwgeom_clone_deep(vg->kernel);
	if(HAS_PUNION(vgeom->flags)) {
		vgeom->allextension = lwgeom_clone_deep(vg->allextension);
	} else {
		vgeom->allextension = NULL;
	}
	return vgeom;
}

/* textual vague type */
char *vaguegeom_gettype(const VAGUEGEOM *vgeom) {
	return vaguegeom_gettype_i(vgeom->type);
}

char *vaguegeom_gettype_i(uint8_t type) {
	switch (type)
	{
	case VAGUEPOINTTYPE:
		return "VAGUEPOINT";
	case VAGUELINETYPE:
		return "VAGUELINESTRING";
	case VAGUEPOLYGONTYPE:
		return "VAGUEPOLYGON";
	case VAGUEMULTIPOINTTYPE:
		return "VAGUEMULTIPOINT";
	case VAGUEMULTILINETYPE:
		return "VAGUEMULTILINESTRING";
	case VAGUEMULTIPOLYGONTYPE:
		return "VAGUEMULTIPOLYGON";
	case VAGUECOLLECTIONTYPE:
		return "VAGUEGEOMETRYCOLLECTION";
	default:
		return "\0";
	}
	return '\0';
}

char *vaguegeom_gettype_i_notcaps(uint8_t type) {
	switch (type)
	{
	case VAGUEPOINTTYPE:
		return "VaguePoint";
	case VAGUELINETYPE:
		return "VagueLineString";
	case VAGUEPOLYGONTYPE:
		return "VaguePolygon";
	case VAGUEMULTIPOINTTYPE:
		return "VagueMultiPoint";
	case VAGUEMULTILINETYPE:
		return "VagueMultiLineString";
	case VAGUEMULTIPOLYGONTYPE:
		return "VagueMultiPolygon";
	case VAGUECOLLECTIONTYPE:
		return "VagueGeometryCollection";
	default:
		return "\0";
	}
	return '\0';
}

uint8_t vaguegeom_gettype_char(char *type) {
	if(strcasecmp(type, "VAGUEPOINT")==0) {
		return VAGUEPOINTTYPE;
	} else if (strcasecmp(type, "VAGUELINESTRING")==0) {
		return VAGUELINETYPE;
	} else if (strcasecmp(type, "VAGUEPOLYGON")==0) {
		return VAGUEPOLYGONTYPE;
	} else if(strcasecmp(type, "VAGUEMULTIPOINT")==0) {
		return VAGUEMULTIPOINTTYPE;
	} else if(strcasecmp(type, "VAGUEMULTILINESTRING")==0) {
		return VAGUEMULTILINETYPE;
	} else if(strcasecmp(type, "VAGUEMULTIPOLYGON") == 0) {
		return VAGUEMULTIPOLYGONTYPE;
	} else if(strcasecmp(type, "VAGUEGEOMETRYCOLLECTION")==0) {
		return VAGUECOLLECTIONTYPE;
	}else {
		return -1;
	}
}

void vaguegeom_free(VAGUEGEOM *vgeom) {
	if(vgeom!=NULL) {
		lwgeom_free(vgeom->kernel);
		lwgeom_free(vgeom->conjecture);
		lwfree(vgeom);
	}
}
