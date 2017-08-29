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
* Vague Well-Known Text implementation - it is based on the WKT of the PostGIS
*
*************************/

#include "libvgeom.h"
#include <float.h>

#include "lwgeom_pg.h"

static LWGEOM *get_lwgeom(char *str, char *input);
static char *get_crisp_type_from_vaguegeom(uint8_t crisp_type);

LWGEOM *get_lwgeom(char *str, char *input) {	
	LWGEOM_PARSER_RESULT lwg_parser_result;
	int srid=0;
	LWGEOM *lwgeom =NULL;
	lwgeom_parser_result_init(&lwg_parser_result);
	if( strncasecmp(str,"SRID=",5) == 0 ) {
		/* Roll forward to semi-colon */
		char *tmp = str;
		while ( tmp && *tmp != ';' )
			tmp++;
		
		/* Check next character to see if we have WKB  */
		if ( tmp && *(tmp+1) == '0' )
		{
			/* Null terminate the SRID= string */
			*tmp = '\0';
			/* Set str to the start of the real WKB */
			str = tmp + 1;
			/* Move tmp to the start of the numeric part */
			tmp = input + 5;
			/* Parse out the SRID number */
			srid = atoi(tmp);
		}
	}
	
	/* WKB? Let's find out. */
	if ( str[0] == '0' ) {
		size_t hexsize = strlen(str);
		unsigned char *wkb = bytes_from_hexbytes(str, hexsize);
		/* TODO: 20101206: No parser checks! This is inline with current 1.5 behavior, but needs discussion */
		lwgeom = lwgeom_from_wkb(wkb, hexsize/2, LW_PARSER_CHECK_NONE);
		/* If we picked up an SRID at the head of the WKB set it manually */
		if ( srid ) lwgeom_set_srid(lwgeom, srid);
		/* Add a bbox if necessary */
		if ( lwgeom_needs_bbox(lwgeom) ) lwgeom_add_bbox(lwgeom);
		pfree(wkb);
	}
	/* WKT then. */
	else {
		if ( lwgeom_parse_wkt(&lwg_parser_result, str, LW_PARSER_CHECK_ALL) == LW_FAILURE )	{
			PG_PARSER_ERROR(lwg_parser_result);
		}
		lwgeom = lwgeom_clone_deep(lwg_parser_result.geom);
		if ( lwgeom_needs_bbox(lwgeom) )
			lwgeom_add_bbox(lwgeom);
		lwgeom_parser_result_free(&lwg_parser_result);
	}
	return lwgeom;
}

char *get_crisp_type_from_vaguegeom(uint8_t crisp_type) {
	switch (crisp_type)
	{
case VAGUEPOINTTYPE:
		return "POINT";
case VAGUELINETYPE:
		return "LINESTRING";
case VAGUEPOLYGONTYPE:
	return "POLYGON";
case VAGUEMULTIPOINTTYPE:
	return "MULTIPOINT";
case VAGUEMULTILINETYPE:
	return "MULTILINESTRING";
case VAGUEMULTIPOLYGONTYPE:
	return "MULTIPOLYGON";
	default:
		return "";
	}	
}

uint8_t get_potgis_type_from_vaguegeom(uint8_t vg_type) {
	switch (vg_type)
	{
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
	default:
		return VAGUECOLLECTIONTYPE;
	}	
}

