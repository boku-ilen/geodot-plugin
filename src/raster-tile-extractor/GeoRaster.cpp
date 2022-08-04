#include "GeoRaster.h"
#include "gdal-includes.h"
#include <algorithm> // For std::clamp etc
#include <cstring>

void *GeoRaster::get_as_array() {
    GDALRasterIOExtraArg rasterio_args;
    INIT_RASTERIO_EXTRA_ARG(rasterio_args);

    int interpolation = interpolation_type;

    // If we're requesting downscaled data, always use nearest neighbour scaling.
    // TODO: Would be good if this could be overridden with an optional parameter, but any other
    // scaling usually causes very long loading times so this is the default for now
    if (destination_window_size_pixels < source_window_size_pixels) { interpolation = 0; }

    rasterio_args.eResampleAlg = static_cast<GDALRIOResampleAlg>(interpolation);

    // Restrict the offset and extent to the available data
    // TODO: Handle properly:
    // 1. Create array using size destination_window_size_pixels
    // 2. Extract into other array as usual, but with clamped extent
    // 3. Insert other array into 1. array line-by-line using the part which was clamped as the
    // offset. This has the disadvantage of potentially allocating twice as much data, but it seems
    // to be the only option with GDAL's RasterIO.
    int min_raster_size = std::min(data->GetRasterXSize(), data->GetRasterYSize());

    double source_destination_ratio = static_cast<double>(destination_window_size_pixels) /
                                      static_cast<double>(source_window_size_pixels);

    int usable_width = source_window_size_pixels;
    int usable_height = source_window_size_pixels;

    int clamped_pixel_offset_x = pixel_offset_x;
    int clamped_pixel_offset_y = pixel_offset_y;

    int remainder_x_left = 0;
    int remainder_y_top = 0;

    // Required for some datasets which don't have a clean transition from data into nodata.
    // TODO: Consider exposing as an argument
    int padding = 40;

    int available_x = data->GetRasterXSize() - padding;
    int available_y = data->GetRasterYSize() - padding;

    if (pixel_offset_x - padding < 0) {
        // FIXME: There might be a slight logic error here, as there's a slight border on the wrong
        // side
        usable_width += pixel_offset_x - padding;
        remainder_x_left = (-pixel_offset_x + padding) * source_destination_ratio;
        clamped_pixel_offset_x = padding;
    } else if (pixel_offset_x + source_window_size_pixels > available_x) {
        usable_width -= pixel_offset_x + source_window_size_pixels - available_x;
    }

    if (pixel_offset_y - padding < 0) {
        usable_height += pixel_offset_y - padding;
        remainder_y_top = (-pixel_offset_y + padding) * source_destination_ratio;
        clamped_pixel_offset_y = padding;
    } else if (pixel_offset_y + source_window_size_pixels > available_y) {
        usable_height -= pixel_offset_y + source_window_size_pixels - available_y;
    }

    int target_width = usable_width * source_destination_ratio;
    int target_height = usable_height * source_destination_ratio;

    // TODO: We could do more precise error handling by getting the error number using
    // CPLGetLastErrorNo() and returning that to the user somehow - maybe a flag in the
    // GeoRaster.
    CPLErr error = CE_Failure;

    // Depending on the image format, we need to structure the resulting array differently and/or
    // read multiple bands.
    // TODO: Lots of code duplication down there, but due to C++'s lack of reflection and all the
    // subtle differences, it's tricky to generalize it. A templated factory class might help, but
    // seems overkill
    if (format == RF) {
        if (usable_width <= 0 || usable_height <= 0) {
            // Empty results are still valid and should be treated normally, so return an array with
            // only 0s
            float *target_array = new float[get_pixel_size_x() * get_pixel_size_y()];
            std::fill(target_array, target_array + get_pixel_size_x() * get_pixel_size_y(),
                      -1000.0);
            return target_array;
        }

        // Write the data directly into a float array.
        GDALRasterBand *band = data->GetRasterBand(1);
        float *array = new float[get_pixel_size_x() * get_pixel_size_y()];
        std::fill(array, array + get_pixel_size_x() * get_pixel_size_y(), -1000.0);

        error = band->RasterIO(
            GF_Read, clamped_pixel_offset_x, clamped_pixel_offset_y, usable_width, usable_height,
            array + remainder_y_top * destination_window_size_pixels + remainder_x_left,
            target_width, target_height, GDT_Float32, 0, destination_window_size_pixels * 4,
            &rasterio_args);

        if (error < CE_Failure) { return array; }

    } else if (format == RGBA) {
        if (usable_width <= 0 || usable_height <= 0) {
            // Empty results are still valid and should be treated normally, so return an array with
            // only 0s
            uint8_t *target_array = new uint8_t[get_size_in_bytes()];
            std::fill(target_array, target_array + get_size_in_bytes(), 0);
            return target_array;
        }

        // Write the data into a byte array like this:
        // R   R   R
        //  G   G   G
        //   B   B   B
        //    A   A   A
        // So that the result is RGBARGBARGBA.
        uint8_t *array = new uint8_t[get_size_in_bytes()];
        std::fill(array, array + get_size_in_bytes(), 0);

        for (int band_number = 1; band_number < 5; band_number++) {
            GDALRasterBand *band = data->GetRasterBand(band_number);

            // Read into the array with 4 bytes between the pixels
            error = band->RasterIO(
                GF_Read, clamped_pixel_offset_x, clamped_pixel_offset_y, usable_width,
                usable_height,
                array + (remainder_y_top * destination_window_size_pixels + remainder_x_left) * 4 +
                    (band_number - 1),
                target_width, target_height, GDT_Byte, 4, destination_window_size_pixels * 4,
                &rasterio_args);
        }

        if (error < CE_Failure) { return array; }

    } else if (format == RGB) {
        if (usable_width <= 0 || usable_height <= 0) {
            // Empty results are still valid and should be treated normally, so return an array with
            // only 0s
            uint8_t *target_array = new uint8_t[get_size_in_bytes()];
            std::fill(target_array, target_array + get_size_in_bytes(), 0);
            return target_array;
        }

        // Write the data into a byte array like this:
        // R  R  R
        //  G  G  G
        //   B  B  B
        // So that the result is RGBRGBRGB.
        uint8_t *array = new uint8_t[get_size_in_bytes()];
        std::fill(array, array + get_size_in_bytes(), 0);

        for (int band_number = 1; band_number < 4; band_number++) {
            GDALRasterBand *band = data->GetRasterBand(band_number);

            // Read into the array with 4 bytes between the pixels
            error = band->RasterIO(
                GF_Read, clamped_pixel_offset_x, clamped_pixel_offset_y, usable_width,
                usable_height,
                array + (remainder_y_top * destination_window_size_pixels + remainder_x_left) * 3 +
                    (band_number - 1),
                target_width, target_height, GDT_Byte, 3, destination_window_size_pixels * 3,
                &rasterio_args);
        }

        if (error < CE_Failure) { return array; }

    } else if (format == BYTE) {
        if (usable_width <= 0 || usable_height <= 0) {
            // Empty results are still valid and should be treated normally, so return an array with
            // only 0s
            uint8_t *target_array = new uint8_t[get_size_in_bytes()];
            std::fill(target_array, target_array + get_size_in_bytes(), 0);
            return target_array;
        }

        GDALRasterBand *band = data->GetRasterBand(1);
        uint8_t *array = new uint8_t[get_size_in_bytes()];
        std::fill(array, array + get_size_in_bytes(), 0);

        error = band->RasterIO(
            GF_Read, clamped_pixel_offset_x, clamped_pixel_offset_y, usable_width, usable_height,
            array + remainder_y_top * destination_window_size_pixels + remainder_x_left,
            target_width, target_height, GDT_Byte, 0, destination_window_size_pixels,
            &rasterio_args);

        if (error < CE_Failure) { return array; }
    }

    return nullptr;
}

