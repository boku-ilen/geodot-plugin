#ifndef RASTEREXTRACTOR_RASTERTILEEXTRACTOR_H
#define RASTEREXTRACTOR_RASTERTILEEXTRACTOR_H

#include "GeoRaster.h"


class RasterTileExtractor {
public:
    /// Must be called before any other function to initialize GDAL.
    static void initialize();

    /// Reproject the raster file at infile to Webmercator and save the result to outfile.
    /// Adapted from https://gdal.org/tutorials/warp_tut.html
    static void reproject_to_webmercator(const char *infile, const char *outfile);

    /// Returns a GeoRaster with the data at raster_name and the given parameters.
    /// raster_name can be the name of a geotiff or the top folder of a pre-tiled raster pyramid.
    static GeoRaster *
    get_raster_at_position(const char *raster_name, const char *file_ending, double top_left_x, double top_left_y,
                           double size_meters, int img_size, int interpolation_type);

private:
    /// Clip the infile to an image starting at top_left_x, top_left_y with a given size (in meters).
    /// The resulting image has the resolution img_size x img_size (pixels).
    /// __The returned GDALDataset needs to be closed with GDALClose()!__
    static GDALDataset *clip(const char *infile, double top_left_x, double top_left_y, double size_meters, int img_size,
                             int interpolation_type);

    /// Get an image from a pre-tiled raster pyramid (named according to the Slippy tilenames).
    /// __The returned GDALDataset needs to be closed with GDALClose()!__
    static GDALDataset *
    get_from_pyramid(const char *infile, const char *file_ending, double top_left_x, double top_left_y,
                     double size_meters, int img_size,
                     int interpolation_type);
};


#endif //RASTEREXTRACTOR_RASTERTILEEXTRACTOR_H
