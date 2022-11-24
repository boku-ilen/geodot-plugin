#include "RasterTileExtractor.h"
#include "gdal-includes.h"
#include <cstddef>
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

class DatasetPositionData {
  public:
    DatasetPositionData(GDALDataset *dataset, double meters_x, double meters_y, double size_meters)
        : meters_x(meters_x), meters_y(meters_y), size_meters(size_meters) {
        // Get the current Transform of the source image
        double transform[6];
        dataset->GetGeoTransform(transform);

        // Adjust the top left coordinates according to the input variables
        double previous_meters_x = transform[0];
        double previous_meters_y = transform[3];
        double pixel_size = transform[1];

        double offset_meters_x = meters_x - previous_meters_x;
        double offset_meters_y = previous_meters_y - meters_y;

        // Convert meters to pixels using the pixel size in meters
        pixels_x = static_cast<int>(offset_meters_x / pixel_size);
        pixels_y = static_cast<int>(offset_meters_y / pixel_size);

        // Calculate the desired size in pixels
        size_pixels = static_cast<int>(ceil(size_meters / pixel_size));
    }

    double meters_x;
    double meters_y;
    double size_meters;

    int pixels_x;
    int pixels_y;
    int size_pixels;
};

GeoRaster *RasterTileExtractor::clip_dataset(GDALDataset *dataset, double top_left_x,
                                             double top_left_y, double size_meters, int img_size,
                                             int interpolation_type) {
    DatasetPositionData position_data(dataset, top_left_x, top_left_y, size_meters);

    // With these parameters, we can construct a GeoRaster!
    return new GeoRaster(dataset, position_data.pixels_x, position_data.pixels_y,
                         position_data.size_pixels, img_size, interpolation_type);
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

void RasterTileExtractor::write_into_dataset(GDALDataset *dataset, double center_x, double center_y,
                                             void *values, double scale, int interpolation_type) {
    DatasetPositionData position_data(dataset, center_x, center_y, 0);

    GDALDataType data_type = dataset->GetRasterBand(1)->GetRasterDataType();

    if (data_type == GDALDataType::GDT_Float32) {
        // Float32
        GDALRasterBand *band = dataset->GetRasterBand(1);
        CPLErr error =
            band->RasterIO(GDALRWFlag::GF_Write, position_data.pixels_x, position_data.pixels_y, 1,
                           1, values, 1, 1, GDT_Float32, 0, 1);
    } else if (data_type == GDALDataType::GDT_Byte) {
        // Byte, RGB, or RGBA
        unsigned int band_count = dataset->GetRasterCount();

        for (int i = 1; i <= band_count; i++) {
            GDALRasterBand *band = dataset->GetRasterBand(i);
            char value = static_cast<char *>(values)[i - 1];
            char *value_array = &value;

            CPLErr error =
                band->RasterIO(GDALRWFlag::GF_Write, position_data.pixels_x, position_data.pixels_y,
                               1, 1, value_array, 1, 1, GDT_Byte, 0, 1);
        }
    }
}
