#include "VectorExtractor.h"

#ifdef _WIN32
    #include <ogrsf_frmts.h>
    #include <gdal.h>
#elif __unix__
    #include <gdal/ogrsf_frmts.h>
    #include <gdal/gdal.h>
#endif

#include <iostream>
#include <algorithm>

void VectorExtractor::initialize() {
    GDALAllRegister();
}

NativeDataset* VectorExtractor::open_dataset(const char *path) {
    return new NativeDataset(path);
}

NativeLayer* VectorExtractor::get_layer_from_dataset(GDALDataset *dataset, const char *name) {
    return new NativeLayer(dataset->GetLayerByName(name));
}


// Utilify function to get the appropriate VectorExtractor Feature (e.g. LineFeature) for the given OGRFeature.
// Return type is a list because it's possible that multiple features are retreived in the case of e.g. a MULTILINESTRING.
std::list<Feature *> get_specialized_features(OGRFeature *feature) {
    std::list<Feature *> list = std::list<Feature *>();

    const OGRGeometry *geometry_ref = feature->GetGeometryRef();

    // If this feature has no geometry, just return a list with that one Feature
    if (geometry_ref == nullptr) {
        list.emplace_back(new Feature(feature));
        return list;
    }

    std::string geometry_type_name = geometry_ref->getGeometryName();

    // Check which geometry this is and create an object of the corresponding type.
    // TODO: Find a neat design pattern for this, we'd want something like a dictionary to class types
    if (geometry_type_name == "POINT") {
        list.emplace_back(new PointFeature(feature));
    } else if (geometry_type_name == "LINESTRING") {
        list.emplace_back(new LineFeature(feature));
    } else if (geometry_type_name == "MULTILINESTRING") {
        // If this is a MultiFeature, we iterate over all the features in it and add those.
        // All the individual Features then share the same OGRFeature (with the same attributes etc).
        const OGRGeometryCollection *collection = feature->GetGeometryRef()->toGeometryCollection();

        for (const OGRGeometry *geometry : collection) {
            list.emplace_back(new LineFeature(feature, geometry));
        }
    } else if (geometry_type_name == "POLYGON") {
        list.emplace_back(new PolygonFeature(feature));
    }

    return list;
}


std::list<Feature *> VectorExtractor::get_features(OGRLayer *layer) {
    auto list = std::list<Feature *>();
    
    OGRFeature *current_feature = current_feature = layer->GetNextFeature();

    while (current_feature != nullptr) {
        // Add the Feature objects from the next OGRFeature in the layer to the list
        list.splice(list.end(), get_specialized_features(current_feature));

        current_feature = layer->GetNextFeature();
    }

    return list;
}

std::list<Feature *> VectorExtractor::get_features_near_position(OGRLayer *layer, double pos_x, double pos_y, double radius, int max_amount) {
    std::list<Feature *> list = std::list<Feature *>();

    // We want to extract the features within the circle constructed with the given position and radius from the vector layer.
    // For this circle, we have to create a new dataset + layer + feature + geometry because layers can only be
    //  intersected with other layers, and layers need a dataset.

    // Create the circle geometry
    OGRGeometry *circle = new OGRPoint(pos_x, pos_y);
    OGRGeometry *circle_buffer = circle->Buffer(radius);

    layer->SetSpatialFilter(circle_buffer);

    // Put the resulting features into the returned list. We add as many features as were returned unless they're more
    //  than the given max_amount.
    int num_features = layer->GetFeatureCount();
    int iterations = std::min(num_features, max_amount);

    for (int i = 0; i < iterations; i++) {
        // Add the Feature objects from the next OGRFeature in the layer to the list
        list.splice(list.end(), get_specialized_features(layer->GetNextFeature()));
    }

    return list;
}

std::list<LineFeature *>
VectorExtractor::crop_lines_to_square(const char *path, double top_left_x, double top_left_y, double size_meters,
                                      int max_amount) {
    auto list = std::list<LineFeature *>();

    // TODO: Remove this and pass a OGRLayer* instead of the path
    GDALDataset *poDS;

    poDS = (GDALDataset *) GDALOpenEx(path, GDAL_OF_VECTOR, nullptr,
                                      nullptr, nullptr);
    if (poDS == nullptr) {
        // The dataset couldn't be opened for some reason - likely it doesn't exist.
        // Return an empty list.
        // FIXME: We'd want to output this error to Godot, so we need to hand this information over somehow! Maybe an Exception?
        std::cerr << "No dataset was found at " << path << "!" << std::endl;
        return list;
    }

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


NativeDataset::~NativeDataset() {
    delete dataset;
}

NativeDataset *NativeDataset::get_subdataset(const char *name) {
    return new NativeDataset((path + ":" + std::string(name)).c_str());
}

NativeDataset::NativeDataset(const char *path) : path(path) {
    dataset = (GDALDataset *) GDALOpenEx(path, 0, nullptr, nullptr, nullptr);
}
