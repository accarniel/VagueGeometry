--**********************************************************************
-- *
-- * VagueGeometry - Vague Spatial Objects for PostgreSQL
-- * http://gbd.dc.ufscar.br/vaguegeometry/
-- *
-- * Copyright 2013-2016 Anderson Chaves Carniel <accarniel@gmail.com>
-- *
-- * This is free software; you can redistribute and/or modify it under
-- * the terms of the GNU General Public Licence. See the COPYING file.
-- *
-- * Fully developed by Anderson Chaves Carniel
-- *
-- **********************************************************************/

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION vasavgeom" to load this file. \quit

-------------------------------------------------------------------
-- VAGUE GEOMETRY TYPE (VGEOM)
-------------------------------------------------------------------
CREATE OR REPLACE FUNCTION vg_in(cstring)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME','VG_in'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION vg_out(vaguegeometry)
	RETURNS cstring
	AS 'MODULE_PATHNAME','VG_out'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION vg_typmod_in(cstring[])
	RETURNS integer
	AS 'MODULE_PATHNAME','VG_typmod_in'
	LANGUAGE 'c' IMMUTABLE STRICT; 

CREATE OR REPLACE FUNCTION vg_typmod_out(integer)
	RETURNS cstring
	AS 'MODULE_PATHNAME','VG_typmod_out'
	LANGUAGE 'c' IMMUTABLE STRICT; 

CREATE OR REPLACE FUNCTION vg_recv(internal)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME','VG_recv'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION vg_send(vaguegeometry)
	RETURNS bytea
	AS 'MODULE_PATHNAME','VG_send'
	LANGUAGE 'c' IMMUTABLE STRICT;

--TODO implement the analyze function

CREATE TYPE vaguegeometry (
	internallength = variable,
	input = vg_in,
	output = vg_out,
	send = vg_send,
	receive = vg_recv,
	typmod_in = vg_typmod_in,
	typmod_out = vg_typmod_out,
	delimiter = ':',
	category = 'G',
	alignment = double,
	storage = main
);

-- Special cast for enforcing the typmod restrictions
CREATE OR REPLACE FUNCTION vg(vaguegeometry, integer, boolean)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME','VG_enforce_typmod'
	LANGUAGE 'c' IMMUTABLE STRICT; 

CREATE CAST (vaguegeometry AS vaguegeometry) WITH FUNCTION vg(vaguegeometry, integer, boolean) AS IMPLICIT;

------------------------------------------------------
-- VAGUE BOOL ---
------------------------------------------------------

CREATE OR REPLACE FUNCTION vb_in(cstring)
	RETURNS vaguebool
	AS 'MODULE_PATHNAME','VB_in'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION vb_out(vaguebool)
	RETURNS cstring
	AS 'MODULE_PATHNAME','VB_out'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION vb_recv(internal)
	RETURNS vaguebool
	AS 'MODULE_PATHNAME','VB_recv'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION vb_send(vaguebool)
	RETURNS bytea
	AS 'MODULE_PATHNAME','VB_send'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE TYPE vaguebool (
	internallength = 1,
	input = vb_in,
	output = vb_out,
	send = vb_send,
	receive = vb_recv,
	delimiter = ',',
	category = 'B',
	alignment = char,
	storage = plain
);

--IMPLICIT CAST
CREATE OR REPLACE FUNCTION vaguebool_cast(vaguebool, integer, boolean)
	RETURNS boolean
	AS 'MODULE_PATHNAME','VB_enforce_cast'
	LANGUAGE 'c' IMMUTABLE STRICT; 

CREATE CAST (vaguebool AS boolean) WITH FUNCTION vaguebool_cast(vaguebool, integer, boolean) AS IMPLICIT;


-- OPERATORS

---------------------------------------------------------------
-- TRUTH TABLE OF VAGUEBOOL --
---------------------------------------------------------------
CREATE OR REPLACE FUNCTION not_vb(vaguebool)
	RETURNS vaguebool
	AS 'MODULE_PATHNAME', 'VB_not_op'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION vb_and_vb(vaguebool, vaguebool)
	RETURNS vaguebool
	AS 'MODULE_PATHNAME', 'VB_and_VB'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION vb_or_vb(vaguebool, vaguebool)
	RETURNS vaguebool
	AS 'MODULE_PATHNAME', 'VB_or_VB'
	LANGUAGE 'c' IMMUTABLE STRICT;

