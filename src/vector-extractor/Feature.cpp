#include "Feature.h"
#include "gdal-includes.h"

#include <iostream>

Feature::Feature(OGRFeature *feature) : feature(feature) {
    std::cout << "Feature created" << std::endl;
}

Feature::~Feature() {
    std::cout << "Feature deleted" << std::endl;

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

bool Feature::intersects_with(std::shared_ptr<Feature> other) const {
    if (geometry_type == NONE or other->geometry_type == NONE) {
        // Features without goemetry cannot intersect
        return false;
    }

    return feature->GetGeometryRef()->Intersects(other->feature->GetGeometryRef());
}