#include "LineFeature.h"

#ifdef _WIN32
#include <gdal_priv.h>
#include <ogr_geometry.h>
#elif __APPLE__
#include <gdal_priv.h>
#include <ogr_geometry.h>
#elif __unix__
#include <gdal/gdal_priv.h>
#include <gdal/ogr_geometry.h>
#endif

LineFeature::LineFeature(OGRFeature *feature) : Feature(feature) {
    line = feature->GetGeometryRef()->toLineString();
    point_count = line->getNumPoints();
    geometry_type = LINE;
}

LineFeature::LineFeature(OGRFeature *feature, const OGRGeometry *linestring) : Feature(feature) {
    line = linestring->toLineString();
    point_count = line->getNumPoints();
    geometry_type = LINE;
}

std::vector<double> LineFeature::get_line_point(int index) {
    return std::vector<double>{line->getX(index), line->getY(index), line->getZ(index)};
}

int LineFeature::get_point_count() {
    return point_count;
}

double LineFeature::get_line_point_x(int index) {
    return line->getX(index);
}

double LineFeature::get_line_point_y(int index) {
    return line->getY(index);
}

double LineFeature::get_line_point_z(int index) {
    return line->getZ(index);
}
