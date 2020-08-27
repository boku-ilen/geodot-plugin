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
    load_mutex = Ref<Mutex>(Mutex::_new());
    image_cache = Dictionary();
}

void Geodot::_register_methods() {
    register_method("get_dataset", &Geodot::get_dataset);
}

GeoDataset *Geodot::get_dataset(String path) {
    GeoDataset *dataset = GeoDataset::_new();

    dataset->load_from_file(path);

    return dataset;
}
