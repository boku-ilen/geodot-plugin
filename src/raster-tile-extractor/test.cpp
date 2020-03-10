// Used for testing the libRasterTileExtractor library.

#include <iostream>
#include "RasterTileExtractor.h"

int main() {
    RasterTileExtractor::initialize();

    float new_top_left_x = 1546670.0;
    float new_top_left_y = 5918250.0;
    float new_size = 38000.0;
    int img_size = 256;

    GeoRaster *raster = RasterTileExtractor::get_raster_at_position("/home/retour/LandscapeLab/testdata/bmaporthophoto30cm", "jpg", new_top_left_x, new_top_left_y, new_size, img_size, 0);
    std::cout << raster->get_format() << std::endl;

    return 0;
}