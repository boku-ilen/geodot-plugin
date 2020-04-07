#include "geodot.h"
#include "RasterTileExtractor.h"
#include <algorithm> // For std::clamp
#include <functional> // For std::hash

using namespace godot;

void Geodot::_register_methods() {
    register_method("get_image", &Geodot::get_image);
    register_method("get_lines_near_position", &Geodot::get_lines_near_position);
    register_method("crop_lines_to_square", &Geodot::crop_lines_to_square);
}

Geodot::Geodot() {
}

Geodot::~Geodot() {
}

void Geodot::_init() {
    RasterTileExtractor::initialize();
    VectorExtractor::initialize();
    load_mutex = Ref<Mutex>(Mutex::_new());
    image_cache = Dictionary();
}

Ref<GeoImage> Geodot::get_image(String path, String file_ending,
                                double top_left_x, double top_left_y, double size_meters,
                                int img_size, int interpolation_type) {
    load_mutex->lock();

    int image_hash = std::hash<std::string>()(std::string(path.utf8().get_data()) + std::string(file_ending.utf8().get_data()) + std::to_string(top_left_x) + std::to_string(top_left_y) + std::to_string(size_meters) + std::to_string(img_size) + std::to_string(interpolation_type));

    if (image_cache.has(image_hash)) {
        load_mutex->unlock();
        return image_cache[image_hash];
    } else {
        // This strange __internal_constructor call is required to prevent a memory leak
        // See https://github.com/GodotNativeTools/godot-cpp/issues/215
        Ref<GeoImage> image = Ref<GeoImage>::__internal_constructor(GeoImage::_new());

        GeoRaster *raster = RasterTileExtractor::get_raster_at_position(
            path.utf8().get_data(),
            file_ending.utf8().get_data(),
            top_left_x, top_left_y, size_meters,
            img_size, interpolation_type);

        load_mutex->unlock();

        if (raster == nullptr) {
            Godot::print_error("No valid data was available for the requested path and position!", "Geodot::get_image", "geodot.cpp", 26);
            return image;
        }

        image->set_raster(raster, interpolation_type);

        image_cache[image_hash] = image;
        load_mutex->unlock();

        return image;
    }
}

Array Geodot::get_lines_near_position(String path, double pos_x, double pos_y, double radius, int max_lines) {
    Array lines = Array();

    std::list<LineFeature *> linefeatures = VectorExtractor::get_lines_near_position(path.utf8().get_data(), pos_x, pos_y, radius, max_lines);

    for (LineFeature *linefeature : linefeatures) {
        Ref<GeoLine> line = GeoLine::_new();
        line->set_line(linefeature);

        lines.push_back(line);
    }

    return lines;
}

Array Geodot::crop_lines_to_square(String path, double top_left_x, double top_left_y, double size_meters, int max_lines) {
    Array lines = Array();

    std::list<LineFeature *> linefeatures = VectorExtractor::crop_lines_to_square(path.utf8().get_data(), top_left_x, top_left_y, size_meters, max_lines);

    for (LineFeature *linefeature : linefeatures) {
        Ref<GeoLine> line = GeoLine::_new();
        line->set_line(linefeature);

        lines.push_back(line);
    }

    return lines;
}

union {
    float fval;
    int8_t bval[4];
} floatAsBytes;


// Geodot::GeoImage

GeoImage::GeoImage() {

}

GeoImage::~GeoImage() {
    delete raster;
}

void GeoImage::_init() {
    // This is required - returning a Reference to a locally created GeoImage throws a segfault otherwise!
    init_ref();

    normalmap_load_mutex = Ref<Mutex>(Mutex::_new());
}

