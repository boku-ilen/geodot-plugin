#include "NativeLayer.h"
#include "NativeDataset.h"
#include "PointFeature.h"
#include "PolygonFeature.h"

NativeLayer::NativeLayer(OGRLayer *layer) : layer(layer) {
    // We want users to be able to create and modify features, but we also don't necessarily want to
    // write those changes to disk. So we create a in-RAM layer with the same footprint as this
    // layer. It starts empty and is filled with new user-created features once they are created.
    GDALDriver *out_driver = (GDALDriver *)GDALGetDriverByName("Memory");
    GDALDataset *intersection_dataset = out_driver->Create("", 0, 0, 0, GDT_Unknown, nullptr);
    ram_layer = intersection_dataset->CreateLayer(layer->GetName(), layer->GetSpatialRef(),
                                                  layer->GetGeomType());

    disk_feature_count = layer->GetFeatureCount();
    ram_feature_count = 0;
}

void NativeLayer::save_override() {
    // Write cached features to RAM layer
    for (auto feature_list : feature_cache) {
        if (!feature_list.second.front()->is_deleted) {
            OGRErr error = ram_layer->SetFeature(feature_list.second.front()->feature);
        }
    }

    // Write changes from RAM layer into this layer
    ram_layer->ResetReading();            // Reset the reading cursor
    ram_layer->SetSpatialFilter(nullptr); // Reset the spatial filter
    OGRFeature *current_feature = ram_layer->GetNextFeature();

    while (current_feature != nullptr) {
        if (current_feature->GetFID() <= layer->GetFeatureCount()) {
            OGRErr error = layer->SetFeature(current_feature);
        } else {
            OGRErr error = layer->CreateFeature(current_feature);
        }

        current_feature = ram_layer->GetNextFeature();
    }

    layer->SyncToDisk();
}

void NativeLayer::save_modified_layer(std::string path) {
    GDALDriver *out_driver = (GDALDriver *)GDALGetDriverByName("ESRI Shapefile");
    GDALDataset *out_dataset = out_driver->Create(path.c_str(), 0, 0, 0, GDT_Unknown, nullptr);
    OGRLayer *out_layer = out_dataset->CopyLayer(layer, layer->GetName());

    // Write cached features to RAM layer
    for (auto feature_list : feature_cache) {
        if (!feature_list.second.front()->is_deleted) {
            OGRErr error = ram_layer->SetFeature(feature_list.second.front()->feature);
        }
    }

    // Write changes from RAM layer into the layer copied from the original
    ram_layer->ResetReading();            // Reset the reading cursor
    ram_layer->SetSpatialFilter(nullptr); // Reset the spatial filter
    OGRFeature *current_feature = ram_layer->GetNextFeature();

    while (current_feature != nullptr) {
        if (current_feature->GetFID() <= out_layer->GetFeatureCount()) {
            OGRErr error = out_layer->SetFeature(current_feature);
        } else {
            OGRErr error = out_layer->CreateFeature(current_feature);
        }

        current_feature = ram_layer->GetNextFeature();
    }

    out_layer->SyncToDisk();
    delete out_dataset;
}

bool NativeLayer::is_valid() const {
    if (layer == nullptr) { return false; }

    return true;
}

