#include "loaders.h"
#include "geodata.h"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "godot_cpp/variant/string_name.hpp"
#include <godot_cpp/classes/project_settings.hpp>

#include <iostream>



namespace godot {

// GeoDataset

void GeoDatasetLoader::_bind_methods() {
    // The superclass already registers these virtual methods
}

PackedStringArray GeoDatasetLoader::_get_recognized_extensions() const {
    PackedStringArray extensions;

    // FIXME: Use https://gis.stackexchange.com/questions/175610/list-of-gdal-raster-file-extensions to generate this list

    extensions.append("gpkg");

    return extensions;
}

bool GeoDatasetLoader::_handles_type(const StringName &type) const {
    return type == StringName("GeoDataset");
}

String GeoDatasetLoader::_get_resource_script_class(const String &path) const {
    return "GeoDataset";
}

String GeoDatasetLoader::_get_resource_type(const String &path) const {
    return "GeoDataset";
}

Variant GeoDatasetLoader::_load(const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const {
    Ref<GeoDataset> dataset;
    dataset.instantiate();
    
    dataset->load_from_file(p_path, 0);

    std::cout << "LOADED>" << std::endl;
    
    return dataset;
}

// GeoRasterLayer

void GeoRasterLayerLoader::_bind_methods() {
    // The superclass already registers these virtual methods
}

PackedStringArray GeoRasterLayerLoader::_get_recognized_extensions() const {
    PackedStringArray extensions;

    // FIXME: Use https://gis.stackexchange.com/questions/175610/list-of-gdal-raster-file-extensions to generate this list

    extensions.append("tif");
    extensions.append("mbtiles");

    return extensions;
}

bool GeoRasterLayerLoader::_handles_type(const StringName &type) const {
    return type == StringName("GeoRasterLayer");
}

String GeoRasterLayerLoader::_get_resource_script_class(const String &path) const {
    return "GeoRasterLayer";
}

String GeoRasterLayerLoader::_get_resource_type(const String &path) const {
    return "GeoRasterLayer";
}


Variant GeoRasterLayerLoader::_load(const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const {
    Ref<GeoRasterLayer> layer;
    layer.instantiate();
    
    layer->load_from_file(ProjectSettings::get_singleton()->globalize_path(p_path), 0);
    
    return layer;
}


}