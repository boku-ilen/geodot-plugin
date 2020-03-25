#ifndef VECTOREXTRACTOR_VECTOREXTRACTOR_H
#define VECTOREXTRACTOR_VECTOREXTRACTOR_H


#include "LineFeature.h"

class VectorExtractor {
public:
    /// Test function for vector functionality, mostly from https://gdal.org/tutorials/vector_api_tut.html.
    static LineFeature get_first_feature();
};


#endif //VECTOREXTRACTOR_VECTOREXTRACTOR_H
