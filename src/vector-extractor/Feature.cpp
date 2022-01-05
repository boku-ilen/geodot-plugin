#include "Feature.h"
#include "gdal-includes.h"

Feature::Feature(OGRFeature *feature) : feature(feature) {}

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