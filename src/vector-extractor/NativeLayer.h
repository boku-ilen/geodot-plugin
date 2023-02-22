#include "Feature.h"
#include "LineFeature.h"
#include "gdal-includes.h"
#include "util.h"
#include <list>

class NativeLayer {
  public:
    NativeLayer(OGRLayer *layer);

    ~NativeLayer() = default;

    bool is_valid() const;

    void save_override();

    void save_modified_layer(std::string path);

    ExtentData get_extent();

    /// Create a new feature on the in-RAM layer corresponding to the actual disk layer. No changes
    /// are made on the original layer, so that layer can be opened as read-only.
    /// The exact type of the returned feature corresponds to the layer's geometry type.
    std::shared_ptr<Feature> create_feature();

    void add_field(std::string name);

    void remove_field(std::string name);

    // TODO: Add something like `forget_feature(Feature)` so that the cache can be cleared (and the
    // memory freed) by the user

    /// Return all features, regardless of what the geometry is (or if there even is geometry).
    /// Note that this means that no geometry will be available in those features - this should only
    /// be used for attributes.
    std::list<std::shared_ptr<Feature> > get_features();

    /// Return all features, regardless of what the geometry is, which somehow overlap with the
    /// circle created by the given position and radius. Parts of the geometry outside the circle
    /// are not cut, so features may extend outside of the circle. To prevent hangups from
    /// unexpectedly many features, a maximum amount can be given so that the remaining features are
    /// skipped.
    std::list<std::shared_ptr<Feature> > get_features_near_position(double pos_x, double pos_y, double radius,
                                                    int max_amount);

    /// Return all line data in the dataset at the given path which is within the square formed by
    /// the given parameters. Lines which are only partially within the square are cut to the part
    /// that is within (intersection operation). Thus, this can be used for contiguous tiles. If
    /// there are more lines than the given max_amount within that square, the last few (arbitrarily
    /// chosen) are skipped. This is a safeguard for not loading more lines than can be handled.
    std::list<std::shared_ptr<LineFeature> > crop_lines_to_square(const char *path, double top_left_x,
                                                  double top_left_y, double size_meters,
                                                  int max_amount);

    /// Returns the feature corresponding to the given OGRFeature: Either the cached one, or if
    /// there is none, a new one (then placed within the cache).
    /// Takes ownership of the passed OGRFeature, deleting it if it is not required thanks to a
    /// cache hit, and passing it on to the new Feature otherwise.
    std::list<std::shared_ptr<Feature> > get_feature_for_ogrfeature(OGRFeature *feature);

    /// Returns the feature corresponding to the given ID
    std::list<std::shared_ptr<Feature> > get_feature_by_id(int id);

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
    std::map<GUIntBig, std::list<std::shared_ptr<Feature> >> feature_cache;

    int disk_feature_count;
    int ram_feature_count;
};