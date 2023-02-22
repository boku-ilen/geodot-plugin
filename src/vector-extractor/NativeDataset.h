#include "gdal-includes.h"

class NativeLayer;

class NativeDataset {
  public:
    NativeDataset(std::string path, bool write_access);
    ~NativeDataset() = default;

    /// Return the names of all feature layers as std::strings.
    std::vector<std::string> get_feature_layer_names();

    /// Return the names of all raster layers as std::strings.
    std::vector<std::string> get_raster_layer_names();

    /// Returns the layer from this dataset with the given name, or null if there is no layer
    /// with that name.
    std::shared_ptr<NativeLayer> get_layer(const char *name) const;

    std::shared_ptr<NativeDataset> get_subdataset(const char *name) const;

    std::shared_ptr<NativeDataset> clone();

    bool is_valid() const;

    std::string path;

    bool write_access;

    GDALDataset *dataset;
};