#include "VectorExtractor.h"
#include "LineFeature.h"
#include "PointFeature.h"

#ifdef _WIN32
#include <gdal.h>
#include <ogrsf_frmts.h>
#elif __APPLE__
#include <gdal.h>
#include <ogrsf_frmts.h>
#elif __unix__
#include <gdal/gdal.h>
#include <gdal/ogrsf_frmts.h>
#endif

#include <algorithm>
#include <iostream>

void VectorExtractor::initialize() {
    GDALAllRegister();
}

NativeDataset *VectorExtractor::open_dataset(const char *path) {
    return new NativeDataset(path);
}

NativeLayer *VectorExtractor::get_layer_from_dataset(GDALDataset *dataset, const char *name) {
    return new NativeLayer(dataset->GetLayerByName(name));
}

std::list<Feature *> NativeLayer::get_feature_for_fid(OGRFeature *feature) {
    if (feature_cache.count(feature->GetFID())) {
        // The feature is already cached, return that one
        return feature_cache[feature->GetFID()];
    }

    // The feature is not cached; create a new one and cache that.
    // To do that, check which specific class we need to instance:
    std::list<Feature *> list = std::list<Feature *>();

    const OGRGeometry *geometry_ref = feature->GetGeometryRef();

    // If this feature has no geometry, just return a list with that one Feature
    if (geometry_ref == nullptr) {
        list.emplace_back(new Feature(feature));
        return list;
    }

    std::string geometry_type_name = geometry_ref->getGeometryName();

    // Check which geometry this is and create an object of the corresponding type.
    // TODO: Find a neat design pattern for this, we'd want something like a dictionary to class
    // types
    if (geometry_type_name == "POINT") {
        list.emplace_back(new PointFeature(feature));
    } else if (geometry_type_name == "LINESTRING") {
        list.emplace_back(new LineFeature(feature));
    } else if (geometry_type_name == "MULTILINESTRING") {
        // If this is a MultiFeature, we iterate over all the features in it and add those.
        // All the individual Features then share the same OGRFeature (with the same attributes
        // etc).
        const OGRGeometryCollection *collection = feature->GetGeometryRef()->toGeometryCollection();

        for (const OGRGeometry *geometry : collection) {
            list.emplace_back(new LineFeature(feature, geometry));
        }
    } else if (geometry_type_name == "POLYGON") {
        list.emplace_back(new PolygonFeature(feature));
    } else if (geometry_type_name == "MULTIPOLYGON") {
        // If this is a MultiFeature, we iterate over all the features in it and add those.
        // All the individual Features then share the same OGRFeature (with the same attributes
        // etc).
        const OGRGeometryCollection *collection = feature->GetGeometryRef()->toGeometryCollection();

        for (const OGRGeometry *geometry : collection) {
            list.emplace_back(new PolygonFeature(feature, geometry));
        }
    }

    // Add to the cache and return
    feature_cache[feature->GetFID()] = list;

    return list;
}

std::list<Feature *> NativeLayer::get_features() {
    auto list = std::list<Feature *>();

    OGRFeature *current_feature = layer->GetNextFeature();

    while (current_feature != nullptr) {
        // Add the Feature objects from the next OGRFeature in the layer to the list
        list.splice(list.end(), get_feature_for_fid(current_feature));

        current_feature = layer->GetNextFeature();
    }

    // Same as above but for in-RAM data
    // TODO: Remove code duplication
    current_feature = ram_layer->GetNextFeature();

    while (current_feature != nullptr) {
        // Add the Feature objects from the next OGRFeature in the layer to the list
        list.splice(list.end(), get_feature_for_fid(current_feature));

        current_feature = ram_layer->GetNextFeature();
    }

    return list;
}

