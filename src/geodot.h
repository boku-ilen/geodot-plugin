#ifndef GEODOT_H
#define GEODOT_H

#include <Godot.hpp>
#include <Node.hpp>
#include <Mutex.hpp>
#include <Array.hpp>

#include "RasterTileExtractor.h"
#include "VectorExtractor.h"
#include "defines.h"
#include "geoimage.h"
#include "geofeatures.h"

namespace godot {

class EXPORT Geodot : public Node {
    GODOT_CLASS(Geodot, Node)

private:
    Ref<Mutex> load_mutex;

    Dictionary image_cache;

public:
    /// Automatically called by Godot
    void _init();
    static void _register_methods();

    /// Get a GeoImage from the geodata at the given path, with the given parameters.
    /// The raster data at the given path should contain valid data for the given position and size.
    /// If there is a directory named <path>.pyramid, it is assumed to be pre-tiled data, and it is returned.
    ///  Note that for pre-tiled data, the position and size of the returned image can't exactly match the given position and size - the closest tile is chosen.
    ///  The format follows the OSM standard: <path>.pyramid/<zoom>/<tile_x>/<tile_y>.<file_ending>
    /// Otherwise, a georaster named <path>.<file_ending> is loaded and used.
    Ref<GeoImage> get_image(String path, String file_ending,
                            double top_left_x, double top_left_y, double size_meters,
                            int img_size, int interpolation_type);

    Array get_lines_near_position(String path, double pos_x, double pos_y, double radius, int max_lines);

    Array get_points_near_position(String path, double pos_x, double pos_y, double radius, int max_points);

    Array crop_lines_to_square(String path, double top_left_x, double top_left_y, double size_meters, int max_lines);

    Array get_all_features(String path, String layer_name);

    Geodot();
    ~Geodot();
};

}

#endif
