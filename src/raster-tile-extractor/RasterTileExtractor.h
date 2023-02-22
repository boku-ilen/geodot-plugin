#ifndef RASTEREXTRACTOR_RASTERTILEEXTRACTOR_H
#define RASTEREXTRACTOR_RASTERTILEEXTRACTOR_H

#include "GeoRaster.h"
#include "defines.h"
#include "util.h"

class RasterTileExtractor {
  public:
    /// Must be called before any other function to initialize GDAL.
    static void initialize();

    /// Returns a GeoRaster with the data from the given dataset, in the given area, with the given
    /// resolution and interpolation.
    static GeoRaster *get_tile_from_dataset(GDALDataset *dataset, double top_left_x,
                                            double top_left_y, double size_meters, int img_size,
                                            int interpolation_type);

    static void write_into_dataset(GDALDataset *dataset, double center_x, double center_y,
                                   void *values, double scale, int interpolation_type);

    static void smooth_add_into_dataset(GDALDataset *dataset, double center_x, double center_y,
                                        double summand, double radius);

    static ExtentData get_extent_data(GDALDataset *dataset);

    static float get_min(GDALDataset *dataset);
    static float get_max(GDALDataset *dataset);

  private:
    /// Return a GeoRaster containing the area in the given dataset starting at top_left_x,
    /// top_left_y with a given size (in meters). The resulting image has the resolution
    /// img_size * img_size (pixels).
    static GeoRaster *clip_dataset(GDALDataset *dataset, double top_left_x, double top_left_y,
                                   double size_meters, int img_size, int interpolation_type);
};

#endif // RASTEREXTRACTOR_RASTERTILEEXTRACTOR_H
