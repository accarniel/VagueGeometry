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
* Internal implementations of the vague topological predicates
*
*************************/

#include "libvgeom.h"

/*
* The topological relationship between two gbox objects
*/

/*
* Is g1 inside of g2? in this case it means also that g1 is covered by g2?
*/
int gbox_inside_2d(const GBOX *g1, const GBOX *g2) {
	/* Make sure our boxes are consistent */
	if ( FLAGS_GET_GEODETIC(g1->flags) != FLAGS_GET_GEODETIC(g2->flags) )
		lwerror("gbox_inside: cannot compare geodetic and non-geodetic boxes");

	/* Check X/Y */
	if ( g1->xmin < g2->xmin || g1->ymin < g2->ymin ||
	     g1->xmax > g2->xmax || g1->ymax > g2->ymax )
		return LW_FALSE;
		
	return LW_TRUE;
}

void computePQRS(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2, char *p, char *q, char *r, char *s);
/*
P = vgeom1->kernel X vgeom2->kernel
Q = vgeom1->kernel union vgeom1->conjecture X vgeom2->kernel
R = vgeom1->kernel X vgeom2->kernel union vgeom2->conjecture
S = vgeom1->kernel union vgeom1->conjecture X vgeom2->kernel union vgeom2->conjecture

Where X is the intersection matrix
P, Q, R and S will a array of characters
*/
void computePQRS(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2, char *p, char *q, char *r, char *s) {
	GEOSGeometry *kernel_g1=NULL, *kernel_g2=NULL, *conjecture_g1=NULL, *conjecture_g2=NULL, *union1=NULL, *union2=NULL;
	if(p!=NULL) {		
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
		kernel_g1 = LWGEOM2GEOS(vgeom1->kernel, 0);
		kernel_g2 = LWGEOM2GEOS(vgeom2->kernel, 0);
#else
		kernel_g1 = LWGEOM2GEOS(vgeom1->kernel);
		kernel_g2 = LWGEOM2GEOS(vgeom2->kernel);
#endif
		snprintf(p, 9, "%s", GEOSRelate(kernel_g1, kernel_g2));
	}
	if(q!=NULL) {
		if(HAS_PUNION(vgeom1->flags)) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			union1 = LWGEOM2GEOS(vgeom1->allextension, 0);
#else
			union1 = LWGEOM2GEOS(vgeom1->allextension);
#endif
		} else {
			if(kernel_g1==NULL) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
				kernel_g1 = LWGEOM2GEOS(vgeom1->kernel, 0);
#else
				kernel_g1 = LWGEOM2GEOS(vgeom1->kernel);
#endif
			}
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture, 0);
#else
			conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture);
#endif
			union1 = GEOSUnion(kernel_g1, conjecture_g1);
		}
		if(kernel_g2==NULL) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			kernel_g2 = LWGEOM2GEOS(vgeom2->kernel, 0);
#else
			kernel_g2 = LWGEOM2GEOS(vgeom2->kernel);
#endif
		}
		snprintf(q, 9, "%s", GEOSRelate(union1, kernel_g2));
	}
	if(r!=NULL) {
		if(HAS_PUNION(vgeom2->flags)) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			union2 = LWGEOM2GEOS(vgeom2->allextension, 0);
#else
			union2 = LWGEOM2GEOS(vgeom2->allextension);
#endif
		} else {
			if(kernel_g2==NULL) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
				kernel_g2 = LWGEOM2GEOS(vgeom2->kernel, 0);
#else
				kernel_g2 = LWGEOM2GEOS(vgeom2->kernel);
#endif
			}
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture, 0);
#else
			conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture);
#endif
			union2 = GEOSUnion(kernel_g2, conjecture_g2);
		}
		if(kernel_g1==NULL) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			kernel_g1 = LWGEOM2GEOS(vgeom1->kernel, 0);
#else
			kernel_g1 = LWGEOM2GEOS(vgeom1->kernel);
#endif
		}
		snprintf(r, 9, "%s", GEOSRelate(kernel_g1, union2));
	}
	if(s!=NULL) {
		if(union1==NULL) {
			if(HAS_PUNION(vgeom1->flags)) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
				union1 = LWGEOM2GEOS(vgeom1->allextension, 0);
#else
				union1 = LWGEOM2GEOS(vgeom1->allextension);
#endif
			} else {
				if(kernel_g1==NULL) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
					kernel_g1 = LWGEOM2GEOS(vgeom1->kernel, 0);
#else
					kernel_g1 = LWGEOM2GEOS(vgeom1->kernel);
#endif					
				}
				if(conjecture_g1==NULL) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
					conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture, 0);
#else
					conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture);
#endif	
				}
				union1 = GEOSUnion(kernel_g1, conjecture_g1);
			}
		}
		if(union2==NULL) {
			if(HAS_PUNION(vgeom2->flags)) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
				union2 = LWGEOM2GEOS(vgeom2->allextension, 0);
#else
				union2 = LWGEOM2GEOS(vgeom2->allextension);
#endif
			} else {
				if(kernel_g2==NULL) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
					kernel_g2 = LWGEOM2GEOS(vgeom2->kernel, 0);
#else
					kernel_g2 = LWGEOM2GEOS(vgeom2->kernel);
#endif	
				}
				if(conjecture_g2==NULL) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
					conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture, 0);
