#ifndef VECTOREXTRACTOR_LINEFEATURE_H
#define VECTOREXTRACTOR_LINEFEATURE_H


#include <map>
#include <list>
#include <vector>
#include "Feature.h"

class OGRLineString;
class OGRGeometry;

/// Wrapper for an OGRFeature representing a Line.
class LineFeature : public Feature {
public:
    /// Construct a LineFeature with a feature which contains a LineString.
    explicit LineFeature(OGRFeature *feature);

    /// Construct a LineFeature with a feature that can contain any geometry (usually used for MultiLineStrings) - it is
    /// not accessed, the geometry is given as a separate LineString parameter instead.
    LineFeature(OGRFeature *feature, const OGRGeometry *linestring);

    /// Return the point in the line at the given index as a std::vector with 3 double entries (3D vector).
    /// The index must be between 0 and get_point_count()-1.
    std::vector<double> get_line_point(int index);

    /// Get the individual x, y and z components from the line point at the given index.
    /// The same can be accomplished with the `get_line_point` function, but this one works without special std types.
    double get_line_point_x(int index);
    double get_line_point_y(int index);
    double get_line_point_z(int index);

    /// Return the number of points in the line.
    int get_point_count();

private:
    const OGRLineString *line;

    int point_count;
};


#endif //VECTOREXTRACTOR_LINEFEATURE_H
