#include "geodata.h"
#include "NativeLayer.h"
#include "RasterTileExtractor.h"
#include "geofeatures.h"
#include "godot_cpp/core/error_macros.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/variant.hpp"
#include "vector-extractor/Feature.h"
#include "vector-extractor/VectorExtractor.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <vector>

namespace godot {

GeoDataset::~GeoDataset() {
    // delete dataset;
}

void GeoDataset::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_valid"), &GeoDataset::is_valid);
    ClassDB::bind_method(D_METHOD("get_file_info"), &GeoDataset::get_file_info);
    ClassDB::bind_method(D_METHOD("has_write_access"), &GeoDataset::has_write_access);
    ClassDB::bind_method(D_METHOD("get_raster_layers"), &GeoDataset::get_raster_layers);
    ClassDB::bind_method(D_METHOD("get_feature_layers"), &GeoDataset::get_feature_layers);
    ClassDB::bind_method(D_METHOD("get_raster_layer", "name"), &GeoDataset::get_raster_layer);
    ClassDB::bind_method(D_METHOD("get_feature_layer", "name"), &GeoDataset::get_feature_layer);
    ClassDB::bind_method(D_METHOD("load_from_file", "file_path", "write_access"),
                         &GeoDataset::load_from_file);
}

bool GeoDataset::is_valid() {
    return dataset && dataset->is_valid();
}

Dictionary GeoDataset::get_file_info() {
    Dictionary info;

    info["name"] = name;
    info["path"] = dataset->path.c_str();

    return info;
}

bool GeoDataset::has_write_access() {
    return write_access;
}

Array GeoDataset::get_raster_layers() {
    Array layers = Array();

    std::vector<std::string> names = dataset->get_raster_layer_names();
    int total_names = names.size();
    for (int i = 0; i < total_names; i++ ) {
        std::string name = names[i];
        layers.append(get_raster_layer(name.c_str()));
    }

    return layers;
}

Array GeoDataset::get_feature_layers() {
    Array layers = Array();

    std::vector<std::string> names = dataset->get_feature_layer_names();

    for (std::string name : names) {
        layers.append(get_feature_layer(name.c_str()));
    }

    return layers;
}

Ref<GeoRasterLayer> GeoDataset::get_raster_layer(String name) {
    Ref<GeoRasterLayer> raster_layer;
    raster_layer.instantiate();
    raster_layer->name = name;
    raster_layer->set_native_dataset(dataset->get_subdataset(name.utf8().get_data()));
    raster_layer->set_origin_dataset(this);

    return raster_layer;
}

Ref<GeoFeatureLayer> GeoDataset::get_feature_layer(String name) {
    Ref<GeoFeatureLayer> feature_layer;
    feature_layer.instantiate();
    feature_layer->name = name;
    feature_layer->set_native_layer(dataset->get_layer(name.utf8().get_data()));
    feature_layer->set_origin_dataset(this);

    return feature_layer;
}

void GeoDataset::load_from_file(String file_path, bool write_access) {
    this->write_access = write_access;
    dataset = VectorExtractor::open_dataset(file_path.utf8().get_data(), write_access);
}

void GeoDataset::set_native_dataset(std::shared_ptr<NativeDataset> new_dataset) {
    dataset = new_dataset;
}

void GeoFeatureLayer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_valid"), &GeoFeatureLayer::is_valid);
    ClassDB::bind_method(D_METHOD("get_file_info"), &GeoFeatureLayer::get_file_info);
    ClassDB::bind_method(D_METHOD("get_dataset"), &GeoFeatureLayer::get_dataset);
    ClassDB::bind_method(D_METHOD("get_center"), &GeoFeatureLayer::get_center);
    ClassDB::bind_method(D_METHOD("get_feature_by_id", "id"), &GeoFeatureLayer::get_feature_by_id);
    ClassDB::bind_method(D_METHOD("get_all_features"), &GeoFeatureLayer::get_all_features);
    ClassDB::bind_method(D_METHOD("get_features_near_position", "pos_x", "pos_y", "radius"),
                         &GeoFeatureLayer::get_features_near_position);
    ClassDB::bind_method(D_METHOD("get_features_in_square", "top_left_x", "top_left_x", "size_meters"),
                         &GeoFeatureLayer::get_features_in_square);
    ClassDB::bind_method(D_METHOD("create_feature"), &GeoFeatureLayer::create_feature);
    ClassDB::bind_method(D_METHOD("remove_feature", "feature"), &GeoFeatureLayer::remove_feature);
    ClassDB::bind_method(D_METHOD("save_override"), &GeoFeatureLayer::save_override);
    ClassDB::bind_method(D_METHOD("save_new", "file_path"), &GeoFeatureLayer::save_new);

    ADD_SIGNAL(MethodInfo("feature_added", PropertyInfo(Variant::OBJECT, "new_feature")));
    ADD_SIGNAL(MethodInfo("feature_removed", PropertyInfo(Variant::OBJECT, "removed_feature")));
}