#else
					conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture);
#endif	
				}
				union2 = GEOSUnion(kernel_g2, conjecture_g2);
			}
		}
		snprintf(s, 9, "%s", GEOSRelate(union1, union2));
	}
	/* memory free */
	if(kernel_g1!=NULL)
		GEOSGeom_destroy(kernel_g1);
	if(kernel_g2!=NULL)
		GEOSGeom_destroy(kernel_g2);
	if(conjecture_g1!=NULL)
		GEOSGeom_destroy(conjecture_g1);
	if(conjecture_g2!=NULL)
		GEOSGeom_destroy(conjecture_g2);
	if(union1!=NULL)
		GEOSGeom_destroy(union1);
	if(union2!=NULL)
		GEOSGeom_destroy(union2);
}

/*
P = vgeom1->kernel X vgeom2->kernel
S = vgeom1->kernel union vgeom1->conjecture X vgeom2->kernel union vgeom2->conjecture
*/
VAGUEBOOL *vaguegeom_disjoint(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	VAGUEBOOL *ret = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
	GEOSGeometry *kernel_g1=NULL, *kernel_g2=NULL;
	char *p;

	initGEOS(lwnotice, lwgeom_geos_error);
	
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
	kernel_g1 = LWGEOM2GEOS(vgeom1->kernel, 0);
	kernel_g2 = LWGEOM2GEOS(vgeom2->kernel, 0);
#else
	kernel_g1 = LWGEOM2GEOS(vgeom1->kernel);
	kernel_g2 = LWGEOM2GEOS(vgeom2->kernel);
#endif
	
	p = GEOSRelate(kernel_g1, kernel_g2);
	
	//computePQRS(vgeom1, vgeom2, p, NULL, NULL, s);

	//if union are disjoint then is true, else if kernel and kernel is not disjoint then is false, otherwise is maybe
	if(!GEOSRelatePatternMatch(p, "FF*FF****")) {
		ret->vbool = VG_FALSE;
		GEOSGeom_destroy(kernel_g1);
		GEOSGeom_destroy(kernel_g2);
	} else {
		GEOSGeometry *conjecture_g1=NULL, *conjecture_g2=NULL, *union1=NULL, *union2=NULL;
		char *s;
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
		conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture, 0);
		conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture, 0);
#else
		conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture);
		conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture);
#endif
		
		if(HAS_PUNION(vgeom1->flags)) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			union1 = LWGEOM2GEOS(vgeom1->allextension, 0);
#else
			union1 = LWGEOM2GEOS(vgeom1->allextension);
#endif
		} else {
			union1 = GEOSUnion(kernel_g1, conjecture_g1);
		}
			
		if(HAS_PUNION(vgeom2->flags)) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			union2 = LWGEOM2GEOS(vgeom2->allextension, 0);
#else
			union2 = LWGEOM2GEOS(vgeom2->allextension);
#endif
		}
		else {
			union2 = GEOSUnion(kernel_g2, conjecture_g2);
		}
		
		s = GEOSRelate(union1, union2);
		if(GEOSRelatePatternMatch(s, "FF*FF****")) {
			ret->vbool = VG_TRUE;
		} else {
			ret->vbool = VG_MAYBE;
		}
		GEOSGeom_destroy(kernel_g1);
		GEOSGeom_destroy(kernel_g2);
		GEOSGeom_destroy(conjecture_g1);
		GEOSGeom_destroy(conjecture_g2);
		GEOSGeom_destroy(union1);
		GEOSGeom_destroy(union2);
		GEOSFree(s);
	}
	
	/*	
	if(GEOSRelatePatternMatch(s, "FF*FF****")) {
		ret->vbool = VG_TRUE;
	} else if (!GEOSRelatePatternMatch(p, "FF*FF****")) {
		ret->vbool = VG_FALSE;
	} else {
		ret->vbool = VG_MAYBE;
	}
	*/
	GEOSFree(p);
	return ret;
}