void GeoImage::_register_methods() {
    register_method("get_image", &GeoImage::get_image);
    register_method("get_image_texture", &GeoImage::get_image_texture);
    register_method("get_most_common", &GeoImage::get_most_common);
    register_method("get_normalmap_for_heightmap", &GeoImage::get_normalmap_for_heightmap);
    register_method("get_normalmap_texture_for_heightmap", &GeoImage::get_normalmap_texture_for_heightmap);
}

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

    Image *img = Image::_new();

    // Depending on the data type, the insertion of the raw image into the PoolByteArray is different.
    // The format is dependent on how Godot handles Image->create_from_data.
    GeoRaster::FORMAT format = raster->get_format();

    if (format == GeoRaster::RGB) {
        uint8_t *data = (uint8_t *)raster->get_as_array();

        // This copy is straightforward since the result if ImageRaster::get_data is already in the correct format.
        // Format of the PoolByteArray: (RGB)(RGB)(RGB)...
        for (int i = 0; i < img_size_x * img_size_y * 3; i++) {
            pba.set(i, data[i]);
        }

        // All content of data is now in pba, so we can delete it
        delete[] data;

        // Create an image from the PoolByteArray
        img->create_from_data(img_size_x, img_size_y, false, Image::Format::FORMAT_RGB8, pba);
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
        img->create_from_data(img_size_x, img_size_y, false, Image::Format::FORMAT_RGBA8, pba);
    } else if (format == GeoRaster::BYTE) {
        uint8_t *data = (uint8_t *)raster->get_as_array();

        // 1:1 copy
        // Format of the PoolByteArray: (B)(B)(B)...
        for (int y = 0; y < img_size_y; y++) {
            for (int x = 0; x < img_size_x; x++) {
                // We need to convert the float into 4 bytes because that's the format Godot expects
                pba.set(index++, data[y * img_size_x + x]);
            }
        }

        // All content of data is now in pba, so we can delete it
        delete[] data;

        // Create an image from the PoolByteArray
        img->create_from_data(img_size_x, img_size_y, false, Image::Format::FORMAT_R8, pba);
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
        img->create_from_data(img_size_x, img_size_y, false, Image::Format::FORMAT_RF, pba);
    }

    image = Ref<Image>(img);
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
        Image *img = Image::_new();

        PoolByteArray heightmap_data = image->get_data();

        PoolByteArray normalmap_data;

        int width = image->get_width();
        int height = image->get_height();
        normalmap_data.resize(width * height * 4); // RGBA

        image->lock();

        for (int full_y = 0; full_y < height; full_y++) {
            for (int full_x = 0; full_x < width; full_x++) {
                // Prevent the edges from having flat normals by using the closest
                // valid normal
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

            	normal.x = (top_right + 2.0 * center_right + bottom_right) - (top_left + 2.0 * center_left + bottom_left);
            	normal.y = (bottom_left + 2.0 * bottom_center + bottom_right) - (top_left + 2.0 * top_center + top_right);
            	normal.z = 1.0 / scale;

                normal.normalize();

                normalmap_data.set(xy_to_index(full_x, full_y, width, height) * 4 + 0, 127.5 + normal.x * 127.5);
                normalmap_data.set(xy_to_index(full_x, full_y, width, height) * 4 + 1, 127.5 + normal.y * 127.5);
                normalmap_data.set(xy_to_index(full_x, full_y, width, height) * 4 + 2, 127.5 + normal.z * 127.5);
                normalmap_data.set(xy_to_index(full_x, full_y, width, height) * 4 + 3, 255);
            }
        }

        image->unlock();

        img->create_from_data(width, height, false, Image::Format::FORMAT_RGBA8, normalmap_data);

        normalmap = Ref<Image>(img);
    }

    normalmap_load_mutex->unlock();

    return normalmap;
}

Ref<ImageTexture> GeoImage::get_normalmap_texture_for_heightmap(float scale) {
    // Create an ImageTexture wrapping the Image
    ImageTexture *imgTex = ImageTexture::_new();
    imgTex->set_storage(ImageTexture::STORAGE_RAW);
    imgTex->create_from_image(get_normalmap_for_heightmap(scale), ImageTexture::FLAG_FILTER);

    return Ref<ImageTexture>(imgTex);
}

Ref<ImageTexture> GeoImage::get_image_texture() {
    // Create an ImageTexture wrapping the Image
    ImageTexture *imgTex = ImageTexture::_new();
    imgTex->set_storage(ImageTexture::STORAGE_RAW);

    // By default, the returned texture has the FILTER flag. Only if the interpolation method
    //  is nearest neighbor or one of the modal types, it is disabled, since we most likely
    //  want crisp textures then.
    int flag = ImageTexture::FLAG_FILTER;
    if (interpolation == Geodot::INTERPOLATION::NEAREST
        || interpolation > 5) {
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


// Geodot::GeoLine

GeoLine::GeoLine() {

}

GeoLine::~GeoLine() {
    delete line;
}

void GeoLine::_init() {
    init_ref();
}

void GeoLine::_register_methods() {
    register_method("get_attribute", &GeoLine::get_attribute);
    register_method("get_curve3d", &GeoLine::get_curve3d);
    register_method("get_offset_curve3d", &GeoLine::get_offset_curve3d);
}

void GeoLine::set_line(LineFeature *line) {
    this->line = line;
}

String GeoLine::get_attribute(String name) {
    return line->get_attribute(name.utf8().get_data());
}

Ref<Curve3D> GeoLine::get_offset_curve3d(int offset_x, int offset_y, int offset_z) {
    Ref<Curve3D> curve = Curve3D::_new();

    int point_count = line->get_point_count();

    for (int i = 0; i < point_count; i++) {
        // Note: y and z are swapped because of differences in the coordinate system!
        double x = line->get_line_point_x(i) + static_cast<double>(offset_x);
        double y = line->get_line_point_z(i) + static_cast<double>(offset_y);
        double z = -(line->get_line_point_y(i) + static_cast<double>(offset_z));

        curve->add_point(Vector3(x, y, z));
    }

    return curve;
}

Ref<Curve3D> GeoLine::get_curve3d() {
    return get_offset_curve3d(0, 0, 0);
}
