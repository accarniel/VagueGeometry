# **********************************************************************
# *
# * VagueGeometry - Vague Spatial Objects for PostgreSQL
# * http://gbd.dc.ufscar.br/vaguegeometry/
# * Copyright 2013-2016 Anderson Chaves Carniel <accarniel@gmail.com>
# *
# * This is free software; you can redistribute and/or modify it under
# * the terms of the GNU General Public Licence. See the COPYING file.
# * * Fully developed by Anderson Chaves Carniel
# *
# **********************************************************************

POSTGIS_SOURCE=/opt/postgis-2.2.5

$(shell sed -n -e '/^#define \(POSTGIS_M[AI][JCN][RO][OR]\)_VERSION "\([0-9][0-9]*\)".*/ { s//#define NUM_\1_VERSION \2/p }' $(POSTGIS_SOURCE)/postgis_config.h >libversion.h)

MODULE_big=vasavgeom-1.1
OBJS= \
    operations_geos.o \
    postgis_geojson.o \
    postgis_gml.o \
    postgis_kml.o \
    util_pg.o \
    vague_spatial_operations.o \
    vague_topological_predicates.o \
    vaguebool_pg.o \
    vaguegeom_binaryform.o \
    vaguegeom_inout_pg.o \
    vaguegeom_numericop_pg.o \
    vaguegeom_predicates_pg.o \
    vaguegeom_serialize.o \
    vaguegeom_spatialop_pg.o \
    vaguegeom_vwkt.o \
    vaguegeometry.o \
    vaguenumeric_pg.o \
    vaguegeom_pg.o \
	vaguegeom_module.o
EXTENSION = vasavgeom
DATA = vasavgeom--1.1.sql

SHLIB_LINK = $(POSTGIS_SOURCE)/libpgcommon/libpgcommon.a $(POSTGIS_SOURCE)/postgis/postgis-2.2.so -L/usr/local/lib -lgeos_c -lproj -llwgeom -lxml2 -ljson-c

PG_CPPFLAGS = -I/usr/local/include -I$(POSTGIS_SOURCE)/liblwgeom/ -I$(POSTGIS_SOURCE)/libpgcommon/ -I$(POSTGIS_SOURCE)/postgis/ -I/usr/include/libxml2 -fPIC

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