/*
P = vgeom1->kernel X vgeom2->kernel
S = vgeom1->kernel union vgeom1->conjecture X vgeom2->kernel union vgeom2->conjecture
*/
VAGUEBOOL *vaguegeom_meet(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	VAGUEBOOL *ret = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
	char *p, *s;
	if(vgeom1->type == VAGUEPOINTTYPE || vgeom2->type==VAGUEPOINTTYPE) {
		ret->vbool = VG_FALSE;
		return ret;
	}
	initGEOS(lwnotice, lwgeom_geos_error);
	
	p = lwalloc(sizeof(char) * 9);
	s = lwalloc(sizeof(char) * 9);
	
	/* in this predicate p and s are used by the first IF, then use this function 
	dont make sense split as the others predicates
	*/
	computePQRS(vgeom1, vgeom2, p, NULL, NULL, s);
	
	//if kernel touches with kernel and interior of unions are disjoint is true
	//else if interior of kernel intersects or the unions are disjoint or not exist overlap between unions is false, otherwise is maybe
	if((GEOSRelatePatternMatch(p, "FT*******")||GEOSRelatePatternMatch(p, "F**T*****")||GEOSRelatePatternMatch(p, "F***T****")) && (GEOSRelatePatternMatch(s, "F********"))) {
		ret->vbool = VG_TRUE;
	} else if (GEOSRelatePatternMatch(p, "T********") || GEOSRelatePatternMatch(s, "FF*FF****")) {
		//GEOSRelatePatternMatch(s, "***FFF***") || GEOSRelatePatternMatch(s, "*F**F**F*") THIS PRECIDATES REALLY NEED?? I DONT THINK SO.
		ret->vbool = VG_FALSE;
	} else {
		ret->vbool = VG_MAYBE;
	}	
	lwfree(p);
	lwfree(s);
	return ret;
}

/*
Q = vgeom1->kernel union vgeom1->conjecture X vgeom2->kernel
R = vgeom1->kernel X vgeom2->kernel union vgeom2->conjecture
S = vgeom1->kernel union vgeom1->kernel X vgeom2->kernel union vgeom2->conjecture
*/
VAGUEBOOL *vaguegeom_inside(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2){
	VAGUEBOOL *ret = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
	GEOSGeometry *kernel_g1=NULL, *kernel_g2=NULL, *conjecture_g1=NULL, *union1=NULL;
	char *q;
	
	initGEOS(lwnotice, lwgeom_geos_error);
	
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
	kernel_g1 = LWGEOM2GEOS(vgeom1->kernel, 0);
	conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture, 0);
	kernel_g2 = LWGEOM2GEOS(vgeom2->kernel, 0);
#else
	kernel_g1 = LWGEOM2GEOS(vgeom1->kernel);
	conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture);
	kernel_g2 = LWGEOM2GEOS(vgeom2->kernel);
#endif
	
	if(HAS_PUNION(vgeom1->flags)) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
		union1 = LWGEOM2GEOS(vgeom1->allextension, 0);
#else
		union1 = LWGEOM2GEOS(vgeom1->allextension);
#endif
	}
	else
		union1 = GEOSUnion(kernel_g1, conjecture_g1);
		
	q = GEOSRelate(union1, kernel_g2);
	
	//computePQRS(vgeom1, vgeom2, NULL, q, r, NULL);
	
	//II IB IE BI BB BE EI EB EE
	//is true if union of vgeom1 is contained in kernel of vgeom2 without intersection on borders
	//is false if the kernel of vgeom1 intersects the exterior of union of vgeom2 or there is intersection between the border of vgeom2 and kernel of vgeom1
	if(GEOSRelatePatternMatch(q, "T*F*FF***")) {
		ret->vbool = VG_TRUE;
		GEOSGeom_destroy(kernel_g1);
		GEOSGeom_destroy(kernel_g2);
		GEOSGeom_destroy(conjecture_g1);
		GEOSGeom_destroy(union1);
	} else {
		char *r;
		GEOSGeometry *conjecture_g2=NULL, *union2=NULL;
		
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
		conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture, 0);
#else
		conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture);
#endif	
		if(HAS_PUNION(vgeom2->flags)) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			union2 = LWGEOM2GEOS(vgeom2->allextension, 0);
#else
			union2 = LWGEOM2GEOS(vgeom2->allextension);
#endif
		}
		else
			union2 = GEOSUnion(kernel_g2, conjecture_g2);
			
		r = GEOSRelate(kernel_g1, union2);
		
		if (GEOSRelatePatternMatch(r, "**T******") || GEOSRelatePatternMatch(r, "****T****")) {
			ret->vbool = VG_FALSE;
		} else {
			ret->vbool = VG_MAYBE;
		}
		GEOSGeom_destroy(kernel_g1);
		GEOSGeom_destroy(kernel_g2);
		GEOSGeom_destroy(conjecture_g1);
		GEOSGeom_destroy(union1);
		GEOSGeom_destroy(union2);
		GEOSGeom_destroy(conjecture_g2);
		GEOSFree(r);
	}
	
	/*
	if(GEOSRelatePatternMatch(q, "T*F*FF***")) {
		ret->vbool = VG_TRUE;
	} else if (GEOSRelatePatternMatch(r, "**T******") || GEOSRelatePatternMatch(r, "****T****")) {
		ret->vbool = VG_FALSE;
	} else {
		ret->vbool = VG_MAYBE;
	}
	*/
	GEOSFree(q);
	return ret;
}

