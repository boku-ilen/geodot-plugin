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

void Geodot::save_tile_from_heightmap(String infile, String outfile, float new_top_left_x, float new_top_left_y, float new_size, int img_size) {
    RasterTileExtractor rte;

    rte.clip(infile.utf8().get_data(), outfile.utf8().get_data(), new_top_left_x, new_top_left_y, new_size, img_size);
}
