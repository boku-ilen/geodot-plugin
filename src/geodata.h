#ifndef __GEODATA_H__
#define __GEODATA_H__

#include <Godot.hpp>

#include "RasterTileExtractor.h"
#include "VectorExtractor.h"
#include "defines.h"
#include "geofeatures.h"
#include "geoimage.h"

namespace godot {

/// A layer which contains any number of features.
/// These features consist of attributes and usually (but not necessarily)
/// vector geometry. This layer provides access to these features through
/// various filters. Corresponds to OGRLayer.
class EXPORT GeoFeatureLayer : public Resource {
    GODOT_CLASS(GeoFeatureLayer, Resource)

  public:
    GeoFeatureLayer() = default;
    virtual ~GeoFeatureLayer() = default; // No need to delete anything here - OGRLayers are part of
                                          // the dataset and deleted with it.

    /// Automatically called by Godot
    void _init() {} // Must be here as Godot always calls this for Objects
    static void _register_methods();

    /// Returns true if the layer could successfully be loaded.
    bool is_valid();

    /// Returns the one feature that corresponds to the given ID.
    Ref<GeoFeature> get_feature_by_id(int id);

    /// Returns all features, regardless of the geometry, within this layer.
    Array get_all_features();

    /// Adds the given feature to the layer.
    /// This change has no effect on the dataset on disk unless TODO is called.
    Ref<GeoFeature> create_feature();

    /// Removes the given feature from the layer.
    /// This change has no effect on the dataset on disk unless TODO is called.
    void remove_feature(Ref<GeoFeature> feature);

    /// Returns all features, regardless of the geometry, near the given
    /// position (within the given radius).
    Array get_features_near_position(double pos_x, double pos_y, double radius, int max_features);

    /// Crops features with line geometry to the square created by the given
    /// coordinates and size. Useful for doing tile-based requests.
    /// TODO: Can this be made generic like 'get_features_near_position'?
    Array crop_lines_to_square(double top_left_x, double top_left_y, double size_meters,
                               int max_lines);

    /// Load the first layer of the dataset at the given path into this object.
    /// Useful e.g. for simple shapefiles with only one layer.
    /// Not exposed to Godot since Godot should create datasets and layers from
    /// the Geodot singleton (the factory).
    void load_from_file(String file_path);

    /// Set the OGRLayer object directly.
    /// Not exposed to Godot since Godot doesn't know about GDALDatasets - this
    /// is only for internal use.
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
    void _init() {} // Must be here as Godot always calls this for Objects
    static void _register_methods();

    /// Returns true if the layer could successfully be loaded.
    bool is_valid();

    /// Returns a clone of this layer which points to the same file, but uses a
    /// different object to access it. This is required when using the same
    /// layer in multiple threads.
    Ref<GeoRasterLayer> clone();

    /// Returns a GeoImage corresponding to the given position and size.
    /// The requested section is read from this GeoRasterLayer into that GeoImage, so this
    /// operation is costly for large images. (Consider multithreading.)
    Ref<GeoImage> get_image(double top_left_x, double top_left_y, double size_meters, int img_size,
                            int interpolation_type);

    /// Returns the value in the GeoRasterLayer at exactly the given position.
    /// Note that when reading many values from a confined area, it is more efficient to call
    /// get_image and read the pixels from there.
    float get_value_at_position(double pos_x, double pos_y);

    /// Returns the extent of the layer in projected meters (assuming it is rectangular).
    Rect2 get_extent();

    /// Returns the point in the center of the layer in projected meters.
    /// The y-component is 0.0.
    Vector3 get_center();

    /// Returns the smallest value found in the first raster band of the dataset.
    /// Note that this requires the dataset to have pre-computed statistics!
    float get_min();

    /// Returns the largest value found in the first raster band of the dataset.
    /// Note that this requires the dataset to have pre-computed statistics!
    float get_max();

    /// Load a raster dataset file such as a GeoTIFF into this object.
    /// Not exposed to Godot since Godot should create datasets and layers from
    /// the Geodot singleton (the factory).
    void load_from_file(String file_path);

    /// Set the GDALDataset object for this layer. Must be a valid raster
    /// dataset. Not exposed to Godot since Godot doesn't know about
    /// GDALDatasets - this is only for internal use.
    void set_native_dataset(NativeDataset *new_dataset);

  private:
    NativeDataset *dataset;
    RasterTileExtractor::ExtentData extent_data;
};

/// A layer which contains raster data, loaded from a well-defined pyramid folder structure.
/// (See https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames)
/// Requires a separate class as it does not wrap a GDALDataset and thus handles its data
/// differently.
class EXPORT PyramidGeoRasterLayer : public GeoRasterLayer {
    GODOT_SUBCLASS(PyramidGeoRasterLayer, GeoRasterLayer)

  public:
    PyramidGeoRasterLayer() = default;
    virtual ~PyramidGeoRasterLayer() = default;

    /// Automatically called by Godot
    void _init() {} // Must be here as Godot always calls this for Objects
    static void _register_methods();

    /// Returns true if the layer could successfully be loaded.
    bool is_valid();

    Ref<GeoImage> get_image(double top_left_x, double top_left_y, double size_meters, int img_size,
                            int interpolation_type);

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
    void _init() {} // Must be here as Godot always calls this for Objects
    static void _register_methods();

    /// Returns true if the GeoDataset could successfully be loaded.
    bool is_valid();

    /// Return all GeoRasterLayers objects for this dataset.
    Array get_raster_layers();

    /// Return all GeoFeatureLayer objects for this dataset.
    Array get_feature_layers();

    /// Returns a GeoRasterLayer object of the layer within this dataset with
    /// the given name. It is recommended to check the validity of the returned
    /// object with GeoRasterLayer::is_valid().
    Ref<GeoRasterLayer> get_raster_layer(String name);

    /// Returns a GeoFeatureLayer object of the layer within this dataset with
    /// the given name. It is recommended to check the validity of the returned
    /// object with GeoFeatureLayer::is_valid().
    Ref<GeoFeatureLayer> get_feature_layer(String name);

    /// Load a dataset file such as a Geopackage or a Shapefile into this
    /// object. Not exposed to Godot since Godot should create datasets and
    /// layers from the Geodot singleton (the factory).
    void load_from_file(String file_path);

    /// Set the GDALDataset object directly.
    /// Not exposed to Godot since Godot doesn't know about GDALDatasets - this
    /// is only for internal use.
    void set_native_dataset(NativeDataset *new_dataset);

  private:
    NativeDataset *dataset;
};

} // namespace godot

#endif // __GEODATA_H__