/*
P = vgeom1->kernel X vgeom2->kernel
S = vgeom1->kernel union vgeom1->conjecture X vgeom2->kernel union vgeom2->conjecture
*/
VAGUEBOOL *vaguegeom_intersects(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	VAGUEBOOL *ret = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
	GEOSGeometry *kernel_g1=NULL, *kernel_g2=NULL;
	char *p;

	initGEOS(lwnotice, lwgeom_geos_error);
	
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
	kernel_g1 = LWGEOM2GEOS(vgeom1->kernel, 0);
	kernel_g2 = LWGEOM2GEOS(vgeom2->kernel, 0);
#else
	kernel_g1 = LWGEOM2GEOS(vgeom1->kernel);
	kernel_g2 = LWGEOM2GEOS(vgeom2->kernel);
#endif
	
	p = GEOSRelate(kernel_g1, kernel_g2);
	
	//computePQRS(vgeom1, vgeom2, p, NULL, NULL, s);

	//if union are disjoint else if kernel with kernel is not disjoint then is false, otherwise is maybe
	if(!GEOSRelatePatternMatch(p, "FF*FF****")) {
		ret->vbool = VG_TRUE;
		GEOSGeom_destroy(kernel_g1);
		GEOSGeom_destroy(kernel_g2);
	} else {
		GEOSGeometry *conjecture_g1=NULL, *conjecture_g2=NULL, *union1=NULL, *union2=NULL;
		char *s;

#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
		conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture, 0);
		conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture, 0);
#else
		conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture);
		conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture);
#endif
		
		if(HAS_PUNION(vgeom1->flags)) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			union1 = LWGEOM2GEOS(vgeom1->allextension, 0);
#else
			union1 = LWGEOM2GEOS(vgeom1->allextension);
#endif
		} else
			union1 = GEOSUnion(kernel_g1, conjecture_g1);
			
		if(HAS_PUNION(vgeom2->flags)) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			union2 = LWGEOM2GEOS(vgeom2->allextension, 0);
#else
			union2 = LWGEOM2GEOS(vgeom2->allextension);
#endif
		} else
			union2 = GEOSUnion(kernel_g2, conjecture_g2);
		
		s = GEOSRelate(union1, union2);
		if(GEOSRelatePatternMatch(s, "FF*FF****")) {
			ret->vbool = VG_FALSE;
		} else {
			ret->vbool = VG_MAYBE;
		}
		GEOSGeom_destroy(conjecture_g1);
		GEOSGeom_destroy(union1);
		GEOSGeom_destroy(union2);
		GEOSGeom_destroy(conjecture_g2);
		GEOSFree(s);
	}
	
	GEOSFree(p);
	return ret;
}

/*
P = vgeom1->kernel X vgeom2->kernel
Q = vgeom1->kernel union vgeom1->conjecture X vgeom2->kernel
R = vgeom1->kernel X vgeom2->kernel union vgeom2->conjecture
*/
VAGUEBOOL *vaguegeom_coveredBy(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	VAGUEBOOL *ret = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
	char *p, *q;
	GEOSGeometry *kernel_g1=NULL, *kernel_g2=NULL, *conjecture_g1=NULL, *union1=NULL;
	
	initGEOS(lwnotice, lwgeom_geos_error);
	
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
	kernel_g1 = LWGEOM2GEOS(vgeom1->kernel, 0);
	conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture, 0);
	kernel_g2 = LWGEOM2GEOS(vgeom2->kernel, 0);
#else
	kernel_g1 = LWGEOM2GEOS(vgeom1->kernel);
	conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture);
	kernel_g2 = LWGEOM2GEOS(vgeom2->kernel);
#endif
	
	if(HAS_PUNION(vgeom1->flags)) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
		union1 = LWGEOM2GEOS(vgeom1->allextension, 0);
#else
		union1 = LWGEOM2GEOS(vgeom1->allextension);
#endif
	} else
		union1 = GEOSUnion(kernel_g1, conjecture_g1);
		
	q = GEOSRelate(union1, kernel_g2);
	p = GEOSRelate(kernel_g1, kernel_g2);	
	//computePQRS(vgeom1, vgeom2, p, q, r, NULL);
	
	//II IB IE BI BB BE EI EB EE
	//is true if union of vgeom1 is contained in kernel of vgeom2 with intersection on borders whith kernel or union2
	//is false if the interior of kernel of vgeom1 touches the exterior, or not touches the interior or is inside in union2
	if((GEOSRelatePatternMatch(q, "T*F**F***") || GEOSRelatePatternMatch(q, "*TF**F***") || GEOSRelatePatternMatch(q, "**FT*F***") || GEOSRelatePatternMatch(q, "**F*TF***")) && GEOSRelatePatternMatch(p, "****T****")) {
		ret->vbool = VG_TRUE;
		GEOSGeom_destroy(kernel_g1);
		GEOSGeom_destroy(kernel_g2);
		GEOSGeom_destroy(conjecture_g1);
		GEOSGeom_destroy(union1);
	} else {
		char *r;
		GEOSGeometry *conjecture_g2=NULL, *union2=NULL;
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
		conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture, 0);
#else
		conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture);
#endif
	
		if(HAS_PUNION(vgeom2->flags)) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			union2 = LWGEOM2GEOS(vgeom2->allextension, 0);
#else
			union2 = LWGEOM2GEOS(vgeom2->allextension);
