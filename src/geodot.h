#ifndef GEODOT_H
#define GEODOT_H

#include <Godot.hpp>
#include <Node.hpp>
#include <Image.hpp>
#include <Mutex.hpp>
#include <Resource.hpp>
#include <ImageTexture.hpp>
#include <Array.hpp>
#include <Path.hpp>
#include <Vector3.hpp>
#include <Curve3D.hpp>
#include "GeoRaster.h"
#include "RasterTileExtractor.h"
#include "LineFeature.h"
#include "VectorExtractor.h"

namespace godot {

// Wrapper for a GeoRaster from the RasterTileExtractor.
// Provides data in a format directly compatible with Godot (using Godot types).
// Think of it as an extension of a normal Godot Image / ImageTexture (which can be acquired with get_image / get_image_texture)
// Note that a GeoImage should not be created manually from Godot. It is only used as the return type of Geodot functions such as Geodot::get_image.
class GeoImage : public Resource {
    GODOT_CLASS(GeoImage, Resource)

public:
    GeoImage();
    ~GeoImage();

    /// Automatically called by Godot
    void _init();
    static void _register_methods();

    /// Import a GeoRaster and prepare the Godot Image
    /// Should not be called from the outside; Geodot only returns GeoImages which already have raster data.
    void set_raster(GeoRaster *raster, int interpolation);

    /// Get a Godot Image with the GeoImage's data
    Ref<Image> get_image();

    /// Get a Godot ImageTexture with the GeoImage's data
    Ref<ImageTexture> get_image_texture();

    /// Get the number_of_entries most common values in the raster.
    /// Only functional for single-band BYTE data!
    Array get_most_common(int number_of_entries);

private:
    GeoRaster *raster;

    Ref<Image> image;

    int interpolation;
};

// Wrapper for a LineFeature from the VectorExtractor.
class GeoLine : public Resource {
    GODOT_CLASS(GeoLine, Resource)

public:
    GeoLine();
    ~GeoLine();

    /// Automatically called by Godot
    void _init();
    static void _register_methods();

    /// Import a LineFeature from the libVectorExtractor
    /// Should not be called from the outside
    void set_line(LineFeature *line);

    String get_attribute(String name);

    Ref<Curve3D> get_as_curve3d();

private:
    LineFeature *line;
};

class Geodot : public Node {
    GODOT_CLASS(Geodot, Node)

private:
    Ref<Mutex> load_mutex;

public:
    // TODO: Not exportable? https://github.com/godotengine/godot/issues/15922
    enum INTERPOLATION {
        NEAREST,
        BILINEAR,
        CUBIC,
        CUBICSPLINE,
        LANCZOS,
        AVG,
        MODE,
        MAX,
        MIN,
        MED,
        Q1,
        Q2,
    };

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

    Array get_lines_near_position(String path, double pos_x, double pos_y, double radius);

    Geodot();
    ~Geodot();
};

}

#endif
