#include "geoimage.h"

#include <algorithm>

using namespace godot;

// Geodot::GeoImage

GeoImage::~GeoImage() {
    delete raster;
}

void GeoImage::_register_methods() {
    register_method("get_image", &GeoImage::get_image);
    register_method("get_image_texture", &GeoImage::get_image_texture);
    register_method("get_most_common", &GeoImage::get_most_common);
    register_method("get_normalmap_for_heightmap",
                    &GeoImage::get_normalmap_for_heightmap);
    register_method("get_normalmap_texture_for_heightmap",
                    &GeoImage::get_normalmap_texture_for_heightmap);
}

union {
    float fval;
    int8_t bval[4];
} floatAsBytes;

void GeoImage::set_raster(GeoRaster *raster, int interpolation) {
    this->raster = raster;
    this->interpolation = interpolation;

    int size = raster->get_size_in_bytes();

    PoolByteArray pba;

    // Multiply by 4 since we want to put 32-float values into a byte array
    pba.resize(size);

    int index = 0;
    int img_size_x = raster->get_pixel_size_x();
    int img_size_y = raster->get_pixel_size_y();

    image.instance();

    // Depending on the data type, the insertion of the raw image into the
    // PoolByteArray is different. The format is dependent on how Godot handles
    // Image->create_from_data.
    GeoRaster::FORMAT format = raster->get_format();

    if (format == GeoRaster::RGB) {
        uint8_t *data = (uint8_t *)raster->get_as_array();

        // This copy is straightforward since the result if
        // ImageRaster::get_data is already in the correct format. Format of the
        // PoolByteArray: (RGB)(RGB)(RGB)...
        for (int i = 0; i < img_size_x * img_size_y * 3; i++) {
            pba.set(i, data[i]);
        }

        // All content of data is now in pba, so we can delete it
        delete[] data;

        // Create an image from the PoolByteArray
        image->create_from_data(img_size_x, img_size_y, false,
                                Image::Format::FORMAT_RGB8, pba);
    } else if (format == GeoRaster::RGBA) {
        uint8_t *data = (uint8_t *)raster->get_as_array();

        // Copy each of the 4 bands
        // Format of the PoolByteArray: (RGBA)(RGBA)(RGBA)...
        for (int y = 0; y < img_size_y; y++) {
            for (int x = 0; x < img_size_x; x++) {
                int rgba_index = y * img_size_x + x;
                pba.set(index++, data[rgba_index * 4 + 0]);
                pba.set(index++, data[rgba_index * 4 + 1]);
                pba.set(index++, data[rgba_index * 4 + 2]);
                pba.set(index++, data[rgba_index * 4 + 3]);
            }
        }

        // All content of data is now in pba, so we can delete it
        delete[] data;

        // Create an image from the PoolByteArray
        image->create_from_data(img_size_x, img_size_y, false,
                                Image::Format::FORMAT_RGBA8, pba);
    } else if (format == GeoRaster::BYTE) {
        uint8_t *data = (uint8_t *)raster->get_as_array();

        // 1:1 copy
        // Format of the PoolByteArray: (B)(B)(B)...
        for (int y = 0; y < img_size_y; y++) {
            for (int x = 0; x < img_size_x; x++) {
                // We need to convert the float into 4 bytes because that's the
                // format Godot expects
                pba.set(index++, data[y * img_size_x + x]);
            }
        }

        // All content of data is now in pba, so we can delete it
        delete[] data;

        // Create an image from the PoolByteArray
        image->create_from_data(img_size_x, img_size_y, false,
                                Image::Format::FORMAT_R8, pba);
    } else if (format == GeoRaster::RF) {
        float *data = (float *)raster->get_as_array();

        // Convert the float into 4 bytes and add those to the array
        // Format of the PoolByteArray: (F1F2F3F4)(F1F2F3F4)(F1F2F3F4)...
        for (int y = 0; y < img_size_y; y++) {
            for (int x = 0; x < img_size_x; x++) {
                floatAsBytes.fval = data[y * img_size_x + x];

                pba.set(index, floatAsBytes.bval[0]);
                pba.set(++index, floatAsBytes.bval[1]);
                pba.set(++index, floatAsBytes.bval[2]);
                pba.set(++index, floatAsBytes.bval[3]);

                index++;
            }
        }

        // All content of data is now in pba, so we can delete it
        delete[] data;

        // Create an image from the PoolByteArray
        image->create_from_data(img_size_x, img_size_y, false,
                                Image::Format::FORMAT_RF, pba);
    }
}

Ref<Image> GeoImage::get_image() {
    return image;
}

