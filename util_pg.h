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
 
#include "postgres.h"
#include "libvgeom.h"

/* will be called by postgres function */
extern VAGUEGEOMSERIALIZED *vgeometry_serialize(VAGUEGEOM *vgeom);

extern void valid_convertion_vaguegeom(const VAGUEGEOM *vg);
extern void valid_convertion_serialized(const VAGUEGEOMSERIALIZED *vg);

extern bytea *uint_8_to_bytea(const uint8_t *wkb, size_t wkb_size);