#include "geodot.h"
#include "RasterTileExtractor.h"

#include <algorithm> // For std::clamp
#include <functional> // For std::hash
#include <mutex> // For std::mutex

std::mutex resource_creation_mutex;


using namespace godot;

void Geodot::_init() {
    RasterTileExtractor::initialize();
    VectorExtractor::initialize();
}

void Geodot::_register_methods() {
    register_method("get_dataset", &Geodot::get_dataset);
}

Ref<GeoDataset> Geodot::get_dataset(String path) {
    GeoDataset *dataset = GeoDataset::_new();

    dataset->load_from_file(path);

    return dataset;
}

Ref<GeoRasterLayer> Geodot::get_raster_layer_for_pyramid(String root_folder, String image_ending) {
    PyramidGeoRasterLayer *layer = PyramidGeoRasterLayer::_new();

    layer->set_path(root_folder);
    layer->set_file_ending(image_ending);

    return layer;
}
