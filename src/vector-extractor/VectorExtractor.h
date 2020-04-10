#ifndef VECTOREXTRACTOR_VECTOREXTRACTOR_H
#define VECTOREXTRACTOR_VECTOREXTRACTOR_H


#include "LineFeature.h"
#include "PointFeature.h"
#include "defines.h"

class  EXPORT VectorExtractor {
public:
    /// Must be called before any other function to initialize GDAL.
    static void initialize();

    /// Return a list of all LineFeatures in the dataset at the given path which somehow overlap with the circle created
    ///  by the given position and radius. Parts of lines outside the circle are not cut, so lines can extent outside
    ///  of the given radius.
    /// If there are more lines than the given max_amount within that circle, the last few are skipped.
    /// TODO: Currently the lines which are skipped are chosen arbitrarily. It would be better to skip further away
    ///  lines in this case.
    static std::list<LineFeature *>
    get_lines_near_position(const char *path, double pos_x, double pos_y, double radius, int max_amount);

    /// Return all line data in the dataset at the given path which is within the square formed by the given parameters.
    /// Lines which are only partially within the square are cut to the part that is within (intersection operation).
    /// Thus, this can be used for contiguous tiles.
    /// If there are more lines than the given max_amount within that square, the last few (arbitrarily chosen) are
    ///  skipped. This is a safeguard for not loading more lines than can be handled.
    static std::list<LineFeature *>
    crop_lines_to_square(const char *path, double top_left_x, double top_left_y, double size_meters, int max_amount);

    static std::list<PointFeature *> get_points_near_position(const char *path, double pos_x, double pos_y, double radius, int max_amount);
};


#endif //VECTOREXTRACTOR_VECTOREXTRACTOR_H
