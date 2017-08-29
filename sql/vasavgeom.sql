--create extension if not exists vasavgeom;

-- expected successes

--creating vague geometry objects

--output functions

--begin of wkb in hexadecimal format
select vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 4326, false);
select vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 0, true);
select vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 0);
select vg_vaguegeomfromtext('point empty', 'point(2 2)', 0);
select vg_vaguegeomfromtext('point(1 1)', 'point empty', 0);
select vg_vaguegeomfromtext('point empty', 'point empty', 0);

select vg_vaguegeomfromtext('linestring empty', 'linestring empty', 0);
select vg_vaguegeomfromtext('linestring(1 1, 2 2)', 'linestring empty', 0);
select vg_vaguegeomfromtext('linestring empty', 'linestring(1 1, 2 2)', 0);
select vg_vaguegeomfromtext('linestring(1 1, 2 2)', 'linestring(2 2, 3 3)', 0);

select vg_vaguegeomfromtext('polygon empty', 'polygon empty', 0);
select vg_vaguegeomfromtext('polygon((1 1, 2 2, 3 3, 1 1))', 'polygon empty', 0);
select vg_vaguegeomfromtext('polygon empty', 'polygon((1 1, 2 2, 3 3, 1 1))', 0);
select vg_vaguegeomfromtext('POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235))', 'POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577))', 0);

select vg_vaguegeomfromtext('multipoint(1 1)', 'multipoint(2 2)', 0);
select vg_vaguegeomfromtext('multipoint empty', 'multipoint(2 2)', 0);
select vg_vaguegeomfromtext('multipoint(1 1)', 'multipoint empty', 0);
select vg_vaguegeomfromtext('multipoint empty', 'multipoint empty', 0);

select vg_vaguegeomfromtext('multilinestring empty', 'multilinestring empty', 0);
select vg_vaguegeomfromtext('multilinestring((1 1, 2 2))', 'multilinestring empty', 0);
select vg_vaguegeomfromtext('multilinestring empty', 'multilinestring((1 1, 2 2))', 0);
select vg_vaguegeomfromtext('multilinestring((1 1, 2 2))', 'multilinestring((2 2, 3 3))', 0);

select vg_vaguegeomfromtext('multipolygon empty', 'multipolygon empty', 0);
select vg_vaguegeomfromtext('multipolygon(((1 1, 2 2, 3 3, 1 1)))', 'multipolygon empty', 0);
select vg_vaguegeomfromtext('multipolygon empty', 'multipolygon(((1 1, 2 2, 3 3, 1 1)))', 0);
select vg_vaguegeomfromtext('multipolygon(((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)))', 'multipolygon(((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)))', 0);

select VG_AsEVWKB(VG_VagueGeomFromEVWKT('SRID=4269;VAGUEPOINT(1 1; 2 2)'), 'XDR');
select VG_AsVWKB(VG_VagueGeomFromText('VAGUELINESTRING(linestring(1 1, 2 2); linestring(2 2, 3 3))', 4326), 'XDR');

--end wkb in hex form for output

--begin of vwkt (without srid)
select vg_astext(vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 0));
select vg_astext(vg_vaguegeomfromtext('point empty', 'point(2 2)', 0));
select vg_astext(vg_vaguegeomfromtext('point(1 1)', 'point empty', 0));
select vg_astext(vg_vaguegeomfromtext('point empty', 'point empty', 0));

--simplified version
select vg_astext(vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 0), true);
select vg_astext(vg_vaguegeomfromtext('point empty', 'point(2 2)', 0), true);
select vg_astext(vg_vaguegeomfromtext('point(1 1)', 'point empty', 0), true);
select vg_astext(vg_vaguegeomfromtext('point empty', 'point empty', 0), true);

select vg_astext(vg_vaguegeomfromtext('linestring empty', 'linestring empty', 0));
select vg_astext(vg_vaguegeomfromtext('linestring(1 1, 2 2)', 'linestring empty', 0));
select vg_astext(vg_vaguegeomfromtext('linestring empty', 'linestring(1 1, 2 2)', 0));
select vg_astext(vg_vaguegeomfromtext('linestring(1 1, 2 2)', 'linestring(2 2, 3 3)', 0));

select vg_astext(vg_vaguegeomfromtext('linestring empty', 'linestring empty', 0), true);
select vg_astext(vg_vaguegeomfromtext('linestring(1 1, 2 2)', 'linestring empty', 0), true);
select vg_astext(vg_vaguegeomfromtext('linestring empty', 'linestring(1 1, 2 2)', 0), true);
select vg_astext(vg_vaguegeomfromtext('linestring(1 1, 2 2)', 'linestring(2 2, 3 3)', 0), true);

select vg_astext(vg_vaguegeomfromtext('polygon empty', 'polygon empty', 0));
select vg_astext(vg_vaguegeomfromtext('polygon((1 1, 2 2, 3 3, 1 1))', 'polygon empty', 0));
select vg_astext(vg_vaguegeomfromtext('polygon empty', 'polygon((1 1, 2 2, 3 3, 1 1))', 0));
select vg_astext(vg_vaguegeomfromtext('POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235))', 'POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577))', 0));

select vg_astext(vg_vaguegeomfromtext('polygon empty', 'polygon empty', 0), true);
select vg_astext(vg_vaguegeomfromtext('polygon((1 1, 2 2, 3 3, 1 1))', 'polygon empty', 0), true);
select vg_astext(vg_vaguegeomfromtext('polygon empty', 'polygon((1 1, 2 2, 3 3, 1 1))', 0), true);
select vg_astext(vg_vaguegeomfromtext('POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235))', 'POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577))', 0), true);

select vg_astext(vg_vaguegeomfromtext('multipoint(1 1)', 'multipoint(2 2)', 0));
select vg_astext(vg_vaguegeomfromtext('multipoint empty', 'multipoint(2 2)', 0));
select vg_astext(vg_vaguegeomfromtext('multipoint(1 1)', 'multipoint empty', 0));
select vg_astext(vg_vaguegeomfromtext('multipoint empty', 'multipoint empty', 0));

select vg_astext(vg_vaguegeomfromtext('multipoint(1 1)', 'multipoint(2 2)', 0), true);
select vg_astext(vg_vaguegeomfromtext('multipoint empty', 'multipoint(2 2)', 0), true);
select vg_astext(vg_vaguegeomfromtext('multipoint(1 1)', 'multipoint empty', 0), true);
select vg_astext(vg_vaguegeomfromtext('multipoint empty', 'multipoint empty', 0), true);

select vg_astext(vg_vaguegeomfromtext('multilinestring empty', 'multilinestring empty', 0));
select vg_astext(vg_vaguegeomfromtext('multilinestring((1 1, 2 2))', 'multilinestring empty', 0));
select vg_astext(vg_vaguegeomfromtext('multilinestring empty', 'multilinestring((1 1, 2 2))', 0));
select vg_astext(vg_vaguegeomfromtext('multilinestring((1 1, 2 2))', 'multilinestring((2 2, 3 3))', 0));

select vg_astext(vg_vaguegeomfromtext('multilinestring empty', 'multilinestring empty', 0), true);
select vg_astext(vg_vaguegeomfromtext('multilinestring((1 1, 2 2))', 'multilinestring empty', 0), true);
select vg_astext(vg_vaguegeomfromtext('multilinestring empty', 'multilinestring((1 1, 2 2))', 0), true);
select vg_astext(vg_vaguegeomfromtext('multilinestring((1 1, 2 2))', 'multilinestring((2 2, 3 3))', 0), true);

select vg_astext(vg_vaguegeomfromtext('multipolygon empty', 'multipolygon empty', 0));
select vg_astext(vg_vaguegeomfromtext('multipolygon(((1 1, 2 2, 3 3, 1 1)))', 'multipolygon empty', 0));
select vg_astext(vg_vaguegeomfromtext('multipolygon empty', 'multipolygon(((1 1, 2 2, 3 3, 1 1)))', 0));
select vg_astext(vg_vaguegeomfromtext('multipolygon(((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)))', 'multipolygon(((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)))', 0));

select vg_astext(vg_vaguegeomfromtext('multipolygon empty', 'multipolygon empty', 0), true);
select vg_astext(vg_vaguegeomfromtext('multipolygon(((1 1, 2 2, 3 3, 1 1)))', 'multipolygon empty', 0), true);
select vg_astext(vg_vaguegeomfromtext('multipolygon empty', 'multipolygon(((1 1, 2 2, 3 3, 1 1)))', 0), true);
select vg_astext(vg_vaguegeomfromtext('multipolygon(((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)))', 'multipolygon(((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)))', 0), true);
--end of vwkt (without srid)

