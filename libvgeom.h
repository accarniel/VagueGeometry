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
 
 /*****************
 *
   main .h file of the VagueGeometry extension, it reuses some libraries of the PostGIS.
 *
 ******************/

#ifndef LIBVAGUEGEOM_VASA_V1
#define LIBVAGUEGEOM_VASA_V1 1

#include <liblwgeom_internal.h>
#include <lwgeom_geos.h>
#include "libversion.h"

#define VAGUEPOINTTYPE 1
#define VAGUELINETYPE 2
#define VAGUEPOLYGONTYPE 3
#define VAGUEMULTIPOINTTYPE 4
#define VAGUEMULTILINETYPE 5
#define VAGUEMULTIPOLYGONTYPE 6
#define VAGUECOLLECTIONTYPE 7

/**
* Macros to handle the 'flags' byte of VAGUEGEOM. A uint8_t used as follows: 
* -----UCK
* Five unused bits, followed by HasPreComputedUnion, HasConjecture and HasKernel flags.
*/
#define HAS_KERNEL(flags) ((flags) & 0x01)
#define HAS_CONJECTURE(flags) (((flags) & 0x02)>>1)
#define HAS_PUNION(flags) (((flags) & 0x04)>>2)
#define FLAGS_SET_KERNEL(flags, value) ((flags) = (value) ? ((flags) | 0x01) : ((flags) & 0xFE))
#define FLAGS_SET_CONJECTURE(flags, value) ((flags) = (value) ? ((flags) | 0x02) : ((flags) & 0xFD))
#define FLAGS_SET_PUNION(flags, value) ((flags) = (value) ? ((flags) | 0x04) : ((flags) & 0xFB))
#define HAS_BOTH(flags) (HAS_KERNEL(flags)==1 && HAS_CONJECTURE(flags)==1)
#define HAS_NOTHING(flags) (HAS_KERNEL(flags)==0 && HAS_CONJECTURE(flags)==0)

#if defined(_WIN32) || defined(_WIN64) 
  #define snprintf _snprintf 
  #define vsnprintf _vsnprintf 
  #define strcasecmp _stricmp 
  #define strncasecmp _strnicmp 
#endif

/* vague geometry
 type can be a VAGUEPOINT, VAGUELINE, VAGUEPOLYGON, VAGUEMULTIPOINT, VAGUEMULTILINE, VAGUEMULTIPOLYGON
 The VAGUEGEOM must be same SRID and TYPES of LWGEOM
*/
typedef struct {
	uint8_t type;
	uint8_t flags;
	LWGEOM *kernel;
	LWGEOM *conjecture;
	LWGEOM *allextension; //pre compute the union among kernel and conjecture, if only if the user wants it
} VAGUEGEOM;

/* vague geom serialized */
typedef struct {
	uint32_t size; /*4 bytes - for postgres use only */
	uint8_t srid[3]; /*3 bytes - because of postGIS requires */
	uint8_t flags; /*1 byte - flags of vague geom */
	uint8_t data[1];
} VAGUEGEOMSERIALIZED;

/* internal operations */
/* construct a vaguegeom */
extern VAGUEGEOM *vaguegeom_construct(LWGEOM *kernel, LWGEOM *conjecture, uint8_t punion);
extern VAGUEGEOM *vaguegeom_construct_empty(uint8_t punion);
extern VAGUEGEOM *vaguegeom_construct_precompunion(LWGEOM *kernel, LWGEOM *conjecture, LWGEOM *allextension);
extern VAGUEGEOM *vgeojson_to_vaguegeom(char *vgeojson, uint8_t punion);
extern VAGUEGEOM *vaguegeom_from_vgml(const char *vgml, int *root_srid, uint8_t punion);
extern VAGUEGEOM *vaguegeom_from_vkml(const char *vkml, uint8_t punion);
extern VAGUEGEOM *vgeom_from_vhexwkb(const char *hexwkb, const char check, uint8_t punion);
extern VAGUEGEOM *vgeom_from_vwkb(const uint8_t *vwkb, const size_t size, const char check, uint8_t punion);
extern VAGUEGEOM *vgeom_from_vwkt(char *vwkt, uint8_t punion);

/*
* convertions to text form
*/
extern char *vgeom_to_vwkt(const VAGUEGEOM *vgeom, uint8_t is_simplified, uint8_t is_extended);

