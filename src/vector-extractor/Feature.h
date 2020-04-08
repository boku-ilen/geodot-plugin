#ifndef VECTOREXTRACTOR_FEATURE_H
#define VECTOREXTRACTOR_FEATURE_H

#include <string>
#include <map>

class OGRFeature;

class Feature {
public:
    /// Construct a Feature from an OGRFeature from GDAL.
    explicit Feature(OGRFeature *feature);

    /// Return a map with all attribute names -> values.
    std::map<std::string, std::string> get_attributes();

    /// Return the value of the attribute with the given name.
    /// A field with the given name must exist.
    const char *get_attribute(const char *name);

private:
    OGRFeature *feature;
};


#endif //VECTOREXTRACTOR_FEATURE_H