--begin of e-vwkt (with srid)
select vg_asEVWKT(vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('point empty', 'point(2 2)', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('point(1 1)', 'point empty', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('point empty', 'point empty', 4326));

select vg_asEVWKT(vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 4326), true);
select vg_asEVWKT(vg_vaguegeomfromtext('point empty', 'point(2 2)', 4326), true);
select vg_asEVWKT(vg_vaguegeomfromtext('point(1 1)', 'point empty', 4326), true);
select vg_asEVWKT(vg_vaguegeomfromtext('point empty', 'point empty', 4326), true);

select vg_asEVWKT(vg_vaguegeomfromtext('linestring empty', 'linestring empty', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('linestring(1 1, 2 2)', 'linestring empty', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('linestring empty', 'linestring(1 1, 2 2)', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('linestring(1 1, 2 2)', 'linestring(2 2, 3 3)', 4326));

select vg_asEVWKT(vg_vaguegeomfromtext('polygon empty', 'polygon empty', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('polygon((1 1, 2 2, 3 3, 1 1))', 'polygon empty', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('polygon empty', 'polygon((1 1, 2 2, 3 3, 1 1))', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235))', 'POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577))', 4326));

select vg_asEVWKT(vg_vaguegeomfromtext('multipoint(1 1)', 'multipoint(2 2)', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('multipoint empty', 'multipoint(2 2)', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('multipoint(1 1)', 'multipoint empty', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('multipoint empty', 'multipoint empty', 4326));

select vg_asEVWKT(vg_vaguegeomfromtext('multilinestring empty', 'multilinestring empty', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('multilinestring((1 1, 2 2))', 'multilinestring empty', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('multilinestring empty', 'multilinestring((1 1, 2 2))', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('multilinestring((1 1, 2 2))', 'multilinestring((2 2, 3 3))', 4326));

select vg_asEVWKT(vg_vaguegeomfromtext('multipolygon empty', 'multipolygon empty', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('multipolygon(((1 1, 2 2, 3 3, 1 1)))', 'multipolygon empty', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('multipolygon empty', 'multipolygon(((1 1, 2 2, 3 3, 1 1)))', 4326));
select vg_asEVWKT(vg_vaguegeomfromtext('multipolygon(((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)))', 'multipolygon(((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)))', 4326));
--end of e-vwkt (with srid)

--begin of vgeojson output form
select VG_AsVGeoJson(vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 4326), 15, 5);
select VG_AsVGeoJson(vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 4326), 15, 4);
select VG_AsVGeoJson(vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 4326), 15, 3);
select VG_AsVGeoJson(vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 4326), 15, 2);
select VG_AsVGeoJson(vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 4326), 15, 1);
select VG_AsVGeoJson(vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('point empty', 'point(2 2)', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('point(1 1)', 'point empty', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('point empty', 'point empty', 4326));

select VG_AsVGeoJson(vg_vaguegeomfromtext('linestring empty', 'linestring empty', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('linestring(1 1, 2 2)', 'linestring empty', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('linestring empty', 'linestring(1 1, 2 2)', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('linestring(1 1, 2 2)', 'linestring(2 2, 3 3)', 4326));

select VG_AsVGeoJson(vg_vaguegeomfromtext('polygon empty', 'polygon empty', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('polygon((1 1, 2 2, 3 3, 1 1))', 'polygon empty', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('polygon empty', 'polygon((1 1, 2 2, 3 3, 1 1))', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235))', 'POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577))', 4326));

select VG_AsVGeoJson(vg_vaguegeomfromtext('multipoint(1 1)', 'multipoint(2 2)', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('multipoint empty', 'multipoint(2 2)', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('multipoint(1 1)', 'multipoint empty', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('multipoint empty', 'multipoint empty', 4326));

select VG_AsVGeoJson(vg_vaguegeomfromtext('multilinestring empty', 'multilinestring empty', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('multilinestring((1 1, 2 2))', 'multilinestring empty', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('multilinestring empty', 'multilinestring((1 1, 2 2))', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('multilinestring((1 1, 2 2))', 'multilinestring((2 2, 3 3))', 4326));

select VG_AsVGeoJson(vg_vaguegeomfromtext('multipolygon empty', 'multipolygon empty', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('multipolygon(((1 1, 2 2, 3 3, 1 1)))', 'multipolygon empty', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('multipolygon empty', 'multipolygon(((1 1, 2 2, 3 3, 1 1)))', 4326));
select VG_AsVGeoJson(vg_vaguegeomfromtext('multipolygon(((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)))', 'multipolygon(((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)))', 4326));
--end of vgeojson output form

--begin of vGML output form
select VG_AsVGML(vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 4326), 15, 0);
select VG_AsVGML(vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 4326), 15, 1);
select VG_AsVGML(3, vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 4326), 15, 2);
select VG_AsVGML(3, vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 4326), 15, 4);
select VG_AsVGML(vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('point empty', 'point(2 2)', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('point(1 1)', 'point empty', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('point empty', 'point empty', 4326));

select VG_AsVGML(vg_vaguegeomfromtext('linestring empty', 'linestring empty', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('linestring(1 1, 2 2)', 'linestring empty', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('linestring empty', 'linestring(1 1, 2 2)', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('linestring(1 1, 2 2)', 'linestring(2 2, 3 3)', 4326));

select VG_AsVGML(vg_vaguegeomfromtext('polygon empty', 'polygon empty', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('polygon((1 1, 2 2, 3 3, 1 1))', 'polygon empty', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('polygon empty', 'polygon((1 1, 2 2, 3 3, 1 1))', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235))', 'POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577))', 4326));

select VG_AsVGML(vg_vaguegeomfromtext('multipoint(1 1)', 'multipoint(2 2)', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('multipoint empty', 'multipoint(2 2)', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('multipoint(1 1)', 'multipoint empty', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('multipoint empty', 'multipoint empty', 4326));

select VG_AsVGML(vg_vaguegeomfromtext('multilinestring empty', 'multilinestring empty', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('multilinestring((1 1, 2 2))', 'multilinestring empty', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('multilinestring empty', 'multilinestring((1 1, 2 2))', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('multilinestring((1 1, 2 2))', 'multilinestring((2 2, 3 3))', 4326));

select VG_AsVGML(vg_vaguegeomfromtext('multipolygon empty', 'multipolygon empty', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('multipolygon(((1 1, 2 2, 3 3, 1 1)))', 'multipolygon empty', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('multipolygon empty', 'multipolygon(((1 1, 2 2, 3 3, 1 1)))', 4326));
select VG_AsVGML(vg_vaguegeomfromtext('multipolygon(((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)))', 'multipolygon(((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)))', 4326));
--end of vGML output form

--begin of vKML output form
select VG_AsVKML(vg_vaguegeomfromtext('point(1 1)', 'point(2 2)', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('point empty', 'point(2 2)', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('point(1 1)', 'point empty', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('point empty', 'point empty', 4326));

select VG_AsVKML(vg_vaguegeomfromtext('linestring empty', 'linestring empty', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('linestring(1 1, 2 2)', 'linestring empty', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('linestring empty', 'linestring(1 1, 2 2)', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('linestring(1 1, 2 2)', 'linestring(2 2, 3 3)', 4326));

select VG_AsVKML(vg_vaguegeomfromtext('polygon empty', 'polygon empty', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('polygon((1 1, 2 2, 3 3, 1 1))', 'polygon empty', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('polygon empty', 'polygon((1 1, 2 2, 3 3, 1 1))', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235))', 'POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577))', 4326));

select VG_AsVKML(vg_vaguegeomfromtext('multipoint(1 1)', 'multipoint(2 2)', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('multipoint empty', 'multipoint(2 2)', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('multipoint(1 1)', 'multipoint empty', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('multipoint empty', 'multipoint empty', 4326));

select VG_AsVKML(vg_vaguegeomfromtext('multilinestring empty', 'multilinestring empty', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('multilinestring((1 1, 2 2))', 'multilinestring empty', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('multilinestring empty', 'multilinestring((1 1, 2 2))', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('multilinestring((1 1, 2 2))', 'multilinestring((2 2, 3 3))', 4326));

select VG_AsVKML(vg_vaguegeomfromtext('multipolygon empty', 'multipolygon empty', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('multipolygon(((1 1, 2 2, 3 3, 1 1)))', 'multipolygon empty', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('multipolygon empty', 'multipolygon(((1 1, 2 2, 3 3, 1 1)))', 4326));
select VG_AsVKML(vg_vaguegeomfromtext('multipolygon(((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)))', 'multipolygon(((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)))', 4326));
--end of vKML output form

--end of output functions form

--begin of input functions form (constructing vague geometry objects

--make a vague geometry object from 2 geometry objects
select vg_astext(vg_makevaguegeom(st_geomfromtext('point(1 1)', 0), st_geomfromtext('point(2 2)', 0)));
select vg_astext(vg_makevaguegeom(st_geomfromtext('point empty', 0), st_geomfromtext('point(2 2)', 0)));
select vg_astext(vg_makevaguegeom(st_geomfromtext('point(1 1)', 0), st_geomfromtext('point empty', 0)));
select vg_astext(vg_makevaguegeom(st_geomfromtext('point empty', 0), st_geomfromtext('point empty', 0)));
select vg_astext(vg_makevaguegeom(NULL, st_geomfromtext('point(2 2)', 0)));
select vg_astext(vg_makevaguegeom(st_geomfromtext('point(1 1)', 0), NULL));
--end of make

--enforce make a vague geometry object from 2 geometry objects (i.e. do a difference operation between kernel and conjecture)
select vg_astext(VG_EnforceMakeVagueGeom(st_geomfromtext('point(1 1)', 0), st_geomfromtext('point(2 2)', 0)));
select vg_astext(VG_EnforceMakeVagueGeom(st_geomfromtext('point empty', 0), st_geomfromtext('point(2 2)', 0)));
select vg_astext(VG_EnforceMakeVagueGeom(st_geomfromtext('point(1 1)', 0), st_geomfromtext('point empty', 0)));
select vg_astext(VG_EnforceMakeVagueGeom(st_geomfromtext('point empty', 0), st_geomfromtext('point empty', 0)));
select vg_astext(VG_EnforceMakeVagueGeom(NULL, st_geomfromtext('point(2 2)', 0)));
select vg_astext(VG_EnforceMakeVagueGeom(st_geomfromtext('point(1 1)', 0), NULL));

select vg_astext(VG_EnforceMakeVagueGeom(st_geomfromtext('POLYGON((-53.89141308351281 -3.526662200548747,-47.91485058351441 0.5140706695293608,-45.45391308351505 -2.4734350706689163,-43.69610058351553 -6.15374468714475,-53.89141308351281 -3.526662200548747))', 0), st_geomfromtext('POLYGON((-53.53985058351292 0.16251484894146204,-46.86016308351468 -1.770796706377859,-40.356256833516426 -8.594138668560648,-56.17656933351221 -4.403456066917021,-53.53985058351292 0.16251484894146204))', 0))); -- the kernel and conjecture are overlapped
select vg_astext(vg_enforcemakevaguegeom(st_geomfromtext('point(1 1)', 0), st_geomfromtext('point(1 1)', 0)));
--end of enforce make

--begin of input from vwkt
select vg_astext(vg_vaguegeomfromtext('VAGUEPOINT(point(1 1); point(2 2))', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEPOINT(point empty; point(2 2))', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEPOINT(point(1 1); point empty)', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEPOINT(point empty; point empty)', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEPOINT EMPTY', 4326));

select vg_astext(vg_vaguegeomfromtext('VAGUEPOINT(1 1; 2 2)', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEPOINT(empty; 2 2)', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEPOINT(1 1; empty)', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEPOINT(empty; empty)', 4326));

select vg_astext(vg_vaguegeomfromtext('VAGUELINESTRING(linestring empty; linestring empty)', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUELINESTRING(linestring(1 1, 2 2); linestring empty)', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUELINESTRING(linestring empty; linestring(1 1, 2 2))', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUELINESTRING(linestring(1 1, 2 2); linestring(2 2, 3 3))', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUELINESTRING EMPTY', 4326));

select vg_astext(vg_vaguegeomfromtext('VAGUEPOLYGON(polygon empty; polygon empty)', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEPOLYGON(polygon((1 1, 2 2, 3 3, 1 1)); polygon empty)', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEPOLYGON(polygon empty; polygon((1 1, 2 2, 3 3, 1 1)))', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEPOLYGON(POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)); POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)))', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEPOLYGON EMPTY', 4326));

select vg_astext(vg_vaguegeomfromtext('VAGUEMULTIPOINT(multipoint(1 1); multipoint(2 2))', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEMULTIPOINT(multipoint empty; multipoint(2 2))', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEMULTIPOINT(multipoint(1 1); multipoint empty)', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEMULTIPOINT(multipoint empty; multipoint empty)', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEMULTIPOINT EMPTY', 4326));

select vg_astext(vg_vaguegeomfromtext('VAGUEMULTILINESTRING(multilinestring empty; multilinestring empty)', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEMULTILINESTRING(multilinestring((1 1, 2 2)); multilinestring empty)', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEMULTILINESTRING(multilinestring empty; multilinestring((1 1, 2 2)))', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEMULTILINESTRING(multilinestring((1 1, 2 2)); multilinestring((2 2, 3 3)))', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEMULTILINESTRING EMPTY', 4326));

select vg_astext(vg_vaguegeomfromtext('VAGUEMULTIPOLYGON(multipolygon empty; multipolygon empty)', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEMULTIPOLYGON(multipolygon(((1 1, 2 2, 3 3, 1 1))); multipolygon empty)', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEMULTIPOLYGON(multipolygon empty; multipolygon(((1 1, 2 2, 3 3, 1 1))))', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEMULTIPOLYGON(multipolygon(((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235))); multipolygon(((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577))))', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEMULTIPOLYGON EMPTY', 4326));
--end of vwkt

--begin of evwkt
select vg_astext(vg_vaguegeomfromEVWKT('SRID=4326;VAGUEPOINT(point(1 1); point(2 2))'));
select vg_astext(vg_vaguegeomfromEVWKT('SRID=4326;VAGUEPOINT(point empty; point(2 2))'));
select vg_astext(vg_vaguegeomfromEVWKT('SRID=4326;VAGUEPOINT(point(1 1); point empty)'));
select vg_astext(vg_vaguegeomfromEVWKT('SRID=4326;VAGUEPOINT(point empty; point empty)'));
select vg_astext(vg_vaguegeomfromEVWKT('SRID=4326;VAGUEPOINT EMPTY'));

select vg_astext(vg_vaguegeomfromEVWKT('SRID=4326;VAGUEPOINT(1 1; 2 2)'));
select vg_astext(vg_vaguegeomfromEVWKT('SRID=4326;VAGUEPOINT(empty; 2 2)'));
select vg_astext(vg_vaguegeomfromEVWKT('SRID=4326;VAGUEPOINT(1 1; empty)'));
select vg_astext(vg_vaguegeomfromEVWKT('SRID=4326;VAGUEPOINT(empty; empty)'));
--end of evwkt

--begin of input from vgeojson
select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VaguePoint", "kernel":{"type":"Point","coordinates":[1,1]}, "conjecture":{"type":"Point","coordinates":[2,2]}}'));
select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VaguePoint", "kernel":{"type":"Point","coordinates":[]}, "conjecture":{"type":"Point","coordinates":[2,2]}}'));
select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VaguePoint", "kernel":{"type":"Point","coordinates":[1,1]}, "conjecture":{"type":"Point","coordinates":[]}}'));
select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VaguePoint", "kernel":{"type":"Point","coordinates":[]}}'));
select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VaguePoint", "conjecture":{"type":"Point","coordinates":[]}}'));
select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VaguePoint"}'));


select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VagueLineString", "kernel":{"type":"LineString","coordinates":[[1,1],[2,2]]}, "conjecture":{"type":"LineString","coordinates":[[2,2],[3,3]]}}'));
select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VagueLineString", "kernel":{"type":"LineString","coordinates":[]}, "conjecture":{"type":"LineString","coordinates":[[2,2],[3,3]]}}'));
select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VagueLineString", "kernel":{"type":"LineString","coordinates":[[1,1],[2,2]]}, "conjecture":{"type":"LineString","coordinates":[]}}'));
select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VagueLineString", "kernel":{"type":"LineString","coordinates":[]}, "conjecture":{"type":"LineString","coordinates":[]}}'));


select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VaguePolygon", "kernel":{"type":"Polygon","coordinates":[[[-14.7728537230192,28.0394053260022],[8.43027127697463,21.6639643596292],[-0.007228723023134,10.1921092818201],[-9.85097872302046,16.3479741750216],[-14.7728537230192,28.0394053260022]]]}, "conjecture":{"type":"Polygon","coordinates":[[[-9.8509787230205,16.3479741750216],[-10.1393698363017,16.908528824689],[-10.8782004027077,16.5244751218594],[-9.90865694567673,15.9469646547618],[-9.8509787230205,16.3479741750216]]]}}'));
select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VaguePolygon", "conjecture":{"type":"Polygon","coordinates":[[[-9.8509787230205,16.3479741750216],[-10.1393698363017,16.908528824689],[-10.8782004027077,16.5244751218594],[-9.90865694567673,15.9469646547618],[-9.8509787230205,16.3479741750216]]]}}'));
select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VaguePolygon", "kernel":{"type":"Polygon","coordinates":[[[-14.7728537230192,28.0394053260022],[8.43027127697463,21.6639643596292],[-0.007228723023134,10.1921092818201],[-9.85097872302046,16.3479741750216],[-14.7728537230192,28.0394053260022]]]}}'));
--end of vgeojson

--begin of input from vgml
select VG_AsText(vg_vaguegeomfromVGML('<vgml:VaguePoint srsName="EPSG:4326"><vgml:Kernel><gml:Point><gml:coordinates>1,1</gml:coordinates></gml:Point></vgml:Kernel><vgml:Conjecture><gml:Point><gml:coordinates>2,2</gml:coordinates></gml:Point></vgml:Conjecture></vgml:VaguePoint>'));
select VG_AsText(vg_vaguegeomfromVGML('<vgml:VaguePoint srsName="EPSG:4326"><vgml:Kernel><gml:Point><gml:coordinates>1,1</gml:coordinates></gml:Point></vgml:Kernel></vgml:VaguePoint>'));
select VG_AsText(vg_vaguegeomfromVGML('<vgml:VaguePoint srsName="EPSG:4326"><vgml:Conjecture><gml:Point><gml:coordinates>2,2</gml:coordinates></gml:Point></vgml:Conjecture></vgml:VaguePoint>'));
select VG_AsText(vg_vaguegeomfromVGML('<vgml:VaguePoint srsName="EPSG:4326"></vgml:VaguePoint>'));

select VG_AsText(vg_vaguegeomfromVGML('<vgml:VagueLineString srsName="EPSG:4326"><vgml:Kernel><gml:LineString><gml:coordinates>1,1 2,2</gml:coordinates></gml:LineString></vgml:Kernel><vgml:Conjecture><gml:LineString><gml:coordinates>2,2 3,3</gml:coordinates></gml:LineString></vgml:Conjecture></vgml:VagueLineString>'));
select VG_AsText(vg_vaguegeomfromVGML('<vgml:VagueLineString srsName="EPSG:4326"><vgml:Kernel><gml:LineString><gml:coordinates>1,1 2,2</gml:coordinates></gml:LineString></vgml:Kernel></vgml:VagueLineString>'));
select VG_AsText(vg_vaguegeomfromVGML('<vgml:VagueLineString srsName="EPSG:4326"><vgml:Conjecture><gml:LineString><gml:coordinates>2,2 3,3</gml:coordinates></gml:LineString></vgml:Conjecture></vgml:VagueLineString>'));
select VG_AsText(vg_vaguegeomfromVGML('<vgml:VagueLineString srsName="EPSG:4326"></vgml:VagueLineString>'));

select VG_AsText(vg_vaguegeomfromVGML('<vgml:VaguePolygon srsName="EPSG:4326"><vgml:Kernel><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-14.772853723019185,28.039405326002235 8.430271276974629,21.663964359629226 -0.007228723023134,10.192109281820079 -9.850978723020456,16.347974175021566 -14.772853723019185,28.039405326002235</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></vgml:Kernel><vgml:Conjecture><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-9.8509787230205,16.347974175021577 -10.139369836301674,16.908528824688961 -10.878200402707728,16.524475121859354 -9.908656945676734,15.946964654761791 -9.8509787230205,16.347974175021577</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></vgml:Conjecture></vgml:VaguePolygon>'));
select VG_AsText(vg_vaguegeomfromVGML('<vgml:VaguePolygon srsName="EPSG:4326"><vgml:Kernel><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-14.772853723019185,28.039405326002235 8.430271276974629,21.663964359629226 -0.007228723023134,10.192109281820079 -9.850978723020456,16.347974175021566 -14.772853723019185,28.039405326002235</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></vgml:Kernel></vgml:VaguePolygon>'));
select VG_AsText(vg_vaguegeomfromVGML('<vgml:VaguePolygon srsName="EPSG:4326"><vgml:Conjecture><gml:Polygon><gml:outerBoundaryIs><gml:LinearRing><gml:coordinates>-9.8509787230205,16.347974175021577 -10.139369836301674,16.908528824688961 -10.878200402707728,16.524475121859354 -9.908656945676734,15.946964654761791 -9.8509787230205,16.347974175021577</gml:coordinates></gml:LinearRing></gml:outerBoundaryIs></gml:Polygon></vgml:Conjecture></vgml:VaguePolygon>'));
select VG_AsText(vg_vaguegeomfromVGML('<vgml:VaguePolygon srsName="EPSG:4326"></vgml:VaguePolygon>'));
--end of vgml

--begin input from vkml
select VG_AsText(vg_vaguegeomfromVKML('<vkml:VaguePoint><vkml:Kernel><Point><coordinates>1,1</coordinates></Point></vkml:Kernel><vkml:Conjecture><Point><coordinates>2,2</coordinates></Point></vkml:Conjecture></vkml:VaguePoint>'));
select VG_AsText(vg_vaguegeomfromVKML('<vkml:VaguePoint><vkml:Kernel><Point><coordinates>1,1</coordinates></Point></vkml:Kernel></vkml:VaguePoint>'));
select VG_AsText(vg_vaguegeomfromVKML('<vkml:VaguePoint><vkml:Conjecture><Point><coordinates>2,2</coordinates></Point></vkml:Conjecture></vkml:VaguePoint>'));
select VG_AsText(vg_vaguegeomfromVKML('<vkml:VaguePoint></vkml:VaguePoint>'));

select VG_AsText(vg_vaguegeomfromVKML('<vkml:VagueLineString><vkml:Kernel><LineString><coordinates>1,1 2,2</coordinates></LineString></vkml:Kernel><vkml:Conjecture><LineString><coordinates>2,2 3,3</coordinates></LineString></vkml:Conjecture></vkml:VagueLineString>'));
select VG_AsText(vg_vaguegeomfromVKML('<vkml:VagueLineString><vkml:Kernel><LineString><coordinates>1,1 2,2</coordinates></LineString></vkml:Kernel></vkml:VagueLineString>'));
select VG_AsText(vg_vaguegeomfromVKML('<vkml:VagueLineString><vkml:Conjecture><LineString><coordinates>2,2 3,3</coordinates></LineString></vkml:Conjecture></vkml:VagueLineString>'));
select VG_AsText(vg_vaguegeomfromVKML('<vkml:VagueLineString></vkml:VagueLineString>'));

select VG_AsText(vg_vaguegeomfromVKML('<vkml:VaguePolygon><vkml:Kernel><Polygon><outerBoundaryIs><LinearRing><coordinates>-14.772853723019185,28.039405326002235 8.430271276974629,21.663964359629226 -0.007228723023134,10.192109281820079 -9.850978723020456,16.347974175021566 -14.772853723019185,28.039405326002235</coordinates></LinearRing></outerBoundaryIs></Polygon></vkml:Kernel><vkml:Conjecture><Polygon><outerBoundaryIs><LinearRing><coordinates>-9.8509787230205,16.347974175021577 -10.139369836301674,16.908528824688961 -10.878200402707728,16.524475121859354 -9.908656945676734,15.946964654761791 -9.8509787230205,16.347974175021577</coordinates></LinearRing></outerBoundaryIs></Polygon></vkml:Conjecture></vkml:VaguePolygon>'));
select VG_AsText(vg_vaguegeomfromVKML('<vkml:VaguePolygon><vkml:Kernel><Polygon><outerBoundaryIs><LinearRing><coordinates>-14.772853723019185,28.039405326002235 8.430271276974629,21.663964359629226 -0.007228723023134,10.192109281820079 -9.850978723020456,16.347974175021566 -14.772853723019185,28.039405326002235</coordinates></LinearRing></outerBoundaryIs></Polygon></vkml:Kernel></vkml:VaguePolygon>'));
select VG_AsText(vg_vaguegeomfromVKML('<vkml:VaguePolygon><vkml:Conjecture><Polygon><outerBoundaryIs><LinearRing><coordinates>-9.8509787230205,16.347974175021577 -10.139369836301674,16.908528824688961 -10.878200402707728,16.524475121859354 -9.908656945676734,15.946964654761791 -9.8509787230205,16.347974175021577</coordinates></LinearRing></outerBoundaryIs></Polygon></vkml:Conjecture></vkml:VaguePolygon>'));
select VG_AsText(vg_vaguegeomfromVKML('<vkml:VaguePolygon></vkml:VaguePolygon>'));
--end


--begin of input from VWKB
select vg_astext('01010000000101000000000000000000F03F000000000000F03F010100000000000000000000400000000000000040'::vaguegeometry);
select vg_astext('0101000020E61000000101000000000000000000F03F000000000000F03F010100000000000000000000400000000000000040'::vaguegeometry);
select vg_asevwkt('0101000020E61000000101000000000000000000F03F000000000000F03F010100000000000000000000400000000000000040'::vaguegeometry);
select vg_astext('0104000000010400000000000000010400000001000000010100000000000000000000400000000000000040'::vaguegeometry);
select vg_astext('0104000000010400000000000000010400000000000000'::vaguegeometry);
--end of VWKB

--end of input functions form

--begin of general functions
select VG_isPreComputingUnion(vg_vaguegeomfromtext('VAGUELINESTRING(linestring(1 1, 2 2); linestring(2 2, 3 3))', 4326, true));
select VG_isPreComputingUnion(vg_vaguegeomfromtext('VAGUELINESTRING(linestring(1 1, 2 2); linestring(2 2, 3 3))', 4326, false));

select VG_isPreComputingUnion(VG_PreComputeUnion(vg_vaguegeomfromtext('VAGUELINESTRING(linestring(1 1, 2 2); linestring(2 2, 3 3))', 4326, true), true));
select VG_isPreComputingUnion(VG_PreComputeUnion(vg_vaguegeomfromtext('VAGUELINESTRING(linestring(1 1, 2 2); linestring(2 2, 3 3))', 4326, false), true));

select VG_isPreComputingUnion(VG_PreComputeUnion(vg_vaguegeomfromtext('VAGUELINESTRING(linestring(1 1, 2 2); linestring(2 2, 3 3))', 4326, true), false));
select VG_isPreComputingUnion(VG_PreComputeUnion(vg_vaguegeomfromtext('VAGUELINESTRING(linestring(1 1, 2 2); linestring(2 2, 3 3))', 4326, false), false));

select VG_GetType(vg_vaguegeomfromtext('VAGUELINESTRING(linestring(1 1, 2 2); linestring(2 2, 3 3))', 4326, false));
select VG_GetType(vg_vaguegeomfromtext('VAGUELINESTRING(linestring(1 1, 2 2); linestring empty)', 4326, false));
select VG_GetType(vg_vaguegeomfromtext('VAGUELINESTRING(linestring empty; linestring(2 2, 3 3))', 4326, false));
select VG_GetType(vg_vaguegeomfromtext('VAGUELINESTRING(linestring empty; linestring empty)', 4326, false));
select VG_GetType(vg_vaguegeomfromtext('VAGUELINESTRING(linestring(1 1, 2 2); linestring(2 2, 3 3))', 4326));
select VG_GetType(vg_vaguegeomfromtext('VAGUELINESTRING(linestring(1 1, 2 2); linestring empty)', 4326));
select VG_GetType(vg_vaguegeomfromtext('VAGUELINESTRING(linestring empty; linestring(2 2, 3 3))', 4326));
select VG_GetType(vg_vaguegeomfromtext('VAGUELINESTRING(linestring empty; linestring empty)', 4326));

select VG_GetType(vg_vaguegeomfromtext('VAGUEPOINT(POINT(1 1); POINT(2 2))', 4326, false));
select VG_GetType(vg_vaguegeomfromtext('VAGUEPOLYGON(POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)); POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)))', 4326));
select VG_GetType(vg_vaguegeomfromtext('VAGUEPOLYGON EMPTY', 4326));
select VG_GetType(vg_vaguegeomfromtext('VAGUEPOLYGON EMPTY', 4326, false));


select VG_GetSRID(VG_VagueGeomFromEVWKT('SRID=4269;VAGUEPOINT(1 1; 2 2)'));
select VG_GetSRID(VG_VagueGeomFromText('VAGUELINESTRING(linestring(1 1, 2 2); linestring(2 2, 3 3))', 4326));
select VG_GetSRID(VG_VagueGeomFromText('LINESTRING(3 3, 5 5)', 'LINESTRING(2 2, 3 3)'));

select VG_AsEVWKT(VG_SetSRID(VG_VagueGeomFromEVWKT('SRID=4269;VAGUELINESTRING(1 1, 5 5; 5 5,7 7)'), 4326));
select VG_AsEVWKT(VG_MakeVagueGeom(ST_Transform(VG_Kernel(vg), 3785), ST_Transform(VG_Conjecture(vg), 3785))) from (SELECT VG_SetSRID(VG_VagueGeomFromText('LINESTRING(1 1, 2 2)', 'LINESTRING(2 2, 3 3)'), 4326) as vg) as vg_table;

--end of general functions

--begin of numerical functions
select VG_NearestDistance(VG_VagueGeomFromText('VAGUEPOLYGON((1 1, 2 1, 2 2, 1 2, 1 1); (1 1, 0 1, 0 2, 1 2, 1 1))', 4326), VG_VagueGeomFromText('VAGUEPOINT(5 5; 7 7)', 4326));
select VG_FarthestDistance(VG_VagueGeomFromText('VAGUEPOLYGON((1 1, 2 1, 2 2, 1 2, 1 1); (1 1, 0 1, 0 2, 1 2, 1 1))', 4326), VG_VagueGeomFromText('VAGUEPOINT(5 5; 7 7)', 4326));
select VG_FarthestDistance(VG_MakeVagueGeom(ST_Transform('SRID=4326;POLYGON((1 1, 2 1, 2 2, 1 2, 1 1))'::geometry, 26986), ST_Transform('SRID=4326;POLYGON((1 1, 0 1, 0 2, 1 2, 1 1))'::geometry, 26986)), VG_MakeVagueGeom(ST_Transform('SRID=4326;POINT(5 5)'::geometry, 26986), ST_Transform('SRID=4326;POINT(7 7)'::geometry, 26986)));
select VG_NearestDistance(VG_MakeVagueGeom(ST_Transform('SRID=4326;POLYGON((1 1, 2 1, 2 2, 1 2, 1 1))'::geometry, 26986), ST_Transform('SRID=4326;POLYGON((1 1, 0 1, 0 2, 1 2, 1 1))'::geometry, 26986)), VG_MakeVagueGeom(ST_Transform('SRID=4326;POINT(5 5)'::geometry, 26986), ST_Transform('SRID=4326;POINT(7 7)'::geometry, 26986)));

select VG_Area(vg) as sqft, VG_MakeVagueNumeric(VG_GetMIN(VG_Area(vg))*POWER(0.3048,2), VG_GetMAX(VG_Area(vg))*POWER(0.3048,2)) as sqm from (select  VG_VagueGeomFromText('VAGUEPOLYGON((1 1, 2 1, 2 2, 1 2, 1 1); (1 1, 0 1, 0 2, 1 2, 1 1))', 2249) as vg) as tab;
select VG_Length(VG_VagueGeomFromText('VAGUELINESTRING(linestring(1 1, 2 2, 10 10); linestring(10 10, 32.4 34.4))', 2249));

select VG_GetMIN(VG_MakeVagueNumeric(3.56345, 10.6756));
select VG_GetMAX(VG_MakeVagueNumeric(3.56345, 10.6756));
select VG_MakeVagueNumeric(3.56, 10.56);

select VG_MaxLength(VG_VagueGeomFromText('VAGUELINESTRING(linestring(1 1, 2 2, 10 10); linestring(10 10, 32.4 34.4))', 2249));
select VG_MinLength(VG_VagueGeomFromText('VAGUELINESTRING(linestring(1 1, 2 2, 10 10); linestring(10 10, 32.4 34.4))', 2249));

select VG_MaxArea(vg) as sqft, VG_MaxArea(vg)*POWER(0.3048,2) as sqm from (select  VG_VagueGeomFromText('VAGUEPOLYGON((1 1, 2 1, 2 2, 1 2, 1 1); (1 1, 0 1, 0 2, 1 2, 1 1))', 2249) as vg) as tab;
select VG_MinArea(vg) as sqft, VG_MinArea(vg)*POWER(0.3048,2) as sqm from (select  VG_VagueGeomFromText('VAGUEPOLYGON((1 1, 2 1, 2 2, 1 2, 1 1); (1 1, 0 1, 0 2, 1 2, 1 1))', 2249) as vg) as tab;

--end of numerical functions

--begin of topological operations
select VG_Disjoint(vg_vaguegeomfromtext('VAGUEPOLYGON(POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)); POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)))', 4326), vg_vaguegeomfromtext('VAGUEPOLYGON(POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)); POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)))', 4326));


CREATE TABLE test ( id serial primary key, vg1 vaguegeometry, vg2 vaguegeometry);
insert into test(vg1, vg2) values (vg_vaguegeomfromtext('VAGUEPOLYGON(POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)); POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)))', 4326), vg_vaguegeomfromtext('VAGUEPOLYGON(POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)); POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)))', 4326));
select VG_Disjoint(vg1, vg2) from test;
select VG_Intersects(vg1, vg2) from test;
select VG_Contains(vg1, vg2) from test;
select VG_CoveredBy(vg1, vg2) from test;
select VG_Covers(vg1, vg2) from test;
select VG_Equals(vg1, vg2) from test;
select VG_inside(vg1, vg2) from test;
select VG_meets(vg1, vg2) from test;
select VG_overlap(vg1, vg2) from test;
drop table test;
CREATE TABLE test ( id serial primary key, vg1 vaguegeometry, vg2 vaguegeometry);
insert into test(vg1, vg2) values (vg_vaguegeomfromtext('VAGUEPOLYGON(POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)); POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)))', 4326, false), vg_vaguegeomfromtext('VAGUEPOLYGON(POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)); POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)))', 4326, false));

select VG_Disjoint(vg1, vg2) from test;
select VG_Intersects(vg1, vg2) from test;
select VG_Contains(vg1, vg2) from test;
select VG_CoveredBy(vg1, vg2) from test;
select VG_Covers(vg1, vg2) from test;
select VG_Equals(vg1, vg2) from test;
select VG_inside(vg1, vg2) from test;
select VG_meets(vg1, vg2) from test;
select VG_overlap(vg1, vg2) from test;

drop table test;
CREATE TABLE test ( id serial primary key, vg1 vaguegeometry, vg2 vaguegeometry);
insert into test(vg1, vg2) values (vg_vaguegeomfromtext('VAGUEPOLYGON(POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)); POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)))', 4326), vg_vaguegeomfromtext('VAGUEPOLYGON(POLYGON((-9.8509787230205 16.347974175021577,-10.139369836301674 16.90852882468896,-10.878200402707728 16.524475121859354,-9.908656945676734 15.946964654761791,-9.8509787230205 16.347974175021577)); POLYGON((-14.772853723019185 28.039405326002235,8.43027127697463 21.663964359629226,-0.007228723023134037 10.192109281820079,-9.850978723020456 16.347974175021566,-14.772853723019185 28.039405326002235)))', 4326, false));

select VG_Disjoint(vg1, vg2) from test;
select VG_Intersects(vg1, vg2) from test;
select VG_Contains(vg1, vg2) from test;
select VG_CoveredBy(vg1, vg2) from test;
select VG_Covers(vg1, vg2) from test;
select VG_Equals(vg1, vg2) from test;
select VG_inside(vg1, vg2) from test;
select VG_meets(vg1, vg2) from test;
select VG_overlap(vg1, vg2) from test;

drop table test;


--end of topological operations

--begin of vague geometric set operations
select vg_astext(VG_Union(VG_VagueGeomFromText('VAGUEMULTIPOINT(-1 -1, 1 0, 2 -1, 1 -2; -2 2, -2 2, -1 0, -2 -2, 1 1, 2 1, 2 2)', 4326), VG_VagueGeomFromText('VAGUEMULTIPOINT(-2 2, 0 2, 0 -1, 2 0, 1 -2; -1 -1, -1 -2, 0 0, 1 0, 2 1, 2 2)', 4326)));
select vg_astext(VG_Intersection(VG_VagueGeomFromText('VAGUEMULTIPOINT(-1 -1, 1 0, 2 -1, 1 -2; -2 2, -2 2, -1 0, -2 -2, 1 1, 2 1, 2 2)', 4326), VG_VagueGeomFromText('VAGUEMULTIPOINT(-2 2, 0 2, 0 -1, 2 0, 1 -2; -1 -1, -1 -2, 0 0, 1 0, 2 1, 2 2)', 4326)));
select vg_astext(VG_Difference(VG_VagueGeomFromText('VAGUEMULTIPOINT(-1 -1, 1 0, 2 -1, 1 -2; -2 2, -2 2, -1 0, -2 -2, 1 1, 2 1, 2 2)', 4326), VG_VagueGeomFromText('VAGUEMULTIPOINT(-2 2, 0 2, 0 -1, 2 0, 1 -2; -1 -1, -1 -2, 0 0, 1 0, 2 1, 2 2)', 4326)));

select vg_astext(VG_Union(VG_VagueGeomFromText('VAGUEMULTILINESTRING((2 0, 1 -1, -1 0, 3 2), (1 1, 0 2); (-1 0, 0 2, 3 2, 2 0, 1 1))', 4326), VG_VagueGeomFromText('VAGUEMULTILINESTRING((-2 0, 0 3, 3 0), (-1 0, 0 2), (-1 0, 1 1), (-1 -1, 1 0); (-2 0, 0 2, 2 0, -1 0))', 4326)));
select vg_astext(VG_Intersection(VG_VagueGeomFromText('VAGUEMULTILINESTRING((2 0, 1 -1, -1 0, 3 2), (1 1, 0 2); (-1 0, 0 2, 3 2, 2 0, 1 1))', 4326), VG_VagueGeomFromText('VAGUEMULTILINESTRING((-2 0, 0 3, 3 0), (-1 0, 0 2), (-1 0, 1 1), (-1 -1, 1 0); (-2 0, 0 2, 2 0, -1 0))', 4326)));
select vg_astext(VG_Difference(VG_VagueGeomFromText('VAGUEMULTILINESTRING((2 0, 1 -1, -1 0, 3 2), (1 1, 0 2); (-1 0, 0 2, 3 2, 2 0, 1 1))', 4326), VG_VagueGeomFromText('VAGUEMULTILINESTRING((-2 0, 0 3, 3 0), (-1 0, 0 2), (-1 0, 1 1), (-1 -1, 1 0); (-2 0, 0 2, 2 0, -1 0))', 4326)));

select vg_astext(vg_vaguegeomfromtext('VAGUEMULTILINESTRING(MULTILINESTRING((2 0,1 -1,0 -0.5),(0 -0.5,-1 0),(-1 0,1 1),(1 1,1.66666666666667 1.33333333333333),(1.66666666666667 1.33333333333333,3 2),(1 1,0 2),(-2 0,0 3,1.66666666666667 1.33333333333333),(1.66666666666667 1.33333333333333,3 0),(-1 0,0 2),(-1 -1,0 -0.5),(0 -0.5,1 0)); MULTILINESTRING((0 2,1 2),(1 2,3 2),(3 2,2.33333333333333 0.666666666666667),(2.33333333333333 0.666666666666667,2 0),(2 0,1 1),(-2 0,0 2),(2 0,1 0),(1 0,-1 0)))', 4326));


select vg_astext(VG_Union(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-2 1, -2 2, -4 2, -4 1, -2 1)), ((0 1, 0 2, 2 2, 2 1, 0 1)); ((0 -1, -2 -1, -2 2, -1 2, -1 4, 0 4, 0 -1)))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-2 -1, -2 0, 1 0, 1 2, 2 2, 2 -1, -2  -1)); ((-3 -1, -3 2, 1 2, 1 0, -2 0, -2 -1, -3 -1)))', 4326)));
select vg_astext(VG_Intersection(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-2 1, -2 2, -4 2, -4 1, -2 1)), ((0 1, 0 2, 2 2, 2 1, 0 1)); ((0 -1, -2 -1, -2 2, -1 2, -1 4, 0 4, 0 -1)))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-2 -1, -2 0, 1 0, 1 2, 2 2, 2 -1, -2  -1)); ((-3 -1, -3 2, 1 2, 1 0, -2 0, -2 -1, -3 -1)))', 4326)));
select vg_astext(VG_Difference(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-2 1, -2 2, -4 2, -4 1, -2 1)), ((0 1, 0 2, 2 2, 2 1, 0 1)); ((0 -1, -2 -1, -2 2, -1 2, -1 4, 0 4, 0 -1)))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-2 -1, -2 0, 1 0, 1 2, 2 2, 2 -1, -2  -1)); ((-3 -1, -3 2, 1 2, 1 0, -2 0, -2 -1, -3 -1)))', 4326)));

--intersection mixing the types
--vpoint X vline
--ERROR HERE... ERROR IF FOLLOWS THE VASA DEFINITION 
select vg_astext(VG_Intersection(VG_VagueGeomFromText('VAGUEMULTIPOINT(-1 -1, 1 0, 2 -1, 1 -2; -2 2, -2 2, -1 0, -2 -2, 1 1, 2 1, 2 2)', 4326), VG_VagueGeomFromText('VAGUEMULTILINESTRING((-2 0, 0 3, 3 0), (-1 0, 0 2), (-1 0, 1 1), (-1 -1, 1 0); (-2 0, 0 2, 2 0, -1 0))', 4326)));
--vpoint X vpolygon
--ERROR HERE... ERROR IF FOLLOWS THE VASA DEFINITION
select vg_astext(VG_Intersection(VG_VagueGeomFromText('VAGUEMULTIPOINT(-1 -1, 1 0, 2 -1, 1 -2; -2 2, -2 2, -1 0, -2 -2, 1 1, 2 1, 2 2)', 4326), VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-2 -1, -2 0, 1 0, 1 2, 2 2, 2 -1, -2  -1)); ((-3 -1, -3 2, 1 2, 1 0, -2 0, -2 -1, -3 -1)))', 4326)));
--vline X vpolygon
select vg_astext(VG_Intersection(VG_VagueGeomFromText('VAGUEMULTILINESTRING((2 0, 1 -1, -1 0, 3 2), (1 1, 0 2); (-1 0, 0 2, 3 2, 2 0, 1 1))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-2 -1, -2 0, 1 0, 1 2, 2 2, 2 -1, -2  -1)); ((-3 -1, -3 2, 1 2, 1 0, -2 0, -2 -1, -3 -1)))', 4326)));

--end

--kernel and conjecture type-dependent functions
select VG_AsText(VG_KernelBoundary(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-2 1, -2 2, -4 2, -4 1, -2 1)), ((0 1, 0 2, 2 2, 2 1, 0 1)); ((0 -1, -2 -1, -2 2, -1 2, -1 4, 0 4, 0 -1)))', 4326)));
select VG_AsText(VG_ConjectureBoundary(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-2 1, -2 2, -4 2, -4 1, -2 1)), ((0 1, 0 2, 2 2, 2 1, 0 1)); ((0 -1, -2 -1, -2 2, -1 2, -1 4, 0 4, 0 -1)))', 4326)));

select VG_AsText(VG_KernelConvexHull(VG_VagueGeomFromText('VAGUEMULTIPOINT(-1 -1, 1 0, 2 -1, 1 -2; -2 2, -2 2, -1 0, -2 -2, 1 1, 2 1, 2 2)', 4326)));
select VG_AsText(VG_ConjectureConvexHull(VG_VagueGeomFromText('VAGUEMULTIPOINT(-1 -1, 1 0, 2 -1, 1 -2; -2 2, -2 2, -1 0, -2 -2, 1 1, 2 1, 2 2)', 4326)));

select VG_AsText(VG_KernelInterior(VG_VagueGeomFromText('VAGUEMULTILINESTRING((-2 -1, -1 0, 0 -1, -2 -1), (0 3, 4 3), (0 0, 2 2, 4 2, 4 -1, 0 0); (-4 4, 0 3), (-3 -2, -3 0, -1 2, 1 1, 1 -2, -3 -2), (2 0, 3 0, 3 1, 2 1, 2 0))', 4326)));
select VG_AsText(VG_ConjectureInterior(VG_VagueGeomFromText('VAGUEMULTILINESTRING((-2 -1, -1 0, 0 -1, -2 -1), (0 3, 4 3), (0 0, 2 2, 4 2, 4 -1, 0 0); (-4 4, 0 3), (-3 -2, -3 0, -1 2, 1 1, 1 -2, -3 -2), (2 0, 3 0, 3 1, 2 1, 2 0))', 4326)));

select VG_AsText(VG_KernelVertices(VG_VagueGeomFromText('VAGUEMULTILINESTRING((-2 0, 0 3, 3 0), (-1 0, 0 2), (-1 0, 1 1), (-1 -1, 1 0); (-2 0, 0 2, 2 0, -1 0))', 4326)));
select VG_AsText(VG_ConjectureVertices(VG_VagueGeomFromText('VAGUEMULTILINESTRING((-2 0, 0 3, 3 0), (-1 0, 0 2), (-1 0, 1 1), (-1 -1, 1 0); (-2 0, 0 2, 2 0, -1 0))', 4326)));

--end


--common points and common border
select vg_astext(VG_CommonPoints(VG_VagueGeomFromText('VAGUEMULTILINESTRING((2 0, 1 -1, -1 0, 3 2), (1 1, 0 2); (-1 0, 0 2, 3 2, 2 0, 1 1))', 4326), VG_VagueGeomFromText('VAGUEMULTILINESTRING((-2 0, 0 3, 3 0), (-1 0, 0 2), (-1 0, 1 1), (-1 -1, 1 0); (-2 0, 0 2, 2 0, -1 0))', 4326)));

select VG_AsText(VG_CommonBorder(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-2 1, -2 2, -4 2, -4 1, -2 1)), ((0 1, 0 2, 2 2, 2 1, 0 1)); ((0 -1, -2 -1, -2 2, -1 2, -1 4, 0 4, 0 -1)))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-2 -1, -2 0, 1 0, 1 2, 2 2, 2 -1, -2  -1)); ((-3 -1, -3 2, 1 2, 1 0, -2 0, -2 -1, -3 -1)))', 4326)));
--end

--vague topological predicates

--disjoint
select VG_Disjoint(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-2 1, -2 2, -4 2, -4 1, -2 1)), ((0 1, 0 2, 2 2, 2 1, 0 1)); ((0 -1, -2 -1, -2 2, 0 2, 0 -1)))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((4 0, 4 -1, 8 -1, 8 2, 7 2, 7 0, 4 0)); ((3 -1, 3 2, 7 2, 7 0, 4 0, 4 -1, 3 -1)))', 4326));

select VG_Disjoint(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-1 -1, -1 0, 0 1, 0 -1, -1 -1)), ((3 0, 5 0, 5 -1, 3 -1, 3 0)); ((1 0, 1 -3, 3 -3, 3 0, 1 0)))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-4 0, -4 -1, 0 -1, 0 2, -1 2, -1 0, -4 0)); ((-4 -1, -5 -1, -5 2, -1 2, -1 0, -4 0, -4 -1)))', 4326));

select VG_Disjoint(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((0 3, 0 4, -2 4, -2 3, 0 3)), ((-4 3, -6 3, -6 4, -4 4, -4 3)); ((-2 1, -4 1, -4 4, -2 4, -2 1)))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-4 0, -4 -1, 0 -1, 0 2, -1 2, -1 0, -4 0)); ((-4 -1, -5 -1, -5 2, -1 2, -1 0, -4 0, -4 -1)))', 4326));

 select VG_Disjoint(VG_VagueGeomFromText('VAGUELINESTRING(LINESTRING EMPTY; LINESTRING(30 30, 10 10, 1 1))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOINT(1 1, 2 2; 10 10)', 4326));

 select VG_Disjoint(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((0 3, 0 4, -2 4, -2 3, 0 3)), ((-4 3, -6 3, -6 4, -4 4, -4 3)); ((-2 1, -4 1, -4 4, -2 4, -2 1)))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOINT(1 1, 2 2; 10 10)', 4326)); 

 --intersects
 select VG_Intersects(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-2 1, -2 2, -4 2, -4 1, -2 1)), ((0 1, 0 2, 2 2, 2 1, 0 1)); ((0 -1, -2 -1, -2 2, 0 2, 0 -1)))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((4 0, 4 -1, 8 -1, 8 2, 7 2, 7 0, 4 0)); ((3 -1, 3 2, 7 2, 7 0, 4 0, 4 -1, 3 -1)))', 4326));

select VG_Intersects(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-1 -1, -1 0, 0 1, 0 -1, -1 -1)), ((3 0, 5 0, 5 -1, 3 -1, 3 0)); ((1 0, 1 -3, 3 -3, 3 0, 1 0)))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-4 0, -4 -1, 0 -1, 0 2, -1 2, -1 0, -4 0)); ((-4 -1, -5 -1, -5 2, -1 2, -1 0, -4 0, -4 -1)))', 4326));

select VG_Intersects(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((0 3, 0 4, -2 4, -2 3, 0 3)), ((-4 3, -6 3, -6 4, -4 4, -4 3)); ((-2 1, -4 1, -4 4, -2 4, -2 1)))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-4 0, -4 -1, 0 -1, 0 2, -1 2, -1 0, -4 0)); ((-4 -1, -5 -1, -5 2, -1 2, -1 0, -4 0, -4 -1)))', 4326));

 select VG_Intersects(VG_VagueGeomFromText('VAGUELINESTRING(LINESTRING EMPTY; LINESTRING(30 30, 10 10, 1 1))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOINT(1 1, 2 2; 10 10)', 4326));

 select VG_Intersects(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((0 3, 0 4, -2 4, -2 3, 0 3)), ((-4 3, -6 3, -6 4, -4 4, -4 3)); ((-2 1, -4 1, -4 4, -2 4, -2 1)))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOINT(1 1, 2 2; 10 10)', 4326)); 
 
 select VG_Intersects(vg1, vg2), VG_Disjoint(vg1, vg2) from VG_VagueGeomFromText('VAGUELINESTRING(LINESTRING(1 1, -3 3, -4 4); LINESTRING(30 30, 10 10, 1 1))', 4326) as vg1, VG_VagueGeomFromText('VAGUEMULTIPOINT(1 1, 2 2; 10 10)', 4326) as vg2;

 --meets
 select VG_Meets(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-2 1, -2 2, -4 2, -4 1, -2 1)), ((0 1, 0 2, 2 2, 2 1, 0 1)); ((0 -1, -2 -1, -2 2, 0 2, 0 -1)))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((2 2, 3 2, 3 0, 6 0, 6 -1, 2 -1, 2 2)); ((3 0, 6 0, 6 -1, 7 -1, 7 2, 3 2, 3 0)))', 4326));

