#ifndef VECTOREXTRACTOR_VECTOREXTRACTOR_H
#define VECTOREXTRACTOR_VECTOREXTRACTOR_H

#include "LineFeature.h"
#include "PointFeature.h"
#include "PolygonFeature.h"
#include "defines.h"

// Forward declarations
class OGRLayer;
class GDALDataset;

class EXPORT NativeDataset {
  public:
    NativeDataset(const char *path);
    ~NativeDataset();

    NativeDataset *get_subdataset(const char *name);

    NativeDataset *clone();

    bool is_valid() const;

    std::string path;

    GDALDataset *dataset;
};

class EXPORT NativeLayer {
  public:
    NativeLayer(OGRLayer *layer) : layer(layer){};

    ~NativeLayer() = default;

    bool is_valid() const;

    OGRLayer *layer;
};

class EXPORT VectorExtractor {
  public:
    /// Must be called before any other function to initialize GDAL.
    static void initialize();

    /// Returns the GDALDataset at the given path, or null.
    /// TODO: This could also be in the RasterExtractor, it's not raster- or vector-specific...
    static NativeDataset *open_dataset(const char *path);

    /// Returns the layer from the given dataset with the given name, or null if there is no layer
    /// with that name.
    static NativeLayer *get_layer_from_dataset(GDALDataset *dataset, const char *name);

    /// Return all features, regardless of what the geometry is (or if there even is geometry).
    /// Note that this means that no geometry will be available in those features - this should only
    /// be used for attributes.
    static std::list<Feature *> get_features(OGRLayer *layer);

    /// Return all features, regardless of what the geometry is, which somehow overlap with the
    /// circle created by the given position and radius. Parts of the geometry outside the circle
    /// are not cut, so features may extend outside of the circle. To prevent hangups from
    /// unexpectedly many features, a maximum amount can be given so that the remaining features are
    /// skipped.
    static std::list<Feature *> get_features_near_position(OGRLayer *layer, double pos_x,
                                                           double pos_y, double radius,
                                                           int max_amount);

    /// Return the names of all feature layers as std::strings.
    static std::vector<std::string> get_feature_layer_names(NativeDataset *dataset);

    /// Return the names of all raster layers as std::strings.
    static std::vector<std::string> get_raster_layer_names(NativeDataset *dataset);

    /// Return all line data in the dataset at the given path which is within the square formed by
    /// the given parameters. Lines which are only partially within the square are cut to the part
    /// that is within (intersection operation). Thus, this can be used for contiguous tiles. If
    /// there are more lines than the given max_amount within that square, the last few (arbitrarily
    /// chosen) are
    ///  skipped. This is a safeguard for not loading more lines than can be handled.
    static std::list<LineFeature *> crop_lines_to_square(const char *path, double top_left_x,
                                                         double top_left_y, double size_meters,
                                                         int max_amount);
};

#endif // VECTOREXTRACTOR_VECTOREXTRACTOR_H
