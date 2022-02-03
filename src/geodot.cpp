#include "geodot.h"
#include "RasterTileExtractor.h"

#include <algorithm>  // For std::clamp
#include <functional> // For std::hash

using namespace godot;

void Geodot::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_dataset"), &Geodot::get_dataset);
    ClassDB::bind_method(D_METHOD("get_raster_layer"), &Geodot::get_raster_layer);
    ClassDB::bind_method(D_METHOD("get_raster_layer_for_pyramid"),
                         &Geodot::get_raster_layer_for_pyramid);
    ClassDB::bind_method(D_METHOD("transform_coordinates"), &Geodot::transform_coordinates);
}

Ref<GeoDataset> Geodot::get_dataset(String path) {
    Ref<GeoDataset> dataset;
    dataset.instantiate();

    dataset->load_from_file(path);

    return dataset;
}

Ref<GeoRasterLayer> Geodot::get_raster_layer(String path) {
    Ref<GeoRasterLayer> layer;
    layer.instantiate();

    layer->load_from_file(path);

    return layer;
}

Ref<GeoRasterLayer> Geodot::get_raster_layer_for_pyramid(String root_folder, String image_ending) {
    Ref<PyramidGeoRasterLayer> layer = Ref<PyramidGeoRasterLayer>();
    layer.instantiate();

    layer->set_pyramid_base(root_folder);
    layer->set_file_ending(image_ending);

    return layer;
}

Vector3 Geodot::transform_coordinates(Vector3 coordinates, String from, String to) {
    std::vector<double> transformed = VectorExtractor::transform_coordinates(
        coordinates.x, coordinates.z, from.utf8().get_data(), to.utf8().get_data());

    return Vector3(transformed[0], 0.0, transformed[1]);
}