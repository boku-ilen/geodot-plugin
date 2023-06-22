#include "geotransform.h"
#include "RasterTileExtractor.h"

#include <algorithm>  // For std::clamp
#include <functional> // For std::hash

using namespace godot;

void GeoTransform::_bind_methods() {
    ClassDB::bind_static_method("GeoTransform", D_METHOD("transform_coordinates"), &GeoTransform::transform_coordinates);
}

Vector3 GeoTransform::transform_coordinates(Vector3 coordinates, int from, int to) {
    std::vector<double> transformed = VectorExtractor::transform_coordinates(
        coordinates.x, coordinates.z, from, to);

    return Vector3(transformed[0], 0.0, transformed[1]);
}