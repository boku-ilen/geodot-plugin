#include "geotransform.h"
#include "RasterTileExtractor.h"
#include "VectorExtractor.h"

#include <algorithm>  // For std::clamp
#include <functional> // For std::hash

using namespace godot;

GeoTransform::~GeoTransform() {
    delete transform;
}

void GeoTransform::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_transform"), &GeoTransform::set_transform);
    ClassDB::bind_method(D_METHOD("transform_coordinates"), &GeoTransform::transform_coordinates);
}

void GeoTransform::set_transform(int from, int to) {
    transform = new CoordinateTransform(from, to);
}

Vector3 GeoTransform::transform_coordinates(Vector3 coordinates) {
    std::vector<double> transformed = transform->transform_coordinates(coordinates.x, coordinates.z);

    return Vector3(transformed[0], 0.0, transformed[1]);
}