bool GeoFeatureLayer::is_valid() {
    return layer && layer->is_valid();
}

Dictionary GeoFeatureLayer::get_file_info() {
    Dictionary info;

    info["name"] = name;
    info["path"] = origin_dataset->dataset->path.c_str();

    return info;
}

Ref<GeoDataset> GeoFeatureLayer::get_dataset() {
    return origin_dataset;
}

Vector3 GeoFeatureLayer::get_center() {
    return Vector3(extent_data.left + (extent_data.right - extent_data.left) / 2.0, 0.0,
                   extent_data.top + (extent_data.down - extent_data.top) / 2.0);
}

// Utility function for converting a Processing Library Feature to the appropriate GeoFeature
Ref<GeoFeature> get_specialized_feature(std::shared_ptr<Feature> raw_feature) {
    // Check which geometry this feature has, and cast it to the according
    // specialized class
    if (raw_feature->geometry_type == raw_feature->POINT) {
        Ref<GeoPoint> point;
        point.instantiate();

        std::shared_ptr<PointFeature> point_feature = std::dynamic_pointer_cast<PointFeature>(raw_feature);

        point->set_gdal_feature(point_feature);

        return point;
    } else if (raw_feature->geometry_type == raw_feature->LINE) {
        Ref<GeoLine> line;
        line.instantiate();

        std::shared_ptr<LineFeature> line_feature = std::dynamic_pointer_cast<LineFeature>(raw_feature);

        line->set_gdal_feature(line_feature);

        return line;
    } else if (raw_feature->geometry_type == raw_feature->POLYGON) {
        Ref<GeoPolygon> polygon;
        polygon.instantiate();

        std::shared_ptr<PolygonFeature> polygon_feature = std::dynamic_pointer_cast<PolygonFeature>(raw_feature);

        polygon->set_gdal_feature(polygon_feature);

        return polygon;
    } else {
        // Geometry type is NONE or unknown
        Ref<GeoFeature> feature;
        feature.instantiate();

        feature->set_gdal_feature(raw_feature);

        return feature;
    }
}

Ref<GeoFeature> GeoFeatureLayer::get_feature_by_id(int id) {
    std::list<std::shared_ptr<Feature> > features = layer->get_feature_by_id(id);
    if (features.empty()) return nullptr;

    // TODO: How to deal with MultiFeatures? Currently we just use the first one
    Ref<GeoFeature> feature = get_specialized_feature(features.front());

    return feature;
}

Array GeoFeatureLayer::get_all_features() {
    Array geofeatures = Array();

    std::list<std::shared_ptr<Feature> > gdal_features = layer->get_features();
    Array features = Array();

    for (std::shared_ptr<Feature> raw_feature : gdal_features) {
        features.push_back(get_specialized_feature(raw_feature));
    }

    return features;
}

Ref<GeoFeature> GeoFeatureLayer::create_feature() {
    std::shared_ptr<Feature> gdal_feature = layer->create_feature();

    Ref<GeoFeature> feature = get_specialized_feature(gdal_feature);

    emit_signal("feature_added", feature);
    return feature;
}

void GeoFeatureLayer::remove_feature(Ref<GeoFeature> feature) {
    // Mark the feature for deletion
    feature->set_deleted(true);

    emit_signal("feature_removed", feature);
}

void GeoFeatureLayer::save_override() {
    ERR_FAIL_COND_MSG(!origin_dataset->write_access,
                      "Cannot override a layer whose dataset was not opened with write access!");
    layer->save_override();
}

void GeoFeatureLayer::save_new(String file_path) {
    layer->save_modified_layer(file_path.utf8().get_data());
}