--UNARY OPERATORS
CREATE OR REPLACE FUNCTION true_vb(vaguebool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'VB_true_op'
	LANGUAGE 'c' IMMUTABLE STRICT;
	
CREATE OR REPLACE FUNCTION false_vb(vaguebool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'VB_false_op'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION maybe_vb(vaguebool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'VB_maybe_op'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION maybe_maybe_vb(vaguebool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'VB_maybe_maybe_op'
	LANGUAGE 'c' IMMUTABLE STRICT;
	

--OPERATORS FOR VAGUEBOOL
CREATE OPERATOR ! (
	RIGHTARG = vaguebool,
	PROCEDURE = not_vb
);

CREATE OPERATOR && (
	PROCEDURE = vb_and_vb,
	RIGHTARG = vaguebool,
	LEFTARG = vaguebool,
	COMMUTATOR = &&
);

CREATE OPERATOR || (
	PROCEDURE = vb_or_vb,
	RIGHTARG = vaguebool,
	LEFTARG = vaguebool,
	COMMUTATOR = ||
);

CREATE OPERATOR & (
	RIGHTARG = vaguebool,
	PROCEDURE = true_vb
);

CREATE OPERATOR !! (
	RIGHTARG = vaguebool,
	PROCEDURE = false_vb
);

CREATE OPERATOR ~ (
	RIGHTARG = vaguebool,
	PROCEDURE = maybe_vb
);

CREATE OPERATOR ~~ (
	RIGHTARG = vaguebool,
	PROCEDURE = maybe_maybe_vb
);

------------------------------------------------------
-- VAGUE NUMERIC ---
------------------------------------------------------

CREATE OR REPLACE FUNCTION vn_in(cstring)
	RETURNS vaguenumeric
	AS 'MODULE_PATHNAME','VN_in'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION vn_out(vaguenumeric)
	RETURNS cstring
	AS 'MODULE_PATHNAME','VN_out'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION vn_recv(internal)
	RETURNS vaguenumeric
	AS 'MODULE_PATHNAME','VN_recv'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION vn_send(vaguenumeric)
	RETURNS bytea
	AS 'MODULE_PATHNAME','VN_send'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE TYPE vaguenumeric (
	internallength = 16,
	input = vn_in,
	output = vn_out,
	send = vn_send,
	receive = vn_recv,
	delimiter = ',',
	category = 'N',
	alignment = double,
	storage = plain
);

-- OPERATOR FOR VAGUE NUMERIC

CREATE OR REPLACE FUNCTION maybe_vn(vaguenumeric, float8)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'VN_maybe_op'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION equals_vn(vaguenumeric, float8)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'VN_equals_op'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION nequals_vn(vaguenumeric, float8)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'VN_nequals_op'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OPERATOR ~ (
	PROCEDURE = maybe_vn,
	LEFTARG = vaguenumeric,
	RIGHTARG = float8,
	COMMUTATOR = ~
);

CREATE OPERATOR = (
	PROCEDURE = equals_vn,
	LEFTARG = vaguenumeric,
	RIGHTARG = float8,
	COMMUTATOR = =,
	NEGATOR = !=,
	RESTRICT = eqsel,
	JOIN = eqjoinsel
);

CREATE OPERATOR != (
	PROCEDURE = nequals_vn,
	LEFTARG = vaguenumeric,
	RIGHTARG = float8,
	COMMUTATOR = =,
	NEGATOR = !=,
	RESTRICT = neqsel,
	JOIN = neqjoinsel
);

---------------------------------------------------
-- GEOMETRIC SET OPERATIONS --
--------------------------------------------------

CREATE OR REPLACE FUNCTION VG_Intersection(vaguegeometry, vaguegeometry)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_intersection'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION VG_Union(vaguegeometry, vaguegeometry)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_union'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION VG_Union(vaguegeometry[])
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_union_vgarray'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE AGGREGATE VG_Union (
	basetype = vaguegeometry,
	sfunc = VG_Union,
	stype = vaguegeometry
);

CREATE OR REPLACE FUNCTION VG_Difference(vaguegeometry, vaguegeometry)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_difference'
	LANGUAGE 'c' IMMUTABLE STRICT;

------------------------------------------------------------
-- BASIC GENERAL OPERATIONS --
------------------------------------------------------------

--vgeom, int
CREATE OR REPLACE FUNCTION VG_SetSRID(vaguegeometry, int4)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_setsrid'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom
CREATE OR REPLACE FUNCTION VG_GetSRID(vaguegeometry)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'VG_getsrid'
	LANGUAGE 'c' IMMUTABLE STRICT;

--postgis geom(kernel), postgis geom(conjecture)
--this function will compute the union for true or not for false
CREATE OR REPLACE FUNCTION VG_MakeVagueGeom(kernel geometry, conjecture geometry, punion boolean default true)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_make'
	LANGUAGE 'c' IMMUTABLE;

--postgis geom(kernel), postgis geom(conjecture)
--this function will compute the union for true or not for false
--compute the difference between kernel and conjecture to construct a vague geometry object
CREATE OR REPLACE FUNCTION VG_EnforceMakeVagueGeom(kernel geometry, conjecture geometry, punion boolean default true)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_enforcemake'
	LANGUAGE 'c' IMMUTABLE;
	
--text to vaguegeometry using the e-vwkt form
CREATE OR REPLACE FUNCTION VG_VaguegeomfromEVWKT(evwkt text, punion boolean default true)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_vgeomfromevwkt'
	LANGUAGE 'c' IMMUTABLE STRICT;
	
--text to vaguegeometry using the vague wkt form, srid
CREATE OR REPLACE FUNCTION VG_Vaguegeomfromtext(vwkt text, srid int4, punion boolean default true)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_vgeomfromvwkt'
	LANGUAGE 'c' IMMUTABLE STRICT;

--text to vaguegeometry (kernel, conjecture)
CREATE OR REPLACE FUNCTION VG_Vaguegeomfromtext(kernel text, conjecture text, punion boolean default true)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_vgeomfromtext'
	LANGUAGE 'c' IMMUTABLE STRICT;

--text to vaguegeometry (kernel, conjecture, srid, boolean(precompute the union or not?))
CREATE OR REPLACE FUNCTION VG_Vaguegeomfromtext(kernel text, conjecture text, srid int4, punion boolean default true)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_vgeomfromtext'
	LANGUAGE 'c' IMMUTABLE STRICT;

--in vwkt form
CREATE OR REPLACE FUNCTION VG_AsText(vgeom vaguegeometry, simplified boolean default false)
	RETURNS text
	AS 'MODULE_PATHNAME', 'VG_astext'
	LANGUAGE 'c' IMMUTABLE STRICT;

-- in evwkt form	
CREATE OR REPLACE FUNCTION VG_AsEVWKT(vgeom vaguegeometry, simplified boolean default false)
	RETURNS text
	AS 'MODULE_PATHNAME', 'VG_asevwkt'
	LANGUAGE 'c' IMMUTABLE STRICT;

-----------------------------------------------------------------------
-- VAGUEGEOJSON INPUT AND OUTPUT
-----------------------------------------------------------------------
CREATE OR REPLACE FUNCTION VG_VagueGeomFromVGeoJson(vgeojson text, punion boolean DEFAULT true) 
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_vgeomfromvgeojson'
	LANGUAGE 'c' IMMUTABLE STRICT;
	
CREATE OR REPLACE FUNCTION _VG_AsvGeoJson(int4, vaguegeometry, int4, int4)
	RETURNS text
	AS 'MODULE_PATHNAME','VG_asvgeojson'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION VG_AsvGeoJson(geom vaguegeometry, maxdecimaldigits int4 DEFAULT 15, options int4 DEFAULT 0)
	RETURNS text
	AS $$ SELECT _VG_AsvGeoJson(1, $1, $2, $3); $$
	LANGUAGE 'sql' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION VG_AsvGeoJson(gj_version int4, geom vaguegeometry, maxdecimaldigits int4 DEFAULT 15, options int4 DEFAULT 0)
	RETURNS text
	AS $$ SELECT _VG_AsvGeoJson($1, $2, $3, $4); $$
	LANGUAGE 'sql' IMMUTABLE STRICT;

-----------------------------------------------------------------------
-- VAGUEGML INPUT AND OUTPUT
-----------------------------------------------------------------------
CREATE OR REPLACE FUNCTION VG_VagueGeomFromVGML(vgml text, srid int4, punion boolean DEFAULT true) 
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_vgeomfromvgml'
	LANGUAGE 'c' IMMUTABLE STRICT;
	
CREATE OR REPLACE FUNCTION VG_VagueGeomFromVGML(vgml text, punion boolean DEFAULT true) 
	RETURNS vaguegeometry
	AS $$ SELECT VG_VagueGeomFromVGML($1, 0, $2); $$
	LANGUAGE 'sql' IMMUTABLE STRICT;

	--(version of gml, vaguegeometry, precision default 15, option)
CREATE OR REPLACE FUNCTION _VG_AsVGML(int4, vaguegeometry, int4, int4)
	RETURNS text
	AS 'MODULE_PATHNAME','VG_asvgml'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION VG_AsVGML(geom vaguegeometry, maxdecimaldigits int4 DEFAULT 15, options int4 DEFAULT 0)
	RETURNS text
	AS $$ SELECT _VG_AsVGML(2, $1, $2, $3); $$
	LANGUAGE 'sql' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION VG_AsVGML(gj_version int4, geom vaguegeometry, maxdecimaldigits int4 DEFAULT 15, options int4 DEFAULT 0)
	RETURNS text
	AS $$ SELECT _VG_AsVGML($1, $2, $3, $4); $$
	LANGUAGE 'sql' IMMUTABLE STRICT;

-----------------------------------------------------------------------
-- VAGUEKML INPUT AND OUTPUT
-----------------------------------------------------------------------
CREATE OR REPLACE FUNCTION VG_VagueGeomFromVKML(vkml text, srid int4 DEFAULT 0, punion boolean DEFAULT true) 
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_vgeomfromvkml'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION VG_AsVKML(geom vaguegeometry, maxdecimaldigits int4 DEFAULT 15, nnamespace text DEFAULT '')
	RETURNS text
	AS 'MODULE_PATHNAME','VG_asVKML'
	LANGUAGE 'c' IMMUTABLE STRICT;

-----------------------------------------------------------------------
-- VAGUE WKB INPUT AND OUTPUT
-----------------------------------------------------------------------
CREATE OR REPLACE FUNCTION VG_VagueGeomFromVWKB(vwkb bytea, srid int4 DEFAULT 0, punion boolean DEFAULT true) 
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_vgeomfromvwkb'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION VG_AsVWKB(geom vaguegeometry)
	RETURNS bytea
	AS 'MODULE_PATHNAME', 'VG_asvwkb'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION VG_AsVWKB(geom vaguegeometry, endian text)
	RETURNS bytea
	AS 'MODULE_PATHNAME', 'VG_asvwkb'
	LANGUAGE 'c' IMMUTABLE STRICT;
	
-- E-VWKB INPUT AND OUTPUT
CREATE OR REPLACE FUNCTION VG_VagueGeomFromEVWKB(evwkb bytea, punion boolean DEFAULT true) 
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_vgeomfromevwkb'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION VG_AsEVWKB(geom vaguegeometry)
	RETURNS bytea
	AS 'MODULE_PATHNAME', 'VG_asevwkb'
	LANGUAGE 'c' IMMUTABLE STRICT;
	
CREATE OR REPLACE FUNCTION VG_AsEVWKB(geom vaguegeometry, endian text)
	RETURNS bytea
	AS 'MODULE_PATHNAME', 'VG_asevwkb'
	LANGUAGE 'c' IMMUTABLE STRICT;


---------------------------------------------------------------
-- ANOTHER OPERATIONS --
-------------------------------------------------------------

--return if is pre computing the union or not
CREATE OR REPLACE FUNCTION VG_isPreComputingUnion(vaguegeometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'VG_pcunion'
	LANGUAGE 'c' IMMUTABLE STRICT;

--return the type of a vague geometry
CREATE OR REPLACE FUNCTION VG_GetType(vaguegeometry)
	RETURNS text
	AS 'MODULE_PATHNAME', 'VG_gettype'
	LANGUAGE 'c' IMMUTABLE STRICT;

CREATE OR REPLACE FUNCTION VG_PreComputeUnion(vaguegeometry, boolean)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_precompute'
	LANGUAGE 'c' IMMUTABLE STRICT;

--/*common border (vagueline, vagueregion) or (vagueregion, vagueregion) */
CREATE OR REPLACE FUNCTION VG_CommonBorder(vaguegeometry, vaguegeometry)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_common_border'
	LANGUAGE 'c' IMMUTABLE STRICT;

--this function is the same of intersects but will return vague points as result of intersection between two vague lines
--/*common points (vagueline, vagueline) */
CREATE OR REPLACE FUNCTION VG_CommonPoints(vaguegeometry, vaguegeometry)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_common_points'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom - vaguepolygon
CREATE OR REPLACE FUNCTION VG_ConjectureBoundary(vaguegeometry)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_conjecture_boundary'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom - vaguepoints
CREATE OR REPLACE FUNCTION VG_ConjectureConvexHull(vaguegeometry)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_conjecture_convexhull'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom - vaguelines
CREATE OR REPLACE FUNCTION VG_ConjectureInterior(vaguegeometry)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_conjecture_interior'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom -vaguepolygon and vaguelines
CREATE OR REPLACE FUNCTION VG_ConjectureVertices(vaguegeometry)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_conjecture_vertices'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom
CREATE OR REPLACE FUNCTION VG_KernelBoundary(vaguegeometry)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_kernel_boundary'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom
CREATE OR REPLACE FUNCTION VG_KernelConvexHull(vaguegeometry)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_kernel_convexhull'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom
CREATE OR REPLACE FUNCTION VG_KernelInterior(vaguegeometry)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_kernel_interior'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom
CREATE OR REPLACE FUNCTION VG_KernelVertices(vaguegeometry)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_kernel_vertices'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom
CREATE OR REPLACE FUNCTION VG_Conjecture(vaguegeometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'VG_get_conjecture'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom
CREATE OR REPLACE FUNCTION VG_Kernel(vaguegeometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'VG_get_kernel'
	LANGUAGE 'c' IMMUTABLE STRICT;

--TODO fazer opera\E7\F5es que setam a conjectura e o kernel


--vgeom
CREATE OR REPLACE FUNCTION VG_Invert(vaguegeometry)
	RETURNS vaguegeometry
	AS 'MODULE_PATHNAME', 'VG_invert'
	LANGUAGE 'c' IMMUTABLE STRICT;


--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_same(vaguegeometry, vaguegeometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'VG_same'
	LANGUAGE 'c' IMMUTABLE STRICT;

---------------------------------------------------------------
-- PREDICATES RETURNING A VAGUE BOOL --
---------------------------------------------------------------

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_Contains(vaguegeometry, vaguegeometry)
	RETURNS vaguebool
	AS 'MODULE_PATHNAME', 'VG_contains'
	LANGUAGE 'c' IMMUTABLE STRICT
	COST 100;

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_CoveredBy(vaguegeometry, vaguegeometry)
	RETURNS vaguebool
	AS 'MODULE_PATHNAME', 'VG_coveredBy'
	LANGUAGE 'c' IMMUTABLE STRICT
	COST 100;

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_Covers(vaguegeometry, vaguegeometry)
	RETURNS vaguebool
	AS 'MODULE_PATHNAME', 'VG_covers'
	LANGUAGE 'c' IMMUTABLE STRICT
	COST 100;

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_Crosses(vaguegeometry, vaguegeometry)
	RETURNS vaguebool
	AS 'MODULE_PATHNAME', 'VG_crosses'
	LANGUAGE 'c' IMMUTABLE STRICT
	COST 100;

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_Disjoint(vaguegeometry, vaguegeometry)
	RETURNS vaguebool
	AS 'MODULE_PATHNAME', 'VG_disjoint'
	LANGUAGE 'c' IMMUTABLE STRICT
	COST 100;

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_Equals(vaguegeometry, vaguegeometry)
	RETURNS vaguebool
	AS 'MODULE_PATHNAME', 'VG_equal'
	LANGUAGE 'c' IMMUTABLE STRICT
	COST 100;

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_Inside(vaguegeometry, vaguegeometry)
	RETURNS vaguebool
	AS 'MODULE_PATHNAME', 'VG_inside'
	LANGUAGE 'c' IMMUTABLE STRICT
	COST 100;

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_Intersects(vaguegeometry, vaguegeometry)
	RETURNS vaguebool
	AS 'MODULE_PATHNAME', 'VG_intersects'
	LANGUAGE 'c' IMMUTABLE STRICT
	COST 100;

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_Meets(vaguegeometry, vaguegeometry)
	RETURNS vaguebool
	AS 'MODULE_PATHNAME', 'VG_meets'
	LANGUAGE 'c' IMMUTABLE STRICT
	COST 100;

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_onBorderOf(vaguegeometry, vaguegeometry)
	RETURNS vaguebool
	AS 'MODULE_PATHNAME', 'VG_on_border_of'
	LANGUAGE 'c' IMMUTABLE STRICT
	COST 100;

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_Overlap(vaguegeometry, vaguegeometry)
	RETURNS vaguebool
	AS 'MODULE_PATHNAME', 'VG_overlap'
	LANGUAGE 'c' IMMUTABLE STRICT
	COST 100;

-------------------------------------------------------------------
-- GENERAL OPERATIONS OF A VAGUE NUMERIC --
-------------------------------------------------------------------

--float8, float8
CREATE OR REPLACE FUNCTION VG_MakeVagueNumeric(float8, float8)
	RETURNS vaguenumeric
	AS 'MODULE_PATHNAME', 'VG_MakeVagueNumeric'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vaguenumeric
CREATE OR REPLACE FUNCTION VG_GetMAX(vaguenumeric)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'VG_GetMAX'
	LANGUAGE 'c' IMMUTABLE STRICT;
	
--vaguenumeric
CREATE OR REPLACE FUNCTION VG_GetMIN(vaguenumeric)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'VG_GetMIN'
	LANGUAGE 'c' IMMUTABLE STRICT;
	
-------------------------------------------------------------------
-- DISTANCE OPERATIONS RETURNING A VAGUE NUMERIC --
-------------------------------------------------------------------

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_FarthestDistance(vaguegeometry, vaguegeometry)
	RETURNS vaguenumeric
	AS 'MODULE_PATHNAME', 'VG_farthest_distance'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_NearestDistance(vaguegeometry, vaguegeometry)
	RETURNS vaguenumeric
	AS 'MODULE_PATHNAME', 'VG_nearest_distance'
	LANGUAGE 'c' IMMUTABLE STRICT;

-------------------------------------------------------------------
-- DISTANCE OPERATIONS RETURNING A DOUBLE --
-------------------------------------------------------------------

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_MaxFarthestDistance(vaguegeometry, vaguegeometry)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'VG_farthest_max_distance'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_MinFarthestDistance(vaguegeometry, vaguegeometry)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'VG_farthest_min_distance'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_MaxNearestDistance(vaguegeometry, vaguegeometry)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'VG_nearest_max_distance'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom, vgeom
CREATE OR REPLACE FUNCTION VG_MinNearestDistance(vaguegeometry, vaguegeometry)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'VG_nearest_min_distance'
	LANGUAGE 'c' IMMUTABLE STRICT;

-------------------------------------------------------------------
-- NUMERIC OPERATIONS RETURNING A VAGUE NUMERIC --
------------------------------------------------------------------

--vgeom
CREATE OR REPLACE FUNCTION VG_Area(vaguegeometry)
	RETURNS vaguenumeric
	AS 'MODULE_PATHNAME', 'VG_area'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom
CREATE OR REPLACE FUNCTION VG_Length(vaguegeometry)
	RETURNS vaguenumeric
	AS 'MODULE_PATHNAME', 'VG_length'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom
CREATE OR REPLACE FUNCTION VG_Ncomp(vaguegeometry)
	RETURNS vaguenumeric
	AS 'MODULE_PATHNAME', 'VG_ncomp'
	LANGUAGE 'c' IMMUTABLE STRICT;

-------------------------------------------------------------------
-- NUMERIC OPERATIONS RETURNING A DOUBLE --
------------------------------------------------------------------

--vgeom
CREATE OR REPLACE FUNCTION VG_MaxArea(vaguegeometry)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'VG_max_area'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom
CREATE OR REPLACE FUNCTION VG_MinArea(vaguegeometry)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'VG_min_area'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom
CREATE OR REPLACE FUNCTION VG_MaxLength(vaguegeometry)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'VG_max_length'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom
CREATE OR REPLACE FUNCTION VG_MinLength(vaguegeometry)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'VG_min_length'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom
CREATE OR REPLACE FUNCTION VG_MaxNcomp(vaguegeometry)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'VG_max_ncomp'
	LANGUAGE 'c' IMMUTABLE STRICT;

--vgeom
CREATE OR REPLACE FUNCTION VG_MinNcomp(vaguegeometry)
	RETURNS float8
	AS 'MODULE_PATHNAME', 'VG_min_ncomp'
	LANGUAGE 'c' IMMUTABLE STRICT;

