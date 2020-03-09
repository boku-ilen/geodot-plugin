#ifndef GEODOT_H
#define GEODOT_H

#include <Godot.hpp>
#include <Node.hpp>
#include <Image.hpp>
#include <Mutex.hpp>
#include <Resource.hpp>
#include <ImageTexture.hpp>
#include <Array.hpp>

namespace godot {

class GeoImage : public Resource {
    GODOT_CLASS(GeoImage, Resource)

public:
    GeoImage();
    ~GeoImage();

    // Create a new GeoImage with a GeoTIF, position and size

    // Create a new GeoImage with a pre-tiled pyramid path, position and LOD

    void _init();

    static void _register_methods();

    void test_print();

    Ref<Image> get_image();

    Ref<ImageTexture> get_image_texture();

    Array get_histogram(int number_of_entries);
};

class Geodot : public Node {
    GODOT_CLASS(Geodot, Node)

private:
    float time_passed;

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

    Ref<GeoImage> get_image();

    Geodot();
    ~Geodot();

    void _init(); // our initializer called by Godot

    void _process(float delta);

    float get_time_passed();

    void reproject_to_webmercator(String infile, String outfile);

    Ref<ImageTexture> save_tile_from_heightmap(String infile, float new_top_left_x, float new_top_left_y, float new_size, int img_size, int interpolation);
};

}

#endif
