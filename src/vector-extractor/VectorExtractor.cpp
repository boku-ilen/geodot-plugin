#include "VectorExtractor.h"

#ifdef _WIN32
    #include <ogrsf_frmts.h>
    #include <gdal.h>
#elif __unix__
    #include <gdal/ogrsf_frmts.h>
    #include <gdal/gdal.h>
#endif

#include <iostream>

void VectorExtractor::initialize() {
    GDALAllRegister();
}

template<class T>
std::list<T *>
get_features_near_position(const char *path, double pos_x, double pos_y, double radius, int max_amount,
                           const char *feature_name, const char *multi_feature_name) {
    auto list = std::list<T *>();

    GDALDataset *poDS;

    poDS = (GDALDataset *) GDALOpenEx(path, GDAL_OF_VECTOR, nullptr,
                                      nullptr, nullptr);
    if (poDS == nullptr) {
        printf("Open failed.\n");
        exit(1);
    }

    // TODO: Check poDS->GetLayerCount() to make sure there's exactly one layer? Or handle >1 layers too?
    OGRLayer *poLayer = poDS->GetLayers()[0];

    // We want to extract the features within the circle constructed with the given position and radius from the vector layer.
    // For this circle, we have to create a new dataset + layer + feature + geometry because layers can only be
    //  intersected with other layers, and layers need a dataset.

    // Create the circle geometry
    OGRGeometry *circle = new OGRPoint(pos_x, pos_y);
    OGRGeometry *circle_buffer = circle->Buffer(radius);

    poLayer->SetSpatialFilter(circle_buffer);

    // Put the resulting features into the returned list. We add as many features as were returned unless they're more
    //  than the given max_amount.
    int num_features = poLayer->GetFeatureCount();
    int iterations = std::min(num_features, max_amount);

    for (int i = 0; i < iterations; i++) {
        auto feature = poLayer->GetNextFeature();
        std::string geometry_type_name = feature->GetGeometryRef()->getGeometryName();

        if (geometry_type_name == feature_name) {
            // If this is a LineString, we can add it directly as a LineFeature.
            list.emplace_back(new T(feature));
        } else if (geometry_type_name == multi_feature_name) {
            // If this is a MultiLineString, we iterate over all the lines in the LineString and add those.
            // All the individual LineStrings (and thus LineFeatures) then share the same Feature (with attributes etc).
            const OGRGeometryCollection *collection = feature->GetGeometryRef()->toGeometryCollection();

            for (const OGRGeometry *geometry : collection) {
                list.emplace_back(new T(feature, geometry));
            }
        }
    }

    return list;
}

std::list<LineFeature *>
VectorExtractor::get_lines_near_position(const char *path, double pos_x, double pos_y, double radius, int max_amount) {
    return get_features_near_position<LineFeature>(path, pos_x, pos_y, radius, max_amount, "LINESTRING", "MULTILINESTRING");
}

std::list<LineFeature *>
VectorExtractor::crop_lines_to_square(const char *path, double top_left_x, double top_left_y, double size_meters,
                                      int max_amount) {
    auto list = std::list<LineFeature *>();

    GDALDataset *poDS;

    poDS = (GDALDataset *) GDALOpenEx(path, GDAL_OF_VECTOR, nullptr,
                                      nullptr, nullptr);
    if (poDS == nullptr) {
        printf("Open failed.\n");
        exit(1);
    }

    // TODO: Check poDS->GetLayerCount() to make sure there's exactly one layer? Or handle >1 layers too?
    OGRLayer *poLayer = poDS->GetLayers()[0];

    // We want to extract the features within the circle constructed with the given position and radius from the vector layer.
    // For this circle, we have to create a new dataset + layer + feature + geometry because layers can only be
    //  intersected with other layers, and layers need a dataset.

    // Create the circle geometry
    OGRLinearRing *square_outline = new OGRLinearRing();
    square_outline->addPoint(top_left_x, top_left_y);
    square_outline->addPoint(top_left_x + size_meters, top_left_y);
    square_outline->addPoint(top_left_x + size_meters, top_left_y - size_meters);
    square_outline->addPoint(top_left_x, top_left_y - size_meters);
    square_outline->addPoint(top_left_x, top_left_y);

    OGRPolygon *square = new OGRPolygon();
    square->addRing(square_outline);

    // Create the dataset in RAM
    GDALDriver *out_driver = (GDALDriver *) GDALGetDriverByName("Memory");
    GDALDataset *intersection_dataset = out_driver->Create("", 0, 0, 0, GDT_Unknown, nullptr);

    // Create the layer for that dataset
    OGRLayer *square_layer = intersection_dataset->CreateLayer("IntersectionSquare");

    // Create the feature for that layer
    OGRFeature *square_feature = OGRFeature::CreateFeature(square_layer->GetLayerDefn());
    square_feature->SetGeometry(square);
    square_layer->CreateFeature(square_feature);

    // Finally do the actual intersection, save the result to a new layer in the previously created dataset
    OGRLayer *lines_within = intersection_dataset->CreateLayer("LinesWithinCircle");
    poLayer->Intersection(square_layer, lines_within);

    // Put the resulting features into the returned list. We add as many features as were returned unless they're more
    //  than the given max_amount.
    int num_features = lines_within->GetFeatureCount();
    int iterations = std::min(num_features, max_amount);

    for (int i = 0; i < iterations; i++) {
        auto feature = lines_within->GetNextFeature();
        std::string geometry_type_name = feature->GetGeometryRef()->getGeometryName();

        if (geometry_type_name == "LINESTRING") {
            // If this is a LineString, we can add it directly as a LineFeature.
            list.emplace_back(new LineFeature(feature));
        } else if (geometry_type_name == "MULTILINESTRING") {
            // If this is a MultiLineString, we iterate over all the lines in the LineString and add those.
            // All the individual LineStrings (and thus LineFeatures) then share the same Feature (with attributes etc).
            const OGRMultiLineString *linestrings = feature->GetGeometryRef()->toMultiLineString();

            for (const OGRLineString *linestring : linestrings) {
                list.emplace_back(new LineFeature(feature, linestring));
            }
        }

    }

    return list;
}
