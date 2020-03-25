#include <iostream>
#include "VectorExtractor.h"

int main() {
    LineFeature line = VectorExtractor::get_first_feature();

    std::cout << "Length: " << line.get_attribute("LENGTH") << std::endl;

    std::cout << "Points:" << std::endl;

    int point_count = line.get_point_count();
    for (int i = 0; i < point_count; i++) {
        auto point = line.get_line_point(i);
        std::cout << point[0] << ", " << point[1] << ", " << point[2] << std::endl;
    }

    return 0;
}
