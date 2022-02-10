#include "geoutil.h"
#include "RasterTileExtractor.h"

#include <algorithm>  // For std::clamp
#include <functional> // For std::hash

using namespace godot;

void GeoUtil::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_raster_layer_for_pyramid"),
                         &GeoUtil::get_raster_layer_for_pyramid);
    ClassDB::bind_method(D_METHOD("transform_coordinates"), &GeoUtil::transform_coordinates);
}

Ref<GeoRasterLayer> GeoUtil::get_raster_layer_for_pyramid(String root_folder, String image_ending) {
    Ref<PyramidGeoRasterLayer> layer = Ref<PyramidGeoRasterLayer>();
    layer.instantiate();

    layer->set_pyramid_base(root_folder);
    layer->set_file_ending(image_ending);

    return layer;
}

Vector3 GeoUtil::transform_coordinates(Vector3 coordinates, String from, String to) {
    std::vector<double> transformed = VectorExtractor::transform_coordinates(
        coordinates.x, coordinates.z, from.utf8().get_data(), to.utf8().get_data());

    return Vector3(transformed[0], 0.0, transformed[1]);
}