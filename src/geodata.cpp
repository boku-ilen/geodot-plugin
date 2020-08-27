#include "geodata.h"
#include "vector-extractor/Feature.h"
#include "geofeatures.h"
#include "vector-extractor/VectorExtractor.h"
#include "raster-tile-extractor/RasterTileExtractor.h"

#ifdef _WIN32
    #include <gdal_priv.h>
#elif __unix__
    #include <gdal/gdal_priv.h>
#endif


namespace godot {

GeoDataset::~GeoDataset() {
    delete dataset;
}

void GeoDataset::_init() {
    // This is required - returning a Reference to a locally created object throws a segfault otherwise!
    init_ref();
}

void GeoDataset::_register_methods() {
    register_method("is_valid", &GeoDataset::is_valid);
    register_method("get_raster_layer", &GeoDataset::get_raster_layer);
    register_method("get_feature_layer", &GeoDataset::get_feature_layer);
}

bool GeoDataset::is_valid() {
    // TODO
    return true;
}

GeoRasterLayer* GeoDataset::get_raster_layer(String name) {
    GeoRasterLayer *raster_layer = GeoRasterLayer::_new();

    // In GDAL, raster data is actually stored in datasets, not in layers.
    // FIXME: Because this is the assumption in GDAL, we currently only support datasets with exactly
    //  one "layer" of raster data. In the future, we will want to support datasaets (geopackages) with
    //  multiple layers of raster data.
    //  We will need to figure out how this works in GDAL, and whether we also want to support custom
    //  types, such as a path to a pre-tiled raster pyramid (which could be wrapped by a custom object).
    raster_layer->set_gdaldataset(dataset);

    return raster_layer;
}

GeoFeatureLayer* GeoDataset::get_feature_layer(String name) {
    GeoFeatureLayer *feature_layer = GeoFeatureLayer::_new();

    feature_layer->set_ogrlayer(VectorExtractor::get_layer_from_dataset(dataset, name.utf8().get_data()));

    return feature_layer;
}

void GeoDataset::load_from_file(String file_path) {
    dataset = VectorExtractor::open_dataset(file_path.utf8().get_data());
}

void GeoFeatureLayer::_init() {
    // This is required - returning a Reference to a locally created object throws a segfault otherwise!
    init_ref();
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

    std::list<Feature *> gdal_features = VectorExtractor::get_features(layer);

    for (Feature *gdal_feature : gdal_features) {
        Ref<GeoFeature> geofeature = GeoFeature::_new();
        geofeature->set_gdal_feature(gdal_feature);

        geofeatures.push_back(geofeature);
    }

    return geofeatures;
}

Array GeoFeatureLayer::get_features_near_position(double pos_x, double pos_y, double radius, int max_features) {
    Array features = Array();

    std::list<Feature *> raw_features = VectorExtractor::get_features_near_position(layer, pos_x, pos_y, radius, max_features);

    for (Feature *raw_feature : raw_features) {
        // Check which geometry this feature has, and cast it to the according specialized class
        if (raw_feature->geometry_type == raw_feature->NONE) {
            Ref<GeoFeature> feature = GeoFeature::_new();
            feature->set_gdal_feature(raw_feature);

            features.push_back(feature);
        } else if (raw_feature->geometry_type == raw_feature->POINT) {
            Ref<GeoPoint> point = GeoPoint::_new();
            PointFeature *point_feature = dynamic_cast<PointFeature *> (raw_feature);

            point->set_gdal_feature(point_feature);

            features.push_back(point);
        } else if (raw_feature->geometry_type == raw_feature->LINE) {
            Ref<GeoLine> line = GeoLine::_new();
            LineFeature *line_feature = dynamic_cast<LineFeature *> (raw_feature);

            line->set_gdal_feature(line_feature);

            features.push_back(line);
        } else if (raw_feature->geometry_type == raw_feature->POLYGON) {
            Ref<GeoPolygon> polygon = GeoPolygon::_new();
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

void GeoFeatureLayer::set_ogrlayer(OGRLayer *new_layer) {
    layer = new_layer;
}

void GeoRasterLayer::_init() {
    // This is required - returning a Reference to a locally created object throws a segfault otherwise!
    init_ref();
}

void GeoRasterLayer::_register_methods() {
    register_method("is_valid", &GeoRasterLayer::is_valid);
    register_method("get_image", &GeoRasterLayer::get_image);
}

bool GeoRasterLayer::is_valid() {
    // TODO
    return true;
}

Ref<GeoImage> GeoRasterLayer::get_image(double top_left_x, double top_left_y, double size_meters,
                            int img_size, int interpolation_type) {
    // This strange __internal_constructor call is required to prevent a memory leak
    // See https://github.com/GodotNativeTools/godot-cpp/issues/215
    // TODO: Should be fixed according to that issue!
    // FIXME: Check whether really fixed!
    Ref<GeoImage> image = GeoImage::_new();

    GeoRaster *raster = RasterTileExtractor::get_tile_from_dataset(
        dataset, top_left_x, top_left_y, size_meters, img_size, interpolation_type);

    if (raster == nullptr) {
        // TODO: Set validity to false
        Godot::print_error("No valid data was available for the requested path and position!", "Geodot::get_image", "geodot.cpp", 26);
        return image;
    }

    image->set_raster(raster, interpolation_type);

    return image;
}

}