select VG_Meets(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-1 -1, -1 0, 0 1, 0 -1, -1 -1)), ((3 0, 5 0, 5 -1, 3 -1, 3 0)); ((1 0, 1 -3, 3 -3, 3 0, 1 0)))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-4 0, -4 -1, 0 -1, 0 2, -1 2, -1 0, -4 0)); ((-4 -1, -5 -1, -5 2, -1 2, -1 0, -4 0, -4 -1)))', 4326));

select VG_Meets(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((0 3, 0 4, 2 4, 2 3, 0 3)), ((-2 3, -4 3, -4 4, -2 4, -2 3)); ((0 1, -2 1, -2 4, 0 4, 0 1)))', 4326),VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-4 0, -4 -1, 0 -1, 0 2, -1 2, -1 0, -4 0)); ((-4 -1, -5 -1, -5 2, -1 2, -1 0, -4 0, -4 -1)))', 4326));

select VG_Meets(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((0 3, 0 4, 2 4, 2 3, 0 3)), ((-2 3, -4 3, -4 4, -2 4, -2 3)); ((0 1, -2 1, -2 4, 0 4, 0 1)))', 4326),VG_VagueGeomFromText('VAGUEPOINT(10 10; 50 50)', 4326));

select VG_Meets(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((0 3, 0 4, 2 4, 2 3, 0 3)), ((-2 3, -4 3, -4 4, -2 4, -2 3)); ((0 1, -2 1, -2 4, 0 4, 0 1)))', 4326),VG_VagueGeomFromText('VAGUELINESTRING(EMPTY; -4 -1, -5 -1, 1 1)', 4326));