Array GeoFeatureLayer::get_features_near_position(double pos_x, double pos_y, double radius,
                                                  int max_features) {
    Array features = Array();

    std::list<std::shared_ptr<Feature> > raw_features =
        layer->get_features_near_position(pos_x, pos_y, radius, max_features);

    for (std::shared_ptr<Feature> raw_feature : raw_features) {
        features.push_back(get_specialized_feature(raw_feature));
    }

    return features;
}

Array GeoFeatureLayer::get_features_in_square(double top_left_x, double top_left_y, double size_meters, int max_features) {
    Array features = Array();

    std::list<std::shared_ptr<Feature> > raw_features =
        layer->get_features_in_square(top_left_x, top_left_y, size_meters, max_features);

    for (std::shared_ptr<Feature> raw_feature : raw_features) {
        features.push_back(get_specialized_feature(raw_feature));
    }

    return features;
}

void GeoFeatureLayer::set_native_layer(std::shared_ptr<NativeLayer> new_layer) {
    layer = new_layer;
    extent_data = layer->get_extent();
}

void GeoFeatureLayer::set_origin_dataset(Ref<GeoDataset> dataset) {
    this->origin_dataset = dataset;
}

void GeoRasterLayer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_valid"), &GeoRasterLayer::is_valid);
    ClassDB::bind_method(D_METHOD("has_write_access"), &GeoRasterLayer::has_write_access);
    ClassDB::bind_method(D_METHOD("get_file_info"), &GeoRasterLayer::get_file_info);
    ClassDB::bind_method(D_METHOD("get_format"), &GeoRasterLayer::get_format);
    ClassDB::bind_method(D_METHOD("get_band_count"), &GeoRasterLayer::get_band_count);
    ClassDB::bind_method(D_METHOD("get_band_descriptions"), &GeoRasterLayer::get_band_descriptions);
    ClassDB::bind_method(D_METHOD("get_dataset"), &GeoRasterLayer::get_dataset);
    ClassDB::bind_method(D_METHOD("get_image", "top_left_x", "top_left_y", "size_meters",
                                  "img_size", "interpolation_type"),
                         &GeoRasterLayer::get_image);
    ClassDB::bind_method(D_METHOD("get_band_image", "top_left_x", "top_left_y", "size_meters",
                                  "img_size", "interpolation_type"),
                         &GeoRasterLayer::get_band_image);
    ClassDB::bind_method(D_METHOD("get_value_at_position", "pos_x", "pos_y"),
                         &GeoRasterLayer::get_value_at_position);
    ClassDB::bind_method(D_METHOD("get_value_at_position_with_resolution"),
                         &GeoRasterLayer::get_value_at_position_with_resolution);
    ClassDB::bind_method(D_METHOD("set_value_at_position", "pos_x", "pos_y", "value"),
                         &GeoRasterLayer::set_value_at_position);
    ClassDB::bind_method(
        D_METHOD("smooth_add_value_at_position", "pos_x", "pos_y", "summand", "radius"),
        &GeoRasterLayer::smooth_add_value_at_position);
    ClassDB::bind_method(D_METHOD("overlay_image_at_position", "pos_x", "pos_y", "image", "scale"),
                         &GeoRasterLayer::overlay_image_at_position);
    ClassDB::bind_method(D_METHOD("get_extent"), &GeoRasterLayer::get_extent);
    ClassDB::bind_method(D_METHOD("get_center"), &GeoRasterLayer::get_center);
    ClassDB::bind_method(D_METHOD("get_min"), &GeoRasterLayer::get_min);
    ClassDB::bind_method(D_METHOD("get_max"), &GeoRasterLayer::get_max);
    ClassDB::bind_method(D_METHOD("get_pixel_size"), &GeoRasterLayer::get_pixel_size);
    ClassDB::bind_method(D_METHOD("clone"), &GeoRasterLayer::clone);
    ClassDB::bind_method(D_METHOD("load_from_file", "file_path", "write_access"),
                         &GeoRasterLayer::load_from_file);
}

bool GeoRasterLayer::is_valid() {
    return dataset && dataset->is_valid();
}

bool GeoRasterLayer::has_write_access() {
    return write_access;
}

