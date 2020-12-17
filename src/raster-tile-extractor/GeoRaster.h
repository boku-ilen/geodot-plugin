#ifndef RASTERTILEEXTRACTOR_GEORASTER_H
#define RASTERTILEEXTRACTOR_GEORASTER_H

#include "defines.h"
#include <cstdint>

// Forward declaration of GDALDataset from <gdal/gdal_priv.h>
class GDALDataset;

/// Wrapper for GDALDataset and its relevant functions.
/// Provides easy access without GDAL dependencies to library users.
class EXPORT GeoRaster {
  public:
    enum FORMAT {
        RGB,  // 3 8-bit int channels
        RGBA, // 4 8-bit int channels
        RF,   // 1 32-bit float channel
        BYTE  // 1 8-bit int channel
    };

    GeoRaster(GDALDataset *data, int interpolation_type);

    GeoRaster(GDALDataset *data, int pixel_offset_x, int pixel_offset_y,
              int source_window_size_pixels, int destination_window_size_pixels,
              int interpolation_type);

    ~GeoRaster() = default;

    /// Return the data of the GeoRaster as an array. The array contains type of the raster format
    /// (get_format).
    /// BYTE -> (B)(B)(B) with B of type uint8_t
    /// RGB -> (RGB)(RGB)(RGB) with R, G, B of type uint8_t
    /// RGBA -> (RGBA)(RGBA)(RGBA) with R, G, B, A of type uint8_t
    /// RF -> (F)(F)(F) with F of type float
    void *get_as_array();

    /// Return the total size of the data in bytes. Useful in conjunction with get_as_array.
    int get_size_in_bytes();

    /// Return the format of the data of this GeoRaster.
    FORMAT get_format();

    int get_pixel_size_x();

    int get_pixel_size_y();

    /// Return a histogram in the format of ID -> number of occurrences.
    /// Only works with BYTE images!
    uint64_t *get_histogram();

    /// Return the number_of_elements most common IDs in the GeoRaster.
    /// Note: This function is optimized for few elements (less than ~10). For use cases where a
    /// more complete sorted
    ///  list is required, the array should be sorted with a proper sorting algorithm.
    int *get_most_common(int number_of_elements);

  private:
    GDALDataset *data;

    FORMAT format;

    int pixel_offset_x;

    int pixel_offset_y;

    int source_window_size_pixels;

    int destination_window_size_pixels;

    int interpolation_type;
};

#endif // RASTERTILEEXTRACTOR_GEORASTER_H
