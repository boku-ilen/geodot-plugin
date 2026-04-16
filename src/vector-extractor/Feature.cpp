#include "Feature.h"

#include <algorithm>
#include <cstdint>
#include <ogr_feature.h>

#include "gdal-includes.h"

Feature::Feature(OGRFeature *feature) : feature(feature) {}

Feature::~Feature() {
    // See e.g. https://gdal.org/api/ogrlayer_cpp.html#_CPPv4N8OGRLayer14GetNextFeatureEv
    OGRFeature::DestroyFeature(feature);
}

std::map<std::string, std::string> Feature::get_attributes() {
    std::map<std::string, std::string> ret;

    for (auto &&oField : *feature) {
        if(oField.GetType() != OFTBinary)
            ret[oField.GetName()] = oField.GetAsString();
    }

    return ret;
}

const char *Feature::get_attribute(const char *name) {
    return feature->GetFieldAsString(name);
}


const uint8_t *Feature::get_binary_attribute(const char *name, int* n_bytes) {
    int field = feature->GetFieldIndex(name);
    // temporary return value
    GByte *data = feature->GetFieldAsBinary(field, n_bytes);
    const auto ret = new uint8_t[*n_bytes];
    std::copy_n(data, *n_bytes, ret);
    return ret;
}

void Feature::set_attribute(const char *name, const char *value) {
    feature->SetField(name, value);
}

void Feature::set_binary_attribute(const char *name, uint8_t *value, int n_bytes) {
    feature->SetField(feature->GetFieldIndex(name), n_bytes, value);
}


int Feature::get_id() const {
    return feature->GetFID();
}

bool Feature::intersects_with(std::shared_ptr<Feature> other) const {
    if (geometry_type == NONE or other->geometry_type == NONE) {
        // Features without goemetry cannot intersect
        return false;
    }

    return feature->GetGeometryRef()->Intersects(other->feature->GetGeometryRef());
}