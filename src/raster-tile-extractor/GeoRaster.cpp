#include "GeoRaster.h"

GeoRaster::~GeoRaster() {
    GDALClose(data);
}

void *GeoRaster::get_as_array() {
    // TODO: Implement for BYTE and RGBA

    if (format == RF) {
        GDALRasterBand *band = data->GetRasterBand(1);
        float *array = new float[get_size_in_bytes()];

        GDALRasterIO(band, GF_Read, 0, 0, get_pixel_size_x(), get_pixel_size_y(),
                      array, get_pixel_size_x(), get_pixel_size_y(), GDT_Float32,
                      0, 0 );

        return array;
    }

    return nullptr;
}

int GeoRaster::get_size_in_bytes() {
    int pixels = get_pixel_size_x() * get_pixel_size_y();

    if (format == BYTE) {
        return pixels;
    } else if (format == RF) {
        return pixels * 4;  // 32-bit float
    } else if (format == RGBA) {
        return pixels * 4;
    } else {
        // Invalid format!
        return 0;
    }
}

GeoRaster::FORMAT GeoRaster::get_format() {
    return format;
}

int GeoRaster::get_pixel_size_x() {
    return data->GetRasterXSize();
}

int GeoRaster::get_pixel_size_y() {
    return data->GetRasterYSize();
}

int *GeoRaster::get_histogram(int number_of_entries) {
    // TODO: Make sure this is only called on a GeoRaster with format BYTE
    //  It doesn't make sense for Float32 and we would need a different method for RGBA
    GDALRasterBand *band = data->GetRasterBand(1);
    GUIntBig *histogram = new GUIntBig[number_of_entries];

    band->GetHistogram(0.0, 255.5, number_of_entries, histogram, false, true, GDALDummyProgress, nullptr);

    // TODO: This breaks the array
    return reinterpret_cast<int *>(histogram);
}
