#include "NativeLayer.h"
#include "NativeDataset.h"
#include "PointFeature.h"
#include "PolygonFeature.h"

std::list<Feature *> NativeLayer::get_feature_for_ogrfeature(OGRFeature *feature) {
    std::list<Feature *> list = std::list<Feature *>();

    // FIXME: This should never happen - would be better to throw an error towards Godot here
    if (feature == nullptr) { return list; }

    if (feature_cache.count(feature->GetFID())) {
        // The feature is already cached, return that
        std::list<Feature *> cached_feature = feature_cache[feature->GetFID()];

        // Remove deleted features from the list
        // FIXME: This also requires a `delete` on the Feature object!
        cached_feature.remove_if([](const Feature *feature) { return feature->is_deleted; });

        // Since there already is an OGRFeature object inside of the cached Feature, we don't need this one anymore
        OGRFeature::DestroyFeature(feature);

        return cached_feature;
    }

    // The feature is not cached; create a new one and cache that.
    // To do that, check which specific class we need to instance:

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
        OGRGeometryCollection *collection = feature->GetGeometryRef()->toGeometryCollection();

        for (OGRGeometry *geometry : collection) {
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

ExtentData NativeLayer::get_extent() {
    OGREnvelope *envelope = new OGREnvelope();
    OGRErr error = layer->GetExtent(envelope);

    ExtentData extent(envelope->MinX, envelope->MaxX, envelope->MinY, envelope->MaxY);

    delete envelope;
    return extent;
}

std::list<Feature *> NativeLayer::get_feature_by_id(int id) {
    OGRFeature *feature = layer->GetFeature(id);

    return get_feature_for_ogrfeature(feature);
}

std::list<Feature *> NativeLayer::get_features() {
    auto list = std::list<Feature *>();

    layer->ResetReading();            // Reset the reading cursor
    layer->SetSpatialFilter(nullptr); // Reset the spatial filter
    OGRFeature *current_feature = layer->GetNextFeature();

    while (current_feature != nullptr) {
        // Add the Feature objects from the next OGRFeature in the layer to the list
        list.splice(list.end(), get_feature_for_ogrfeature(current_feature));

        current_feature = layer->GetNextFeature();
    }

    // Same as above but for in-RAM data
    // TODO: Remove code duplication
    ram_layer->ResetReading();            // Reset the reading cursor
    ram_layer->SetSpatialFilter(nullptr); // Reset the spatial filter
    current_feature = ram_layer->GetNextFeature();

    while (current_feature != nullptr) {
        // Add the Feature objects from the next OGRFeature in the layer to the list
        list.splice(list.end(), get_feature_for_ogrfeature(current_feature));

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
    layer->ResetReading();

    // Put the resulting features into the returned list. We add as many features as were returned
    // unless they're more
    //  than the given max_amount.
    int num_features = layer->GetFeatureCount();
    int iterations = std::min(num_features, max_amount);

    for (int i = 0; i < iterations; i++) {
        // Add the Feature objects from the next OGRFeature in the layer to the list
        list.splice(list.end(), get_feature_for_ogrfeature(layer->GetNextFeature()));
    }

    // Also check the RAM layer
    // FIXME: Take care of max_amount here too
    // TODO: Code duplication (similar as in `get_features`)
    ram_layer->SetSpatialFilter(nullptr);
    ram_layer->ResetReading();

    // FIXME: In this case, we need to copy the cached features into the RAM dataset with
    // layer->CreateFeature because otherwise, the spatial filter doesn't return them! So we should
    // iterate over the values in cached_features and call CreateFeature with them, and perhaps
    // optimize it by checking if there was a change first?

    for (int i = 0; i < ram_layer->GetFeatureCount(); i++) {
        // Add the Feature objects from the next OGRFeature in the layer to the list
        list.splice(list.end(), get_feature_for_ogrfeature(ram_layer->GetNextFeature()));
    }

    delete circle;
    delete circle_buffer;

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

void NativeLayer::add_field(std::string name) {
    OGRFieldDefn *field_definition = new OGRFieldDefn(name.c_str(), OGRFieldType::OFTString);

    layer->CreateField(field_definition);

    delete field_definition;
}

void NativeLayer::remove_field(std::string name) {
    layer->DeleteField(layer->GetLayerDefn()->GetFieldIndex(name.c_str()));
}