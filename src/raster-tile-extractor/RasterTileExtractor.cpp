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
    // Save the result in RAM
    GDALDriver *driver = (GDALDriver *)GDALGetDriverByName("MEM");

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

GeoRaster *RasterTileExtractor::get_raster_from_pyramid(const char *base_path,
                                                        const char *file_ending, double top_left_x,
                                                        double top_left_y, double size_meters,
                                                        int img_size, int interpolation_type) {
    GDALDataset *dataset_from_pyramid = get_from_pyramid(
        base_path, file_ending, top_left_x, top_left_y, size_meters, img_size, interpolation_type);

    if (dataset_from_pyramid != nullptr) {
        return new GeoRaster(dataset_from_pyramid, interpolation_type);
    }

    // If there was neither a single file nor a pyramid, return null
    return nullptr;
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

#define WEBMERCATOR_MAX 20037508.0
#define PI 3.14159265358979323846
#define CIRCUMEFERENCE 40075016.686

class path;

GDALDataset *RasterTileExtractor::get_from_pyramid(const char *base_path, const char *file_ending,
                                                   double top_left_x, double top_left_y,
                                                   double size_meters, int img_size,
                                                   int interpolation_type) {
    // Norm webmercator position (in meters) to value between -1 and 1
    double norm_x = 0.5 + ((top_left_x + size_meters / 2.0) / WEBMERCATOR_MAX) * 0.5;

    double norm_y = 1.0 - (0.5 + ((top_left_y - size_meters / 2.0) / WEBMERCATOR_MAX) * 0.5);

    // Get latitude and use it to calculate the zoom level here
    double latitude = webmercator_to_latitude(top_left_y);
    // Original formula: size = C * cos(latitude) / pow(2, zoom_level) (from
    // https://wiki.openstreetmap.org/wiki/Zoom_levels)
    int zoom_level = (int)round(log2(CIRCUMEFERENCE * cos(latitude) / size_meters)) + 1;

    // Number of tiles at this zoom level
    int num_tiles = pow(2.0, zoom_level);

    // Tile coordinates in OSM pyramid
    int tile_x = (int)floor(norm_x * num_tiles);
    int tile_y = (int)floor(norm_y * num_tiles);

    // Build the complete path
    std::string path = std::string(base_path);
    path += "/";
    path += std::to_string(zoom_level);
    path += "/";
    path += std::to_string(tile_x);
    path += "/";
    path += std::to_string(tile_y);
    path += ".";
    path += file_ending;

    // Load the result as a GDALDataset and return it
    return (GDALDataset *)GDALOpen(path.c_str(), GA_ReadOnly);
}
