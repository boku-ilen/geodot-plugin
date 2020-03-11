#include "GeoRaster.h"

GeoRaster::~GeoRaster() {
    GDALClose(data);
}

void *GeoRaster::get_as_array() {
    // Depending on the image format, we need to structure the resulting array differently and/or read multiple bands.
    if (format == RF) {
        // Write the data directly into a float array.
        GDALRasterBand *band = data->GetRasterBand(1);
        float *array = new float[get_size_in_bytes()];

        GDALRasterIO(band, GF_Read, 0, 0, get_pixel_size_x(), get_pixel_size_y(),
                      array, get_pixel_size_x(), get_pixel_size_y(), GDT_Float32,
                      0, 0 );

        return array;
    } else if (format == RGBA) {
        // Write the data into a byte array like this:
        // R   R   R
        //  G   G   G
        //   B   B   B
        //    A   A   A
        // So that the result is RGBARGBARGBA.
        uint8_t *array = new uint8_t[get_size_in_bytes()];

        for (int band_number = 1; band_number < 5; band_number++) {
            GDALRasterBand *band = data->GetRasterBand(band_number);

            // Read into the array with 4 bytes between the pixels
            band->RasterIO(GF_Read, 0, 0, get_pixel_size_x(), get_pixel_size_y(),
                           array + (band_number - 1), get_pixel_size_x(), get_pixel_size_y(), GDT_Byte,
                           4, 0);
        }

        return array;
    } else if (format == RGB) {
        // Write the data into a byte array like this:
        // R  R  R
        //  G  G  G
        //   B  B  B
        // So that the result is RGBRGBRGB.
        uint8_t *array = new uint8_t[get_size_in_bytes()];

        for (int band_number = 1; band_number < 4; band_number++) {
            GDALRasterBand *band = data->GetRasterBand(band_number);

            // Read into the array with 3 bytes between the pixels
            band->RasterIO(GF_Read, 0, 0, get_pixel_size_x(), get_pixel_size_y(),
                           array + (band_number - 1), get_pixel_size_x(), get_pixel_size_y(), GDT_Byte,
                           3, 0);
        }

        return array;
    } else if (format == BYTE) {
        // Simply write the bytes directly into a byte array.
        uint8_t *array = new uint8_t[get_size_in_bytes()];

        GDALRasterBand *band = data->GetRasterBand(1);

        // Read into the array with 4 bytes between the pixels
        band->RasterIO(GF_Read, 0, 0, get_pixel_size_x(), get_pixel_size_y(),
                       array, get_pixel_size_x(), get_pixel_size_y(), GDT_Byte,
                       0, 0);

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
    } else if (format == RGB) {
        return pixels * 3;
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

uint64_t *GeoRaster::get_histogram() {
    // TODO: Make sure this is only called on a GeoRaster with format BYTE
    //  It doesn't make sense for Float32 and we would need a different method for RGBA
    GDALRasterBand *band = data->GetRasterBand(1);
    GUIntBig *histogram = new GUIntBig[256];

    band->GetHistogram(-0.5, 255.5, 256, histogram, false, true, GDALDummyProgress, nullptr);

    // TODO: This breaks the array
    return reinterpret_cast<uint64_t *>(histogram);
}

GeoRaster::GeoRaster(GDALDataset *data) : data(data) {
    int raster_count = data->GetRasterCount();
    GDALDataType raster_type = data->GetRasterBand(1)->GetRasterDataType();

    if (raster_count == 3 && raster_type == GDT_Byte) {
        format = RGB;
    }

    if (raster_type == GDT_Byte) {
        if (raster_count == 4) {
            format = RGBA;
        } else if (raster_count == 3) {
            format = RGB;
        } else {
            format = BYTE;
        }
    } else {
        // TODO: Is this fine as a fallback, or would another type be better? Maybe we should assert?
        format = RF;
    }
}

int get_index_of_highest_value(const uint64_t *array, int array_size) {
    int max_index = 0;
    int max_value = 0;

    for (int index = 0; index < array_size; index++) {
        if (array[index] > max_value) {
            max_value = array[index];
            max_index = index;
        }
    }

    return max_index;
}

int *GeoRaster::get_most_common(int number_of_elements) {
    int *elements = new int[number_of_elements];
    uint64_t *histogram = get_histogram();

    for (int element_index = 0; element_index < number_of_elements; element_index++) {
        // Get the index of the currently highest value
        int highest_index = get_index_of_highest_value(histogram, 256);

        // Add the currently highest index to the returned array
        // The index is used, not the value, since the value corresponds to the number of occurences which we don't care about; we want the ID
        elements[element_index] = highest_index;

        // Set the value at this position to 0 so that it is not found again next iteration
        histogram[highest_index] = 0;
    }

    delete[] histogram;

    return elements;
}
