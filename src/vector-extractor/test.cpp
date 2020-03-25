#include <iostream>
#include "VectorExtractor.h"

int main() {
    LineFeature line = VectorExtractor::get_first_feature();

    std::cout << "Length: " << line.get_attribute("LENGTH") << std::endl;

    std::cout << "Points:" << std::endl;

    int point_count = line.get_point_count();
    for (int i = 0; i < point_count; i++) {
        std::cout << line.get_line_point_x(i) << ", " << line.get_line_point_y(i) << ", " << line.get_line_point_z(i) << std::endl;
    }

    return 0;
}
