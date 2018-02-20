#define PL_ARITY_AS_SIZE 1
#include <cstdio>
#include <cstdlib>
#include <SWI-cpp.h>
#include <SWI-Prolog.h>
#include <SWI-Stream.h>

//DEB
#include <iostream>

// GEOS C API
#define GEOS_USE_ONLY_R_API
#include <geos_c.h>

static const PlAtom ATOM_geos_version {"geos_version"};

GEOSGeometry *parse_geometry(const char *s);

const GEOSContextHandle_t handle {GEOS_init_r()};
GEOSWKTWriter *w = GEOSWKTWriter_create_r(handle);

// gis_halt_ is det.
PREDICATE(gis_halt_, 0)
{
  GEOS_finish_r(handle);
  PL_succeed;
}

// gis_property_(?Property:compound) is det.
PREDICATE(gis_property_, 1)
{
  PlTerm value {A1[1]};
  if (A1.name() == ATOM_geos_version) {
    return (value = GEOSversion());
  } else {
    throw PlDomainError("gis_property", A1);
  }
}

// gis_boundary_(+Wkt:atom, -Boundary:atom) is det.
PREDICATE(gis_boundary_, 2)
{
  size_t len1;
  char *s1;
  if (!PL_get_nchars(A1, &len1, &s1, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g1 = parse_geometry(s1);
  const GEOSGeometry *g2 = GEOSBoundary_r(handle, g1);
  const char *s2 = GEOSWKTWriter_write_r(handle, w, g2);
  GEOSWKTWriter_destroy_r(handle, w);
  return A2 = s2;
}

// gis_contains_(+Wkt1:atom, +Wkt2:atom) is semidet.
PREDICATE(gis_contains_, 2)
{
  size_t len1, len2;
  char *s1, *s2;
  if (!PL_get_nchars(A1, &len1, &s1, CVT_ATOM) ||
      !PL_get_nchars(A2, &len2, &s2, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g1 = parse_geometry(s1);
  const GEOSGeometry *g2 = parse_geometry(s2);
  const char status {GEOSContains_r(handle, g1, g2)};
  switch(status) {
  case 0:
    PL_fail;
  case 1:
    PL_succeed;
  default:
    std::cerr << "Cannot determine whether or not the former shape contains the latter.\n";
    PL_fail;
  }
}

// gis_convex_hull_(+Wkt:atom, -ConvexHull:atom) is det.
PREDICATE(gis_convex_hull_, 2)
{
  size_t len1;
  char *s1;
  if (!PL_get_nchars(A1, &len1, &s1, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g1 = parse_geometry(s1);
  const GEOSGeometry *g2 = GEOSConvexHull_r(handle, g1);
  const char *s2 = GEOSWKTWriter_write_r(handle, w, g2);
  GEOSWKTWriter_destroy_r(handle, w);
  return A3 = s2;
}

// gis_crosses_(+Wkt1:atom, +Wkt2:atom) is semidet.
PREDICATE(gis_crosses_, 2)
{
  size_t len1, len2;
  char *s1, *s2;
  if (!PL_get_nchars(A1, &len1, &s1, CVT_ATOM) ||
      !PL_get_nchars(A2, &len2, &s2, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g1 = parse_geometry(s1);
  const GEOSGeometry *g2 = parse_geometry(s2);
  const char status {GEOSCrosses_r(handle, g1, g2)};
  switch(status) {
  case 0:
    PL_fail;
  case 1:
    PL_succeed;
  default:
    std::cerr << "Cannot determine whether or not two shapes cross each other.\n";
    PL_fail;
  }
}

// gis_difference_(+Wkt1:atom, +Wkt2:atom, -Wkt3:atom) is det.
PREDICATE(gis_difference_, 3)
{
  size_t len1, len2;
  char *s1, *s2;
  if (!PL_get_nchars(A1, &len1, &s1, CVT_ATOM) ||
      !PL_get_nchars(A2, &len2, &s2, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g1 = parse_geometry(s1);
  const GEOSGeometry *g2 = parse_geometry(s2);
  const GEOSGeometry *g3 = GEOSDifference_r(handle, g1, g2);
  const char *s3 = GEOSWKTWriter_write_r(handle, w, g3);
  GEOSWKTWriter_destroy_r(handle, w);
  return A3 = s3;
}

// gis_disjoint_(+Wkt1:atom, +Wkt2:atom) is semidet.
PREDICATE(gis_disjoint_, 2)
{
  size_t len1, len2;
  char *s1, *s2;
  if (!PL_get_nchars(A1, &len1, &s1, CVT_ATOM) ||
      !PL_get_nchars(A2, &len2, &s2, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g1 = parse_geometry(s1);
  const GEOSGeometry *g2 = parse_geometry(s2);
  const char status {GEOSDisjoint_r(handle, g1, g2)};
  switch(status) {
  case 0:
    PL_fail;
  case 1:
    PL_succeed;
  default:
    std::cerr << "Cannot determine whether or not two shapes are disjoint.\n";
    PL_fail;
  }
}

// gis_distance_(+Wkt1:atom, +Wkt2:atom, -Distance:float) is semidet.
PREDICATE(gis_distance_, 3)
{
  size_t len1, len2;
  char *s1, *s2;
  double dist {0.0};
  if (!PL_get_nchars(A1, &len1, &s1, CVT_ATOM) ||
      !PL_get_nchars(A2, &len2, &s2, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g1 = parse_geometry(s1);
  const GEOSGeometry *g2 = parse_geometry(s2);
  const int rc {GEOSDistance_r(handle, g1, g2, &dist)};
  if (rc == 1) {
    return (A3 = dist);
  } else {
    PL_fail;
  }
}

// gis_envelope_(+Wkt1:atom, -Envelope:atom) is det.
PREDICATE(gis_envelope_, 2)
{
  size_t len1;
  char *s1;
  if (!PL_get_nchars(A1, &len1, &s1, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g1 = parse_geometry(s1);
  const GEOSGeometry *g2 = GEOSEnvelope_r(handle, g1);
  const char *s2 = GEOSWKTWriter_write_r(handle, w, g2);
  GEOSWKTWriter_destroy_r(handle, w);
  return A2 = s2;
}

// gis_equals_(+Wkt1:atom, +Wkt2:atom) is semidet.
PREDICATE(gis_equals_, 2)
{
  size_t len1, len2;
  char *s1, *s2;
  if (!PL_get_nchars(A1, &len1, &s1, CVT_ATOM) ||
      !PL_get_nchars(A2, &len2, &s2, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g1 = parse_geometry(s1);
  const GEOSGeometry *g2 = parse_geometry(s2);
  const char status {GEOSEquals_r(handle, g1, g2)};
  switch(status) {
  case 0:
    PL_fail;
  case 1:
    PL_succeed;
  default:
    std::cerr << "Cannot determine whether or not two shapes are equal.\n";
    PL_fail;
  }
}

// gis_intersection_(+Wkt1:atom, +Wkt2:atom, -Wkt3:atom) is det.
PREDICATE(gis_intersection_, 3)
{
  size_t len1, len2;
  char *s1, *s2;
  if (!PL_get_nchars(A1, &len1, &s1, CVT_ATOM) ||
      !PL_get_nchars(A2, &len2, &s2, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g1 = parse_geometry(s1);
  const GEOSGeometry *g2 = parse_geometry(s2);
  const GEOSGeometry *g3 = GEOSIntersection_r(handle, g1, g2);
  const char *s3 = GEOSWKTWriter_write_r(handle, w, g3);
  GEOSWKTWriter_destroy_r(handle, w);
  return A3 = s3;
}

// gis_intersects_(+Wkt1:atom, +Wkt2:atom) is semidet.
PREDICATE(gis_intersects_, 2)
{
  size_t len1, len2;
  char *s1, *s2;
  if (!PL_get_nchars(A1, &len1, &s1, CVT_ATOM) ||
      !PL_get_nchars(A2, &len2, &s2, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g1 = parse_geometry(s1);
  const GEOSGeometry *g2 = parse_geometry(s2);
  const char status {GEOSIntersects_r(handle, g1, g2)};
  switch(status) {
  case 0:
    PL_fail;
  case 1:
    PL_succeed;
  default:
    std::cerr << "Cannot determine whether or not two shapes intersect.\n";
    PL_fail;
  }
}

// gis_overlaps_(+Wkt1:atom, +Wkt2:atom) is semidet.
PREDICATE(gis_overlaps_, 2)
{
  size_t len1, len2;
  char *s1, *s2;
  if (!PL_get_nchars(A1, &len1, &s1, CVT_ATOM) ||
      !PL_get_nchars(A2, &len2, &s2, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g1 = parse_geometry(s1);
  const GEOSGeometry *g2 = parse_geometry(s2);
  const char status {GEOSOverlaps_r(handle, g1, g2)};
  switch(status) {
  case 0:
    PL_fail;
  case 1:
    PL_succeed;
  default:
    std::cerr << "Cannot determine whether or not two shapes overlap.\n";
    PL_fail;
  }
}

// gis_symmetric_difference_(+Wkt1:atom, +Wkt2:atom, -Wkt3:atom) is det.
PREDICATE(gis_symmetric_difference_, 3)
{
  size_t len1, len2;
  char *s1, *s2;
  if (!PL_get_nchars(A1, &len1, &s1, CVT_ATOM) ||
      !PL_get_nchars(A2, &len2, &s2, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g1 = parse_geometry(s1);
  const GEOSGeometry *g2 = parse_geometry(s2);
  const GEOSGeometry *g3 = GEOSSymDifference_r(handle, g1, g2);
  const char *s3 = GEOSWKTWriter_write_r(handle, w, g3);
  GEOSWKTWriter_destroy_r(handle, w);
  return A3 = s3;
}

// gis_touches_(+Wkt1:atom, +Wkt2:atom) is semidet.
PREDICATE(gis_touches_, 2)
{
  size_t len1, len2;
  char *s1, *s2;
  if (!PL_get_nchars(A1, &len1, &s1, CVT_ATOM) ||
      !PL_get_nchars(A2, &len2, &s2, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g1 = parse_geometry(s1);
  const GEOSGeometry *g2 = parse_geometry(s2);
  const char status {GEOSTouches_r(handle, g1, g2)};
  switch(status) {
  case 0:
    PL_fail;
  case 1:
    PL_succeed;
  default:
    std::cerr << "Cannot determine whether or not two shapes touch.\n";
    PL_fail;
  }
}

// gis_union_(+Wkt1:atom, +Wkt2:atom, -Wkt3:atom) is det.
PREDICATE(gis_union_, 3)
{
  size_t len1, len2;
  char *s1, *s2;
  if (!PL_get_nchars(A1, &len1, &s1, CVT_ATOM) ||
      !PL_get_nchars(A2, &len2, &s2, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g1 = parse_geometry(s1);
  const GEOSGeometry *g2 = parse_geometry(s2);
  const GEOSGeometry *g3 = GEOSUnion_r(handle, g1, g2);
  const char *s3 = GEOSWKTWriter_write_r(handle, w, g3);
  GEOSWKTWriter_destroy_r(handle, w);
  return A3 = s3;
}

// gis_within_(+Wkt1:atom, +Wkt2:atom) is semidet.
PREDICATE(gis_within_, 2)
{
  size_t len1, len2;
  char *s1, *s2;
  if (!PL_get_nchars(A1, &len1, &s1, CVT_ATOM) ||
      !PL_get_nchars(A2, &len2, &s2, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g1 = parse_geometry(s1);
  const GEOSGeometry *g2 = parse_geometry(s2);
  const char status {GEOSWithin_r(handle, g1, g2)};
  switch(status) {
  case 0:
    PL_fail;
  case 1:
    PL_succeed;
  default:
    std::cerr << "Cannot determine whether or not the former shape lies within the latter.\n";
    PL_fail;
  }
}

// shape_type_(+Wkt:atom, -Type:atom) is det.
PREDICATE(shape_type_, 2)
{
  size_t len;
  char *s;
  if (!PL_get_nchars(A1, &len, &s, CVT_ATOM)) {
    PL_fail;
  }
  const GEOSGeometry *g = parse_geometry(s);
  int rc = (A2 = GEOSGeomType_r(handle, g));
  return rc;
}

GEOSGeometry *parse_geometry(const char *s)
{
  GEOSWKTReader *r = GEOSWKTReader_create_r(handle);
  GEOSGeometry *g = GEOSWKTReader_read_r(handle, r, s);
  if (g == NULL) {
    throw PlTypeError("geometry", s);
  }
  GEOSWKTReader_destroy_r(handle, r);
  return g;
}

extern "C" {
  install_t install_gis()
  {
  }
}
