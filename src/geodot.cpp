#include "geodot.h"
#include "RasterTileExtractor.h"

using namespace godot;

void Geodot::_register_methods() {
    register_method("get_image", &Geodot::get_image);
}

Geodot::Geodot() {
}

Geodot::~Geodot() {
}

void Geodot::_init() {
    RasterTileExtractor::initialize();
    load_mutex = Ref<Mutex>(Mutex::_new());
}

Ref<GeoImage> Geodot::get_image(String path, String file_ending,
                                double top_left_x, double top_left_y, double size_meters,
                                int img_size, int interpolation_type) {
    Ref<GeoImage> image = GeoImage::_new();

    image->set_raster(RasterTileExtractor::get_raster_at_position(
        path.utf8().get_data(),
        file_ending.utf8().get_data(),
        top_left_x, top_left_y, size_meters,
        img_size, interpolation_type), interpolation_type);

    return image;
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
}

void GeoImage::_register_methods() {
    register_method("get_image", &GeoImage::get_image);
    register_method("get_image_texture", &GeoImage::get_image_texture);
    register_method("get_most_common", &GeoImage::get_most_common);
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
