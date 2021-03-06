#ifndef VECTOREXTRACTOR_VECTOREXTRACTOR_H
#define VECTOREXTRACTOR_VECTOREXTRACTOR_H

#include "gdal-includes.h"

#include "LineFeature.h"
#include "PointFeature.h"
#include "PolygonFeature.h"
#include "defines.h"
#include <list>
#include <map>

// Forward declarations
class OGRLayer;
class GDALDataset;

class NativeDataset {
  public:
    NativeDataset(const char *path);
    ~NativeDataset();

    /// Return the names of all feature layers as std::strings.
    std::vector<std::string> get_feature_layer_names();

    /// Return the names of all raster layers as std::strings.
    std::vector<std::string> get_raster_layer_names();

    NativeDataset *get_subdataset(const char *name);

    NativeDataset *clone();

    bool is_valid() const;

    std::string path;

    GDALDataset *dataset;
};

class NativeLayer {
  public:
    NativeLayer(OGRLayer *layer);

    ~NativeLayer() = default;

    bool is_valid() const;

    /// Create a new feature on the in-RAM layer corresponding to the actual disk layer. No changes
    /// are made on the original layer, so that layer can be opened as read-only.
    /// The exact type of the returned feature corresponds to the layer's geometry type.
    Feature *create_feature();

    // TODO: Add something like `forget_feature(Feature)` so that the cache can be cleared (and the
    // memory freed) by the user

    /// Return all features, regardless of what the geometry is (or if there even is geometry).
    /// Note that this means that no geometry will be available in those features - this should only
    /// be used for attributes.
    std::list<Feature *> get_features();

    /// Return all features, regardless of what the geometry is, which somehow overlap with the
    /// circle created by the given position and radius. Parts of the geometry outside the circle
    /// are not cut, so features may extend outside of the circle. To prevent hangups from
    /// unexpectedly many features, a maximum amount can be given so that the remaining features are
    /// skipped.
    std::list<Feature *> get_features_near_position(double pos_x, double pos_y, double radius,
                                                    int max_amount);

    /// Return all line data in the dataset at the given path which is within the square formed by
    /// the given parameters. Lines which are only partially within the square are cut to the part
    /// that is within (intersection operation). Thus, this can be used for contiguous tiles. If
    /// there are more lines than the given max_amount within that square, the last few (arbitrarily
    /// chosen) are skipped. This is a safeguard for not loading more lines than can be handled.
    std::list<LineFeature *> crop_lines_to_square(const char *path, double top_left_x,
                                                  double top_left_y, double size_meters,
                                                  int max_amount);

    /// Returns the feature corresponding to the given OGRFeature: Either the cached one, or if
    /// there is none, a new one (then placed within the cache).
    std::list<Feature *> get_feature_for_fid(OGRFeature *feature);

    OGRLayer *layer;
    OGRLayer *ram_layer;

    /// Keeps track of all features which are in use due to having been returned, in order to ensure
    /// that we don't return two different Feature instances pointing to the same data. That way,
    /// updates to features don't need to be written anywhere: they exist within the Features of the
    /// feature cache.
    ///
    /// A list of Feature pointers is used for the case of multi-features (e.g. MULTILINESTRING),
    /// where one OGRFeature corresponds to multiple of our Features. Usually though, each list has
    /// only one element.
    std::map<GUIntBig, std::list<Feature *>> feature_cache;
};

class VectorExtractor {
  public:
    /// Must be called before any other function to initialize GDAL.
    static void initialize();

    /// Returns the GDALDataset at the given path, or null.
    /// TODO: This could also be in the RasterExtractor, it's not raster- or vector-specific...
    static NativeDataset *open_dataset(const char *path);

    /// Returns the layer from the given dataset with the given name, or null if there is no layer
    /// with that name.
    static NativeLayer *get_layer_from_dataset(GDALDataset *dataset, const char *name);
};

#endif // VECTOREXTRACTOR_VECTOREXTRACTOR_H
