#ifndef __POLYGONFEATURE_H__
#define __POLYGONFEATURE_H__

#include "Feature.h"

#include <list>
#include <vector>

// Foward declarations
class OGRPolygon;

class PolygonFeature : public Feature {
  public:
    /// Construct a PolygonFeature with a feature which contains a Polygon.
    explicit PolygonFeature(OGRFeature *feature);

    /// Construct a PolygonFeature with a feature that can contain any geometry (usually used for
    /// MultiPolygons) - it is not accessed, the geometry is given as a separate PolygonFeature
    /// parameter instead.
    PolygonFeature(OGRFeature *feature, OGRGeometry *ogrPolygon);

    /// Get the vertices of the base shape
    std::list<std::vector<double>> get_outer_vertices();

    /// Replace the outer vertices with new geometry (holes, if any, are left unchanged)
    void set_outer_vertices(std::list<std::vector<double>> vertices);

    /// Get all cutout shapes
    std::list<std::list<std::vector<double>>> get_holes();

  private:
    OGRPolygon *polygon;
};

#endif // __POLYGONFEATURE_H__