extern VAGUEGEOM *vgeom_from_vwkt(char *vwkt, uint8_t punion) {
	LWGEOM *lwgeom_kernel=NULL, *lwgeom_conjecture=NULL;
	char *str = vwkt;
	int srid=0;
	//need read the srid too
	while(isspace(*str))
		str++;
	if( strncasecmp(str,"SRID=",5) == 0 ) {
		str+=5;	
		vwkt+=5;
		srid = strtol(vwkt, &str, 10);
		str+=1; //jump the ';'
	}			
	while(isspace(*str))
		str++;

	if(strncasecmp(str, "VAGUE", 5)==0) {
		char *after_semicolon;
		int pos=0;
		uint8_t type;
		if(strncasecmp(str, "VAGUEPOINT", 10)==0) {
			type = VAGUEPOINTTYPE;
			str+=10;
		} else if (strncasecmp(str, "VAGUELINESTRING", 15)==0) {
			type = VAGUELINETYPE;
			str+=15;
		} else if (strncasecmp(str, "VAGUEPOLYGON", 12)==0) {
			type = VAGUEPOLYGONTYPE;
			str+=12;
		} else if(strncasecmp(str, "VAGUEMULTIPOINT", 15)==0) {
			type = VAGUEMULTIPOINTTYPE;
			str+=15;
		} else if(strncasecmp(str, "VAGUEMULTILINESTRING", 20)==0) {
			type = VAGUEMULTILINETYPE;
			str+=20;
		} else if(strncasecmp(str, "VAGUEMULTIPOLYGON", 17)==0) {
			type = VAGUEMULTIPOLYGONTYPE;
			str+=17;
		} else {
			//there is not type then report an error
			ereport(ERROR,(errmsg("parse error - invalid vague geometry type")));
		}

		while(isspace(*str))
			str++;

		//vague geometry EMPTY
		if(strncasecmp(str, "EMPTY", 5)==0) {
			str+=5;
			while(isspace(*str))
				str++;
			if(*str != '\0')
				ereport(ERROR,(errmsg("parse error - to define a empty vaguegeometry please inform just VAGUETYPE EMPTY")));

			lwgeom_kernel = lwgeom_construct_empty(get_potgis_type_from_vaguegeom(type), srid, 0, 0);
			lwgeom_conjecture = lwgeom_construct_empty(get_potgis_type_from_vaguegeom(type), srid, 0, 0);
			return vaguegeom_construct(lwgeom_kernel, lwgeom_conjecture, punion); 
		}

		while ( str && *str != '(' ) str++;
		str+=1; //jump the (
		after_semicolon = (char*)lwalloc(strlen(str));
		
		strcpy(after_semicolon, str);
		
		while(*after_semicolon != ';') {
			++pos;
			after_semicolon++;
		}
				
		if(*after_semicolon == '\0') {
			//there is only kernel but need to definy a conjecture too!
			ereport(ERROR,(errmsg("parse error - only kernel was detected, the conjecture part must be definied as well.")));
		} else { //kernel and conjecture
			char *kernel, *conjecture;
			char *input2, *input;
			after_semicolon+=1; //jump the semicolon

			//verifies if is the simplify version or not
			if(strncasecmp(str, "EMPTY", 5)==0) {
				kernel = (char*)lwalloc(20+strlen(str));
				str[pos] = '\0';
				snprintf(kernel, 20+strlen(str), "%s %s", get_crisp_type_from_vaguegeom(type), str);
			} else if(isdigit(*str) || *str == '(' || *str == '-') {
				kernel = (char*)lwalloc(50+strlen(str));
				str[pos] = '\0';
				snprintf(kernel, 50+strlen(str), "%s(%s)", get_crisp_type_from_vaguegeom(type), str);
			} else {
				kernel = (char*)lwalloc(pos+1);		
				str[pos] = '\0';			
				snprintf(kernel, pos+1, "%s", str);
			}
			while(isspace(*after_semicolon))
				after_semicolon++;
			if(strncasecmp(after_semicolon, "EMPTY", 5)==0) {
				conjecture = (char*)lwalloc(20+strlen(after_semicolon));
				after_semicolon[strlen(after_semicolon)-1] = '\0';
				snprintf(conjecture, 20+strlen(after_semicolon), "%s %s", get_crisp_type_from_vaguegeom(type), after_semicolon);				
			} else if(isdigit(*after_semicolon) || *after_semicolon == '(' || *after_semicolon == '-') {
				conjecture = (char*)lwalloc(50+strlen(after_semicolon)-1);
				snprintf(conjecture, 50+strlen(after_semicolon)-1, "%s(%s", get_crisp_type_from_vaguegeom(type), after_semicolon);
			} else {
				int len = strlen(after_semicolon);
				conjecture = (char*) lwalloc(len);
				after_semicolon[len-1] = '\0';
				snprintf(conjecture, len, "%s", after_semicolon);
			}

			input = kernel;
			lwgeom_kernel = get_lwgeom(kernel, input);

			input2 = conjecture;
			lwgeom_conjecture = get_lwgeom(conjecture, input2);

			lwfree(conjecture);
			lwfree(kernel);	
		} 
		if(lwgeom_kernel != NULL) {
			/*check if the kernel type is compatible with the vaguegeometry type */
			if(vaguegeom_compatible_type(vaguegeom_gettype_i(type), lwgeom_kernel->type) ==LW_FALSE) {
				lwerror("The vague geometry type (%s) is not compatible with the conjecture type (%s)", vaguegeom_gettype_i(type), lwtype_name(lwgeom_kernel->type));
			}
		}
		if(lwgeom_conjecture != NULL) {
			/*check if the conjecture type is compatible with the vaguegeometry type */
			if(vaguegeom_compatible_type(vaguegeom_gettype_i(type), lwgeom_conjecture->type) ==LW_FALSE) {
				lwerror("The vague geometry type (%s) is not compatible with the conjecture type (%s)", vaguegeom_gettype_i(type), lwtype_name(lwgeom_conjecture->type));
			}
		}
	} else {
		//there is not type or geometries.. then report an error
		ereport(ERROR,(errmsg("parse error - invalid vague geometry - if the vaguegeometry is empty, specify the kernel and conjecture types using empty objects or use the format VAGUETYPE EMPTY")));
	}
	if(lwgeom_kernel!=NULL)
		lwgeom_set_srid(lwgeom_kernel, srid);
	if(lwgeom_conjecture!=NULL)
		lwgeom_set_srid(lwgeom_conjecture, srid);
		
	return vaguegeom_construct(lwgeom_kernel, lwgeom_conjecture, punion);
}

