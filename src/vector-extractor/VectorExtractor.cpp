#include "VectorExtractor.h"
#include "LineFeature.h"
#include "NativeLayer.h"
#include "PointFeature.h"
#include "gdal-includes.h"

#include <algorithm>
#include <iostream>

void VectorExtractor::initialize() {
    GDALAllRegister();
}

NativeDataset *VectorExtractor::open_dataset(const char *path, bool write_access) {
    return new NativeDataset(path, write_access);
}

std::vector<double> VectorExtractor::transform_coordinates(double input_x, double input_z,
                                                           std::string from, std::string to) {
    OGRSpatialReference source_reference, target_reference;

    source_reference.importFromWkt(from.c_str());
    target_reference.importFromWkt(to.c_str());

    OGRCoordinateTransformation *transformation =
        OGRCreateCoordinateTransformation(&source_reference, &target_reference);

    double output_x = input_x;
    double output_z = input_z;

    transformation->Transform(1, &output_x, &output_z);

    return std::vector{output_x, output_z};
}

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

Feature *NativeLayer::create_feature() {
    OGRFeature *new_feature = new OGRFeature(layer->GetLayerDefn()); // TOOD: delete somewhere
    Feature *feature;                                                // TODO: delete somewhere

    // Create an instance of a specific class based on the layer's geometry type
    OGRwkbGeometryType geometry_type = layer->GetGeomType();

    if (geometry_type == OGRwkbGeometryType::wkbPoint) {
        new_feature->SetGeometry(new OGRPoint());
        feature = new PointFeature(new_feature);
    } else if (geometry_type == OGRwkbGeometryType::wkbPolygon) {
        new_feature->SetGeometry(new OGRPolygon());
        feature = new PolygonFeature(new_feature);
    } else if (geometry_type == OGRwkbGeometryType::wkbLineString) {
        new_feature->SetGeometry(new OGRLineString());
        feature = new LineFeature(new_feature);
    } else {
        // Either no geometry or unknown -- create a basic feature without geometry
        feature = new Feature(new_feature);
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

    feature_cache[id] = std::list<Feature *>{feature};

    return feature;
}

NativeDataset::NativeDataset(std::string path, bool write_access)
    : path(path), write_access(write_access) {
    unsigned int open_access = write_access ? GDAL_OF_UPDATE : GDAL_OF_READONLY;

    dataset = (GDALDataset *)GDALOpenEx(path.c_str(), open_access, nullptr, nullptr, nullptr);
}
