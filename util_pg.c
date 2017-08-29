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
* Utilities functions based on the PostGIS.
**************************************************/

#include "util_pg.h"
/**
* Utility method to call the serialization and then set the
* PgSQL varsize header appropriately with the serialized size.
*/
VAGUEGEOMSERIALIZED *vgeometry_serialize(VAGUEGEOM *vgeom) {
	size_t ret_size = 0;
	VAGUEGEOMSERIALIZED *g = NULL;
	g = vaguegeom_to_serialization(vgeom, &ret_size);
	SET_VARSIZE(g, ret_size);
	return g;
}

bytea *uint_8_to_bytea(const uint8_t *wkb, size_t wkb_size) {
	bytea *result;
	/* Prepare the PgSQL bytea return type */
	result = (bytea*)palloc(wkb_size + VARHDRSZ);
	memcpy(VARDATA(result), wkb, wkb_size);
	SET_VARSIZE(result, wkb_size+VARHDRSZ);
	return result;
}

void valid_convertion_vaguegeom(const VAGUEGEOM *vg) {
	if(vg==NULL)  {
		ereport(ERROR, (
		            errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		            errmsg("Convertion from vague geometry serialized to vague geometry failed") ));
	}
}

void valid_convertion_serialized(const VAGUEGEOMSERIALIZED *vg) {
	if(vg==NULL)  {
		ereport(ERROR, (
		            errcode(ERRCODE_INVALID_PARAMETER_VALUE),
		            errmsg("Convertion from vague geometry to vague geometry serialized failed") ));
	}
}
