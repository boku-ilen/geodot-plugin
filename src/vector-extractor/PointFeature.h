//
// Created by karl on 08.04.20.
//

#ifndef VECTOREXTRACTOR_POINTFEATURE_H
#define VECTOREXTRACTOR_POINTFEATURE_H

#include "Feature.h"

class OGRPoint;

class PointFeature : public Feature {
  public:
    /// Construct a LineFeature with a feature which contains a LineString.
    explicit PointFeature(OGRFeature *feature);

    /// Construct a LineFeature with a feature that can contain any geometry (usually used for
    /// MultiLineStrings) - it is not accessed, the geometry is given as a separate LineString
    /// parameter instead.
    PointFeature(OGRFeature *feature, OGRGeometry *ogrPoint);

    /// Get the individual x, y and z components from the line point at the given index.
    /// The same can be accomplished with the `get_line_point` function, but this one works without
    /// special std types.
    double get_x();
    double get_y();
    double get_z();

    void set_vector(double x, double y, double z);

  private:
    OGRPoint *point;
};

#endif // VECTOREXTRACTOR_POINTFEATURE_H
