#include <iostream>
#include "VectorExtractor.h"

int main() {
    VectorExtractor::initialize();

    std::list<LineFeature *> lines = VectorExtractor::get_lines_near_position(
            "/home/karl/BOKU/geodata/streets/2_linknetz_ogd_WM.shp", 1546670.5, 5918250.8, 500.0, 100);

    int count_iterated = 0;

    for (auto line : lines) {
        int point_count = line->get_point_count();

        std::cout << count_iterated << ": " << point_count << " points" << std::endl;

        std::cout << "Length: " << line->get_attribute("LENGTH") << std::endl;

        for (int i = 0; i < point_count; i++) {
            std::cout << line->get_line_point_x(i) << ", " << line->get_line_point_y(i) << ", "
                      << line->get_line_point_z(i)
                      << std::endl;
        }

        count_iterated++;
    }

    return 0;
}
