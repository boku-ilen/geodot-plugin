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

    // Return the data of the GeoRaster as an array. The array contains type of the raster format (get_format).
    void *get_as_array();

    // Returns the total size of the data in bytes. Useful in conjunction with get_as_array.
    int get_size_in_bytes();

    // Returns the format of the data of this GeoRaster.
    FORMAT get_format();

    int get_pixel_size_x();

    int get_pixel_size_y();

    // Returns the (approximate) number_of_entries most common entries in the GeoRaster.
    int *get_histogram(int number_of_entries);

private:
    GDALDataset *data;

    FORMAT format;
};


#endif //RASTERTILEEXTRACTOR_GEORASTER_H
