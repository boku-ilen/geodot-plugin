#include "Feature.h"
#include "gdal-includes.h"

Feature::Feature(OGRFeature *feature) : feature(feature) {}

Feature::~Feature() {
    // See e.g. https://gdal.org/api/ogrlayer_cpp.html#_CPPv4N8OGRLayer14GetNextFeatureEv
    OGRFeature::DestroyFeature(feature);
}

std::map<std::string, std::string> Feature::get_attributes() {
    std::map<std::string, std::string> ret;

    for (auto &&oField : *feature) {
        ret[oField.GetName()] = oField.GetAsString();
    }

    return ret;
}

const char *Feature::get_attribute(const char *name) {
    return feature->GetFieldAsString(name);
}

void Feature::set_attribute(const char *name, const char *value) {
    feature->SetField(name, value);
}

int Feature::get_id() const {
    return feature->GetFID();
}

bool Feature::intersects_with(Feature *other) const {
    if (geometry_type == NONE or other->geometry_type == NONE) {
        // Features without goemetry cannot intersect
        return false;
    }

    return feature->GetGeometryRef()->Intersects(other->feature->GetGeometryRef());
}