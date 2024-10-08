#include "PolygonFeature.h"
#include "gdal-includes.h"

PolygonFeature::PolygonFeature(OGRFeature *feature) : Feature(feature) {
    polygon = feature->GetGeometryRef()->toPolygon();
    geometry_type = POLYGON;
}

PolygonFeature::PolygonFeature(OGRFeature *feature, OGRGeometry *ogrPolygon)
    : Feature(feature), polygon(ogrPolygon->toPolygon()) {
    geometry_type = POLYGON;
}

std::list<std::vector<double>> PolygonFeature::get_outer_vertices() {
    std::list<std::vector<double>> vertices = std::list<std::vector<double>>();

    OGRLinearRing *ring = polygon->getExteriorRing();

    if (ring == nullptr) {
        // If the polygon is empty, the return value of getExteriorRing is a valid nullptr.
        // In that case, just return the empty list.
        return vertices;
    }

    if (!ring->isClockwise()) {
        ring->reversePoints();
    }

    // Iterate over all points in that ring and append them as vertices
    for (const OGRPoint &point : ring) {
        vertices.emplace_back(std::vector<double>{point.getX(), point.getY()});
    }

    return vertices;
}

void PolygonFeature::set_outer_vertices(std::list<std::vector<double>> vertices) {
    OGRLinearRing *ring = polygon->getExteriorRing();
    ring->setNumPoints(vertices.size());
    
    int counter = 0;
    for (std::vector<double> vertex : vertices) {
        ring->setPoint(counter, vertex[0], vertex[1]);
        counter++;
    }

    ring->closeRings();
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
