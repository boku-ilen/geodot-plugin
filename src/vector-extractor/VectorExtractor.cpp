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

std::shared_ptr<NativeDataset> VectorExtractor::open_dataset(const char *path, bool write_access) {
    return std::make_shared<NativeDataset> (path, write_access);
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