int GeoRaster::get_size_in_bytes() {
    int pixel_size = get_pixel_size_x() * get_pixel_size_y();

    if (format == BYTE) {
        return pixel_size;
    } else if (format == RF) {
        return pixel_size * 4; // 32-bit float
    } else if (format == RGBA) {
        return pixel_size * 4;
    } else if (format == RGB) {
        return pixel_size * 3;
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
    uint64_t *histogram = new uint64_t[256];

    // Initialize array
    for (int i = 0; i < 256; i++) {
        histogram[i] = 0;
    }

    uint8_t *array = reinterpret_cast<uint8_t *>(get_as_array());

    int array_size = 0;
    int step;

    // In the case of RGB and RGBA arrays, we only check the R value.
    // This is because the function likely doesn't make sense on real RGB(A) images such as
    // orthophotos, but BYTE images may present themselves as RGB images, e.g. in the case of
    // GeoPackage rasters.
    if (format == BYTE) {
        array_size = get_pixel_size_x() * get_pixel_size_y();
        step = 1;
    } else if (format == RGB) {
        array_size = get_pixel_size_x() * get_pixel_size_y() * 3;
        step = 3;
    } else if (format == RGBA) {
        array_size = get_pixel_size_x() * get_pixel_size_y() * 4;
        step = 4;
    }

    for (int i = 0; i < array_size; i += step) {
        histogram[array[i]]++;
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
