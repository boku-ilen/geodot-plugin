#ifndef __POLYGONFEATURE_H__
#define __POLYGONFEATURE_H__

#include "Feature.h"

// Foward declarations
class OGRPolygon;

class EXPORT PolygonFeature : public Feature {
public:
    /// Construct a PolygonFeature with a feature which contains a Polygon.
    explicit PolygonFeature(OGRFeature *feature);

    /// Construct a PolygonFeature with a feature that can contain any geometry (usually used for MultiPolygons) - it is
    /// not accessed, the geometry is given as a separate PolygonFeature parameter instead.
    PolygonFeature(OGRFeature *feature, const OGRGeometry *ogrPolygon);

    /// Get the vertices of the base shape
    // TODO

    /// Get all cutout shapes
    // TODO

private:
    const OGRPolygon *polygon;
};

#endif // __POLYGONFEATURE_H__