Array GeoRasterLayer::get_band_descriptions() {
    Array result = Array();

    std::vector<std::string> descriptions = dataset->get_raster_band_descriptions();
    for (int i = 0; i < descriptions.size(); i++) {
        std::string description = descriptions[i];
        result.append(description.c_str());
    }

    return result;
}

Dictionary GeoRasterLayer::get_file_info() {
    Dictionary info;

    // If this dataset comes from another dataset as a subdataset, the origin_dataset is set.
    info["is_subdataset"] = origin_dataset != nullptr;

    // If origin_dataset is not set, this means that this layer is not a subdataset; therefore, the
    // name should only be the file name without the path. Otherwise, if this is a subdataset, the
    // name is just the name.
    info["name"] = origin_dataset == nullptr ? name.get_file() : name;

    // If origin_dataset is not set, the name equals the path. Otherwise, the path comes from the
    // origin_dataset and is set in the `path` attribute.
    info["path"] = origin_dataset == nullptr ? name : dataset->path.c_str();

    return info;
}

Image::Format GeoRasterLayer::get_format() {
    GeoRaster::FORMAT format = GeoRaster::get_format_for_dataset(dataset->dataset);

    switch (format) {
        case GeoRaster::BYTE:
            return Image::FORMAT_R8;
        case GeoRaster::RF:
            return Image::FORMAT_RF;
        case GeoRaster::RGB:
            return Image::FORMAT_RGB8;
        case GeoRaster::RGBA:
            return Image::FORMAT_RGBA8;
        default:
            // FORMAT_MAX is returned as a fallback for mixed, and unknown
            return Image::FORMAT_MAX;
    }
}

int GeoRasterLayer::get_band_count() {
    return dataset->dataset->GetRasterCount();
}

Ref<GeoDataset> GeoRasterLayer::get_dataset() {
    return origin_dataset;
}

Ref<GeoImage> GeoRasterLayer::get_image(double top_left_x, double top_left_y, double size_meters,
                                        int img_size, GeoImage::INTERPOLATION interpolation_type) {

    Ref<GeoImage> image;
    image.instantiate();

    if (dataset == nullptr || !dataset->is_valid()) {
        // TODO: Set validity to false
        UtilityFunctions::push_error("Raster layer '", name,
                                     "' is invalid, cannot perform get_image!");
        return image;
    }

    GeoRaster *raster = RasterTileExtractor::get_tile_from_dataset(
        dataset->dataset, top_left_x, top_left_y, size_meters, img_size, interpolation_type);

    if (raster == nullptr) {
        // TODO: Set validity to false
        UtilityFunctions::push_error("No valid data was available in the raster layer '",
                                     name, "' at the requested position position!");
        return image;
    }

    image->set_raster(raster, interpolation_type);

    return image;
}

Ref<GeoImage> GeoRasterLayer::get_band_image(double top_left_x, double top_left_y, double size_meters,
                                        int img_size, GeoImage::INTERPOLATION interpolation_type, int band_index) {
    Ref<GeoImage> image;
    image.instantiate();
    if (dataset == nullptr || !dataset->is_valid()) {
        UtilityFunctions::push_error("Raster layer '", name, "', index '",
                band_index, "', is invalid. cannot get_band_image!");
        return image;
    }
    GeoRaster *raster = RasterTileExtractor::get_tile_from_dataset(
        dataset->dataset, top_left_x, top_left_y, size_meters, img_size, interpolation_type);
    if (raster == nullptr) {
        UtilityFunctions::push_error("No valid data was available in the raster layer '",
                name, "', index '", band_index, "', at the requested position position!");
        return image;
    }
    image->set_raster_from_band(raster, interpolation_type, band_index);
    return image;
}

float GeoRasterLayer::get_value_at_position(double pos_x, double pos_y) {
    // 0.0001 meters are used for the size because it can't be 0, but should be a pinpoint value.
    return get_value_at_position_with_resolution(pos_x, pos_y, 0.0001);
}