select VG_Meets(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((0 3, 0 4, 2 4, 2 3, 0 3)), ((-2 3, -4 3, -4 4, -2 4, -2 3)); ((0 1, -2 1, -2 4, 0 4, 0 1)))', 4326),VG_VagueGeomFromText('VAGUELINESTRING(EMPTY; 1 1, 10 10, -4 4)', 4326));

--equals
select VG_Equals(VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 3 0, 3 2, 0 3, 0 0); EMPTY)', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 3 0, 3 2, 0 3, 0 0); EMPTY)', 4326));

select VG_Equals(VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((-2 1, -2 2, -4 2, -4 1, -2 1)), ((0 1, 0 2, 2 2, 2 1, 0 1)); ((0 -1, -2 -1, -2 2, 0 2, 0 -1)))', 4326), VG_VagueGeomFromText('VAGUEMULTIPOLYGON(((4 0, 4 -1, 8 -1, 8 2, 7 2, 7 0, 4 0)); ((3 -1, 3 2, 7 2, 7 0, 4 0, 4 -1, 3 -1)))', 4326));

select VG_Equals(VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 3 0, 3 2, 0 2, 0 0); (0 2, 0 3, 3 3, 3 2, 0 2))', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 3 0, 3 3, 0 3, 0 0); (0 0, -2 0, -2 1, 0 1, 0 0))', 4326)); --maybe

