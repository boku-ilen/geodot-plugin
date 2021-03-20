#include "PointFeature.h"

#ifdef _WIN32
#include <gdal_priv.h>
#elif __APPLE__
#include <gdal_priv.h>
#elif __unix__
#include <gdal/gdal_priv.h>
#endif

PointFeature::PointFeature(OGRFeature *feature) : Feature(feature) {
    point = feature->GetGeometryRef()->toPoint();
    geometry_type = POINT;
}

PointFeature::PointFeature(OGRFeature *feature, OGRGeometry *ogrPoint)
    : Feature(feature), point(ogrPoint->toPoint()) {
    geometry_type = POINT;
}

double PointFeature::get_x() {
    return point->getX();
}

double PointFeature::get_y() {
    return point->getY();
}

double PointFeature::get_z() {
    return point->getZ();
}

void PointFeature::set_vector(double x, double y, double z) {
    point->setX(x);
    point->setY(y);
    point->setZ(z);
}