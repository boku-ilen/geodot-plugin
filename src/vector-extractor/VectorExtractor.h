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
    /// @RequiresManualDelete
    static NativeDataset *open_dataset(const char *path, bool write_access);

    static std::vector<double> transform_coordinates(double input_x, double input_z,
                                                     std::string from, std::string to);
};

#endif // VECTOREXTRACTOR_VECTOREXTRACTOR_H
