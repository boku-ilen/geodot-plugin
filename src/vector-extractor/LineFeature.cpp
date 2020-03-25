#include "LineFeature.h"

std::map<std::string, std::string> LineFeature::get_attributes() {
    std::map<std::string, std::string> ret;

    for (auto &&oField: *feature) {
        ret[oField.GetName()] = oField.GetAsString();
    }

    return ret;
}

std::string LineFeature::get_attribute(const std::string& name) {
    return feature->GetFieldAsString(name.c_str());
}

double *LineFeature::get_line_point(int index) {
    return nullptr;
}
