#include "LineFeature.h"
#include <gdal/gdal_priv.h>

LineFeature::LineFeature(const OGRFeature *feature) : feature(feature) {
    line = feature->GetGeometryRef()->toLineString();
    point_count = line->getNumPoints();
}

LineFeature::LineFeature(const OGRFeature *feature, const OGRLineString *linestring) : feature(feature) {
    line = linestring;
    point_count = line->getNumPoints();
}

std::map<std::string, std::string> LineFeature::get_attributes() {
    std::map<std::string, std::string> ret;

    for (auto &&oField: *feature) {
        ret[oField.GetName()] = oField.GetAsString();
    }

    return ret;
}

const char *LineFeature::get_attribute(const char *name) {
    return feature->GetFieldAsString(name);
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
