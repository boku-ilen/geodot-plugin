#include <iostream>
#include "VectorExtractor.h"

int main() {
    LineFeature *line = VectorExtractor::get_lines_near_position("/home/karl/BOKU/geodata/streets/2_linknetz_ogd_WM.shp", 1755382.5, 6156320.8, 1000.0, 10).front();

    if (line) {
        std::cout << "Length: " << line->get_attribute("LENGTH") << std::endl;

        std::cout << "Points:" << std::endl;

        int point_count = line->get_point_count();

        std::cout << point_count << " points" << std::endl;

        for (int i = 0; i < point_count; i++) {
            std::cout << line->get_line_point_x(i) << ", " << line->get_line_point_y(i) << ", " << line->get_line_point_z(i)
                      << std::endl;
        }
    }

    return 0;
}
