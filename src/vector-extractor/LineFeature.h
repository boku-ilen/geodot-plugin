#ifndef VECTOREXTRACTOR_LINEFEATURE_H
#define VECTOREXTRACTOR_LINEFEATURE_H


#include <map>
#include <list>
#include <gdal/gdal_priv.h>

class LineFeature {
public:
    std::map<std::string, std::string> get_attributes();

    std::string get_attribute(const std::string& name);

    double *get_line_point(int index);

private:
    OGRFeature *feature;
};


#endif //VECTOREXTRACTOR_LINEFEATURE_H
