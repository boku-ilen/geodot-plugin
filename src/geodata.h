#ifndef __GEODATA_H__
#define __GEODATA_H__

#include <Godot.hpp>

#include "defines.h"
#include "geoimage.h"

namespace godot {

/// A dataset which contains layers of geodata.
/// Corresponds to GDALDataset.
class EXPORT GeoDataset : public Resource {
    GODOT_CLASS(GeoDataset, Resource)

public:
    GeoDataset();
    virtual ~GeoDataset();

    /// Automatically called by Godot
    void _init();
    static void _register_methods();

    /// Returns true if the GeoDataset could successfully be loaded.
    bool is_valid();

    GeoRasterLayer get_raster_layer(String name);

    GeoFeatureLayer get_feature_layer(String name);
};

/// A layer which contains any number of features.
/// These features consist of attributes and usually (but not necessarily) vector geometry.
/// This layer provides access to these features through various filters.
/// Corresponds to OGRLayer.
class EXPORT GeoFeatureLayer : public Resource {
    GODOT_CLASS(GeoFeatureLayer, Resource)

public:
    GeoFeatureLayer();
    virtual ~GeoFeatureLayer();

    /// Automatically called by Godot
    void _init();
    static void _register_methods();

    /// Returns true if the layer could successfully be loaded.
    bool is_valid();

    Array get_all_features();

    Array get_lines_near_position(double pos_x, double pos_y, double radius, int max_lines);

    Array get_points_near_position(double pos_x, double pos_y, double radius, int max_points);

    Array crop_lines_to_square(double top_left_x, double top_left_y, double size_meters, int max_lines);
};

/// A layer which contains raster data.
/// Corresponds to a Raster GDALDataset or Subdataset.
class EXPORT GeoRasterLayer : public Resource {
    GODOT_CLASS(GeoRasterLayer, Resource)

public:
    GeoRasterLayer();
    virtual ~GeoRasterLayer();

    /// Automatically called by Godot
    void _init();
    static void _register_methods();

    /// Returns true if the layer could successfully be loaded.
    bool is_valid();

    Ref<GeoImage> get_image(double top_left_x, double top_left_y, double size_meters,
                            int img_size, int interpolation_type);
};

}

#endif // __GEODATA_H__
