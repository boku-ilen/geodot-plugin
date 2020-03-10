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
    register_method("get_histogram", &GeoImage::get_histogram);
}

void GeoImage::set_raster(GeoRaster *raster, int interpolation) {
    this->raster = raster;
    this->interpolation = interpolation;

    int size = raster->get_size_in_bytes();

    float *data = (float *)raster->get_as_array(); // TODO: Make it work for other types
    PoolByteArray pba;

    // Multiply by 4 since we want to put 32-float values into a byte array
    pba.resize(size);

    int index = 0;
    int img_size_x = raster->get_pixel_size_x();
    int img_size_y = raster->get_pixel_size_y();

    // Put the raw image data into a PoolByteArray
    for (int y = 0; y < img_size_y; y++) {
        for (int x = 0; x < img_size_x; x++) {
            // We need to convert the float into 4 bytes because that's the format Godot expects
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
    Image *img = Image::_new();
    img->create_from_data(img_size_x, img_size_y, false, Image::Format::FORMAT_RF, pba);

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

Array GeoImage::get_histogram(int number_of_entries) {
    return Array(); // TODO
}
