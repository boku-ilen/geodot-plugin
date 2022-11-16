#include "NativeDataset.h"
#include "NativeLayer.h"

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

NativeLayer *NativeDataset::get_layer(const char *name) const {
    return new NativeLayer(dataset->GetLayerByName(name));
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
        // FIXME: Check for this problem in the outer layer and emit a Godot error
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
            OGRMultiLineString *linestrings = feature->GetGeometryRef()->toMultiLineString();

            for (OGRLineString *linestring : linestrings) {
                list.emplace_back(new LineFeature(feature, linestring));
            }
        }
    }

    return list;
}

NativeDataset *NativeDataset::get_subdataset(const char *name) const {
    // TODO: Hardcoded for the way GeoPackages work - do we want to support others too?
    return new NativeDataset(("GPKG:" + path + std::string(":") + std::string(name)).c_str(),
                             write_access);
}

NativeDataset *NativeDataset::clone() {
    return new NativeDataset(path, write_access);
}

bool NativeDataset::is_valid() const {
    // No dataset at all?
    if (dataset == nullptr) { return false; }

    return true;
}
