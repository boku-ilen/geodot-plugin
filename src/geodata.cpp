#include "geodata.h"
#include "geofeatures.h"
#include "raster-tile-extractor/RasterTileExtractor.h"
#include "vector-extractor/Feature.h"
#include "vector-extractor/VectorExtractor.h"

namespace godot {

GeoDataset::~GeoDataset() {
    delete dataset;
}

void GeoDataset::_register_methods() {
    register_method("is_valid", &GeoDataset::is_valid);
    register_method("get_raster_layers", &GeoDataset::get_raster_layers);
    register_method("get_feature_layers", &GeoDataset::get_feature_layers);
    register_method("get_raster_layer", &GeoDataset::get_raster_layer);
    register_method("get_feature_layer", &GeoDataset::get_feature_layer);
}

bool GeoDataset::is_valid() {
    return dataset && dataset->is_valid();
}

Array GeoDataset::get_raster_layers() {
    Array layers = Array();

    std::vector<std::string> names = dataset->get_raster_layer_names();

    for (std::string name : names) {
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
    raster_layer.instance();

    raster_layer->set_native_dataset(dataset->get_subdataset(name.utf8().get_data()));
    raster_layer->set_name(name);

    return raster_layer;
}

Ref<GeoFeatureLayer> GeoDataset::get_feature_layer(String name) {
    Ref<GeoFeatureLayer> feature_layer;
    feature_layer.instance();

    feature_layer->set_native_layer(
        VectorExtractor::get_layer_from_dataset(dataset->dataset, name.utf8().get_data()));
    feature_layer->set_name(name);

    return feature_layer;
}

void GeoDataset::load_from_file(String file_path) {
    dataset = VectorExtractor::open_dataset(file_path.utf8().get_data());
}

void GeoDataset::set_native_dataset(NativeDataset *new_dataset) {
    dataset = new_dataset;
}

void GeoFeatureLayer::_register_methods() {
    register_method("is_valid", &GeoFeatureLayer::is_valid);
    register_method("get_all_features", &GeoFeatureLayer::get_all_features);
    register_method("get_features_near_position", &GeoFeatureLayer::get_features_near_position);
    register_method("create_feature", &GeoFeatureLayer::create_feature);
    register_method("remove_feature", &GeoFeatureLayer::remove_feature);

    register_signal<GeoFeatureLayer>((char *)"feature_added", "new_feature",
                                     GODOT_VARIANT_TYPE_OBJECT);

    register_signal<GeoFeatureLayer>((char *)"feature_removed", "removed_feature",
                                     GODOT_VARIANT_TYPE_OBJECT);
}

bool GeoFeatureLayer::is_valid() {
    return layer && layer->is_valid();
}

Array GeoFeatureLayer::get_all_features() {
    Array geofeatures = Array();

    std::list<Feature *> gdal_features = layer->get_features();

    for (Feature *gdal_feature : gdal_features) {
        Ref<GeoFeature> geofeature;
        geofeature.instance();

        geofeature->set_gdal_feature(gdal_feature);

        geofeatures.push_back(geofeature);
    }

    return geofeatures;
}

// Utility function for converting a Processing Library Feature to the appropriate GeoFeature
Ref<GeoFeature> get_specialized_feature(Feature *raw_feature) {
    // Check which geometry this feature has, and cast it to the according
    // specialized class
    if (raw_feature->geometry_type == raw_feature->POINT) {
        Ref<GeoPoint> point;
        point.instance();

        PointFeature *point_feature = dynamic_cast<PointFeature *>(raw_feature);

        point->set_gdal_feature(point_feature);

        return point;
    } else if (raw_feature->geometry_type == raw_feature->LINE) {
        Ref<GeoLine> line;
        line.instance();

        LineFeature *line_feature = dynamic_cast<LineFeature *>(raw_feature);

        line->set_gdal_feature(line_feature);

        return line;
    } else if (raw_feature->geometry_type == raw_feature->POLYGON) {
        Ref<GeoPolygon> polygon;
        polygon.instance();

        PolygonFeature *polygon_feature = dynamic_cast<PolygonFeature *>(raw_feature);

        polygon->set_gdal_feature(polygon_feature);

        return polygon;
    } else {
        // Geometry type is NONE or unknown
        Ref<GeoFeature> feature;
        feature.instance();

        feature->set_gdal_feature(raw_feature);

        return feature;
    }
}

Ref<GeoFeature> GeoFeatureLayer::create_feature() {
    Feature *gdal_feature = layer->create_feature();

    Ref<GeoFeature> feature = get_specialized_feature(gdal_feature);

    emit_signal("feature_added", feature);
    return feature;
}

void GeoFeatureLayer::remove_feature(Ref<GeoFeature> feature) {
    // Mark the feature for deletion

    // TODO: Implement

    emit_signal("feature_removed", feature);
}

Array GeoFeatureLayer::get_features_near_position(double pos_x, double pos_y, double radius,
                                                  int max_features) {
    Array features = Array();

    std::list<Feature *> raw_features =
        layer->get_features_near_position(pos_x, pos_y, radius, max_features);

    for (Feature *raw_feature : raw_features) {
        features.push_back(get_specialized_feature(raw_feature));
    }

    return features;
}

Array GeoFeatureLayer::crop_lines_to_square(double top_left_x, double top_left_y,
                                            double size_meters, int max_lines) {
    // TODO
    return Array();
}

void GeoFeatureLayer::set_native_layer(NativeLayer *new_layer) {
    layer = new_layer;
}

GeoRasterLayer::~GeoRasterLayer() {
    delete dataset;
}

void GeoRasterLayer::_register_methods() {
    register_method("is_valid", &GeoRasterLayer::is_valid);
    register_method("get_image", &GeoRasterLayer::get_image);
    register_method("get_value_at_position", &GeoRasterLayer::get_value_at_position);
    register_method("get_extent", &GeoRasterLayer::get_extent);
    register_method("get_center", &GeoRasterLayer::get_center);
    register_method("get_min", &GeoRasterLayer::get_min);
    register_method("get_max", &GeoRasterLayer::get_max);
    register_method("clone", &GeoRasterLayer::clone);
}

bool GeoRasterLayer::is_valid() {
    return dataset && dataset->is_valid();
}

Ref<GeoImage> GeoRasterLayer::get_image(double top_left_x, double top_left_y, double size_meters,
                                        int img_size, int interpolation_type) {
    Ref<GeoImage> image;
    image.instance();

    GeoRaster *raster = RasterTileExtractor::get_tile_from_dataset(
        dataset->dataset, top_left_x, top_left_y, size_meters, img_size, interpolation_type);

    if (raster == nullptr) {
        // TODO: Set validity to false
        Godot::print_error("No valid data was available for the requested path and position!",
                           "Geodot::get_image", "geodot.cpp", 26);
        return image;
    }

    image->set_raster(raster, interpolation_type);

    return image;
}

float GeoRasterLayer::get_value_at_position(double pos_x, double pos_y) {
    // Get the GeoRaster for this position with a resolution of 1x1px.
    // 0.0001 meters are used for the size because it can't be 0, but should be a pinpoint value.
    GeoRaster *raster =
        RasterTileExtractor::get_tile_from_dataset(dataset->dataset, pos_x, pos_y, 0.0001, 1, 1);

    // TODO: Currently only implemented for RF type.
    // For others, we would either need a completely generic return value, or other specific
    // functions (as the user likely knows or wants to know the exact type).
    if (raster->get_format() == GeoRaster::FORMAT::RF) {
        float *array = (float *)raster->get_as_array();

        return array[0];
    }

    return -1.0;
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

bool PyramidGeoRasterLayer::is_valid() {
    // TODO
    return true;
}

Ref<GeoRasterLayer> GeoRasterLayer::clone() {
    Ref<GeoRasterLayer> layer_clone;
    layer_clone.instance();

    layer_clone->set_native_dataset(dataset->clone());

    return layer_clone;
}

Ref<GeoImage> PyramidGeoRasterLayer::get_image(double top_left_x, double top_left_y,
                                               double size_meters, int img_size,
                                               int interpolation_type) {
    Ref<GeoImage> image = Ref<GeoImage>(GeoImage::_new());

    GeoRaster *raster = RasterTileExtractor::get_raster_from_pyramid(
        path.utf8().get_data(), ending.utf8().get_data(), top_left_x, top_left_y, size_meters,
        img_size, interpolation_type);

    if (raster == nullptr) {
        // TODO: Set validity to false
        Godot::print_error("No valid data was available for the requested path and position!",
                           "Geodot::get_image", "geodot.cpp", 26);
        return image;
    }

    image->set_raster(raster, interpolation_type);

    return image;
}

void GeoRasterLayer::load_from_file(String file_path) {
    dataset = VectorExtractor::open_dataset(file_path.utf8().get_data());
}

void GeoRasterLayer::set_native_dataset(NativeDataset *new_dataset) {
    dataset = new_dataset;
    extent_data = RasterTileExtractor::get_extent_data(new_dataset->dataset);
}

void PyramidGeoRasterLayer::_register_methods() {
    // The function pointers need to point to the overwritten functions
    register_method("is_valid", &PyramidGeoRasterLayer::is_valid);
    register_method("get_image", &PyramidGeoRasterLayer::get_image);
}

void PyramidGeoRasterLayer::set_pyramid_base(String path) {
    this->path = path;
}

void PyramidGeoRasterLayer::set_file_ending(String ending) {
    this->ending = ending;
}

} // namespace godot