#include "VectorExtractor.h"
#include <gdal/ogrsf_frmts.h>
#include <gdal/gdal.h>
#include <iostream>

void VectorExtractor::initialize() {
    GDALAllRegister();
}

std::list<LineFeature *>
VectorExtractor::get_lines_near_position(const char *path, double pos_x, double pos_y, double radius, int max_amount) {
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
    OGRGeometry *circle = new OGRPoint(pos_x, pos_y);
    OGRGeometry *circle_buffer = circle->Buffer(radius);

    // Create the dataset in RAM
    GDALDriver *out_driver = (GDALDriver *) GDALGetDriverByName("Memory");
    GDALDataset *intersection_dataset = out_driver->Create("", 0, 0, 0, GDT_Unknown, nullptr);

    // Create the layer for that dataset
    OGRLayer *circle_layer = intersection_dataset->CreateLayer("IntersectionCircle");

    // Create the feature for that layer
    OGRFeature *circle_feature = OGRFeature::CreateFeature(circle_layer->GetLayerDefn());
    circle_feature->SetGeometry(circle_buffer);
    circle_layer->CreateFeature(circle_feature);

    // Finally do the actual intersection, save the result to a new layer in the previously created dataset
    OGRLayer *lines_within = intersection_dataset->CreateLayer("LinesWithinCircle");
    poLayer->Intersection(circle_layer, lines_within);

    // Put the resulting features into the returned list. We add as many features as were returned unless they're more
    //  than the given max_amount.
    int num_features = lines_within->GetFeatureCount();
    int iterations = std::min(num_features, max_amount);

    for (int i = 0; i < iterations; i++) {
        list.emplace_back(new LineFeature(lines_within->GetNextFeature()));
    }

    return list;
}