float GeoRasterLayer::get_value_at_position_with_resolution(double pos_x, double pos_y,
                                                            double pixel_size_meters) {
    // TODO: Figure out what exactly we need to clamp to for precise values
    // pos_x -= std::fmod(pos_x, pixel_size_meters);
    // pos_y -= std::fmod(pos_y, pixel_size_meters);

    // Get the GeoRaster for this position with a resolution of 1x1px.
    GeoRaster *raster = RasterTileExtractor::get_tile_from_dataset(dataset->dataset, pos_x, pos_y,
                                                                   pixel_size_meters, 1, 1);

    // TODO: Currently only implemented for RF type.
    // For others, we would either need a completely generic return value, or other specific
    // functions (as the user likely knows or wants to know the exact type).
    if (raster->get_format() == GeoRaster::FORMAT::RF) {
        float *array = (float *)raster->get_as_array();

        return array[0];
    }

    return -1.0;
}

void GeoRasterLayer::set_value_at_position(double pos_x, double pos_y, Variant value) {
    // Validate against Raster type to see whether the passed Variant is sensible
    if (value.get_type() == Variant::Type::FLOAT && get_format() == Image::FORMAT_RF) {
        float godot_float = static_cast<float>(value);
        float *values = new float[1];

        values[0] = godot_float;
        RasterTileExtractor::write_into_dataset(dataset->dataset, pos_x, pos_y, values, 1.0, 0);

        delete[] values;
    } else if (value.get_type() == Variant::Type::COLOR && (get_format() == Image::FORMAT_RGB8 || get_format() == Image::FORMAT_RGBA8)) {
        Color color = static_cast<Color>(value);
        char *values = new char[3];

        values[0] = color.r * 255.0;
        values[1] = color.g * 255.0;
        values[2] = color.b * 255.0;

        RasterTileExtractor::write_into_dataset(dataset->dataset, pos_x, pos_y, values, 1.0, 0);

        delete[] values;

    } else if (value.get_type() == Variant::Type::INT && get_format() == Image::FORMAT_R8) {
        // TODO: Should we check for precision loss and emit a warning here?
        char godot_int = static_cast<int>(value); // Downcast to char, since there are no int images
        char *values = new char[1];

        values[0] = godot_int;
        RasterTileExtractor::write_into_dataset(dataset->dataset, pos_x, pos_y, values, 1.0, 0);

        delete[] values;
    } else {
        std::cout << "Type mismatch: value of type " << value.get_type() << " and dataset of type " << get_format() << std::endl;
    }

    dataset->dataset->FlushCache();
}

void GeoRasterLayer::smooth_add_value_at_position(double pos_x, double pos_y, double summand,
                                                  double radius) {
    // FIXME: Like overlay_image_at_position, this could be done much more efficiently by batch-reading and writing

    float resolution = get_pixel_size();

    for (float offset_x = -radius; offset_x <= radius; offset_x += resolution) {
        for (float offset_y = -radius; offset_y <= radius; offset_y += resolution) {
            float distance_to_center = sqrt(offset_x * offset_x + offset_y * offset_y) / radius;
            float summand_factor = 1.0 - distance_to_center;

            if (summand_factor <= 0.0) {
                continue;
            }

            float pos_here_x = pos_x + offset_x;
            float pos_here_y = pos_y + offset_y;

            float existing_value = get_value_at_position(pos_here_x, pos_here_y);
            float new_value = existing_value + summand_factor * summand;

            set_value_at_position(pos_here_x, pos_here_y, new_value);
        }
    }
}

