

// TODO: As a proof-of-concept, this is currently a standalone executable. It should be turned into a library and called
//  from Godot.

#include "RasterTileExtractor.h"

int main() {
    RasterTileExtractor rte;

    // TODO: Only call this if the input file is not Webmercator already or if we haven't reprojected
    rte.reproject_to_webmercator("data/25m_EU_clip.tif", "data/25m_EU_clip_webm.tif");

    float new_top_left_x = 1470287.0;
    float new_top_left_y = 6013574.0;
    float new_size = 500000.0;
    int img_size = 256;

    rte.clip("data/25m_EU_clip_webm.tif", "data/25m_EU_clip_webm_tile.tif", new_top_left_x, new_top_left_y, new_size, img_size);

    return 0;
}