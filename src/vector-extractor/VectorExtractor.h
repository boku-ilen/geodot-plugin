#ifndef VECTOREXTRACTOR_VECTOREXTRACTOR_H
#define VECTOREXTRACTOR_VECTOREXTRACTOR_H


#include "LineFeature.h"

class VectorExtractor {
public:
    static std::list<LineFeature *> get_lines_near_position(const char *path, double pos_x, double pos_y, double radius, int max_amount);
};


#endif //VECTOREXTRACTOR_VECTOREXTRACTOR_H
