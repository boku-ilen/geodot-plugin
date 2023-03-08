#include "NativeDataset.h"
#include "NativeLayer.h"


NativeDataset::NativeDataset(std::string path, bool write_access)
    : path(path), write_access(write_access) {
    unsigned int open_access = write_access ? GDAL_OF_UPDATE : GDAL_OF_READONLY;

    dataset = (GDALDataset *)GDALOpenEx(path.c_str(), open_access, nullptr, nullptr, nullptr);
}

NativeDataset::~NativeDataset() {
    GDALClose(dataset);
}


std::vector<std::string> NativeDataset::get_raster_layer_names() {
    std::vector<std::string> names;

    char **subdataset_metadata = dataset->GetMetadata("SUBDATASETS");

    // This metadata is formated like this:
    // SUBDATASET_1_NAME=GPKG:/path/to/geopackage.gpkg:dhm
    // SUBDATASET_1_DESC=dhm - dhm
    // SUBDATASET_2_NAME=GPKG:/path/to/geopackage.gpkg:ndom
    // SUBDATASET_2_DESC=ndom - ndom
    // We want every second line (since that has the name), and only the portion after the last ':'.

    if (subdataset_metadata != nullptr) {
        for (int i = 0; subdataset_metadata[i] != nullptr; i += 2) {
            std::string subdataset_name(subdataset_metadata[i]); // char* to std::string
            size_t last_colon = subdataset_name.find_last_of(
                ':'); // Find the last colon, which separates the path from the subdataset name
            subdataset_name =
                subdataset_name.substr(last_colon + 1); // Add 1 so the ':' isn't included

            names.emplace_back(subdataset_name);
        }
    }

    return names;
}

std::shared_ptr<NativeLayer> NativeDataset::get_layer(const char *name) const {
    return std::make_shared<NativeLayer>(dataset->GetLayerByName(name));
}

std::shared_ptr<NativeDataset> NativeDataset::get_subdataset(const char *name) const {
    // TODO: Hardcoded for the way GeoPackages work - do we want to support others too?
    return std::make_shared<NativeDataset> (("GPKG:" + path + std::string(":") + std::string(name)).c_str(),
                             write_access);
}

std::shared_ptr<NativeDataset> NativeDataset::clone() {
    return std::make_shared<NativeDataset> (path, write_access);
}

bool NativeDataset::is_valid() const {
    // No dataset at all?
    if (dataset == nullptr) { return false; }

    return true;
}
