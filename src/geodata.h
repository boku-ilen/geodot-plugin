#ifndef __GEODATA_H__
#define __GEODATA_H__

#include "RasterTileExtractor.h"
#include "VectorExtractor.h"
#include "defines.h"
#include "geofeatures.h"
#include "geoimage.h"
#include "godot_cpp/variant/variant.hpp"

namespace godot {

// Forward decaration
class EXPORT GeoDataset;

/// A layer which contains any number of features.
/// These features consist of attributes and usually (but not necessarily)
/// vector geometry. This layer provides access to these features through
/// various filters. Corresponds to OGRLayer.
/// Its `name` property corresponds to the layer name.
class EXPORT GeoFeatureLayer : public Resource {
    GDCLASS(GeoFeatureLayer, Resource)

  protected:
    static void _bind_methods();

  public:
    GeoFeatureLayer() = default;
    virtual ~GeoFeatureLayer() = default; // No need to delete anything here - OGRLayers are part of
                                          // the dataset and deleted with it.

    /// Returns true if the layer could successfully be loaded.
    bool is_valid();

    /// Returns the dataset which this layer was opened from.
    Ref<GeoDataset> get_dataset();

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

    /// Applies all changes made to the layer to this layer, overriding the previous data
    /// permanently.
    void save_override();

    /// Applies all changes made to the layer to a copy of the original layer which is created at
    /// the given path. Will be created as a Shapefile.
    void save_new(String file_path);

    /// Returns all features, regardless of the geometry, near the given
    /// position (within the given radius).
    Array get_features_near_position(double pos_x, double pos_y, double radius, int max_features);

    /// Crops features with line geometry to the square created by the given
    /// coordinates and size. Useful for doing tile-based requests.
    /// TODO: Can this be made generic like 'get_features_near_position'?
    Array crop_lines_to_square(double top_left_x, double top_left_y, double size_meters,
                               int max_lines);

    /// Set the OGRLayer object directly.
    /// Not exposed to Godot since Godot doesn't know about GDALDatasets - this
    /// is only for internal use.
    void set_native_layer(NativeLayer *new_layer);

    /// Sets the dataset which this layer was opened from.
    /// Not exposed to Godot since it should never construct GeoFeatureLayers by hand.
    void set_origin_dataset(Ref<GeoDataset> dataset);

  private:
    NativeLayer *layer;
    Ref<GeoDataset> origin_dataset;
};

/// A layer which contains raster data.
/// Corresponds to a Raster GDALDataset or Subdataset.
/// Its `name` property is either the layer name, or the full path if it wasn't opened from a
/// dataset (i.e. get_dataset() returns null).
class EXPORT GeoRasterLayer : public Resource {
    GDCLASS(GeoRasterLayer, Resource)

  protected:
    static void _bind_methods();

  public:
    GeoRasterLayer() : origin_dataset(nullptr) {}
    virtual ~GeoRasterLayer();

    /// Returns true if the layer could successfully be loaded.
    bool is_valid();

    /// Returns the dataset which this layer was opened from or null if it was opened directly, e.g.
    /// from a GeoTIFF.
    Ref<GeoDataset> get_dataset();

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

    /// Returns the value in the GeoRasterLayer at exactly the given position, at the given
    /// resolution. This is useful for getting heights which match up with the heights of a lower
    /// resolution image which was fetched using `get_image`, rather than being as detailed as
    /// possible.
    float get_value_at_position_with_resolution(double pos_x, double pos_y,
                                                double pixel_size_meters);

    /// Replaces exactly one pixel at the given position with the given value.
    /// The value must correspond to this layer's type (e.g. a float for Float32 images and a Color
    /// for RGB images).
    /// Useful for modifying datasets on a small scale, e.g. correcting land-use values.
    void set_value_at_position(double pos_x, double pos_y, Variant value);

    /// Adds the given summand to the previous value at the given position and smoothly cross-fades
    /// into the original data along the given radius.
    /// Useful for terraforming terrain, e.g. creating a new hill with smooth slopes.
    void smooth_add_value_at_position(double pos_x, double pos_y, double summand, double radius);

    /// Adds the given image to the raster dataset at the given position. Fully opaque values are
    /// replaced; values with alpha between 0 and 1 are interpolated between original and new.
    /// A scale of 1 assumes that both images have the same resolution per meter; 2 means that one
    /// pixel in this image corresponds to two pixels in the dataset.
    /// Useful for drawing into datasets with custom brushes, e.g. a pre-defined land-use pattern.
    void overlay_image_at_position(double pos_x, double pos_y, Ref<Image> image, double scale);

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
    void load_from_file(String file_path, bool write_access);

    /// Set the GDALDataset object for this layer. Must be a valid raster
    /// dataset. Not exposed to Godot since Godot doesn't know about
    /// GDALDatasets - this is only for internal use.
    void set_native_dataset(NativeDataset *new_dataset);

    /// Sets the dataset which this layer was opened from.
    /// Not exposed to Godot since it should never construct GeoFeatureLayers by hand.
    void set_origin_dataset(Ref<GeoDataset> dataset);

    bool write_access;

  private:
    Ref<GeoDataset> origin_dataset;
    NativeDataset *dataset;
    RasterTileExtractor::ExtentData extent_data;
};

/// A layer which contains raster data, loaded from a well-defined pyramid folder structure.
/// (See https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames)
/// Requires a separate class as it does not wrap a GDALDataset and thus handles its data
/// differently.
class EXPORT PyramidGeoRasterLayer : public GeoRasterLayer {
    GDCLASS(PyramidGeoRasterLayer, GeoRasterLayer)

  protected:
    static void _bind_methods();

  public:
    PyramidGeoRasterLayer() = default;
    virtual ~PyramidGeoRasterLayer() = default;

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
    GDCLASS(GeoDataset, Resource)

  protected:
    static void _bind_methods();

  public:
    GeoDataset() = default;
    ~GeoDataset();

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
    /// object.
    void load_from_file(String file_path, bool write_access);

    /// Set the GDALDataset object directly.
    /// Not exposed to Godot since Godot doesn't know about GDALDatasets - this
    /// is only for internal use.
    void set_native_dataset(NativeDataset *new_dataset);

    bool write_access;

  private:
    NativeDataset *dataset;
};

} // namespace godot

#endif // __GEODATA_H__
