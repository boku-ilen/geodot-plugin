#include "PolygonFeature.h"
#include "gdal.h"

PolygonFeature::PolygonFeature(OGRFeature *feature) : Feature(feature) {
    polygon = feature->GetGeometryRef()->toPolygon();
    geometry_type = POLYGON;
}

PolygonFeature::PolygonFeature(OGRFeature *feature, const OGRGeometry *ogrPolygon)
    : Feature(feature), polygon(ogrPolygon->toPolygon()) {
    geometry_type = POLYGON;
}

std::list<std::vector<double>> PolygonFeature::get_outer_vertices() {
    std::list<std::vector<double>> vertices = std::list<std::vector<double>>();

    const OGRLinearRing *ring = polygon->getExteriorRing();

    if (ring == nullptr) {
        // If the polygon is empty, the return value of getExteriorRing is a valid nullptr.
        // In that case, just return the empty list.
        return vertices;
    }

    // Iterate over all points in that ring and append them as vertices
    for (const OGRPoint &point : ring) {
        vertices.emplace_back(std::vector<double>{point.getX(), point.getY()});
    }

    return vertices;
}

std::list<std::list<std::vector<double>>> PolygonFeature::get_holes() {
    std::list<std::list<std::vector<double>>> holes = std::list<std::list<std::vector<double>>>();

    int interior_ring_count = polygon->getNumInteriorRings();

    for (int i = 0; i < interior_ring_count; i++) {
        std::list<std::vector<double>> hole_vertices;
        const OGRLinearRing *ring = polygon->getInteriorRing(i);

        if (ring != nullptr) {
            // Iterate over all points in that ring and append them as vertices
            for (const OGRPoint &point : ring) {
                hole_vertices.emplace_back(std::vector<double>{point.getX(), point.getY()});
            }
        }

        holes.emplace_back(hole_vertices);
    }

    return holes;
}