#endif
		} else
			union2 = GEOSUnion(kernel_g2, conjecture_g2);
			
		r = GEOSRelate(kernel_g1, union2);
		if(GEOSRelatePatternMatch(r, "**T******") || GEOSRelatePatternMatch(r, "F********") || GEOSRelatePatternMatch(q, "T*F*FF***")) {
			ret->vbool = VG_FALSE;
		} else {
			ret->vbool = VG_MAYBE;
		}
		GEOSGeom_destroy(kernel_g1);
		GEOSGeom_destroy(kernel_g2);
		GEOSGeom_destroy(conjecture_g1);
		GEOSGeom_destroy(union1);
		GEOSGeom_destroy(conjecture_g2);
		GEOSGeom_destroy(union2);
		GEOSFree(r);
	} 
	
	/*
	if((GEOSRelatePatternMatch(q, "T*F**F***") || GEOSRelatePatternMatch(q, "*TF**F***") || GEOSRelatePatternMatch(q, "**FT*F***") || GEOSRelatePatternMatch(q, "**F*TF***")) && GEOSRelatePatternMatch(p, "****T****")) {
		ret->vbool = VG_TRUE;
	} else if(GEOSRelatePatternMatch(r, "**T******") || GEOSRelatePatternMatch(r, "F********") || GEOSRelatePatternMatch(q, "T*F*FF***")) {
		ret->vbool = VG_FALSE;
	} else {
		ret->vbool = VG_MAYBE;
	}
	*/
	GEOSFree(p);
	GEOSFree(q);
	return ret;
}

/*
P = vgeom1->kernel X vgeom2->kernel
R = vgeom1->kernel X vgeom2->kernel union vgeom2->conjecture
*/
VAGUEBOOL *vaguegeom_equal(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	VAGUEBOOL *ret = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
	GEOSGeometry *kernel_g1=NULL, *kernel_g2=NULL;
	char *p;

	initGEOS(lwnotice, lwgeom_geos_error);
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
	kernel_g1 = LWGEOM2GEOS(vgeom1->kernel, 0);
	kernel_g2 = LWGEOM2GEOS(vgeom2->kernel, 0);
#else
	kernel_g1 = LWGEOM2GEOS(vgeom1->kernel);
	kernel_g2 = LWGEOM2GEOS(vgeom2->kernel);
#endif
	
	p = GEOSRelate(kernel_g1, kernel_g2);
	
	//computePQRS(vgeom1, vgeom2, p, NULL, r, NULL);
		
	//II IB IE BI BB BE EI EB EE
	//true if the kernels are equal and there are not conjectures, false if exterior of kernel touches the border and vice versa
	if(GEOSRelatePatternMatch(p, "T*F**FFF*") && (!HAS_CONJECTURE(vgeom1->flags) && !HAS_CONJECTURE(vgeom2->flags))) {
		ret->vbool = VG_TRUE;
		GEOSGeom_destroy(kernel_g1);
		GEOSGeom_destroy(kernel_g2);
	} else {
		char *r;
		GEOSGeometry *union2=NULL;
			
		if(HAS_PUNION(vgeom2->flags)) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			union2 = LWGEOM2GEOS(vgeom2->allextension, 0);
#else
			union2 = LWGEOM2GEOS(vgeom2->allextension);
#endif
		} else {
			GEOSGeometry *conjecture_g2=NULL;
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture, 0);
#else
			conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture);
#endif
			union2 = GEOSUnion(kernel_g2, conjecture_g2);
			GEOSGeom_destroy(conjecture_g2);
		}
			
		r = GEOSRelate(kernel_g1, union2);
		if(GEOSRelatePatternMatch(r, "**T******")) {
			ret->vbool = VG_FALSE;
		} else {
			char *q;
			GEOSGeometry *union1=NULL;
				
			if(HAS_PUNION(vgeom1->flags)) {
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
				union1 = LWGEOM2GEOS(vgeom1->allextension, 0);
#else
				union1 = LWGEOM2GEOS(vgeom1->allextension);
#endif
			} else {
				GEOSGeometry *conjecture_g1=NULL;
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
				conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture, 0);
#else
				conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture);
#endif
				union1 = GEOSUnion(kernel_g1, conjecture_g1);
				GEOSGeom_destroy(conjecture_g1);
			}
			q = GEOSRelate(union1, kernel_g2);
			if(GEOSRelatePatternMatch(q, "******T**")) {
				ret->vbool = VG_FALSE;
			} else {
				ret->vbool = VG_MAYBE;
			}
			GEOSFree(q);
			GEOSGeom_destroy(union1);
		}
		GEOSGeom_destroy(kernel_g1);
		GEOSGeom_destroy(kernel_g2);
		GEOSGeom_destroy(union2);
		GEOSFree(r);
	}
	GEOSFree(p);
	return ret;
}

