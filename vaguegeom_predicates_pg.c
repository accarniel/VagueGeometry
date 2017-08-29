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
 
/************************
*
* Processing of the vague topological predicates by using the MBRVP improvement.
*
*************************/

#include "util_pg.h"
#include "fmgr.h"

/*
* IMPORTANT TO NOTE: int gbox_overlaps(const GBOX *g1, const GBOX *g2) MEANS INTERSECTS!
*/

/*
* TO-DO short-circuits considering the following combinations:
* KERNEL AND CONJECTURE X KERNEL, KERNEL AND CONJECTURE X CONJECTURE
* KERNEL X KERNEL AND CONJECTURE, KERNEL X KERNEL, KERNEL X CONJECTURE
* CONJECTURE X KERNEL AND CONJECTURE, CONJECTURE X KERNEL, CONJECTURE X CONJECTURE
*/

/*predicates */
Datum VG_contains(PG_FUNCTION_ARGS);
Datum VG_coveredBy(PG_FUNCTION_ARGS);
Datum VG_covers(PG_FUNCTION_ARGS);
Datum VG_crosses(PG_FUNCTION_ARGS);
Datum VG_disjoint(PG_FUNCTION_ARGS);
Datum VG_equal(PG_FUNCTION_ARGS);
Datum VG_inside(PG_FUNCTION_ARGS);
Datum VG_intersects(PG_FUNCTION_ARGS);
Datum VG_meets(PG_FUNCTION_ARGS);
Datum VG_on_border_of(PG_FUNCTION_ARGS);
Datum VG_border_in_common(PG_FUNCTION_ARGS);
Datum VG_overlap(PG_FUNCTION_ARGS);

