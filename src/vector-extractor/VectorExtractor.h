#ifndef VECTOREXTRACTOR_VECTOREXTRACTOR_H
#define VECTOREXTRACTOR_VECTOREXTRACTOR_H


#include "LineFeature.h"

class VectorExtractor {
public:
    /// Test function for vector functionality, mostly from https://gdal.org/tutorials/vector_api_tut.html.
    static LineFeature get_first_feature();

    static std::list<LineFeature *> get_lines_near_position(const char *path, double pos_x, double pos_y, double radius, int max_amount);
};


#endif //VECTOREXTRACTOR_VECTOREXTRACTOR_H