/*
P = vgeom1->kernel X vgeom2->kernel
Q = vgeom1->kernel union vgeom1->conjecture X vgeom2->kernel
R = vgeom1->kernel X vgeom2->kernel union vgeom2->conjecture
S = vgeom1->kernel union vgeom1->conjecture X vgeom2->kernel union vgeom2->conjecture
*/
VAGUEBOOL *vaguegeom_overlap(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	VAGUEBOOL *ret = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
	char *p, *q, *r;
	GEOSGeometry *kernel_g1=NULL, *kernel_g2=NULL, *conjecture_g1=NULL, *conjecture_g2=NULL, *union1=NULL, *union2=NULL;
	
	initGEOS(lwnotice, lwgeom_geos_error);

#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
	kernel_g1 = LWGEOM2GEOS(vgeom1->kernel, 0);
	conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture, 0);
	kernel_g2 = LWGEOM2GEOS(vgeom2->kernel, 0);
	conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture, 0);
	
	if(HAS_PUNION(vgeom1->flags))
		union1 = LWGEOM2GEOS(vgeom1->allextension, 0);
	else
		union1 = GEOSUnion(kernel_g1, conjecture_g1);
		
	if(HAS_PUNION(vgeom2->flags))
		union2 = LWGEOM2GEOS(vgeom2->allextension, 0);
	else
		union2 = GEOSUnion(kernel_g2, conjecture_g2);
#else
	kernel_g1 = LWGEOM2GEOS(vgeom1->kernel);
	conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture);
	kernel_g2 = LWGEOM2GEOS(vgeom2->kernel);
	conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture);
	
	if(HAS_PUNION(vgeom1->flags))
		union1 = LWGEOM2GEOS(vgeom1->allextension);
	else
		union1 = GEOSUnion(kernel_g1, conjecture_g1);
		
	if(HAS_PUNION(vgeom2->flags))
		union2 = LWGEOM2GEOS(vgeom2->allextension);
	else
		union2 = GEOSUnion(kernel_g2, conjecture_g2);
#endif
		
	q = GEOSRelate(union1, kernel_g2);
	p = GEOSRelate(kernel_g1, kernel_g2);	
	r = GEOSRelate(kernel_g1, union2);
	//computePQRS(vgeom1, vgeom2, p, q, r, s);
	
	//II IB IE BI BB BE EI EB EE
	//to be true the kernels must be overlapped and the exterior must touches in exterior
	if((GEOSRelatePatternMatch(p, "T*T***T**") || GEOSRelatePatternMatch(p, "1*T***T**")) && 
		GEOSRelatePatternMatch(r, "**T******") && GEOSRelatePatternMatch(q, "******T**")) {
		ret->vbool = VG_TRUE;
	} else {
		char *s = GEOSRelate(union1, union2);		
		if(GEOSRelatePatternMatch(s, "F********") || GEOSRelatePatternMatch(q, "**F******") || GEOSRelatePatternMatch(r, "******F**")) {
			ret->vbool = VG_FALSE;
		} else {
			ret->vbool = VG_MAYBE;
		}
		GEOSFree(s);
	}
	/*
	if((GEOSRelatePatternMatch(p, "T*T***T**") || GEOSRelatePatternMatch(p, "1*T***T**")) && 
		GEOSRelatePatternMatch(r, "**T******") && GEOSRelatePatternMatch(q, "******T**")) {
		ret->vbool = VG_TRUE;
	} else if(GEOSRelatePatternMatch(s, "F********") || GEOSRelatePatternMatch(q, "**F******") || GEOSRelatePatternMatch(r, "******F**")) {
		ret->vbool = VG_FALSE;
	} else {
		ret->vbool = VG_MAYBE;
	}
	*/
	GEOSGeom_destroy(kernel_g1);
	GEOSGeom_destroy(kernel_g2);
	GEOSGeom_destroy(conjecture_g1);
	GEOSGeom_destroy(conjecture_g2);
	GEOSGeom_destroy(union1);
	GEOSGeom_destroy(union2);
	GEOSFree(p);
	GEOSFree(q);
	GEOSFree(r);
	return ret;
}

VAGUEBOOL *vaguegeom_contains(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	return vaguegeom_inside(vgeom2, vgeom1);
}

VAGUEBOOL *vaguegeom_covers(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	return vaguegeom_coveredBy(vgeom2, vgeom1);
}

//TO-DO use the intersection matrix as other operators.
VAGUEBOOL *vaguegeom_crosses(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	GEOSGeometry *kernel_g1, *kernel_g2;
	VAGUEBOOL *ret = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
	initGEOS(lwnotice, lwgeom_geos_error);

#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
	kernel_g1 = LWGEOM2GEOS(vgeom1->kernel, 0);
	kernel_g2 = LWGEOM2GEOS(vgeom2->kernel, 0);
#else
	kernel_g1 = LWGEOM2GEOS(vgeom1->kernel);
	kernel_g2 = LWGEOM2GEOS(vgeom2->kernel);
#endif
		
	if(GEOSCrosses(kernel_g1, kernel_g2)) {
		ret->vbool = VG_TRUE;
	} else {
		GEOSGeometry *conjecture_g1, *conjecture_g2, *union1, *union2;
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
		conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture, 0);
		conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture, 0);
		if(HAS_PUNION(vgeom1->flags))
			union1 = LWGEOM2GEOS(vgeom1->allextension, 0);
		else
			union1 = GEOSUnion(kernel_g1, conjecture_g1);
		if(HAS_PUNION(vgeom2->flags))
			union2 = LWGEOM2GEOS(vgeom2->allextension, 0);
		else
			union2 = GEOSUnion(kernel_g2, conjecture_g2);
