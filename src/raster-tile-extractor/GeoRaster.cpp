#include "GeoRaster.h"

#ifdef _WIN32
#include <gdal_priv.h>
#elif __unix__
#include <gdal/gdal_priv.h>
#endif

void *GeoRaster::get_as_array() {
    GDALRasterIOExtraArg rasterio_args;
    INIT_RASTERIO_EXTRA_ARG(rasterio_args);

    int interpolation = interpolation_type;

    // If we're requesting downscaled data, always use nearest neighbour scaling.
    // TODO: Would be good if this could be overridden with an optional parameter, but any other
    // scaling usually causes
    //  very long loading times so this is the default for now
    if (destination_window_size_pixels < source_window_size_pixels) { interpolation = 0; }
    rasterio_args.eResampleAlg = static_cast<GDALRIOResampleAlg>(interpolation);

    // Check whether the requested image exceeds the extent of the data
    if ((pixel_offset_x < 0 ||
         pixel_offset_x + source_window_size_pixels > data->GetRasterXSize()) ||
        (pixel_offset_y < 0 ||
         pixel_offset_y + source_window_size_pixels > data->GetRasterYSize())) {
        // TODO: Handle properly:
        // 1. Create array using size destination_window_size_pixels
        // 2. Extract into other array as usual, but with clamped extent
        // 3. Insert other array into 1. array line-by-line using the part which was clamped as the
        // offset This has the disadvantage of potentially allocating twice as much data, but it
        // seems to be the only option with GDAL's RasterIO. Until this is done, return null:
        return nullptr;
    }

    // Depending on the image format, we need to structure the resulting array differently and/or
    // read multiple bands.
    if (format == RF) {
        // Write the data directly into a float array.
        GDALRasterBand *band = data->GetRasterBand(1);
        float *array = new float[get_size_in_bytes()];

        band->RasterIO(GF_Read, pixel_offset_x, pixel_offset_y, source_window_size_pixels,
                       source_window_size_pixels, array, destination_window_size_pixels,
                       destination_window_size_pixels, GDT_Float32, 0, 0, &rasterio_args);

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
            band->RasterIO(GF_Read, pixel_offset_x, pixel_offset_y, source_window_size_pixels,
                           source_window_size_pixels, array + (band_number - 1),
                           destination_window_size_pixels, destination_window_size_pixels, GDT_Byte,
                           4, 0, &rasterio_args);
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
            band->RasterIO(GF_Read, pixel_offset_x, pixel_offset_y, source_window_size_pixels,
                           source_window_size_pixels, array + (band_number - 1),
                           destination_window_size_pixels, destination_window_size_pixels, GDT_Byte,
                           3, 0, &rasterio_args);
        }

        return array;
    } else if (format == BYTE) {
        // Simply write the bytes directly into a byte array.
        uint8_t *array = new uint8_t[get_size_in_bytes()];

        GDALRasterBand *band = data->GetRasterBand(1);

        // Read into the array with 4 bytes between the pixels
        band->RasterIO(GF_Read, pixel_offset_x, pixel_offset_y, source_window_size_pixels,
                       source_window_size_pixels, array, destination_window_size_pixels,
                       destination_window_size_pixels, GDT_Byte, 0, 0, &rasterio_args);

        return array;
    }

    return nullptr;
}

int GeoRaster::get_size_in_bytes() {
    int pixels = get_pixel_size_x() * get_pixel_size_y();

    if (format == BYTE) {
        return pixels;
    } else if (format == RF) {
        return pixels * 4; // 32-bit float
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
    return destination_window_size_pixels;
}

int GeoRaster::get_pixel_size_y() {
    return destination_window_size_pixels;
}

uint64_t *GeoRaster::get_histogram() {
    // TODO: Make sure this is only called on a GeoRaster with format BYTE
    //  It doesn't make sense for Float32 and we would need a different method for RGBA
    uint64_t *histogram = new uint64_t[256];

    // Initialize array
    for (int i = 0; i < 256; i++) {
        histogram[i] = 0;
    }

    uint8_t *array = reinterpret_cast<uint8_t *>(get_as_array());

    for (int y = 0; y < get_pixel_size_y(); y++) {
        for (int x = 0; x < get_pixel_size_x(); x++) {
            int index = array[y * get_pixel_size_x() + x];
            histogram[index]++;
        }
    }

    return histogram;
}

GeoRaster::GeoRaster(GDALDataset *data, int interpolation_type)
    : GeoRaster(data, 0, 0, data->GetRasterXSize(), data->GetRasterXSize(), interpolation_type) {}

GeoRaster::GeoRaster(GDALDataset *data, int pixel_offset_x, int pixel_offset_y,
                     int source_window_size_pixels, int destination_window_size_pixels,
                     int interpolation_type)
    : data(data), pixel_offset_x(pixel_offset_x), pixel_offset_y(pixel_offset_y),
      source_window_size_pixels(source_window_size_pixels),
      destination_window_size_pixels(destination_window_size_pixels),
      interpolation_type(interpolation_type) {
    int raster_count = data->GetRasterCount();
    GDALDataType raster_type = data->GetRasterBand(1)->GetRasterDataType();

    if (raster_count == 3 && raster_type == GDT_Byte) { format = RGB; }

    if (raster_type == GDT_Byte) {
        if (raster_count == 4) {
            format = RGBA;
        } else if (raster_count == 3) {
            format = RGB;
        } else {
            format = BYTE;
        }
    } else {
        // TODO: Is this fine as a fallback, or would another type be better? Maybe we should
        // assert?
        format = RF;
    }
}

int get_index_of_highest_value(const uint64_t *array, int array_size) {
    int max_index = 0;
    uint64_t max_value = 0;

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
        // The index is used, not the value, since the value corresponds to the number of occurences
        // which we don't care about; we want the ID
        elements[element_index] = highest_index;

        // Set the value at this position to 0 so that it is not found again next iteration
        histogram[highest_index] = 0;
    }

    delete[] histogram;

    return elements;
}