int xy_to_index(int x, int y, int width, int height) {
    x = std::clamp(x, 0, width - 1);
    y = std::clamp(y, 0, height - 1);

    return y * width + x;
}

Ref<Image> GeoImage::get_normalmap_for_heightmap(float scale) {
    normalmap_load_mutex->lock();

    if (normalmap == nullptr) {
        // As described in https://github.com/godotengine/godot/issues/35539,
        // locking the image is not thread-safe in the way one would expect.
        // Thus, to be safe, we work on a duplicate of the `image` here.
        Ref<Image> image = this->image->duplicate();

        Image *img = Image::_new();

        PoolByteArray heightmap_data = image->get_data();

        PoolByteArray normalmap_data;

        int width = image->get_width();
        int height = image->get_height();
        normalmap_data.resize(width * height * 4); // RGBA

        image->lock();

        for (int full_y = 0; full_y < height; full_y++) {
            for (int full_x = 0; full_x < width; full_x++) {
                // Prevent the edges from having flat normals by using the
                // closest valid normal
                int x = std::clamp(full_x, 1, width - 2);
                int y = std::clamp(full_y, 1, height - 2);

                // Sobel filter for getting the normal at this position
                float bottom_left = image->get_pixel(x + 1, y + 1).r;
                float bottom_center = image->get_pixel(x, y + 1).r;
                float bottom_right = image->get_pixel(x - 1, y + 1).r;

                float center_left = image->get_pixel(x + 1, y).r;
                float center_center = image->get_pixel(x, y).r;
                float center_right = image->get_pixel(x - 1, y).r;

                float top_left = image->get_pixel(x + 1, y - 1).r;
                float top_center = image->get_pixel(x, y - 1).r;
                float top_right = image->get_pixel(x - 1, y - 1).r;

                Vector3 normal;

                normal.x = (top_right + 2.0 * center_right + bottom_right) -
                           (top_left + 2.0 * center_left + bottom_left);
                normal.y = (bottom_left + 2.0 * bottom_center + bottom_right) -
                           (top_left + 2.0 * top_center + top_right);
                normal.z = 1.0 / scale;

                normal.normalize();

                normalmap_data.set(
                    xy_to_index(full_x, full_y, width, height) * 4 + 0,
                    127.5 + normal.x * 127.5);
                normalmap_data.set(
                    xy_to_index(full_x, full_y, width, height) * 4 + 1,
                    127.5 + normal.y * 127.5);
                normalmap_data.set(
                    xy_to_index(full_x, full_y, width, height) * 4 + 2,
                    127.5 + normal.z * 127.5);
                normalmap_data.set(
                    xy_to_index(full_x, full_y, width, height) * 4 + 3, 255);
            }
        }

        image->unlock();

        img->create_from_data(width, height, false, Image::Format::FORMAT_RGBA8,
                              normalmap_data);

        normalmap = Ref<Image>(img);
    }

    normalmap_load_mutex->unlock();

    return normalmap;
}

Ref<ImageTexture> GeoImage::get_normalmap_texture_for_heightmap(float scale) {
    Ref<Image> heightmap_image = get_normalmap_for_heightmap(scale);

    // Create an ImageTexture wrapping the Image
    Ref<ImageTexture> imgTex;
    imgTex.instance();

    imgTex->set_storage(ImageTexture::STORAGE_RAW);
    imgTex->create_from_image(heightmap_image, ImageTexture::FLAG_FILTER);

    return Ref<ImageTexture>(imgTex);
}

Ref<ImageTexture> GeoImage::get_image_texture() {
    // Create an ImageTexture wrapping the Image
    Ref<ImageTexture> imgTex;
    imgTex.instance();

    imgTex->set_storage(ImageTexture::STORAGE_RAW);

    // By default, the returned texture has the FILTER flag. Only if the
    // interpolation method
    //  is nearest neighbor or one of the modal types, it is disabled, since we
    //  most likely want crisp textures then.
    int flag = ImageTexture::FLAG_FILTER;
    if (interpolation == INTERPOLATION::NEAREST || interpolation > 5) {
        flag = 0;
    }

    imgTex->create_from_image(Ref<Image>(image), flag);

    return Ref<ImageTexture>(imgTex);
}

Array GeoImage::get_most_common(int number_of_entries) {
    int *most_common = raster->get_most_common(number_of_entries);
    Array ret_array = Array();
    ret_array.resize(number_of_entries);

    // Translate C-style array to Godot Array
    for (int i = 0; i < number_of_entries; i++) {
        ret_array[i] = most_common[i];
    }

    return ret_array;
}