std::shared_ptr<Feature> NativeLayer::create_feature() {
    OGRFeature *new_feature = new OGRFeature(layer->GetLayerDefn());
    std::shared_ptr<Feature> feature;

    // Create an instance of a specific class based on the layer's geometry type
    OGRwkbGeometryType geometry_type = layer->GetGeomType();

    // SetGeometryDirectly takes ownership of the passed geometry, so no need to delete it
    if (geometry_type == OGRwkbGeometryType::wkbPoint) {
        new_feature->SetGeometryDirectly(new OGRPoint());
        feature = std::make_shared<PointFeature>(new_feature);
    } else if (geometry_type == OGRwkbGeometryType::wkbPolygon) {
        new_feature->SetGeometryDirectly(new OGRPolygon());
        feature = std::make_shared<PolygonFeature>(new_feature);
    } else if (geometry_type == OGRwkbGeometryType::wkbLineString) {
        new_feature->SetGeometryDirectly(new OGRLineString());
        feature = std::make_shared<LineFeature>(new_feature);
    } else {
        // Either no geometry or unknown -- create a basic feature without geometry
        feature = std::make_shared<Feature>(new_feature);
    }

    // Generate a new ID based on the highest ID within the original data plus the highest added ID
    GUIntBig id = disk_feature_count + ram_feature_count;

    // Ensure that this ID is unused - can be relevant if there are gaps in the original data
    while (layer->GetFeature(id) != nullptr) {
        disk_feature_count++;
        id++;
    }

    new_feature->SetFID(id);
    ram_feature_count++;

    // Create the feature on the in-RAM layer. Calling layer->CreateFeature would attempt to write
    // to disk! Note that CreateFeature only copies the current data, it must be kept up-to-date
    // with SetFeature.
    const OGRErr error = ram_layer->CreateFeature(new_feature);
    // (No need to check that error, we're in a self-owned in-RAM dataset)

    feature_cache[id] = std::list<std::shared_ptr<Feature> >{feature};

    return feature;
}

std::list<std::shared_ptr<Feature> > NativeLayer::get_feature_for_ogrfeature(OGRFeature *feature) {
    std::list<std::shared_ptr<Feature> > list = std::list<std::shared_ptr<Feature> >();

    // FIXME: This should never happen - would be better to throw an error towards Godot here
    if (feature == nullptr) { return list; }

    if (feature_cache.count(feature->GetFID())) {
        // The feature is already cached, return that
        std::list<std::shared_ptr<Feature> > cached_feature = feature_cache[feature->GetFID()];

        // Remove deleted features from the list
        // FIXME: This also requires a `delete` on the Feature object!
        cached_feature.remove_if([](const std::shared_ptr<Feature> feature) { return feature->is_deleted; });

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
            list.emplace_back(new LineFeature(feature->Clone(), geometry));
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

std::list<std::shared_ptr<Feature> > NativeLayer::get_feature_by_id(int id) {
    OGRFeature *feature = layer->GetFeature(id);

    return get_feature_for_ogrfeature(feature);
}

std::list<std::shared_ptr<Feature> > NativeLayer::get_features() {
    auto list = std::list<std::shared_ptr<Feature> >();

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

std::list<std::shared_ptr<Feature> > NativeLayer::get_features_inside_geometry(OGRGeometry *geometry, int max_amount) {
    std::list<std::shared_ptr<Feature> > list = std::list<std::shared_ptr<Feature> >();

    // We want to extract the features within the given geometry.
    // For this geometry, we have to create a new dataset + layer + feature + geometry because layers can only be
    // intersected with other layers, and layers need a dataset.

    layer->SetSpatialFilter(geometry);
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

    return list;
}

std::list<std::shared_ptr<Feature> > NativeLayer::get_features_near_position(double pos_x, double pos_y,
                                                             double radius, int max_amount) {
    // Create the circle geometry
    OGRGeometry *circle = new OGRPoint(pos_x, pos_y);
    OGRGeometry *circle_buffer = circle->Buffer(radius);

    std::list<std::shared_ptr<Feature> > features = get_features_inside_geometry(circle_buffer, max_amount);

    delete circle;
    delete circle_buffer;

    return features;
}

std::list<std::shared_ptr<Feature>> NativeLayer::get_features_in_square(double top_left_x,
                                                           double top_left_y, double size_meters,
                                                           int max_amount) {
    // Create the circle geometry
    OGRLinearRing *square_outline = new OGRLinearRing();
    square_outline->addPoint(top_left_x, top_left_y);
    square_outline->addPoint(top_left_x + size_meters, top_left_y);
    square_outline->addPoint(top_left_x + size_meters, top_left_y - size_meters);
    square_outline->addPoint(top_left_x, top_left_y - size_meters);
    square_outline->addPoint(top_left_x, top_left_y);

    OGRPolygon *square = new OGRPolygon();
    square->addRing(square_outline);

    std::list<std::shared_ptr<Feature> > features = get_features_inside_geometry(square, max_amount);

    delete square_outline;
    delete square;

    return features;
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