#else
		conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture);
		conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture);
		if(HAS_PUNION(vgeom1->flags))
			union1 = LWGEOM2GEOS(vgeom1->allextension);
		else
			union1 = GEOSUnion(kernel_g1, conjecture_g1);
		if(HAS_PUNION(vgeom2->flags))
			union2 = LWGEOM2GEOS(vgeom2->allextension);
		else
			union2 = GEOSUnion(kernel_g2, conjecture_g2);
#endif

		if(GEOSCrosses(union1, union2)) {
			ret->vbool = VG_MAYBE;
		} else {
			ret->vbool = VG_FALSE;
		}
		GEOSGeom_destroy(conjecture_g1);
		GEOSGeom_destroy(conjecture_g2);
		GEOSGeom_destroy(union1);
		GEOSGeom_destroy(union2);
	}
	GEOSGeom_destroy(kernel_g1);
	GEOSGeom_destroy(kernel_g2);
	return ret;
}

/* vague point, vague line or vague point, vague polygon */
VAGUEBOOL *vaguegeom_on_border_of(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	VAGUEBOOL *ret = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
	if(vgeom1->type != VAGUEPOINTTYPE || vgeom1->type != VAGUEMULTIPOINTTYPE) {
		ret->vbool = VG_FALSE;
		return ret;
	}
	if(vgeom2->type != VAGUELINETYPE || vgeom2->type != VAGUEMULTILINETYPE || 
		vgeom2->type != VAGUEPOLYGONTYPE || vgeom2->type != VAGUEMULTIPOLYGONTYPE) {
			ret->vbool = VG_FALSE;
			return ret;
	}

	if(vgeom2->type == VAGUELINETYPE || vgeom2->type == VAGUEMULTILINETYPE) {
		return vaguegeom_inside(vgeom1, vgeom2);
	} else {
		VAGUEGEOM *vbound;
		vbound = vaguegeom_kernel_boundary(vgeom2);
		ret = vaguegeom_inside(vgeom1, vbound);
		vaguegeom_free(vbound);
		return ret;
	}
}

