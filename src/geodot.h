#ifndef GEODOT_H
#define GEODOT_H

#include <Godot.hpp>
#include <Node.hpp>
#include <Image.hpp>
#include <Mutex.hpp>
#include <Resource.hpp>
#include <ImageTexture.hpp>
#include <Array.hpp>
#include "GeoRaster.h"
#include "RasterTileExtractor.h"

namespace godot {

// Wrapper for a RasterTileExtractor GDALDataset.
class GeoImage : public Resource {
    GODOT_CLASS(GeoImage, Resource)

public:
    GeoImage();
    ~GeoImage();

    void _init();

    static void _register_methods();

    void set_raster(GeoRaster *raster, int interpolation);

    Ref<Image> get_image();

    Ref<ImageTexture> get_image_texture();

    Array get_most_common(int number_of_entries);

private:
    GeoRaster *raster;

    Ref<Image> image;

    int interpolation;
};

class Geodot : public Node {
    GODOT_CLASS(Geodot, Node)

private:
    Ref<Mutex> load_mutex;

public:
    // TODO: Not exportable? https://github.com/godotengine/godot/issues/15922
    enum INTERPOLATION {
        NEAREST,
        BILINEAR,
        CUBIC,
        CUBICSPLINE,
        LANCZOS,
        AVG,
        MODE,
        MAX,
        MIN,
        MED,
        Q1,
        Q2,
    };

    static void _register_methods();

    Ref<GeoImage> get_image(String path, String file_ending,
                            double top_left_x, double top_left_y, double size_meters,
                            int img_size, int interpolation_type);

    Geodot();
    ~Geodot();

    void _init(); // our initializer called by Godot
};

}

#endif
