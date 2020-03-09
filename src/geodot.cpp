#include "geodot.h"
#include "RasterTileExtractor.h"

using namespace godot;

void Geodot::_register_methods() {
    register_method("_process", &Geodot::_process);
    register_method("get_time_passed", &Geodot::get_time_passed);
    register_method("reproject_to_webmercator", &Geodot::reproject_to_webmercator);
    register_method("save_tile_from_heightmap", &Geodot::save_tile_from_heightmap);
    register_method("get_image", &Geodot::get_image);
}

Geodot::Geodot() {
}

Geodot::~Geodot() {
    // add your cleanup here
}

void Geodot::_init() {
    // initialize any variables here
    time_passed = 0.0;
    load_mutex = Ref<Mutex>(Mutex::_new());
}

void Geodot::_process(float delta) {
    time_passed += delta;
}

float Geodot::get_time_passed() {
    return time_passed;
}

void Geodot::reproject_to_webmercator(String infile, String outfile) {
    RasterTileExtractor rte;

    rte.reproject_to_webmercator(infile.utf8().get_data(), outfile.utf8().get_data());
}

Ref<GeoImage> Geodot::get_image() {
    Ref<GeoImage> image = GeoImage::_new();

    return image;
}

union {
    float fval;
    int8_t bval[4];
} floatAsBytes;

Ref<ImageTexture> Geodot::save_tile_from_heightmap(String infile, float new_top_left_x, float new_top_left_y, float new_size, int img_size, int interpolation) {
    // We need to lock a mutex because the RasterTileExtractor seems not to be thread-safe (due to gdal?)
    load_mutex->lock();

    RasterTileExtractor rte;

    float *data = new float[sizeof(float) * img_size * img_size];
    rte.clip(infile.utf8().get_data(), new_top_left_x, new_top_left_y, new_size, img_size, interpolation, data);

    PoolByteArray pba;

    // Multiply by 4 since we want to put 32-float values into a byte array
    pba.resize(img_size * img_size * 4);

    load_mutex->unlock();

    int index = 0;

    // Put the raw image data into a PoolByteArray
    for (int y = 0; y < img_size; y++) {
        for (int x = 0; x < img_size; x++) {
            // We need to convert the float into 4 bytes because that's the format Godot expects
            floatAsBytes.fval = data[y * img_size + x];

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
    Image *img = Image::_new();
    img->create_from_data(img_size, img_size, false, Image::Format::FORMAT_RF, pba);

    // Create an ImageTexture wrapping the Image
    ImageTexture *imgTex = ImageTexture::_new();
    imgTex->set_storage(ImageTexture::STORAGE_RAW);

    // By default, the returned texture has the FILTER flag. Only if the interpolation method
    //  is nearest neighbor or one of the modal types, it is disabled, since we most likely
    //  want crisp textures then.
    int flag = ImageTexture::FLAG_FILTER;
    if (interpolation == INTERPOLATION::NEAREST
        || interpolation > 5) {
        flag = 0;
    }

    imgTex->create_from_image(Ref<Image>(img), flag);

    return Ref<ImageTexture>(imgTex);
}

// Geodot::GeoImage

GeoImage::GeoImage() {

}

GeoImage::~GeoImage() {

}

void GeoImage::_init() {
    // This is required - returning a Reference to a locally created GeoImage throws a segfault otherwise!
    init_ref();
}

void GeoImage::_register_methods() {
    register_method("test_print", &GeoImage::test_print);
}

void GeoImage::test_print() {
    Godot::print("Test");
}
