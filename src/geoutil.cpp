#include "geoutil.h"
#include "RasterTileExtractor.h"

#include <algorithm>  // For std::clamp
#include <functional> // For std::hash

using namespace godot;

void GeoUtil::_bind_methods() {
    ClassDB::bind_method(D_METHOD("transform_coordinates"), &GeoUtil::transform_coordinates);
}

Vector3 GeoUtil::transform_coordinates(Vector3 coordinates, String from, String to) {
    std::vector<double> transformed = VectorExtractor::transform_coordinates(
        coordinates.x, coordinates.z, from.utf8().get_data(), to.utf8().get_data());

    return Vector3(transformed[0], 0.0, transformed[1]);
}