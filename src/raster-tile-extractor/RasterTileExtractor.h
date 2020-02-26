#ifndef RASTEREXTRACTOR_RASTERTILEEXTRACTOR_H
#define RASTEREXTRACTOR_RASTERTILEEXTRACTOR_H

#include <iostream>
#include <gdal/gdal_priv.h>
#include <gdal/gdalwarper.h>


class RasterTileExtractor {
public:
    RasterTileExtractor();

    /// Reproject the raster file at infile to Webmercator and save the result to outfile.
    /// Adapted from https://gdal.org/tutorials/warp_tut.html
    void reproject_to_webmercator(const std::string &infile, const std::string &outfile);

    /// Clip the infile to an image starting at top_left_x, top_left_y with a given size (in meters).
    /// The resulting image has the resolution img_size x img_size (pixels).
    void clip(const std::string &infile, const std::string &outfile, double top_left_x, double top_left_y,
                     double size_meters, int img_size);
};


#endif //RASTEREXTRACTOR_RASTERTILEEXTRACTOR_H
