#include "Feature.h"
#include "LineFeature.h"
#include "gdal-includes.h"
#include "util.h"
#include <list>

class NativeLayer {
  public:
    NativeLayer(OGRLayer *new_layer);

    ~NativeLayer() = default;

    bool is_valid() const;

    void write_feature_cache_to_ram_layer();

    void save_override();

    void save_modified_layer(std::string path);

    ExtentData get_extent();

    /// Create a new feature on the in-RAM layer corresponding to the actual disk layer. No changes
    /// are made on the original layer, so that layer can be opened as read-only.
    /// The exact type of the returned feature corresponds to the layer's geometry type.
    std::shared_ptr<Feature> create_feature();

    void add_field(std::string name);

    void remove_field(std::string name);

    bool field_exists(std::string name);

    std::list<std::string> get_field_names();

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

    /// Return all features, regardless of what the geometry is, which somehow overlap with the
    /// rectangle created by the given top-left position and size. Parts of the geometry outside the
    /// rectangle are not cut, so features may extend outside of the circle. To prevent hangups from
    /// unexpectedly many features, a maximum amount can be given so that the remaining features are
    /// skipped.
    std::list<std::shared_ptr<Feature> > get_features_in_square(double top_left_x,
                                                  double top_left_y, double size_meters,
                                                  int max_amount);

    /// Returns the feature corresponding to the given OGRFeature: Either the cached one, or if
    /// there is none, a new one (then placed within the cache).
    /// Takes ownership of the passed OGRFeature, deleting it if it is not required thanks to a
    /// cache hit, and passing it on to the new Feature otherwise.
    std::list<std::shared_ptr<Feature> > get_feature_for_ogrfeature(OGRFeature *feature);

    /// Returns the feature corresponding to the given ID
    std::list<std::shared_ptr<Feature> > get_feature_by_id(int id);

    /// Filter the features of the layer by a given SQL-like query, e.g. "attributename < 1000"
    std::list<std::shared_ptr<Feature> > get_features_by_attribute_filter(std::string filter);

    /// Removes all entries from the feature cache. Make sure this is not called when features exist
    /// which need to remain synchronized (signals across different origins)
    void clear_feature_cache();

    bool is_feature_deleted(OGRFeature *feature);

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

  private:
    std::list<std::shared_ptr<Feature> > get_features_inside_geometry(OGRGeometry *geometry, int max_amount);
};