/* get the kernel of a vaguegeom, if the vague geometry doesnt has a kernel, then return NULL */
extern LWGEOM *vaguegeom_getkernel(const VAGUEGEOM *vgeom);
/* get the conjecture of a vaguegeom, if the vague geometry doesnt has a conjecture, then return NULL */
extern LWGEOM *vaguegeom_getconjecture(const VAGUEGEOM *vgeom);

/*get the lwgeom type and return the vaguegeom compatible type */
extern uint8_t settype_fromlwgeom(uint8_t type);

/* check the vaguegeometry object if kernel and conjecture disjoint or meet */
extern uint8_t vaguegeom_is_valid(const VAGUEGEOM *vgeom);

/* check if the crisp type is compatible with the vague geometry type */
extern uint8_t vaguegeom_compatible_type(const char* type, uint8_t crisp_type);

/*set kernel */
extern void vaguegeom_setkernel(VAGUEGEOM *vgeom, LWGEOM *kernel);

/*set conjecture */
extern void vaguegeom_setconjecture(VAGUEGEOM *vgeom, LWGEOM *conjecture);

/* textual vague type */
extern char *vaguegeom_gettype(const VAGUEGEOM *vgeom);
extern char *vaguegeom_gettype_i(uint8_t type);
extern char *vaguegeom_gettype_i_notcaps(uint8_t type);
extern uint8_t vaguegeom_gettype_char(char *type);

extern uint8_t get_potgis_type_from_vaguegeom(uint8_t vg_type);
extern int vaguegeom_getsrid(const VAGUEGEOM *vg);

extern VAGUEGEOM *vaguegeom_clone(const VAGUEGEOM *vg);

/* memory manager */
extern void vaguegeom_free(VAGUEGEOM *vgeom);

/*
* convertions for serialization
*/
extern VAGUEGEOMSERIALIZED *vaguegeom_to_serialization(VAGUEGEOM *vgeom, size_t *size);

extern VAGUEGEOM *serialization_to_vaguegeom(const VAGUEGEOMSERIALIZED *vg);

extern int32_t vgserialized_get_srid(const VAGUEGEOMSERIALIZED *vg);
extern uint8_t vgserialized_get_type(const VAGUEGEOMSERIALIZED *vg);

extern void vgserialized_set_srid(VAGUEGEOMSERIALIZED *vg, int32_t srid);

extern LWGEOM *get_kernel_from_serialization(const VAGUEGEOMSERIALIZED *vgserialize);
extern LWGEOM *get_conjecture_from_serialization(const VAGUEGEOMSERIALIZED *vgserialize);

/*
* check if g1 is inside of g2
*/
int gbox_inside_2d(const GBOX *g1, const GBOX *g2);


extern int gserialized_read_gbox_p_vaguegeom(const VAGUEGEOMSERIALIZED *vg, GBOX *gbox, uint8_t which);

/*
* convertions to binary form
*/
extern uint8_t *vaguegeom_to_vwkb(const VAGUEGEOM *vgeom, uint8_t variant, size_t *size_out);
extern char* vaguegeom_to_hexvwkb(const VAGUEGEOM *vgeom, uint8_t variant, size_t *size_out);

/* operations of vague geometry of VASA */

/*
* Vague Geometric Set Operations (UNION, INTERSECTION AND DIFFERENCE)
*/
extern VAGUEGEOM *vaguegeom_union(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);
extern VAGUEGEOM *vaguegeom_intersection(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);
extern VAGUEGEOM *vaguegeom_difference(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);

/*
* Type-dependent vague spatial operations
*/

/* kernel and conjectures versions for vertices of VAGUELINES and VAGUEREGIONS */
extern VAGUEGEOM *vaguegeom_kernel_vertices(const VAGUEGEOM *vgeom);
extern VAGUEGEOM *vaguegeom_conjecture_vertices(const VAGUEGEOM *vgeom);

/* kernel and conjectures versions for boundary of VAGUEREGIONS only */
extern VAGUEGEOM *vaguegeom_kernel_boundary(const VAGUEGEOM *vgeom);
extern VAGUEGEOM *vaguegeom_conjecture_boundary(const VAGUEGEOM *vgeom);

/*kernel and conjectures versions for interior of VAGUELINES only */
extern VAGUEGEOM *vaguegeom_kernel_interior(const VAGUEGEOM *vgeom);
extern VAGUEGEOM *vaguegeom_conjecture_interior(const VAGUEGEOM *vgeom);

