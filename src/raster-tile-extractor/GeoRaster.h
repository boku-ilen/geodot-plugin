#ifndef RASTERTILEEXTRACTOR_GEORASTER_H
#define RASTERTILEEXTRACTOR_GEORASTER_H

#include "defines.h"
#include <cstdint>

struct RasterIOHelper {
    int clamped_pixel_offset_x;
    int clamped_pixel_offset_y;
    int min_raster_size; // Currently unused
    int remainder_x_left;
    int remainder_y_top;
    int target_height;
    int target_width;
    int usable_height;
    int usable_width;
};

// Forward declaration of GDALDataset from <gdal/gdal_priv.h>
class GDALDataset;

/// Wrapper for GDALDataset and its relevant functions.
/// Provides easy access without GDAL dependencies to library users.
class GeoRaster {
  public:
    enum FORMAT { // size (bits)    | data type         |   Number of chanels
        RGB,      //    8           | int               |   3
        RGBA,     //    8           | int               |   4
        RF,       //    32<=X<=64   | float             |   X >= 1
        BYTE,     //    8           | int               |   X >= 1
        MIXED,    //    8<=X<=64    | int and/or float  |   X >= 2
        UNKNOWN   //    unknown     | unknown           |   X >= 1
    };

    GeoRaster(GDALDataset *data, int interpolation_type);

    GeoRaster(GDALDataset *data, int pixel_offset_x, int pixel_offset_y,
              int source_window_size_pixels, int destination_window_size_pixels,
              int interpolation_type);

    ~GeoRaster() = default;

    static FORMAT get_format_for_dataset(GDALDataset *data);
    
    /// Return the data of the GeoRaster as an array. The array contains type of the raster format
    /// (get_format).
    /// BYTE -> (B)(B)(B) with B of type uint8_t
    /// RGB -> (RGB)(RGB)(RGB) with R, G, B of type uint8_t
    /// RGBA -> (RGBA)(RGBA)(RGBA) with R, G, B, A of type uint8_t
    /// RF -> (F)(F)(F) with F of type float
    /// @RequiresManualDelete
    void *get_as_array();

    /// @brief Return the data within a single band of the GeoRaster as an array.
    /// The type of the array can be any of: BYTE, RF, or UNKNOWN.
    /// @param band_index the index of the band to be returned as array.
    /// @return the band as array.
    void *get_band_as_array(int band_index);

    /// Return the total size of the data in bytes. Useful in conjunction with get_as_array.
    /// An optional pixel_size can be given if it deviates from the standard size saved in the
    /// object.
    int get_size_in_bytes();

    /// Return the format of the data of this GeoRaster.
    FORMAT get_format();

    /// @brief get the FORMAT of the band at band_index within the dataset.
    /// This function is most useful when working with either MIXED datasets or simply for
    /// extracting specific band information
    /// @param band_index index of the band whose format will be found
    /// @return the FORMAT of the band at band_index
    FORMAT get_band_format(int band_index);

    int get_pixel_size_x();

    int get_pixel_size_y();

    /// Return a histogram in the format of ID -> number of occurrences.
    /// Only works with BYTE images!
    /// @RequiresManualDelete
    uint64_t *get_histogram();

    /// Return the number_of_elements most common IDs in the GeoRaster.
    /// Note: This function is optimized for few elements (less than ~10). For use cases where a
    /// more complete sorted list is required, the array should be sorted with a proper sorting algorithm.
    /// @RequiresManualDelete
    int *get_most_common(int number_of_elements);

  private:
    GDALDataset *data;

    FORMAT format;

    int pixel_offset_x;

    int pixel_offset_y;

    int source_window_size_pixels;

    int destination_window_size_pixels;

    int interpolation_type;

    /// Returns a RasterIOHelper with attributes needed for IO operations with native raster.
    /// Internal function to extract data from native raster.
    RasterIOHelper get_raster_io_helper();
};

#endif // RASTERTILEEXTRACTOR_GEORASTER_H
