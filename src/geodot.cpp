#include "geodot.h"
#include "RasterTileExtractor.h"

using namespace godot;

void Geodot::_register_methods() {
    register_method("_process", &Geodot::_process);
    register_method("get_time_passed", &Geodot::get_time_passed);
    register_method("reproject_to_webmercator", &Geodot::reproject_to_webmercator);
    register_method("save_tile_from_heightmap", &Geodot::save_tile_from_heightmap);
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

union {
    float fval;
    int8_t bval[4];
} floatAsBytes;

Ref<ImageTexture> Geodot::save_tile_from_heightmap(String infile, String outfile, float new_top_left_x, float new_top_left_y, float new_size, int img_size) {
    // We need to lock a mutex because the RasterTileExtractor seems not to be thread-safe (due to gdal?)
    load_mutex->lock();

    RasterTileExtractor rte;

    float *data = new float[sizeof(float) * img_size * img_size];
    rte.clip(infile.utf8().get_data(), outfile.utf8().get_data(), new_top_left_x, new_top_left_y, new_size, img_size, data);
    
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
    imgTex->create_from_image(Ref<Image>(img), ImageTexture::FLAG_FILTER);

    return Ref<ImageTexture>(imgTex);
}
