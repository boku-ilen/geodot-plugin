#include "geodata.h"
#include "vector-extractor/Feature.h"
#include "geofeatures.h"
#include "vector-extractor/VectorExtractor.h"
#include "raster-tile-extractor/RasterTileExtractor.h"


namespace godot {

GeoDataset::~GeoDataset() {
    delete dataset;
}

void GeoDataset::_register_methods() {
    register_method("is_valid", &GeoDataset::is_valid);
    register_method("get_raster_layer", &GeoDataset::get_raster_layer);
    register_method("get_feature_layer", &GeoDataset::get_feature_layer);
}

bool GeoDataset::is_valid() {
    if (dataset->dataset == nullptr) {
        return false;
    }

    // TODO: More sophisticated check, e.g. get raster size

    return true;
}

Ref<GeoRasterLayer> GeoDataset::get_raster_layer(String name) {
    Ref<GeoRasterLayer> raster_layer;
    raster_layer.instance();

    raster_layer->set_native_dataset(dataset->get_subdataset(name.utf8().get_data()));

    return raster_layer;
}

Ref<GeoFeatureLayer> GeoDataset::get_feature_layer(String name) {
    Ref<GeoFeatureLayer> feature_layer;
    feature_layer.instance();

    feature_layer->set_native_layer(VectorExtractor::get_layer_from_dataset(dataset->dataset, name.utf8().get_data()));

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
}

bool GeoFeatureLayer::is_valid() {
    // TODO
    return true;
}

Array GeoFeatureLayer::get_all_features() {
    Array geofeatures = Array();

    std::list<Feature *> gdal_features = VectorExtractor::get_features(layer->layer);

    for (Feature *gdal_feature : gdal_features) {
        Ref<GeoFeature> geofeature;
        geofeature.instance();

        geofeature->set_gdal_feature(gdal_feature);

        geofeatures.push_back(geofeature);
    }

    return geofeatures;
}

Array GeoFeatureLayer::get_features_near_position(double pos_x, double pos_y, double radius, int max_features) {
    Array features = Array();

    std::list<Feature *> raw_features = VectorExtractor::get_features_near_position(layer->layer, pos_x, pos_y, radius, max_features);

    for (Feature *raw_feature : raw_features) {
        // Check which geometry this feature has, and cast it to the according specialized class
        if (raw_feature->geometry_type == raw_feature->NONE) {
            Ref<GeoFeature> feature;
            feature.instance();

            feature->set_gdal_feature(raw_feature);

            features.push_back(feature);
        } else if (raw_feature->geometry_type == raw_feature->POINT) {
            Ref<GeoPoint> point;
            point.instance();

            PointFeature *point_feature = dynamic_cast<PointFeature *> (raw_feature);

            point->set_gdal_feature(point_feature);

            features.push_back(point);
        } else if (raw_feature->geometry_type == raw_feature->LINE) {
            Ref<GeoLine> line;
            line.instance();

            LineFeature *line_feature = dynamic_cast<LineFeature *> (raw_feature);

            line->set_gdal_feature(line_feature);

            features.push_back(line);
        } else if (raw_feature->geometry_type == raw_feature->POLYGON) {
            Ref<GeoPolygon> polygon;
            polygon.instance();

            PolygonFeature *polygon_feature = dynamic_cast<PolygonFeature *> (raw_feature);

            polygon->set_gdal_feature(polygon_feature);

            features.push_back(polygon);
        }
    }

    return features;
}

Array GeoFeatureLayer::crop_lines_to_square(double top_left_x, double top_left_y, double size_meters, int max_lines) {
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
    register_method("clone", &GeoRasterLayer::clone);
}

bool GeoRasterLayer::is_valid() {
    // TODO
    return true;
}

Ref<GeoImage> GeoRasterLayer::get_image(double top_left_x, double top_left_y, double size_meters,
                            int img_size, int interpolation_type) {
    Ref<GeoImage> image;
    image.instance();

    GeoRaster *raster = RasterTileExtractor::get_tile_from_dataset(
        dataset->dataset, top_left_x, top_left_y, size_meters, img_size, interpolation_type);

    if (raster == nullptr) {
        // TODO: Set validity to false
        Godot::print_error("No valid data was available for the requested path and position!", "Geodot::get_image", "geodot.cpp", 26);
        return image;
    }

    image->set_raster(raster, interpolation_type);

    return image;
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

Ref<GeoImage> PyramidGeoRasterLayer::get_image(double top_left_x, double top_left_y, double size_meters,
                            int img_size, int interpolation_type) {
    Ref<GeoImage> image = Ref<GeoImage>(GeoImage::_new());

    GeoRaster *raster = RasterTileExtractor::get_raster_from_pyramid(
        path.utf8().get_data(), ending.utf8().get_data(), top_left_x, top_left_y, size_meters, img_size, interpolation_type);

    if (raster == nullptr) {
        // TODO: Set validity to false
        Godot::print_error("No valid data was available for the requested path and position!", "Geodot::get_image", "geodot.cpp", 26);
        return image;
    }

    image->set_raster(raster, interpolation_type);

    return image;
}

void GeoRasterLayer::set_native_dataset(NativeDataset *new_dataset) {
    dataset = new_dataset;
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

}