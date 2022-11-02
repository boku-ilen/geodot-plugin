#include "geoimage.h"

#include <algorithm>

using namespace godot;

// Geodot::GeoImage

GeoImage::GeoImage() {
    normalmap_load_mutex.instantiate();
}

GeoImage::~GeoImage() {
    // delete raster;
}

void GeoImage::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_image"), &GeoImage::get_image);
    ClassDB::bind_method(D_METHOD("get_image_texture"), &GeoImage::get_image_texture);
    ClassDB::bind_method(D_METHOD("get_most_common", "number_of_entries"),
                         &GeoImage::get_most_common);
    ClassDB::bind_method(D_METHOD("get_normalmap_for_heightmap", "scale"),
                         &GeoImage::get_normalmap_for_heightmap);
    ClassDB::bind_method(D_METHOD("get_normalmap_texture_for_heightmap", "scale"),
                         &GeoImage::get_normalmap_texture_for_heightmap);
    ClassDB::bind_method(D_METHOD("get_shape_for_heightmap"), &GeoImage::get_shape_for_heightmap);
    ClassDB::bind_method(D_METHOD("is_valid"), &GeoImage::is_valid);
}

bool GeoImage::is_valid() {
    return validity;
}

void GeoImage::set_raster(GeoRaster *raster, int interpolation) {
    this->raster = raster;
    this->interpolation = interpolation;

    int size = raster->get_size_in_bytes();

    PackedByteArray pba;

    // Multiply by 4 since we want to put 32-float values into a byte array
    pba.resize(size);

    int index = 0;
    int img_size_x = raster->get_pixel_size_x();
    int img_size_y = raster->get_pixel_size_y();

    image.instantiate();

    // Depending on the data type, the insertion of the raw image into the
    // PoolByteArray is different. The format is dependent on how Godot handles
    // Image->create_from_data.
    GeoRaster::FORMAT format = raster->get_format();

    if (format == GeoRaster::RGB) {
        uint8_t *data = (uint8_t *)raster->get_as_array();

        if (data == nullptr) return;

        // This copy is straightforward since the result if
        // ImageRaster::get_data is already in the correct format. Format of the
        // PoolByteArray: (RGB)(RGB)(RGB)...
        for (int i = 0; i < img_size_x * img_size_y * 3; i++) {
            pba.set(i, data[i]);
        }

        // All content of data is now in pba, so we can delete it
        delete[] data;

        // Create an image from the PoolByteArray
        image->create_from_data(img_size_x, img_size_y, false, Image::Format::FORMAT_RGB8, pba);
    } else if (format == GeoRaster::RGBA) {
        uint8_t *data = (uint8_t *)raster->get_as_array();

        if (data == nullptr) return;

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
        image->create_from_data(img_size_x, img_size_y, false, Image::Format::FORMAT_RGBA8, pba);
    } else if (format == GeoRaster::BYTE) {
        uint8_t *data = (uint8_t *)raster->get_as_array();

        if (data == nullptr) return;

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
        image->create_from_data(img_size_x, img_size_y, false, Image::Format::FORMAT_R8, pba);
    } else if (format == GeoRaster::RF) {
        float *data = (float *)raster->get_as_array();

        if (data == nullptr) return;

        // Convert the float into 4 bytes and add those to the array
        // Format of the PoolByteArray: (F1F2F3F4)(F1F2F3F4)(F1F2F3F4)...
        union {
            float fval;
            int8_t bval[4];
        } floatAsBytes;

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
        image->create_from_data(img_size_x, img_size_y, false, Image::Format::FORMAT_RF, pba);
    }

    validity = true;
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

    Ref<Image> img;
    img.instantiate();

    if (normalmap == nullptr) {
        // As described in https://github.com/godotengine/godot/issues/35539,
        // locking the image is not thread-safe in the way one would expect.
        // Thus, to be safe, we work on a duplicate of the `image` here.
        Ref<Image> image = this->image->duplicate();

        PackedByteArray heightmap_data = image->get_data();

        PackedByteArray normalmap_data;

        int width = image->get_width();
        int height = image->get_height();
        normalmap_data.resize(width * height * 4); // RGBA

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

                normalmap_data.set(xy_to_index(full_x, full_y, width, height) * 4 + 0,
                                   127.5 + normal.x * 127.5);
                normalmap_data.set(xy_to_index(full_x, full_y, width, height) * 4 + 1,
                                   127.5 + normal.y * 127.5);
                normalmap_data.set(xy_to_index(full_x, full_y, width, height) * 4 + 2,
                                   127.5 + normal.z * 127.5);
                normalmap_data.set(xy_to_index(full_x, full_y, width, height) * 4 + 3, 255);
            }
        }

        img->create_from_data(width, height, false, Image::Format::FORMAT_RGBA8, normalmap_data);
    }

    normalmap_load_mutex->unlock();

    return img;
}

Ref<HeightMapShape3D> GeoImage::get_shape_for_heightmap() {
    Ref<HeightMapShape3D> shape;
    shape.instantiate();

    if (raster->get_format() == GeoRaster::RF) {
        float *data = (float *)raster->get_as_array();

        if (data == nullptr) return shape;

        PackedFloat32Array array;
        array.resize(raster->get_pixel_size_x() * raster->get_pixel_size_y());

        // Copy all bytes from the raw raster array into the PackedFloat32Array
        memcpy(array.ptrw(), data, raster->get_pixel_size_x() * raster->get_pixel_size_y() * 4);

        shape->set_map_width(raster->get_pixel_size_x());
        shape->set_map_depth(raster->get_pixel_size_y());
        shape->set_map_data(array);
    }

    return shape;
}

Ref<ImageTexture> GeoImage::get_normalmap_texture_for_heightmap(float scale) {
    return ImageTexture::create_from_image(get_normalmap_for_heightmap(scale));
}

Ref<ImageTexture> GeoImage::get_image_texture() {
    return ImageTexture::create_from_image(image);
}

Array GeoImage::get_most_common(int number_of_entries) {
    int *most_common = raster->get_most_common(number_of_entries);
    Array ret_array = Array();

    // Translate C-style array to Godot Array
    for (int i = 0; i < number_of_entries; i++) {
        if (most_common[i] == 0)
            continue; // An entry of 0 means that nothing with this index was found, so we're done

        ret_array.append(most_common[i]);
    }

    return ret_array;
}