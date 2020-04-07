#ifndef VECTOREXTRACTOR_VECTOREXTRACTOR_H
#define VECTOREXTRACTOR_VECTOREXTRACTOR_H


#include "LineFeature.h"

class VectorExtractor {
public:
    /// Must be called before any other function to initialize GDAL.
    static void initialize();

    /// Return a list of all LineFeatures in the dataset at the given path which are within the circle created by the
    ///  given position and radius.
    /// If there are more lines than the given max_amount within that circle, the last few are skipped.
    /// TODO: Currently the lines which are skipped are chosen arbitrarily. It would be better to skip further away
    ///  lines in this case.
    static std::list<LineFeature *>
    get_lines_near_position(const char *path, double pos_x, double pos_y, double radius, int max_amount);

    static std::list<LineFeature *>
    get_lines_in_square(const char *path, double top_left_x, double top_left_y, double size_meters, int max_amount);
};


#endif //VECTOREXTRACTOR_VECTOREXTRACTOR_H
