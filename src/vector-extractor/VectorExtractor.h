#ifndef VECTOREXTRACTOR_VECTOREXTRACTOR_H
#define VECTOREXTRACTOR_VECTOREXTRACTOR_H

#include "gdal-includes.h"

#include "LineFeature.h"
#include "NativeDataset.h"
#include "PointFeature.h"
#include "PolygonFeature.h"
#include "defines.h"
#include <map>

class VectorExtractor {
  public:
    /// Must be called before any other function to initialize GDAL.
    static void initialize();

    /// Returns the GDALDataset at the given path, or null.
    /// TODO: This could also be in the RasterExtractor, it's not raster- or vector-specific...
    static std::shared_ptr<NativeDataset> open_dataset(const char *path, bool write_access);
};

class CoordinateTransform {
  public:
    CoordinateTransform(int from, int to);

    std::vector<double> transform_coordinates(double input_x, double input_z);
  
  private:
    OGRSpatialReference source_reference, target_reference;
    OGRCoordinateTransformation *transformation;
};

#endif // VECTOREXTRACTOR_VECTOREXTRACTOR_H