/* vague line X vague line, vague line X vague region, vague region X vague line, vague region X vague region */
VAGUEBOOL *vaguegeom_border_in_common(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) {
	VAGUEBOOL *ret = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
	if(vgeom1->type != VAGUEPOINTTYPE || vgeom1->type != VAGUEMULTIPOINTTYPE) {
		ret->vbool = VG_FALSE;
		return ret;
	}
	if(vgeom2->type != VAGUEPOINTTYPE || vgeom2->type != VAGUEMULTIPOINTTYPE) {
		ret->vbool = VG_FALSE;
		return ret;
	}

	if((vgeom1->type == VAGUELINETYPE || vgeom1->type == VAGUEMULTILINETYPE) && 
		(vgeom2->type == VAGUELINETYPE || vgeom2->type == VAGUEMULTILINETYPE)) {
		GEOSGeometry *kernel_g1, *kernel_g2;
#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
		kernel_g1 = LWGEOM2GEOS(vgeom1->kernel, 0);
		kernel_g2 = LWGEOM2GEOS(vgeom2->kernel, 0);
#else
		kernel_g1 = LWGEOM2GEOS(vgeom1->kernel);
		kernel_g2 = LWGEOM2GEOS(vgeom2->kernel);
#endif

		if(GEOSIntersects(kernel_g1, kernel_g2)) {
			GEOSGeometry *intersection = GEOSIntersection(kernel_g1, kernel_g2);
			switch (GEOSGeomTypeId(intersection))
			{
			case GEOS_MULTILINESTRING:
			case GEOS_LINESTRING:
			case GEOS_LINEARRING:
				ret->vbool = VG_TRUE;
				GEOSGeom_destroy(intersection);
				GEOSGeom_destroy(kernel_g1);
				GEOSGeom_destroy(kernel_g2);
				return ret;
			}
		} else {
			GEOSGeometry *conjecture_g1, *conjecture_g2, *int_k1, *int_k2, *int_c1, *int_c2, *un1, *un2, *un3;

#if ((NUM_POSTGIS_MAJOR_VERSION == 2 && NUM_POSTGIS_MINOR_VERSION >= 2 && NUM_POSTGIS_MICRO_VERSION >= 1) || NUM_POSTGIS_MAJOR_VERSION > 2)
			conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture, 0);
			conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture, 0);
#else
			conjecture_g1 = LWGEOM2GEOS(vgeom1->conjecture);
			conjecture_g2 = LWGEOM2GEOS(vgeom2->conjecture);
#endif

			int_k1 = GEOSIntersection(kernel_g1, kernel_g2);
			int_c1 = GEOSIntersection(conjecture_g1, conjecture_g2);

			switch (GEOSGeomTypeId(int_c1))
			{
			case GEOS_MULTILINESTRING:
			case GEOS_LINESTRING:
			case GEOS_LINEARRING:
				ret->vbool = VG_MAYBE;
				GEOSGeom_destroy(int_c1);
				GEOSGeom_destroy(int_k1);
				GEOSGeom_destroy(kernel_g1);
				GEOSGeom_destroy(kernel_g2);
				GEOSGeom_destroy(conjecture_g1);
				GEOSGeom_destroy(conjecture_g2);
				return ret;
			}

			int_k2 = GEOSIntersection(kernel_g1, conjecture_g2);

			switch (GEOSGeomTypeId(int_k2))
			{
			case GEOS_MULTILINESTRING:
			case GEOS_LINESTRING:
			case GEOS_LINEARRING:
				ret->vbool = VG_MAYBE;
				GEOSGeom_destroy(int_c1);
				GEOSGeom_destroy(int_k1);
				GEOSGeom_destroy(int_k2);
				GEOSGeom_destroy(kernel_g1);
				GEOSGeom_destroy(kernel_g2);
				GEOSGeom_destroy(conjecture_g1);
				GEOSGeom_destroy(conjecture_g2);
				return ret;
			}

			int_c2 = GEOSIntersection(conjecture_g1, kernel_g2);

			switch (GEOSGeomTypeId(int_c2))
			{
			case GEOS_MULTILINESTRING:
			case GEOS_LINESTRING:
			case GEOS_LINEARRING:
				ret->vbool = VG_MAYBE;
				GEOSGeom_destroy(int_c1);
				GEOSGeom_destroy(int_c2);
				GEOSGeom_destroy(int_k1);
				GEOSGeom_destroy(int_k2);
				GEOSGeom_destroy(kernel_g1);
				GEOSGeom_destroy(kernel_g2);
				GEOSGeom_destroy(conjecture_g1);
				GEOSGeom_destroy(conjecture_g2);
				return ret;
			}

			un1 = GEOSUnion(int_k1, int_k2);
			un2 = GEOSUnion(un1, int_c1);
			un3 = GEOSUnion(un2, int_c2);

			switch (GEOSGeomTypeId(un3))
			{
			case GEOS_MULTILINESTRING:
			case GEOS_LINESTRING:
			case GEOS_LINEARRING:
				ret->vbool = VG_MAYBE;
				GEOSGeom_destroy(int_c1);
				GEOSGeom_destroy(int_c2);
				GEOSGeom_destroy(int_k1);
				GEOSGeom_destroy(int_k2);
				GEOSGeom_destroy(kernel_g1);
				GEOSGeom_destroy(kernel_g2);
				GEOSGeom_destroy(conjecture_g1);
				GEOSGeom_destroy(conjecture_g2);

				GEOSGeom_destroy(un1);
				GEOSGeom_destroy(un2);
				GEOSGeom_destroy(un3);
				return ret;
			}

			ret->vbool = VG_FALSE;
			GEOSGeom_destroy(int_c1);
			GEOSGeom_destroy(int_c2);
			GEOSGeom_destroy(int_k1);
			GEOSGeom_destroy(int_k2);
			GEOSGeom_destroy(kernel_g1);
			GEOSGeom_destroy(kernel_g2);
			GEOSGeom_destroy(conjecture_g1);
			GEOSGeom_destroy(conjecture_g2);

			GEOSGeom_destroy(un1);
			GEOSGeom_destroy(un2);
			GEOSGeom_destroy(un3);
			return ret;
		}
	//LINE X POLYGON
	} else if ((vgeom1->type == VAGUELINETYPE || vgeom1->type == VAGUEMULTILINETYPE) && 
		(vgeom2->type == VAGUEPOLYGONTYPE || vgeom2->type == VAGUEMULTIPOLYGONTYPE)) {
		VAGUEGEOM *vbound;
		vbound = vaguegeom_kernel_boundary(vgeom2);
		ret = vaguegeom_border_in_common(vgeom1, vbound);
		vaguegeom_free(vbound);
		return ret;
	} else if ((vgeom1->type == VAGUEPOLYGONTYPE || vgeom1->type == VAGUEMULTIPOLYGONTYPE) && 
		(vgeom2->type == VAGUELINETYPE || vgeom2->type == VAGUEMULTILINETYPE)) {
		VAGUEGEOM *vbound;
		vbound = vaguegeom_kernel_boundary(vgeom1);
		ret = vaguegeom_border_in_common(vbound, vgeom2);
		vaguegeom_free(vbound);
		return ret;
	} else { //two polygons....
		VAGUEGEOM *vbound1, *vbound2;
		vbound1 = vaguegeom_kernel_boundary(vgeom1);
		vbound2 = vaguegeom_kernel_boundary(vgeom2);
		ret = vaguegeom_border_in_common(vbound1, vbound2);
		vaguegeom_free(vbound1);
		vaguegeom_free(vbound2);
		return ret;
	}
}
