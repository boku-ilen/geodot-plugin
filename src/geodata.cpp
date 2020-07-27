#include "geodata.h"
#include "vector-extractor/Feature.h"
#include "geofeatures.h"
#include "vector-extractor/VectorExtractor.h"

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
    // TODO
    return GeoRasterLayer::_new();
}

GeoFeatureLayer* GeoDataset::get_feature_layer(String name) {
    // TODO
    return GeoFeatureLayer::_new();
}

void GeoDataset::load_from_file(String file_path) {
    // TODO
}

void GeoFeatureLayer::_init() {
    // This is required - returning a Reference to a locally created object throws a segfault otherwise!
    init_ref();
}

void GeoFeatureLayer::_register_methods() {
    register_method("is_valid", &GeoFeatureLayer::is_valid);
    register_method("get_all_features", &GeoFeatureLayer::get_all_features);
    register_method("get_features_near_position", &GeoFeatureLayer::get_all_features);
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
        // Depending on the type of the raw_feature, we need a different specialization of GeoFeature.
        // FIXME: This works, but the manual class name comparison feels ugly and fragile... Is there a nicer pattern for this?
        if (typeid(raw_feature) == typeid(PointFeature)) {
            Ref<GeoPoint> point = GeoPoint::_new();
            point->set_gdal_feature(raw_feature);

            features.push_back(point);
        } else if (typeid(raw_feature) == typeid(LineFeature)) {
            Ref<GeoLine> line = GeoLine::_new();
            line->set_gdal_feature(raw_feature);

            features.push_back(line);
        } else {
            Ref<GeoFeature> feature = GeoFeature::_new();
            feature->set_gdal_feature(raw_feature);

            features.push_back(feature);
        }
    }

    return features;
}

Array GeoFeatureLayer::crop_lines_to_square(double top_left_x, double top_left_y, double size_meters, int max_lines) {
    // TODO
    return Array();
}

GeoRasterLayer::~GeoRasterLayer() {
    delete dataset;
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
    // TODO
    return GeoImage::_new();
}

}