std::list<Feature *> NativeLayer::get_features_near_position(double pos_x, double pos_y,
                                                             double radius, int max_amount) {
    std::list<Feature *> list = std::list<Feature *>();

    // We want to extract the features within the circle constructed with the given position and
    // radius from the vector layer. For this circle, we have to create a new dataset + layer +
    // feature + geometry because layers can only be
    //  intersected with other layers, and layers need a dataset.

    // Create the circle geometry
    OGRGeometry *circle = new OGRPoint(pos_x, pos_y);
    OGRGeometry *circle_buffer = circle->Buffer(radius);

    layer->SetSpatialFilter(circle_buffer);

    // Put the resulting features into the returned list. We add as many features as were returned
    // unless they're more
    //  than the given max_amount.
    int num_features = layer->GetFeatureCount();
    int iterations = std::min(num_features, max_amount);

    for (int i = 0; i < iterations; i++) {
        // Add the Feature objects from the next OGRFeature in the layer to the list
        list.splice(list.end(), get_feature_for_fid(layer->GetNextFeature()));
    }

    // Also check the RAM layer
    // FIXME: Take care of max_amount here too
    // TODO: Code duplication (similar as in `get_features`)
    ram_layer->SetSpatialFilter(circle_buffer);

    for (int i = 0; i < ram_layer->GetFeatureCount(); i++) {
        // Add the Feature objects from the next OGRFeature in the layer to the list
        list.splice(list.end(), get_feature_for_fid(ram_layer->GetNextFeature()));
    }

    return list;
}

std::vector<std::string> NativeDataset::get_feature_layer_names() {
    std::vector<std::string> names;

    int layer_count = dataset->GetLayerCount();

    // Get each layer and emplace its name in the array
    for (int i = 0; i < layer_count; i++) {
        names.emplace_back(dataset->GetLayer(i)->GetName());
    }

    return names;
}

std::vector<std::string> NativeDataset::get_raster_layer_names() {
    std::vector<std::string> names;

    char **subdataset_metadata = dataset->GetMetadata("SUBDATASETS");

    // This metadata is formated like this:
    // SUBDATASET_1_NAME=GPKG:/path/to/geopackage.gpkg:dhm
    // SUBDATASET_1_DESC=dhm - dhm
    // SUBDATASET_2_NAME=GPKG:/path/to/geopackage.gpkg:ndom
    // SUBDATASET_2_DESC=ndom - ndom
    // We want every second line (since that has the name), and only the portion after the last ':'.

    if (subdataset_metadata != nullptr) {
        for (int i = 0; subdataset_metadata[i] != nullptr; i += 2) {
            std::string subdataset_name(subdataset_metadata[i]); // char* to std::string
            size_t last_colon = subdataset_name.find_last_of(
                ':'); // Find the last colon, which separates the path from the subdataset name
            subdataset_name =
                subdataset_name.substr(last_colon + 1); // Add 1 so the ':' isn't included

            names.emplace_back(subdataset_name);
        }
    }

    return names;
}

std::list<LineFeature *> NativeLayer::crop_lines_to_square(const char *path, double top_left_x,
                                                           double top_left_y, double size_meters,
                                                           int max_amount) {
    auto list = std::list<LineFeature *>();

    // TODO: Remove this and pass a OGRLayer* instead of the path
    GDALDataset *poDS;

    poDS = (GDALDataset *)GDALOpenEx(path, GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
    if (poDS == nullptr) {
        // The dataset couldn't be opened for some reason - likely it doesn't exist.
        // Return an empty list.
        // FIXME: We'd want to output this error to Godot, so we need to hand this information over
        // somehow! Maybe an Exception?
        std::cerr << "No dataset was found at " << path << "!" << std::endl;
        return list;
    }

    OGRLayer *poLayer = poDS->GetLayers()[0];

    // We want to extract the features within the circle constructed with the given position and
    // radius from the vector layer. For this circle, we have to create a new dataset + layer +
    // feature + geometry because layers can only be
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
    GDALDriver *out_driver = (GDALDriver *)GDALGetDriverByName("Memory");
    GDALDataset *intersection_dataset = out_driver->Create("", 0, 0, 0, GDT_Unknown, nullptr);

    // Create the layer for that dataset
    OGRLayer *square_layer = intersection_dataset->CreateLayer("IntersectionSquare");

    // Create the feature for that layer
    OGRFeature *square_feature = OGRFeature::CreateFeature(square_layer->GetLayerDefn());
    square_feature->SetGeometry(square);
    OGRErr error = square_layer->CreateFeature(square_feature);

    // This shouldn't happen since we're in a custom RAM dataset, but just to make sure
    if (error != OGRERR_NONE) { return list; }

    // Finally do the actual intersection, save the result to a new layer in the previously created
    // dataset
    OGRLayer *lines_within = intersection_dataset->CreateLayer("LinesWithinCircle");
    poLayer->Intersection(square_layer, lines_within);

    // Put the resulting features into the returned list. We add as many features as were returned
    // unless they're more
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
            // If this is a MultiLineString, we iterate over all the lines in the LineString and add
            // those. All the individual LineStrings (and thus LineFeatures) then share the same
            // Feature (with attributes etc).
            const OGRMultiLineString *linestrings = feature->GetGeometryRef()->toMultiLineString();

            for (const OGRLineString *linestring : linestrings) {
                list.emplace_back(new LineFeature(feature, linestring));
            }
        }
    }

    return list;
}

