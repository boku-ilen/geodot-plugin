#ifndef RASTERTILEEXTRACTOR_GEORASTER_H
#define RASTERTILEEXTRACTOR_GEORASTER_H


#include <gdal/gdal_priv.h>

/// Wrapper for GDALDataset and its relevant functions.
/// Provides easy access without GDAL dependencies to library users.
class GeoRaster {
public:
    enum FORMAT {
        RGB,   // 3 8-bit int channels
        RGBA,  // 4 8-bit int channels
        RF,    // 1 32-bit float channel
        BYTE   // 1 8-bit int channel
    };

    GeoRaster(GDALDataset *data);

    ~GeoRaster();

    /// Return the data of the GeoRaster as an array. The array contains type of the raster format (get_format).
    /// BYTE -> (B)(B)(B) with B of type uint8_t
    /// RGB -> (RGB)(RGB)(RGB) with R, G, B of type uint8_t
    /// RGBA -> (RGBA)(RGBA)(RGBA) with R, G, B, A of type uint8_t
    /// RF -> (F)(F)(F) with F of type float
    void *get_as_array();

    /// Returnsthe total size of the data in bytes. Useful in conjunction with get_as_array.
    int get_size_in_bytes();

    /// Return the format of the data of this GeoRaster.
    FORMAT get_format();

    int get_pixel_size_x();

    int get_pixel_size_y();

    /// Return the (approximate) number_of_entries most common entries in the GeoRaster.
    int *get_histogram(int number_of_entries);

private:
    GDALDataset *data;

    FORMAT format;
};


#endif //RASTERTILEEXTRACTOR_GEORASTER_H
