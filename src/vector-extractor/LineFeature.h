#ifndef VECTOREXTRACTOR_LINEFEATURE_H
#define VECTOREXTRACTOR_LINEFEATURE_H


#include <map>
#include <list>
#include <gdal/gdal_priv.h>

class LineFeature {
public:
    LineFeature(OGRFeature *feature);

    std::map<std::string, std::string> get_attributes();

    std::string get_attribute(const std::string& name);

    std::vector<double> get_line_point(int index);

    double get_line_point_x(int index);

    double get_line_point_y(int index);

    double get_line_point_z(int index);

    int get_point_count();

private:
    OGRFeature *feature;

    OGRLineString *line;

    int point_count;
};


#endif //VECTOREXTRACTOR_LINEFEATURE_H
