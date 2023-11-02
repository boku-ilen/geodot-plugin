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

CoordinateTransform::CoordinateTransform(int from, int to) {
    // Workaround for https://github.com/OSGeo/gdal/issues/1546
    source_reference.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);
    target_reference.SetAxisMappingStrategy(OAMS_TRADITIONAL_GIS_ORDER);

    source_reference.importFromEPSG(from);
    target_reference.importFromEPSG(to);

    transformation =
        OGRCreateCoordinateTransformation(&source_reference, &target_reference);
}

std::vector<double> CoordinateTransform::transform_coordinates(double input_x, double input_z) {
    double output_x = input_x;
    double output_z = input_z;

    transformation->Transform(1, &output_x, &output_z);

    return std::vector{output_x, output_z};
}