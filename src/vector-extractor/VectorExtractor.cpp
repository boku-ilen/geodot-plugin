#include "VectorExtractor.h"
#include <gdal/ogrsf_frmts.h>
#include <gdal/gdal.h>
#include <iostream>

void VectorExtractor::print_first_feature() {
    GDALAllRegister();

    GDALDataset *poDS;

    poDS = (GDALDataset *) GDALOpenEx("/home/karl/BOKU/geodata/streets/2_linknetz_ogd_WM.shp", GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
    if (poDS == nullptr) {
        printf("Open failed.\n");
        exit(1);
    }

    std::cout << poDS->GetLayerCount() << " layers" << std::endl;
    OGRLayer *poLayer = poDS->GetLayers()[0];

    OGRFeature *poFeature = poLayer->GetNextFeature();

    for (auto &&oField: *poFeature) {
        std::cout << oField.GetName() << ": " << oField.GetAsString() << std::endl;
    }
}
