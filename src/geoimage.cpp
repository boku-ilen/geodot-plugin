#include "geoimage.h"
#include "GeoRaster.h"

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

    BIND_ENUM_CONSTANT(AVG);
    BIND_ENUM_CONSTANT(BILINEAR);
    BIND_ENUM_CONSTANT(CUBIC);
    BIND_ENUM_CONSTANT(CUBICSPLINE);
    BIND_ENUM_CONSTANT(LANCZOS);
    BIND_ENUM_CONSTANT(MAX);
    BIND_ENUM_CONSTANT(MED);
    BIND_ENUM_CONSTANT(MIN);
    BIND_ENUM_CONSTANT(MODE);
    BIND_ENUM_CONSTANT(NEAREST);
    BIND_ENUM_CONSTANT(Q1);
    BIND_ENUM_CONSTANT(Q2);
}

bool GeoImage::is_valid() {
    return validity;
}

void GeoImage::set_raster(GeoRaster *raster, INTERPOLATION interpolation) {
    this->raster = raster;
    this->interpolation = interpolation;

    int size = raster->get_size_in_bytes();

    PackedByteArray pba;

    pba.resize(size);

    int index = 0;
    int img_size_x = raster->get_pixel_size_x();
    int img_size_y = raster->get_pixel_size_y();

    // Set variables depending on the data type
    GeoRaster::FORMAT format = raster->get_format();
    Image::Format image_format;

    if (format == GeoRaster::RF) {
        image_format = Image::FORMAT_RF;
    } else if (format == GeoRaster::BYTE) {
        image_format = Image::FORMAT_R8;
    } else if (format == GeoRaster::RGB) {
        image_format = Image::FORMAT_RGB8;
    } else if (format == GeoRaster::RGBA) {
        image_format = Image::FORMAT_RGBA8;
    } else {
        // We can't handle this type
        return;
    }

    uint8_t *data = (uint8_t *)raster->get_as_array();
    if (data == nullptr) return;

    // Reserve space in the PBA and directly copy the array to its pointer
    memcpy((void*)pba.ptr(), data, size);

    // All content of data is now in pba, so we can delete it
    delete[] data;

    // Create an image from the PoolByteArray
    image = Image::create_from_data(img_size_x, img_size_y, false, image_format, pba);
    
    validity = true;
}

void GeoImage::set_raster_from_band(GeoRaster *raster, INTERPOLATION interpolation, int band_index) {
    this->raster = raster;
    this->interpolation = interpolation;
    int size = raster->get_pixel_size_x() * raster->get_pixel_size_y();
    GeoRaster::FORMAT band_format = raster->get_band_format(band_index);
    if (band_format == GeoRaster::RF) {
        // Only two types currently supported at the moment FLOAT and BYTE
        // and the former is 4 x bigger than the latter.
        size *= 4;
    }

    PackedByteArray pba;

    // Multiply by 4 since we want to put 32-float values into a byte array
    pba.resize(size);

    int index = 0;
    int img_size_x = raster->get_pixel_size_x();
    int img_size_y = raster->get_pixel_size_y();

    // Set variables depending on the data type
    Image::Format image_format;

    if (band_format == GeoRaster::RF) {
        image_format = Image::FORMAT_RF;
    } else if (band_format == GeoRaster::BYTE) {
        image_format = Image::FORMAT_R8;
    } else {
        // We can't handle this type
        return;
    }

    uint8_t *data = (uint8_t *)raster->get_as_array();
    if (data == nullptr) return;

    // Reserve space in the PBA and directly copy the array to its pointer
    memcpy((void*)pba.ptr(), data, size);

    // All content of data is now in pba, so we can delete it
    delete[] data;

    // Create an image from the PoolByteArray
    image = Image::create_from_data(img_size_x, img_size_y, false, image_format, pba);

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

        img = Image::create_from_data(width, height, false, Image::Format::FORMAT_RGBA8,
                                      normalmap_data);
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

#ifdef REAL_T_IS_DOUBLE
        PackedFloat64Array array;

        for (int i; i < raster->get_pixel_size_x() * raster->get_pixel_size_y(); i++) {
            array.append(data[i]);
        }
#else
        PackedFloat32Array array;

        array.resize(raster->get_pixel_size_x() * raster->get_pixel_size_y());

        // Copy all bytes from the raw raster array into the PackedFloat32Array
        memcpy(array.ptrw(), data, raster->get_pixel_size_x() * raster->get_pixel_size_y() * 4);
#endif

        delete[] data;

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

    delete[] most_common;

    return ret_array;
}