PG_FUNCTION_INFO_V1(VG_coveredBy);
Datum VG_coveredBy(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	VAGUEBOOL *result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	/*short circuit 1 */
	if(HAS_NOTHING(vg1->flags) || HAS_NOTHING(vg2->flags)) {
		result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
		result->vbool = VG_FALSE;
		
		PG_FREE_IF_COPY(vg1, 0);
		PG_FREE_IF_COPY(vg2, 1);
		PG_RETURN_POINTER(result);
	} else {
		GBOX box2, kbox1, kbox2, cbox2;
		int aux1=LW_FAILURE, aux2=LW_FAILURE;
		/*
		 * short-circuit 2: if the kernel of vg1 is not inside of vg2 then we can return false
		 * Do the test IFF BOUNDING BOX AVAILABLE.
		 */
		if(HAS_BOTH(vg1->flags) && HAS_BOTH(vg2->flags)) {
			if( gserialized_read_gbox_p_vaguegeom(vg1, &kbox1, 1) ) {
				aux1 = LW_SUCCESS;
			}
			
			if(HAS_PUNION(vg2->flags)) {
				aux2 = gserialized_read_gbox_p_vaguegeom(vg2, &box2, 3);
			} else {
				if( gserialized_read_gbox_p_vaguegeom(vg2, &kbox2, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg2, &cbox2, 2) ){
					aux2 = LW_SUCCESS;
					gbox_union(&kbox2, &cbox2, &box2);
				}
			}
			
			if( aux1 && aux2 ){
				//the kernel is inside of the allextension? if not, then FALSE
				if ( gbox_inside_2d(&kbox1, &box2) == LW_FALSE ) {
					result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
					result->vbool = VG_FALSE;
					
					PG_FREE_IF_COPY(vg1, 0);
					PG_FREE_IF_COPY(vg2, 1);
					PG_RETURN_POINTER(result);
				} 
								
				//the second has union
				if(HAS_PUNION(vg2->flags)) {
					if( gserialized_read_gbox_p_vaguegeom(vg2, &kbox2, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg2, &cbox2, 2) ){
						aux2 = LW_SUCCESS;
					} else {
						aux2 = LW_FAILURE;
					}
				}
				
				if(aux1 && aux2) {
					//check if they are disjoint: KERNEL X KERNEL, KERNEL X CONJECTURE
					if ( gbox_overlaps_2d(&kbox1, &kbox2) == LW_FALSE && gbox_overlaps_2d(&kbox1, &cbox2) == LW_FALSE) {
						result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
						result->vbool = VG_FALSE;

						PG_FREE_IF_COPY(vg1, 0);
						PG_FREE_IF_COPY(vg2, 1);
						PG_RETURN_POINTER(result);
					} 
				}
			}
		}
	}
	
	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	result = vaguegeom_coveredBy(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_covers);
Datum VG_covers(PG_FUNCTION_ARGS) {
	/* call the VG_coveredBy as vaguegeom_coveredby(vgeom2, vgeom1); */
	PG_RETURN_POINTER(
	  DatumGetPointer(
	    DirectFunctionCall2(VG_coveredBy, PG_GETARG_DATUM(1), PG_GETARG_DATUM(0))
		));
}

PG_FUNCTION_INFO_V1(VG_crosses);
Datum VG_crosses(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	VAGUEBOOL *result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	/*short circuit 1 */
	if(HAS_NOTHING(vg1->flags) || HAS_NOTHING(vg2->flags)) {
		result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
		result->vbool = VG_FALSE;
		
		PG_FREE_IF_COPY(vg1, 0);
		PG_FREE_IF_COPY(vg2, 1);
		PG_RETURN_POINTER(result);
	} else {
		GBOX box1, box2, kbox1, cbox1, kbox2, cbox2;
		int aux1=LW_FAILURE, aux2=LW_FAILURE;
		/*
		 * short-circuit 2: if geom2 bounding box does not overlap
		 * geom1 bounding box we can prematurely return TRUE.
		 * Do the test IFF BOUNDING BOX AVAILABLE.
		 */
		if(HAS_BOTH(vg1->flags) && HAS_BOTH(vg2->flags)) {
			if(HAS_PUNION(vg1->flags)) {
				aux1 = gserialized_read_gbox_p_vaguegeom(vg1, &box1, 3);
			} else {
				if( gserialized_read_gbox_p_vaguegeom(vg1, &kbox1, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg1, &cbox1, 2) ){
					aux1 = LW_SUCCESS;
					gbox_union(&kbox1, &cbox1, &box1);
				}
			}
			
			if(HAS_PUNION(vg2->flags)) {
				aux2 = gserialized_read_gbox_p_vaguegeom(vg2, &box2, 3);
			} else {
				if( gserialized_read_gbox_p_vaguegeom(vg2, &kbox2, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg2, &cbox2, 2) ){
					aux2 = LW_SUCCESS;
					gbox_union(&kbox2, &cbox2, &box2);
				}
			}
			
			if( aux1 && aux2 ){
				//allextension may overlap the other allextension?
				if ( gbox_overlaps_2d(&box1, &box2) == LW_FALSE ) {
					result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
					result->vbool = VG_FALSE;					
					
					PG_FREE_IF_COPY(vg1, 0);
					PG_FREE_IF_COPY(vg2, 1);
					PG_RETURN_POINTER(result);
				} 
				//they can overlap, lets check each kernel and conjecture
				
				//the first has union
				if(HAS_PUNION(vg1->flags)) {
					if( gserialized_read_gbox_p_vaguegeom(vg1, &kbox1, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg1, &cbox1, 2) ){
						aux1 = LW_SUCCESS;
					} else {
						aux1 = LW_FAILURE;
					}
				}
				//the second has union
				if(HAS_PUNION(vg2->flags)) {
					if( gserialized_read_gbox_p_vaguegeom(vg2, &kbox2, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg2, &cbox2, 2) ){
						aux2 = LW_SUCCESS;
					} else {
						aux2 = LW_FAILURE;
					}
				}
				
				if(aux1 && aux2) {
					//check if they are disjoint: KERNEL X KERNEL, KERNEL X CONJECTURE, CONJECTURE X KERNEL, CONJECTURE X CONJECTURE
					if ( gbox_overlaps_2d(&kbox1, &kbox2) == LW_FALSE && gbox_overlaps_2d(&kbox1, &cbox2) == LW_FALSE &&
					 gbox_overlaps_2d(&cbox1, &kbox2) == LW_FALSE && gbox_overlaps_2d(&cbox1, &cbox2) == LW_FALSE) {
						result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
						result->vbool = VG_FALSE;
						
						PG_FREE_IF_COPY(vg1, 0);
						PG_FREE_IF_COPY(vg2, 1);
						PG_RETURN_POINTER(result);
					} 
				}
			}
		}
	}
	
	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	result = vaguegeom_crosses(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_disjoint);
Datum VG_disjoint(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	VAGUEBOOL *result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	/*short circuit 1 */
	if(HAS_NOTHING(vg1->flags) || HAS_NOTHING(vg2->flags)) {
		result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
		result->vbool = VG_TRUE;
		
		PG_FREE_IF_COPY(vg1, 0);
		PG_FREE_IF_COPY(vg2, 1);
		PG_RETURN_POINTER(result);
	} else {
		GBOX box1, box2, kbox1, cbox1, kbox2, cbox2;
		int aux1=LW_FAILURE, aux2=LW_FAILURE;
		/*
		 * short-circuit 2: if geom2 bounding box does not overlap
		 * geom1 bounding box we can prematurely return TRUE.
		 * Do the test IFF BOUNDING BOX AVAILABLE.
		 */
		if(HAS_BOTH(vg1->flags) && HAS_BOTH(vg2->flags)) {
			if(HAS_PUNION(vg1->flags)) {
				aux1 = gserialized_read_gbox_p_vaguegeom(vg1, &box1, 3);
			} else {			
				if( gserialized_read_gbox_p_vaguegeom(vg1, &kbox1, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg1, &cbox1, 2) ){
					aux1 = LW_SUCCESS;
					gbox_union(&kbox1, &cbox1, &box1);
				}
			}
			
			if(HAS_PUNION(vg2->flags)) {
				aux2 = gserialized_read_gbox_p_vaguegeom(vg2, &box2, 3);
			} else {
				if( gserialized_read_gbox_p_vaguegeom(vg2, &kbox2, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg2, &cbox2, 2) ){
					aux2 = LW_SUCCESS;
					gbox_union(&kbox2, &cbox2, &box2);
				}
			}			
			if( aux1 && aux2 ){
				//allextension may overlap the other allextension?
				if ( gbox_overlaps_2d(&box1, &box2) == LW_FALSE ) {
					result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
					result->vbool = VG_TRUE;	
					
					PG_FREE_IF_COPY(vg1, 0);
					PG_FREE_IF_COPY(vg2, 1);
					PG_RETURN_POINTER(result);
				} 
				//they can overlap, lets check each kernel and conjecture
				
				//the first does not have union
				if(HAS_PUNION(vg1->flags)) {
					if( gserialized_read_gbox_p_vaguegeom(vg1, &kbox1, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg1, &cbox1, 2) ){
						aux1 = LW_SUCCESS;
					} else {
						aux1 = LW_FAILURE;
					}
				}
				//the second does not have union
				if(HAS_PUNION(vg2->flags)) {
					if( gserialized_read_gbox_p_vaguegeom(vg2, &kbox2, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg2, &cbox2, 2) ){
						aux2 = LW_SUCCESS;
					} else {
						aux2 = LW_FAILURE;
					}
				}
				
				if(aux1 && aux2) {
					//check if they are disjoint: KERNEL X KERNEL, KERNEL X CONJECTURE, CONJECTURE X KERNEL, CONJECTURE X CONJECTURE
					if ( gbox_overlaps_2d(&kbox1, &kbox2) == LW_FALSE && gbox_overlaps_2d(&kbox1, &cbox2) == LW_FALSE &&
					 gbox_overlaps_2d(&cbox1, &kbox2) == LW_FALSE && gbox_overlaps_2d(&cbox1, &cbox2) == LW_FALSE) {
						result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
						result->vbool = VG_TRUE;
												
						PG_FREE_IF_COPY(vg1, 0);
						PG_FREE_IF_COPY(vg2, 1);
						PG_RETURN_POINTER(result);
					} 
				}
			}
		}
	}
		
	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	result = vaguegeom_disjoint(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_equal);
Datum VG_equal(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	VAGUEBOOL *result;
	uint8_t type1, type2;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	/*short circuit 1 */
	if(HAS_NOTHING(vg1->flags) && HAS_NOTHING(vg2->flags)) {
		result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
		result->vbool = VG_TRUE;
		
		PG_FREE_IF_COPY(vg1, 0);
		PG_FREE_IF_COPY(vg2, 1);
		PG_RETURN_POINTER(result);
	} else {
		GBOX box1, box2, kbox1, cbox1, kbox2, cbox2;
		int aux1=LW_FAILURE, aux2=LW_FAILURE;
		/*
		 * short-circuit 2: if geom2 bounding box does not overlap
		 * geom1 bounding box we can prematurely return TRUE.
		 * Do the test IFF BOUNDING BOX AVAILABLE.
		 */
		 
		if(HAS_BOTH(vg1->flags) && HAS_BOTH(vg2->flags)) {
			if(HAS_PUNION(vg1->flags)) {
				aux1 = gserialized_read_gbox_p_vaguegeom(vg1, &box1, 3);
			} else {			
				if( gserialized_read_gbox_p_vaguegeom(vg1, &kbox1, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg1, &cbox1, 2) ){
					aux1 = LW_SUCCESS;
					gbox_union(&kbox1, &cbox1, &box1);
				}
			}
			
			if(HAS_PUNION(vg2->flags)) {
				aux2 = gserialized_read_gbox_p_vaguegeom(vg2, &box2, 3);
			} else {
				if( gserialized_read_gbox_p_vaguegeom(vg2, &kbox2, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg2, &cbox2, 2) ){
					aux2 = LW_SUCCESS;
					gbox_union(&kbox2, &cbox2, &box2);
				}
			}			
			if( aux1 && aux2 ){
				//allextension may overlap the other allextension?
				if ( gbox_overlaps_2d(&box1, &box2) == LW_FALSE ) {
					result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
					result->vbool = VG_FALSE;	
					
					PG_FREE_IF_COPY(vg1, 0);
					PG_FREE_IF_COPY(vg2, 1);
					PG_RETURN_POINTER(result);
				} 
				//they can overlap, lets check each kernel and conjecture
				
				//the first does not have union
				if(HAS_PUNION(vg1->flags)) {
					if( gserialized_read_gbox_p_vaguegeom(vg1, &kbox1, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg1, &cbox1, 2) ){
						aux1 = LW_SUCCESS;
					} else {
						aux1 = LW_FAILURE;
					}
				}
				//the second does not have union
				if(HAS_PUNION(vg2->flags)) {
					if( gserialized_read_gbox_p_vaguegeom(vg2, &kbox2, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg2, &cbox2, 2) ){
						aux2 = LW_SUCCESS;
					} else {
						aux2 = LW_FAILURE;
					}
				}
				
				if(aux1 && aux2) {
					//check if they are disjoint: KERNEL X KERNEL, KERNEL X CONJECTURE, CONJECTURE X KERNEL, CONJECTURE X CONJECTURE
					if ( gbox_overlaps_2d(&kbox1, &kbox2) == LW_FALSE && gbox_overlaps_2d(&kbox1, &cbox2) == LW_FALSE &&
					 gbox_overlaps_2d(&cbox1, &kbox2) == LW_FALSE && gbox_overlaps_2d(&cbox1, &cbox2) == LW_FALSE) {
						result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
						result->vbool = VG_FALSE;
												
						PG_FREE_IF_COPY(vg1, 0);
						PG_FREE_IF_COPY(vg2, 1);
						PG_RETURN_POINTER(result);
					} 
				}
			}
		}
	}
	
	type1 = vgserialized_get_type(vg1);
	type2 = vgserialized_get_type(vg2);
	if(type1 != type2) {
		result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
		result->vbool = VG_FALSE;
		PG_FREE_IF_COPY(vg1, 0);
		PG_FREE_IF_COPY(vg2, 1);
		PG_RETURN_POINTER(result);
	}
	
	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	result = vaguegeom_equal(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_inside);
Datum VG_inside(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	VAGUEBOOL *result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	/*short circuit 1 */
	if(HAS_NOTHING(vg1->flags) || HAS_NOTHING(vg2->flags)) {
		result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
		result->vbool = VG_FALSE;
		
		PG_FREE_IF_COPY(vg1, 0);
		PG_FREE_IF_COPY(vg2, 1);
		PG_RETURN_POINTER(result);
	} else {
		GBOX box2, kbox1, kbox2, cbox2;
		int aux1=LW_FAILURE, aux2=LW_FAILURE;
		/*
		 * short-circuit 2: if the kernel of vg1 is not inside of vg2 then we can return false
		 * Do the test IFF BOUNDING BOX AVAILABLE.
		 */
		if(HAS_BOTH(vg1->flags) && HAS_BOTH(vg2->flags)) {
			if( gserialized_read_gbox_p_vaguegeom(vg1, &kbox1, 1) ) {
				aux1 = LW_SUCCESS;
			}
			
			if(HAS_PUNION(vg2->flags)) {
				aux2 = gserialized_read_gbox_p_vaguegeom(vg2, &box2, 3);
			} else {
				if( gserialized_read_gbox_p_vaguegeom(vg2, &kbox2, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg2, &cbox2, 2) ){
					aux2 = LW_SUCCESS;
					gbox_union(&kbox2, &cbox2, &box2);
				}
			}
			
			if( aux1 && aux2 ){
				//the kernel is inside of the allextension? if not, then FALSE
				if ( gbox_inside_2d(&kbox1, &box2) == LW_FALSE ) {
					result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
					result->vbool = VG_FALSE;				
					
					PG_FREE_IF_COPY(vg1, 0);
					PG_FREE_IF_COPY(vg2, 1);
					PG_RETURN_POINTER(result);
				} 
								
				//the second has union
				if(HAS_PUNION(vg2->flags)) {
					if( gserialized_read_gbox_p_vaguegeom(vg2, &kbox2, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg2, &cbox2, 2) ){
						aux2 = LW_SUCCESS;
					} else {
						aux2 = LW_FAILURE;
					}
				}
				
				if(aux1 && aux2) {
					//check if they are disjoint: KERNEL X KERNEL, KERNEL X CONJECTURE
					if ( gbox_overlaps_2d(&kbox1, &kbox2) == LW_FALSE && gbox_overlaps_2d(&kbox1, &cbox2) == LW_FALSE) {
						result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
						result->vbool = VG_FALSE;
						
						PG_FREE_IF_COPY(vg1, 0);
						PG_FREE_IF_COPY(vg2, 1);
						PG_RETURN_POINTER(result);
					} 
				}
			}
		}
	}
	
	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	result = vaguegeom_inside(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_contains);
Datum VG_contains(PG_FUNCTION_ARGS) {
	/* call the VG_inside as vaguegeom_inside(vgeom2, vgeom1); */
	PG_RETURN_POINTER(
	  DatumGetPointer(
	    DirectFunctionCall2(VG_inside, PG_GETARG_DATUM(1), PG_GETARG_DATUM(0))
		));
}

PG_FUNCTION_INFO_V1(VG_intersects);
Datum VG_intersects(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	VAGUEBOOL *result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	/*short circuit 1 */
	if(HAS_NOTHING(vg1->flags) || HAS_NOTHING(vg2->flags)) {
		result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
		result->vbool = VG_FALSE;
		
		PG_FREE_IF_COPY(vg1, 0);
		PG_FREE_IF_COPY(vg2, 1);
		PG_RETURN_POINTER(result);
	} else {
		GBOX box1, box2, kbox1, cbox1, kbox2, cbox2;
		int aux1=LW_FAILURE, aux2=LW_FAILURE;
		/*
		 * short-circuit 2: if geom2 bounding box does not overlap
		 * geom1 bounding box we can prematurely return TRUE.
		 * Do the test IFF BOUNDING BOX AVAILABLE.
		 */
		if(HAS_BOTH(vg1->flags) && HAS_BOTH(vg2->flags)) {
			if(HAS_PUNION(vg1->flags)) {
				aux1 = gserialized_read_gbox_p_vaguegeom(vg1, &box1, 3);
			} else {
				if( gserialized_read_gbox_p_vaguegeom(vg1, &kbox1, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg1, &cbox1, 2) ){
					aux1 = LW_SUCCESS;
					gbox_union(&kbox1, &cbox1, &box1);
				}
			}
			
			if(HAS_PUNION(vg2->flags)) {
				aux2 = gserialized_read_gbox_p_vaguegeom(vg2, &box2, 3);
			} else {
				if( gserialized_read_gbox_p_vaguegeom(vg2, &kbox2, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg2, &cbox2, 2) ){
					aux2 = LW_SUCCESS;
					gbox_union(&kbox2, &cbox2, &box2);
				}
			}
			
			if( aux1 && aux2 ){
				//allextension may overlap the other allextension?
				if ( gbox_overlaps_2d(&box1, &box2) == LW_FALSE ) {
					result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
					result->vbool = VG_FALSE;					
				
					PG_FREE_IF_COPY(vg1, 0);
					PG_FREE_IF_COPY(vg2, 1);
					PG_RETURN_POINTER(result);
				} 
				//they can overlap, lets check each kernel and conjecture
				
				//the first does not have union
				if(HAS_PUNION(vg1->flags)) {
					if( gserialized_read_gbox_p_vaguegeom(vg1, &kbox1, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg1, &cbox1, 2) ){
						aux1 = LW_SUCCESS;
					} else {
						aux1 = LW_FAILURE;
					}
				}
				//the second does not have union
				if(HAS_PUNION(vg2->flags)) {
					if( gserialized_read_gbox_p_vaguegeom(vg2, &kbox2, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg2, &cbox2, 2) ){
						aux2 = LW_SUCCESS;
					} else {
						aux2 = LW_FAILURE;
					}
				}
				
				if(aux1 && aux2) {
					//check if they are disjoint: KERNEL X KERNEL, KERNEL X CONJECTURE, CONJECTURE X KERNEL, CONJECTURE X CONJECTURE
					if ( gbox_overlaps_2d(&kbox1, &kbox2) == LW_FALSE && gbox_overlaps_2d(&kbox1, &cbox2) == LW_FALSE &&
					 gbox_overlaps_2d(&cbox1, &kbox2) == LW_FALSE && gbox_overlaps_2d(&cbox1, &cbox2) == LW_FALSE) {
						result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
						result->vbool = VG_FALSE;
						
						PG_FREE_IF_COPY(vg1, 0);
						PG_FREE_IF_COPY(vg2, 1);
						PG_RETURN_POINTER(result);
					} 
				}
			}
		}
	}
	
	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	result = vaguegeom_intersects(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_meets);
Datum VG_meets(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	VAGUEBOOL *result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	/*short circuit 1 */
	if(HAS_NOTHING(vg1->flags) || HAS_NOTHING(vg2->flags)) {
		result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
		result->vbool = VG_FALSE;
		
		PG_FREE_IF_COPY(vg1, 0);
		PG_FREE_IF_COPY(vg2, 1);
		PG_RETURN_POINTER(result);
	} else {
		GBOX box1, box2, kbox1, cbox1, kbox2, cbox2;
		int aux1=LW_FAILURE, aux2=LW_FAILURE;
		/*
		 * short-circuit 2: if geom2 bounding box does not overlap
		 * geom1 bounding box we can prematurely return TRUE.
		 * Do the test IFF BOUNDING BOX AVAILABLE.
		 */
		if(HAS_BOTH(vg1->flags) && HAS_BOTH(vg2->flags)) {
			if(HAS_PUNION(vg1->flags)) {
				aux1 = gserialized_read_gbox_p_vaguegeom(vg1, &box1, 3);
			} else {
				if( gserialized_read_gbox_p_vaguegeom(vg1, &kbox1, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg1, &cbox1, 2) ){
					aux1 = LW_SUCCESS;
					gbox_union(&kbox1, &cbox1, &box1);
				}
			}
			
			if(HAS_PUNION(vg2->flags)) {
				aux2 = gserialized_read_gbox_p_vaguegeom(vg2, &box2, 3);
			} else {
				if( gserialized_read_gbox_p_vaguegeom(vg2, &kbox2, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg2, &cbox2, 2) ){
					aux2 = LW_SUCCESS;
					gbox_union(&kbox2, &cbox2, &box2);
				}
			}
			
			if( aux1 && aux2 ){
				//allextension may overlap the other allextension?
				if ( gbox_overlaps_2d(&box1, &box2) == LW_FALSE ) {
					result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
					result->vbool = VG_FALSE;					
					
					PG_FREE_IF_COPY(vg1, 0);
					PG_FREE_IF_COPY(vg2, 1);
					PG_RETURN_POINTER(result);
				} 
				//they can overlap, lets check each kernel and conjecture
				
				//the first does not have union
				if(HAS_PUNION(vg1->flags)) {
					if( gserialized_read_gbox_p_vaguegeom(vg1, &kbox1, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg1, &cbox1, 2) ){
						aux1 = LW_SUCCESS;
					} else {
						aux1 = LW_FAILURE;
					}
				}
				//the second does not have union
				if(HAS_PUNION(vg2->flags)) {
					if( gserialized_read_gbox_p_vaguegeom(vg2, &kbox2, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg2, &cbox2, 2) ){
						aux2 = LW_SUCCESS;
					} else {
						aux2 = LW_FAILURE;
					}
				}
				
				if(aux1 && aux2) {
					//check if they are disjoint: KERNEL X KERNEL, KERNEL X CONJECTURE, CONJECTURE X KERNEL, CONJECTURE X CONJECTURE
					if ( gbox_overlaps_2d(&kbox1, &kbox2) == LW_FALSE && gbox_overlaps_2d(&kbox1, &cbox2) == LW_FALSE &&
					 gbox_overlaps_2d(&cbox1, &kbox2) == LW_FALSE && gbox_overlaps_2d(&cbox1, &cbox2) == LW_FALSE) {
						result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
						result->vbool = VG_FALSE;
						
						PG_FREE_IF_COPY(vg1, 0);
						PG_FREE_IF_COPY(vg2, 1);
						PG_RETURN_POINTER(result);
					} 
				}
			}
		}
	}
	
	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	result = vaguegeom_meet(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_on_border_of);
Datum VG_on_border_of(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	VAGUEBOOL *result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	result = vaguegeom_on_border_of(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_border_in_common);
Datum VG_border_in_common(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	VAGUEBOOL *result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	result = vaguegeom_border_in_common(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(VG_overlap);
Datum VG_overlap(PG_FUNCTION_ARGS) {
	VAGUEGEOMSERIALIZED *vg1, *vg2;
	VAGUEGEOM *vague1, *vague2;
	VAGUEBOOL *result;

	vg1 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	vg2 = (VAGUEGEOMSERIALIZED*) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));

	/*short circuit 1 */
	if(HAS_NOTHING(vg1->flags) || HAS_NOTHING(vg2->flags)) {
		result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
		result->vbool = VG_FALSE;
		
		PG_FREE_IF_COPY(vg1, 0);
		PG_FREE_IF_COPY(vg2, 1);
		PG_RETURN_POINTER(result);
	} else {
		GBOX box1, box2, kbox1, cbox1, kbox2, cbox2;
		int aux1=LW_FAILURE, aux2=LW_FAILURE;
		/*
		 * short-circuit 2: if geom2 bounding box does not overlap
		 * geom1 bounding box we can prematurely return TRUE.
		 * Do the test IFF BOUNDING BOX AVAILABLE.
		 */
		if(HAS_BOTH(vg1->flags) && HAS_BOTH(vg2->flags)) {
			if(HAS_PUNION(vg1->flags)) {
				aux1 = gserialized_read_gbox_p_vaguegeom(vg1, &box1, 3);
			} else {
				if( gserialized_read_gbox_p_vaguegeom(vg1, &kbox1, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg1, &cbox1, 2) ){
					aux1 = LW_SUCCESS;
					gbox_union(&kbox1, &cbox1, &box1);
				}
			}
			
			if(HAS_PUNION(vg2->flags)) {
				aux2 = gserialized_read_gbox_p_vaguegeom(vg2, &box2, 3);
			} else {
				if( gserialized_read_gbox_p_vaguegeom(vg2, &kbox2, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg2, &cbox2, 2) ){
					aux2 = LW_SUCCESS;
					gbox_union(&kbox2, &cbox2, &box2);
				}
			}
			
			if( aux1 && aux2 ){
				//allextension may overlap the other allextension?
				if ( gbox_overlaps_2d(&box1, &box2) == LW_FALSE ) {
					result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
					result->vbool = VG_FALSE;					
					
					PG_FREE_IF_COPY(vg1, 0);
					PG_FREE_IF_COPY(vg2, 1);
					PG_RETURN_POINTER(result);
				} 
				//they can overlap, lets check each kernel and conjecture
				
				//the first has union
				if(HAS_PUNION(vg1->flags)) {
					if( gserialized_read_gbox_p_vaguegeom(vg1, &kbox1, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg1, &cbox1, 2) ){
						aux1 = LW_SUCCESS;
					} else {
						aux1 = LW_FAILURE;
					}
				}
				//the second has union
				if(HAS_PUNION(vg2->flags)) {
					if( gserialized_read_gbox_p_vaguegeom(vg2, &kbox2, 1) &&
					gserialized_read_gbox_p_vaguegeom(vg2, &cbox2, 2) ){
						aux2 = LW_SUCCESS;
					} else {
						aux2 = LW_FAILURE;
					}
				}
				
				if(aux1 && aux2) {
					//check if they are disjoint: KERNEL X KERNEL, KERNEL X CONJECTURE, CONJECTURE X KERNEL, CONJECTURE X CONJECTURE
					if ( gbox_overlaps_2d(&kbox1, &kbox2) == LW_FALSE && gbox_overlaps_2d(&kbox1, &cbox2) == LW_FALSE &&
					 gbox_overlaps_2d(&cbox1, &kbox2) == LW_FALSE && gbox_overlaps_2d(&cbox1, &cbox2) == LW_FALSE) {
						result = (VAGUEBOOL*) lwalloc(sizeof(VAGUEBOOL));
						result->vbool = VG_FALSE;
					
						PG_FREE_IF_COPY(vg1, 0);
						PG_FREE_IF_COPY(vg2, 1);
						PG_RETURN_POINTER(result);
					} 
				}
			}
		}
	}
	
	vague1 = serialization_to_vaguegeom(vg1);
	valid_convertion_vaguegeom(vague1);

	vague2 = serialization_to_vaguegeom(vg2);
	valid_convertion_vaguegeom(vague2);

	result = vaguegeom_overlap(vague1, vague2);

	/* memory free */
	vaguegeom_free(vague1);
	vaguegeom_free(vague2);
	PG_FREE_IF_COPY(vg1, 0);
	PG_FREE_IF_COPY(vg2, 1);
	PG_RETURN_POINTER(result);
}