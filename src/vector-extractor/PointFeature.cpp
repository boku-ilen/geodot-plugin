#include "PointFeature.h"

#ifdef _WIN32
    #include <gdal_priv.h>
#elif __unix__
    #include <gdal/gdal_priv.h>
#endif


PointFeature::PointFeature(OGRFeature *feature) : Feature(feature) {
    point = feature->GetGeometryRef()->toPoint();
}

PointFeature::PointFeature(OGRFeature *feature, const OGRGeometry *ogrPoint) : Feature(feature), point(ogrPoint->toPoint()) {}

double PointFeature::get_x() {
    return point->getX();
}

double PointFeature::get_y() {
    return point->getY();
}

double PointFeature::get_z() {
    return point->getZ();
}
