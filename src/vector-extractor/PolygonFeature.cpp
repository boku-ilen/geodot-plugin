#include "PolygonFeature.h"

#ifdef _WIN32
    #include <gdal_priv.h>
#elif __unix__
    #include <gdal/gdal_priv.h>
#endif


PolygonFeature::PolygonFeature(OGRFeature *feature) : Feature(feature) {
    polygon = feature->GetGeometryRef()->toPolygon();
    geometry_type = POLYGON;
}

PolygonFeature::PolygonFeature(OGRFeature *feature, const OGRGeometry *ogrPolygon) : Feature(feature), polygon(ogrPolygon->toPolygon()) {
    geometry_type = POLYGON;
}
