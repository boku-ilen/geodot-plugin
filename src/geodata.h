#ifndef __GEODATA_H__
#define __GEODATA_H__

#include <Godot.hpp>

#include "defines.h"
#include "geoimage.h"
#include "VectorExtractor.h"

namespace godot {

/// A layer which contains any number of features.
/// These features consist of attributes and usually (but not necessarily) vector geometry.
/// This layer provides access to these features through various filters.
/// Corresponds to OGRLayer.
class EXPORT GeoFeatureLayer : public Resource {
    GODOT_CLASS(GeoFeatureLayer, Resource)

public:
    GeoFeatureLayer() = default;
    virtual ~GeoFeatureLayer() = default;  // No need to delete anything here - OGRLayers are part of the dataset and deleted with it.

    /// Automatically called by Godot
    void _init() {}  // Must be here as Godot always calls this for Objects
    static void _register_methods();

    /// Returns true if the layer could successfully be loaded.
    bool is_valid();

    /// Returns all features, regardless of the geometry, within this layer.
    Array get_all_features();

    /// Returns all features, regardless of the geometry, near the given position (within the given radius).
    Array get_features_near_position(double pos_x, double pos_y, double radius, int max_features);

    /// Crops features with line geometry to the square created by the given coordinates and size.
    /// Useful for doing tile-based requests.
    /// TODO: Can this be made generic like 'get_features_near_position'?
    Array crop_lines_to_square(double top_left_x, double top_left_y, double size_meters, int max_lines);

    /// Load the first layer of the dataset at the given path into this object.
    /// Useful e.g. for simple shapefiles with only one layer.
    /// Not exposed to Godot since Godot should create datasets and layers from the Geodot singleton (the factory).
    void load_from_file(String file_path);

    /// Set the OGRLayer object directly.
    /// Not exposed to Godot since Godot doesn't know about GDALDatasets - this is only for internal use.
    void set_native_layer(NativeLayer *new_layer);

private:
    NativeLayer *layer;
};

/// A layer which contains raster data.
/// Corresponds to a Raster GDALDataset or Subdataset.
class EXPORT GeoRasterLayer : public Resource {
    GODOT_CLASS(GeoRasterLayer, Resource)

public:
    GeoRasterLayer() = default;
    virtual ~GeoRasterLayer();

    /// Automatically called by Godot
    void _init() {}  // Must be here as Godot always calls this for Objects
    static void _register_methods();

    /// Returns true if the layer could successfully be loaded.
    bool is_valid();

    /// Returns a clone of this layer which points to the same file, but uses a different object to access it.
    /// This is required when using the same layer in multiple threads.
    Ref<GeoRasterLayer> clone();

    Ref<GeoImage> get_image(double top_left_x, double top_left_y, double size_meters,
                            int img_size, int interpolation_type);
    
    /// Load a raster dataset file such as a GeoTIFF into this object.
    /// Not exposed to Godot since Godot should create datasets and layers from the Geodot singleton (the factory).
    void load_from_file(String file_path);

    /// Set the GDALDataset object for this layer. Must be a valid raster dataset.
    /// Not exposed to Godot since Godot doesn't know about GDALDatasets - this is only for internal use.
    void set_native_dataset(NativeDataset *new_dataset);

private:
    NativeDataset *dataset;
};

class EXPORT PyramidGeoRasterLayer : public GeoRasterLayer {
    GODOT_SUBCLASS(PyramidGeoRasterLayer, GeoRasterLayer)

public:
    PyramidGeoRasterLayer() = default;
    virtual ~PyramidGeoRasterLayer() = default;

    /// Automatically called by Godot
    void _init() {}  // Must be here as Godot always calls this for Objects
    static void _register_methods();

    /// Returns true if the layer could successfully be loaded.
    bool is_valid();

    Ref<GeoImage> get_image(double top_left_x, double top_left_y, double size_meters,
                            int img_size, int interpolation_type);
    
    void set_pyramid_base(String path);

    void set_file_ending(String ending);

private:
    String path;
    String ending;
};

/// A dataset which contains layers of geodata.
/// Corresponds to GDALDataset.
class EXPORT GeoDataset : public Resource {
    GODOT_CLASS(GeoDataset, Resource)

public:
    GeoDataset() = default;
    ~GeoDataset();

    /// Automatically called by Godot
    void _init() {}  // Must be here as Godot always calls this for Objects
    static void _register_methods();

    /// Returns true if the GeoDataset could successfully be loaded.
    bool is_valid();

    /// Returns a GeoRasterLayer object of the layer within this dataset with the given name.
    /// It is recommended to check the validity of the returned object with GeoRasterLayer::is_valid().
    Ref<GeoRasterLayer> get_raster_layer(String name);

    /// Returns a GeoFeatureLayer object of the layer within this dataset with the given name.
    /// It is recommended to check the validity of the returned object with GeoFeatureLayer::is_valid().
    Ref<GeoFeatureLayer> get_feature_layer(String name);

    /// Load a dataset file such as a Geopackage or a Shapefile into this object.
    /// Not exposed to Godot since Godot should create datasets and layers from the Geodot singleton (the factory).
    void load_from_file(String file_path);

    /// Set the GDALDataset object directly.
    /// Not exposed to Godot since Godot doesn't know about GDALDatasets - this is only for internal use.
    void set_native_dataset(NativeDataset *new_dataset);

private:
    NativeDataset *dataset;
};

}

#endif // __GEODATA_H__
