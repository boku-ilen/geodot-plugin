#ifndef GEODOT_H
#define GEODOT_H

#include <Godot.hpp>
#include <Node.hpp>
#include <Image.hpp>
#include <ImageTexture.hpp>

namespace godot {

class Geodot : public Node {
    GODOT_CLASS(Geodot, Node)

private:
    float time_passed;

public:
    static void _register_methods();

    Geodot();
    ~Geodot();

    void _init(); // our initializer called by Godot

    void _process(float delta);
    
    float get_time_passed();

    void reproject_to_webmercator(String infile, String outfile);

    Ref<ImageTexture> save_tile_from_heightmap(String infile, String outfile, float new_top_left_x, float new_top_left_y, float new_size, int img_size) const;
};

}

#endif
