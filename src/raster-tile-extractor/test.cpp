

// TODO: As a proof-of-concept, this is currently a standalone executable. It should be turned into a library and called
//  from Godot.

#include <iostream>
#include "RasterTileExtractor.h"

int main() {
    RasterTileExtractor rte;

    // TODO: Only call this if the input file is not Webmercator already or if we haven't reprojected
    // rte.reproject_to_webmercator("data/25m_EU_clip.tif", "data/25m_EU_clip_webm.tif");

    float new_top_left_x = 1546670.0;
    float new_top_left_y = 5918250.0;
    float new_size = 50000.0;
    int img_size = 256;

    auto *result = new float[sizeof(float) * img_size * img_size];
    rte.clip("/home/retour/LandscapeLab/testdata/DGM_K_5m.tif", new_top_left_x, new_top_left_y, new_size, img_size, 0, result);

    std::cout << result[0] << std::endl;

    return 0;
}