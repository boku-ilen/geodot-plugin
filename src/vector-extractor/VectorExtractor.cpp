#include "VectorExtractor.h"
#include <gdal/ogrsf_frmts.h>
#include <gdal/gdal.h>
#include <iostream>

LineFeature VectorExtractor::get_first_feature() {
    GDALAllRegister();

    GDALDataset *poDS;

    poDS = (GDALDataset *) GDALOpenEx("/home/karl/BOKU/geodata/streets/2_linknetz_ogd_WM.shp", GDAL_OF_VECTOR, nullptr,
                                      nullptr, nullptr);
    if (poDS == nullptr) {
        printf("Open failed.\n");
        exit(1);
    }

    // TODO: Check poDS->GetLayerCount() to make sure there's exactly one layer? Or handle >1 layers too?
    OGRLayer *poLayer = poDS->GetLayers()[0];

    return LineFeature(poLayer->GetNextFeature());
}

std::list<LineFeature *>
VectorExtractor::get_lines_near_position(const char *path, double pos_x, double pos_y, double radius, int max_amount) {
    auto list = std::list<LineFeature *>();

    GDALAllRegister();

    GDALDataset *poDS;

    poDS = (GDALDataset *) GDALOpenEx(path, GDAL_OF_VECTOR, nullptr,
                                      nullptr, nullptr);
    if (poDS == nullptr) {
        printf("Open failed.\n");
        exit(1);
    }

    // TODO: Check poDS->GetLayerCount() to make sure there's exactly one layer? Or handle >1 layers too?
    OGRLayer *poLayer = poDS->GetLayers()[0];

    list.emplace_back(new LineFeature(poLayer->GetNextFeature()));
    list.emplace_back(new LineFeature(poLayer->GetNextFeature()));

    return list;
}