select VG_Equals(VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 3 0, 3 2, 0 3, 0 0); (0 0, -3 0, -3 1, 0 1, 0 0))', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 3 0, 3 2, 0 3, 0 0); (-2 1, -3 1, -3 -2, -2 -2, -2 1))', 4326)); --maybe

--covers
select VG_Covers(VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 3, 7 3, 7 0, 0 0); (0 0, -2 0, -2 1, 0 1, 0 0))', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 2, 3 2, 3 0, 0 0); (3 1, 3 2, 6 2, 6 1, 3 1))', 4326)); --true

select VG_Covers(VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 3, 2 3, 2 0, 0 0); (0 0, -2 0, -2 1, 0 1, 0 0))', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 2, 3 2, 3 0, 0 0); (3 1, 3 2, 6 2, 6 1, 3 1))', 4326)); --false

select VG_Covers(VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 3, 4 3, 4 0, 0 0); (0 0, -2 0, -2 1, 0 1, 0 0))', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 2, 3 2, 3 0, 0 0); (3 1, 3 2, 6 2, 6 1, 3 1))', 4326)); -- maybe


--contains
select VG_Contains(VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 3, 7 3, 7 0, 0 0); (0 0, -2 0, -2 1, 0 1, 0 0))', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((1 1, 1 2, 4 2, 4 1, 1 1); (4 1, 4 2, 6 2, 6 1, 4 1))', 4326)); -- true

