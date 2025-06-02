#include "gdal-includes.h"


class NativeLayer;

class NativeDataset {
  public:
    NativeDataset(std::string path, bool write_access);
    ~NativeDataset();

    /// Return the names of all feature layers as std::strings.
    std::vector<std::string> get_feature_layer_names();

    /// Return the names of all raster layers as std::strings.
    std::vector<std::string> get_raster_layer_names();

    /// Return the descriptions of all raster bands as std::strings.
    std::vector<std::string> get_raster_band_descriptions();

    /// Returns true if the dataset has a feature layer of the given name.
    bool has_layer(const char *name) const;

    /// Returns the layer from this dataset with the given name, or null if there is no layer
    /// with that name.
    std::shared_ptr<NativeLayer> get_layer(const char *name) const;

    std::shared_ptr<NativeDataset> get_subdataset(const char *name) const;

    std::shared_ptr<NativeDataset> clone();

    bool is_valid() const;

    // Return the epsg integer code of the spatial reference system if avaialble, -1 otherwise
    int get_epsg_code() const;

    std::string path;

    bool write_access;

    GDALDataset *dataset;
};