void GeoRasterLayer::overlay_image_at_position(double pos_x, double pos_y, Ref<Image> image,
                                               double scale) {
    // FIXME: Rough initial implementation, it works but it is very inefficient!
    // Rather than constantly calling set_value_at_position, we'll want to set the entire image data at once.

    float resolution = get_pixel_size();
    image->resize(ceil(scale / resolution), ceil(scale / resolution));

    PackedByteArray data = image->get_data();

    int image_width = image->get_width();
    int image_height = image->get_height();

    if (image->get_format() == Image::FORMAT_R8 && get_format() == Image::FORMAT_R8) {
        // Single-band byte
        for (int i = 0; i < data.size(); i++) {
            char value = data.decode_u8(i);

            double pos_in_image_x = pos_x + ((float)(i % image_width) / (float)image_width) * scale;
            double pos_in_image_y = pos_y - ((float)floor(i / image_width) / (float)image_height) * scale;

            set_value_at_position(pos_in_image_x, pos_in_image_y, value);
        }

    } else if (image->get_format() == Image::FORMAT_RGB8 && (get_format() == Image::FORMAT_RGB8 || get_format() == Image::FORMAT_RGBA8)) {
        // RGB
        for (int i = 0; i < data.size(); i += 3) {
            float value_r = static_cast<float>(data.decode_u8(i)) / 255.0;
            float value_g = static_cast<float>(data.decode_u8(i + 1)) / 255.0;
            float value_b = static_cast<float>(data.decode_u8(i + 2)) / 255.0;

            double pos_in_image_x = pos_x + ((float)(i/3 % image_width) / (float)image_width) * scale;
            double pos_in_image_y = pos_y - ((float)floor(i/3 / image_width) / (float)image_height) * scale;

            set_value_at_position(pos_in_image_x, pos_in_image_y, Color(value_r, value_g, value_b));
        }
    } else if (image->get_format() == Image::FORMAT_RGBA8 && (get_format() == Image::FORMAT_RGB8 || get_format() == Image::FORMAT_RGBA8)) {
        // RGBA
        for (int i = 0; i < data.size(); i += 4) {
            float value_r = static_cast<float>(data.decode_u8(i)) / 255.0;
            float value_g = static_cast<float>(data.decode_u8(i + 1)) / 255.0;
            float value_b = static_cast<float>(data.decode_u8(i + 2)) / 255.0;
            float value_a = static_cast<float>(data.decode_u8(i + 3)) / 255.0;

            double pos_in_image_x = pos_x + ((float)(i/4 % image_width) / (float)image_width) * scale;
            double pos_in_image_y = pos_y - ((float)floor(i/4 / image_width) / (float)image_height) * scale;

            set_value_at_position(pos_in_image_x, pos_in_image_y, Color(value_r, value_g, value_b, value_a));
        }
    } else if (image->get_format() == Image::FORMAT_RF && get_format() == Image::FORMAT_RF) {
        // 4-byte Float
        for (int i = 0; i < data.size(); i += 4) {
            float value = data.decode_float(i);

            double pos_in_image_x = pos_x + ((float)(i % image_width) / (float)image_width) * scale;
            double pos_in_image_y = pos_y - ((float)floor(i / image_width) / (float)image_height) * scale;

            set_value_at_position(pos_in_image_x, pos_in_image_y, value);
        }
    } else {
        std::cout << "Type mismatch: image of type " << image->get_format() << " and dataset of type " << get_format() << std::endl;
    }
}

Rect2 GeoRasterLayer::get_extent() {
    return Rect2(extent_data.left, extent_data.top, extent_data.right - extent_data.left,
                 extent_data.down - extent_data.top);
}

Vector3 GeoRasterLayer::get_center() {
    return Vector3(extent_data.left + (extent_data.right - extent_data.left) / 2.0, 0.0,
                   extent_data.top + (extent_data.down - extent_data.top) / 2.0);
}

float GeoRasterLayer::get_min() {
    return RasterTileExtractor::get_min(dataset->dataset);
}

float GeoRasterLayer::get_max() {
    return RasterTileExtractor::get_max(dataset->dataset);
}

float GeoRasterLayer::get_pixel_size() {
    return RasterTileExtractor::get_pixel_size(dataset->dataset);
}

void GeoRasterLayer::set_origin_dataset(Ref<GeoDataset> dataset) {
    this->origin_dataset = dataset;
}

Ref<GeoRasterLayer> GeoRasterLayer::clone() {
    Ref<GeoRasterLayer> layer_clone;
    layer_clone.instantiate();
    layer_clone->name = this->name;
    layer_clone->set_native_dataset(dataset->clone());
    layer_clone->set_origin_dataset(origin_dataset);

    return layer_clone;
}

void GeoRasterLayer::load_from_file(String file_path, bool write_access) {
    this->write_access = write_access;
    set_native_dataset(VectorExtractor::open_dataset(file_path.utf8().get_data(), write_access));

    // TODO: Might be better to produce a hard crash here, but CRASH_COND doesn't have the desired
    // effect - see https://github.com/godotengine/godot-cpp/issues/521
    if (!dataset->is_valid()) {
        UtilityFunctions::push_error("Could not load GeoRasterLayer from path '", file_path, "'!");
    }
}

void GeoRasterLayer::set_native_dataset(std::shared_ptr<NativeDataset> new_dataset) {
    dataset = new_dataset;
    extent_data = RasterTileExtractor::get_extent_data(new_dataset->dataset);
}

} // namespace godot