select VG_Contains(VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 3, 4 3, 4 0, 0 0); (0 0, -2 0, -2 1, 0 1, 0 0))', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 2, 3 2, 3 0, 0 0); (3 1, 3 2, 6 2, 6 1, 3 1))', 4326)); --false

select VG_Contains(VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 3, 4 3, 4 0, 0 0); (0 0, -2 0, -2 1, 0 1, 0 0))', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((1 1, 1 2, 3 2, 3 1, 1 1); (3 1, 3 2, 6 2, 6 1, 3 1))', 4326)); --maybe

--coveredBy
select VG_CoveredBy(VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 3, 7 3, 7 0, 0 0); (0 0, -2 0, -2 1, 0 1, 0 0))', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 2, 3 2, 3 0, 0 0); (3 1, 3 2, 6 2, 6 1, 3 1))', 4326)); --false

select VG_CoveredBy(VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 3, 2 3, 2 0, 0 0); (0 0, -2 0, -2 1, 0 1, 0 0))', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 2, 3 2, 3 0, 0 0); (3 1, 3 2, 6 2, 6 1, 3 1))', 4326)); --true

select VG_CoveredBy(VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 3, 4 3, 4 0, 0 0); (0 0, -2 0, -2 1, 0 1, 0 0))', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 2, 3 2, 3 0, 0 0); (3 1, 3 2, 6 2, 6 1, 3 1))', 4326)); --maybe


