#include "RasterTileExtractor.h"
#include "gdal-includes.h"
#include <iostream>

void RasterTileExtractor::initialize() {
    // Register all drivers - without this, GDALGetDriverByName doesn't work
    GDALAllRegister();
}

// Some compilers seem to already include M_E, other's dont...
#ifndef M_E
#define M_E 2.71828182845904523536028747135266250
#endif
double webmercator_to_latitude(double webm_meters) {
    // Adapted from https://gist.github.com/springmeyer/871897#gistcomment-1185502
    return (atan(pow(M_E, ((webm_meters) / 111319.490778) * M_PI / 180.0)) * 2.0 - M_PI / 2.0);
}

GeoRaster *RasterTileExtractor::clip_dataset(GDALDataset *dataset, double top_left_x,
                                             double top_left_y, double size_meters, int img_size,
                                             int interpolation_type) {
    // Get the current Transform of the source image
    double transform[6];
    dataset->GetGeoTransform(transform);

    // Adjust the top left coordinates according to the input variables
    double previous_top_left_x = transform[0];
    double previous_top_left_y = transform[3];
    double pixel_size = transform[1];

    double offset_meters_x = top_left_x - previous_top_left_x;
    double offset_meters_y = previous_top_left_y - top_left_y;

    // Convert meters to pixels using the pixel size in meters
    int offset_pixels_x = static_cast<int>(offset_meters_x / pixel_size);
    int offset_pixels_y = static_cast<int>(offset_meters_y / pixel_size);

    // Calculate the desired size in pixels
    int size_pixels = static_cast<int>(ceil(size_meters / pixel_size));

    // With these parameters, we can construct a GeoRaster!
    return new GeoRaster(dataset, offset_pixels_x, offset_pixels_y, size_pixels, img_size,
                         interpolation_type);
}

GeoRaster *RasterTileExtractor::get_tile_from_dataset(GDALDataset *dataset, double top_left_x,
                                                      double top_left_y, double size_meters,
                                                      int img_size, int interpolation_type) {
    return clip_dataset(dataset, top_left_x, top_left_y, size_meters, img_size, interpolation_type);
}

RasterTileExtractor::ExtentData RasterTileExtractor::get_extent_data(GDALDataset *dataset) {
    // Get the Transform of the image
    double transform[6];
    dataset->GetGeoTransform(transform);

    double x_size = dataset->GetRasterXSize();
    double y_size = dataset->GetRasterYSize();

    ExtentData extent_data;

    extent_data.left = transform[0] + 0 * transform[1] + 0 * transform[2];
    extent_data.right = transform[0] + x_size * transform[1] + 0 * transform[2];
    extent_data.top = transform[3] + 0 * transform[4] + 0 * transform[5];
    extent_data.down = transform[3] + 0 * transform[4] + y_size * transform[5];

    return extent_data;
}

float RasterTileExtractor::get_min(GDALDataset *dataset) {
    return dataset->GetRasterBand(1)->GetMinimum();
}

float RasterTileExtractor::get_max(GDALDataset *dataset) {
    return dataset->GetRasterBand(1)->GetMaximum();
}
