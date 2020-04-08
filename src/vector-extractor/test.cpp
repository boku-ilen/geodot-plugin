#include <iostream>
#include "VectorExtractor.h"

int main() {
    VectorExtractor::initialize();

    std::list<PointFeature *> points = VectorExtractor::get_points_near_position(
            "/home/karl/BOKU/geodata/power/power_poles_and_towers_level_code.shp", 1546670.5, 5918250.8, 10000.0, 100);

    int count_iterated = 0;

    std::cout << "Total: " << points.size() << " lines" << std::endl;

    for (auto point : points) {
        std::cout << point->get_x() << std::endl;

        count_iterated++;
    }

    return 0;
}