--inside
select VG_Inside(VG_VagueGeomFromText('VAGUEPOLYGON((1 1, 1 2, 4 2, 4 1, 1 1); (4 1, 4 2, 6 2, 6 1, 4 1))', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 3, 7 3, 7 0, 0 0); (0 0, -2 0, -2 1, 0 1, 0 0))', 4326)); --true

select VG_Inside(VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 2, 3 2, 3 0, 0 0); (3 1, 3 2, 6 2, 6 1, 3 1))', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 3, 4 3, 4 0, 0 0); (0 0, -2 0, -2 1, 0 1, 0 0))', 4326)); --false

select VG_Inside(VG_VagueGeomFromText('VAGUEPOLYGON((1 1, 1 2, 3 2, 3 1, 1 1); (3 1, 3 2, 6 2, 6 1, 3 1))', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((0 0, 0 3, 4 3, 4 0, 0 0); (0 0, -2 0, -2 1, 0 1, 0 0))', 4326)); --maybe

--crosses
select VG_Crosses(VG_VagueGeomFromText('VAGUELINESTRING(0 3, 4 0; -1 0, 0 3)', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((1 1, 1 2, 3 2, 3 1, 1 1); (3 1, 3 2, 6 2, 6 1, 3 1))', 4326));

select VG_Crosses(VG_VagueGeomFromText('VAGUELINESTRING(0 3, 5 2; -1 0, 0 3)', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((1 1, 1 2, 3 2, 3 1, 1 1); (3 1, 3 2, 6 2, 6 1, 3 1))', 4326));

select VG_Crosses(VG_VagueGeomFromText('VAGUELINESTRING(EMPTY; -1 0, 0 3, 4 0)', 4326), VG_VagueGeomFromText('VAGUEPOLYGON((1 1, 1 2, 3 2, 3 1, 1 1); (3 1, 3 2, 6 2, 6 1, 3 1))', 4326));

--overlap


--end



-- expected failures
select vg_vaguegeomfromtext('point(1 1)', 'point(1 1)', 0);
select vg_astext(vg_makevaguegeom(st_geomfromtext('point(1 1)', 0), st_geomfromtext('point(2 2)', 4326)));
select vg_astext(vg_makevaguegeom(NULL, NULL));
select vg_astext(VG_MakeVagueGeom(st_geomfromtext('POLYGON((-53.89141308351281 -3.526662200548747,-47.91485058351441 0.5140706695293608,-45.45391308351505 -2.4734350706689163,-43.69610058351553 -6.15374468714475,-53.89141308351281 -3.526662200548747))', 0), st_geomfromtext('POLYGON((-53.53985058351292 0.16251484894146204,-46.86016308351468 -1.770796706377859,-40.356256833516426 -8.594138668560648,-56.17656933351221 -4.403456066917021,-53.53985058351292 0.16251484894146204))', 0))); -- the kernel and conjecture are overlapped
select vg_astext(vg_vaguegeomfromtext('VAGUEPOINT EMPTY 7387585', 4326));
select vg_astext(vg_vaguegeomfromtext('VAGUEMULTIPOLYGON(polygon empty; multipolygon(((1 1, 2 2, 3 3, 1 1))))', 4326));
select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VagueLIneString", "kernel":{"type":"Point","coordinates":[]}, "conjecture":{"type":"Point","coordinates":[2,2]}}'));
select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VaguePoint", "kernel":{"type":"Point","coordinates":[]}, "conjecture":{"type":"Point","coordinates":[]}}')); --da error
select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VaguePoin2t"}'));
--there are fatal errors in the next commands including in the PostGIS.
--select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VaguePolygon", "kernel":{"type":"Polygon","coordinates":[]}, "conjecture":{"type":"Polygon","coordinates":[[[-9.8509787230205,16.3479741750216],[-10.1393698363017,16.908528824689],[-10.8782004027077,16.5244751218594],[-9.90865694567673,15.9469646547618],[-9.8509787230205,16.3479741750216]]]}}'));
--select vg_astext(VG_VagueGeomFromVGeoJson('{"type": "VaguePolygon", "kernel":{"type":"Polygon","coordinates":[[[-14.7728537230192,28.0394053260022],[8.43027127697463,21.6639643596292],[-0.007228723023134,10.1921092818201],[-9.85097872302046,16.3479741750216],[-14.7728537230192,28.0394053260022]]]}, "conjecture":{"type":"Polygon","coordinates":[]}}'));
 select vg_area( VG_VagueGeomFromText('VAGUELINESTRING(1 1, 2 1, 2 2, 1 2, 1 1; empty)', 2249));


--drop extension if exists vasavgeom;