/*kernel and conjectures versions for convex-hull of VAGUEPOINTS only */
extern VAGUEGEOM *vaguegeom_kernel_convexhull(const VAGUEGEOM *vgeom);
extern VAGUEGEOM *vaguegeom_conjecture_convexhull(const VAGUEGEOM *vgeom);

/*common points functions (for intersections)*/

/* vagueline, vagueline */
extern VAGUEGEOM *vaguegeom_common_points(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);

/*common border (vagueline, vagueregion) or (vagueregion, vagueline) or (vagueregion, vagueregion) */
extern VAGUEGEOM *vaguegeom_common_border(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);

/* auxiliary functions */
extern LWGEOM *convexhull(const LWGEOM *geom);
extern LWGEOM *boundary(const LWGEOM *geom);

/*
* VAGUE NUMERIC OPERATIONS
*/

typedef struct {
	double max;
	double min;
} VAGUENUMERIC;

/*length of vague line */
extern double vaguegeom_min_length(const VAGUEGEOM *vgeom);
extern double vaguegeom_max_length(const VAGUEGEOM *vgeom);

extern VAGUENUMERIC *vaguegeom_length(const VAGUEGEOM *vgeom);

/*area of vague region */
extern double vaguegeom_min_area(const VAGUEGEOM *vgeom);
extern double vaguegeom_max_area(const VAGUEGEOM *vgeom);

extern VAGUENUMERIC *vaguegeom_area(const VAGUEGEOM *vgeom);

/*diameter of vague region, line and point */
extern double vaguegeom_min_diameter(const VAGUEGEOM *vgeom);
extern double vaguegeom_max_diameter(const VAGUEGEOM *vgeom);

/*number of components of vague point */
extern double vaguegeom_min_ncomp(const VAGUEGEOM *vgeom);
extern double vaguegeom_max_ncomp(const VAGUEGEOM *vgeom);

extern VAGUENUMERIC *vaguegeom_ncomp(const VAGUEGEOM *vgeom);

/*
* VAGUE NUMERIC OPERATIONS (DISTANCE)
*/

/*nearest distance of vague geoms */
extern double vaguegeom_nearest_min_distance(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);
extern double vaguegeom_nearest_max_distance(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);

extern VAGUENUMERIC *vaguegeom_nearest_distance(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);

/*farthest distance of vague geoms */
extern double vaguegeom_farthest_min_distance(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);
extern double vaguegeom_farthest_max_distance(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);

extern VAGUENUMERIC *vaguegeom_farthest_distance(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);

/*
* OTHER GENERICS OPERATIONS
*/
extern VAGUEGEOM *vaguegeom_kernel(const VAGUEGEOM *vgeom);
extern VAGUEGEOM *vaguegeom_conjecture(const VAGUEGEOM *vgeom);
extern VAGUEGEOM *vaguegeom_invert(const VAGUEGEOM *vgeom);
extern LWGEOM *vaguegeom_kernel_projection(const VAGUEGEOM *vgeom);
extern LWGEOM *vaguegeom_conjecture_projection(const VAGUEGEOM *vgeom);
/*really equal, this operation is different of "equal" topological operation */
extern uint8_t vaguegeom_same(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);

/* 
* VAGUE TOPOLOGICAL PREDICATES
*/
#define VG_FALSE 0
#define VG_TRUE 1
#define VG_MAYBE 2

typedef struct {
	unsigned char vbool;
} VAGUEBOOL;

/* the results can be VG_TRUE, VG_FALSE or VG_MAYBE for result of predicates operations handling by vague objects */
extern VAGUEBOOL *vaguegeom_disjoint(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);
extern VAGUEBOOL *vaguegeom_meet(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);
extern VAGUEBOOL *vaguegeom_inside(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);
extern VAGUEBOOL *vaguegeom_intersects(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2) ;
extern VAGUEBOOL *vaguegeom_coveredBy(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);
extern VAGUEBOOL *vaguegeom_equal(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);
extern VAGUEBOOL *vaguegeom_overlap(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);
extern VAGUEBOOL *vaguegeom_contains(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);
extern VAGUEBOOL *vaguegeom_covers(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);
extern VAGUEBOOL *vaguegeom_crosses(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);
/* vague point, vague line or vague point, vague polygon */
extern VAGUEBOOL *vaguegeom_on_border_of(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);
/* vague line X vague line, vague line X vague region, vague region X vague line, vague region X vague region */
extern VAGUEBOOL *vaguegeom_border_in_common(const VAGUEGEOM *vgeom1, const VAGUEGEOM *vgeom2);

#endif