NativeDataset::~NativeDataset() {}

NativeDataset *NativeDataset::get_subdataset(const char *name) {
    // TODO: Hardcoded for the way GeoPackages work - do we want to support others too?
    return new NativeDataset(("GPKG:" + path + ":" + std::string(name)).c_str());
}

NativeDataset *NativeDataset::clone() {
    return new NativeDataset(path.c_str());
}

bool NativeDataset::is_valid() const {
    // No dataset at all?
    if (dataset == nullptr) { return false; }

    // No vector or raster layers?
    if (dataset->GetRasterCount() == 0 && dataset->GetLayerCount() == 0) { return false; }

    return true;
}

NativeLayer::NativeLayer(OGRLayer *layer) : layer(layer) {
    // We want users to be able to create and modify features, but we also don't necessarily want to
    // write those changes to disk. So we create a in-RAM layer with the same footprint as this
    // layer. It starts empty and is filled with new user-created features once they are created.
    GDALDriver *out_driver = (GDALDriver *)GDALGetDriverByName("Memory");
    GDALDataset *intersection_dataset = out_driver->Create("", 0, 0, 0, GDT_Unknown, nullptr);
    ram_layer = intersection_dataset->CreateLayer(layer->GetName(), layer->GetSpatialRef(),
                                                  layer->GetGeomType());
}

bool NativeLayer::is_valid() const {
    if (layer == nullptr) { return false; }

    return true;
}

Feature *NativeLayer::create_feature() {
    OGRFeature *new_feature = new OGRFeature(layer->GetLayerDefn()); // TOOD: delete somewhere
    Feature *feature;                                                // TODO: delete somewhere

    // Create an instance of a specific class based on the layer's geometry type
    OGRwkbGeometryType geometry_type = layer->GetGeomType();

    if (geometry_type == OGRwkbGeometryType::wkbPoint) {
        feature = new PointFeature(new_feature);
    } else if (geometry_type == OGRwkbGeometryType::wkbPolygon) {
        feature = new PolygonFeature(new_feature);
    } else if (geometry_type == OGRwkbGeometryType::wkbLineString) {
        feature = new LineFeature(new_feature);
    } else {
        // Either no geometry or unknown -- create a basic feature without geometry
        feature = new Feature(new_feature);
    }

    // FIXME: Generate a new ID based on the highest ID within the original data plus the highest
    // added ID
    GUIntBig id = 123;
    new_feature->SetFID(id);

    // Create the feature on the in-RAM layer. Calling layer->CreateFeature would attempt to write
    // to disk! Note that CreateFeature only copies the current data, it must be kept up-to-date
    // with SetFeature.
    const OGRErr error = ram_layer->CreateFeature(new_feature);
    // (No need to check that error, we're in a self-owned in-RAM dataset)

    feature_cache[id] = std::list<Feature *>{feature};

    return feature;
}

NativeDataset::NativeDataset(const char *path) : path(path) {
    dataset = (GDALDataset *)GDALOpenEx(path, 0, nullptr, nullptr, nullptr);
}
