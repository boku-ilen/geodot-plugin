#ifndef VECTOREXTRACTOR_FEATURE_H
#define VECTOREXTRACTOR_FEATURE_H

#include "defines.h"
#include <map>
#include <memory>
#include <string>

class OGRFeature;
class OGRGeometry;

class Feature {
  public:
    enum GeometryType { NONE, POINT, LINE, POLYGON };

    /// Construct a Feature from an OGRFeature from GDAL, taking ownership of the OGRFeature object
    explicit Feature(OGRFeature *feature);

    virtual ~Feature();

    /// Construct a Feature with a feature that can contain any geometry - it is
    /// not accessed, the geometry is given as a separate OGRGeometry parameter instead.
    /// Used for GeometryCollections (MultiPoint, MultiLineString, ...)
    Feature(OGRFeature *feature, const OGRGeometry *geometry);

    /// Return a map with all attribute names -> values.
    std::map<std::string, std::string> get_attributes();

    /// Return the value of the attribute with the given name.
    /// A field with the given name must exist.
    const char *get_attribute(const char *name);

    void set_attribute(const char *name, const char *value);

    int get_id() const;

    bool intersects_with(std::shared_ptr<Feature> other) const;

    GeometryType geometry_type = NONE;

    bool is_deleted = false;

    OGRFeature *feature;
};

#endif // VECTOREXTRACTOR_FEATURE_H