extern char *vgeom_to_vwkt(const VAGUEGEOM *vgeom, uint8_t is_simplified, uint8_t is_extended) {
	char *k_wkt, *c_wkt, *ewkt;
	/* Write to WKT and free the geometry */
	k_wkt = lwgeom_to_wkt(vgeom->kernel, WKT_ISO, DBL_DIG, NULL);
	c_wkt = lwgeom_to_wkt(vgeom->conjecture, WKT_ISO, DBL_DIG, NULL);
	
	ewkt = (char*)lwalloc(strlen(k_wkt)+strlen(c_wkt)+100);

	if(is_extended == LW_FALSE) {
		if(is_simplified == LW_TRUE) {
			char *k, *c;
			k = (char*)lwalloc(strlen(k_wkt));
			c = (char*)lwalloc(strlen(c_wkt));
			switch (vgeom->type) {
				case VAGUEPOINTTYPE:
					strncpy(k, k_wkt+6, strlen(k_wkt)-1);
					if(HAS_KERNEL(vgeom->flags))
						k[strlen(k_wkt)-7]='\0';
					strncpy(c, c_wkt+6, strlen(c_wkt)-1);
					if(HAS_CONJECTURE(vgeom->flags))
						c[strlen(c_wkt)-7]='\0';
					break;
				case VAGUELINETYPE:
					strncpy(k, k_wkt+11, strlen(k_wkt)-1);
					if(HAS_KERNEL(vgeom->flags))
						k[strlen(k_wkt)-12]='\0';
					strncpy(c, c_wkt+11, strlen(c_wkt)-1);
					if(HAS_CONJECTURE(vgeom->flags))
						c[strlen(c_wkt)-12]='\0';
					break;
				case VAGUEPOLYGONTYPE:
					strncpy(k, k_wkt+8, strlen(k_wkt)-1);
					if(HAS_KERNEL(vgeom->flags))
						k[strlen(k_wkt)-9]='\0';
					strncpy(c, c_wkt+8, strlen(c_wkt)-1);
					if(HAS_CONJECTURE(vgeom->flags))
						c[strlen(c_wkt)-9]='\0';
					break;
				case VAGUEMULTIPOINTTYPE:
					strncpy(k, k_wkt+11, strlen(k_wkt)-1);
					if(HAS_KERNEL(vgeom->flags))
						k[strlen(k_wkt)-12]='\0';
					strncpy(c, c_wkt+11, strlen(c_wkt)-1);
					if(HAS_CONJECTURE(vgeom->flags))
						c[strlen(c_wkt)-12]='\0';
					break;
				case VAGUEMULTILINETYPE:
					strncpy(k, k_wkt+16, strlen(k_wkt)-1);
					if(HAS_KERNEL(vgeom->flags))
						k[strlen(k_wkt)-17]='\0';
					strncpy(c, c_wkt+16, strlen(c_wkt)-1);
					if(HAS_CONJECTURE(vgeom->flags))
						c[strlen(c_wkt)-17]='\0';
					break;
				case VAGUEMULTIPOLYGONTYPE:
					strncpy(k, k_wkt+13, strlen(k_wkt)-1);
					if(HAS_KERNEL(vgeom->flags))
						k[strlen(k_wkt)-14]='\0';
					strncpy(c, c_wkt+13, strlen(c_wkt)-1);
					if(HAS_CONJECTURE(vgeom->flags))
						c[strlen(c_wkt)-14]='\0';
					break;
			}
			snprintf(ewkt, strlen(k)+strlen(c)+40, "%s(%s; %s)", vaguegeom_gettype_i(vgeom->type), k, c);
			lwfree(c);
			lwfree(k);
		}
		else
			snprintf(ewkt, strlen(k_wkt)+strlen(c_wkt)+40, "%s(%s; %s)", vaguegeom_gettype_i(vgeom->type), k_wkt, c_wkt); 
	} else {
		if(is_simplified == LW_TRUE) {
			char *k, *c;
			k = (char*)lwalloc(strlen(k_wkt));
			c = (char*)lwalloc(strlen(c_wkt));
			switch (vgeom->type) {
				case VAGUEPOINTTYPE:
					strncpy(k, k_wkt+6, strlen(k_wkt)-1);
					if(HAS_KERNEL(vgeom->flags))
						k[strlen(k_wkt)-7]='\0';
					strncpy(c, c_wkt+6, strlen(c_wkt)-1);
					if(HAS_CONJECTURE(vgeom->flags))
						c[strlen(c_wkt)-7]='\0';
					break;
				case VAGUELINETYPE:
					strncpy(k, k_wkt+11, strlen(k_wkt)-1);
					if(HAS_KERNEL(vgeom->flags))
						k[strlen(k_wkt)-12]='\0';
					strncpy(c, c_wkt+11, strlen(c_wkt)-1);
					if(HAS_CONJECTURE(vgeom->flags))
						c[strlen(c_wkt)-12]='\0';
					break;
				case VAGUEPOLYGONTYPE:
					strncpy(k, k_wkt+8, strlen(k_wkt)-1);
					if(HAS_KERNEL(vgeom->flags))
						k[strlen(k_wkt)-9]='\0';
					strncpy(c, c_wkt+8, strlen(c_wkt)-1);
					if(HAS_CONJECTURE(vgeom->flags))
						c[strlen(c_wkt)-9]='\0';
					break;
				case VAGUEMULTIPOINTTYPE:
					strncpy(k, k_wkt+11, strlen(k_wkt)-1);
					if(HAS_KERNEL(vgeom->flags))
						k[strlen(k_wkt)-12]='\0';
					strncpy(c, c_wkt+11, strlen(c_wkt)-1);
					if(HAS_CONJECTURE(vgeom->flags))
						c[strlen(c_wkt)-12]='\0';
					break;
				case VAGUEMULTILINETYPE:
					strncpy(k, k_wkt+16, strlen(k_wkt)-1);
					if(HAS_KERNEL(vgeom->flags))
						k[strlen(k_wkt)-17]='\0';
					strncpy(c, c_wkt+16, strlen(c_wkt)-1);
					if(HAS_CONJECTURE(vgeom->flags))
						c[strlen(c_wkt)-17]='\0';
					break;
				case VAGUEMULTIPOLYGONTYPE:
					strncpy(k, k_wkt+13, strlen(k_wkt)-1);
					if(HAS_KERNEL(vgeom->flags))
						k[strlen(k_wkt)-14]='\0';
					strncpy(c, c_wkt+13, strlen(c_wkt)-1);
					if(HAS_CONJECTURE(vgeom->flags))
						c[strlen(c_wkt)-14]='\0';
					break;
			}
			snprintf(ewkt, strlen(k)+strlen(c)+40, "SRID=%d;%s(%s; %s)", vgeom->kernel->srid, vaguegeom_gettype_i(vgeom->type), k, c);
			lwfree(c);
			lwfree(k);
		}
		else
			snprintf(ewkt, strlen(k_wkt)+strlen(c_wkt)+40, "SRID=%d;%s(%s; %s)", vgeom->kernel->srid, vaguegeom_gettype_i(vgeom->type), k_wkt, c_wkt);
	}

	pfree(k_wkt);
	pfree(c_wkt);
	return ewkt;
}
