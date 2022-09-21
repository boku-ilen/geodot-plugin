#ifndef __RASTER_H__
#define __RASTER_H__

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/mutex.hpp>

#include <godot_cpp/core/binder_common.hpp>

#include "GeoRaster.h"
#include "defines.h"

namespace godot {

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

// Wrapper for a GeoRaster from the RasterTileExtractor.
// Provides data in a format directly compatible with Godot (using Godot types).
// Think of it as an extension of a normal Godot Image / ImageTexture (which can
// be acquired with get_image / get_image_texture) Note that a GeoImage should
// not be created manually from Godot. It is only used as the return type of
// Geodot functions such as Geodot::get_image.
class EXPORT GeoImage : public Resource {
    GDCLASS(GeoImage, Resource)

  protected:
    static void _bind_methods();

  public:
    GeoImage();
    ~GeoImage();

    bool is_valid();

    /// Import a GeoRaster and prepare the Godot Image
    /// Should not be called from the outside; Geodot only returns GeoImages
    /// which already have raster data.
    void set_raster(GeoRaster *raster, int interpolation);

    /// Get a Godot Image with the GeoImage's data
    Ref<Image> get_image();

    /// Get a Godot ImageTexture with the GeoImage's data
    Ref<ImageTexture> get_image_texture();

    /// Assuming the image is a heightmap, return the normal map corresponding
    /// to that heightmap. Somewhat costly because the generated heightmap has a
    /// higher precision than Godot's Image::bumpmap_to_normalmap.
    Ref<Image> get_normalmap_for_heightmap(float scale);

    /// Wrapper for get_normalmap_for_heightmap which directly provides an
    /// ImageTexture with the image.
    Ref<ImageTexture> get_normalmap_texture_for_heightmap(float scale);

    /// Get the number_of_entries most common values in the raster.
    /// Only functional for single-band BYTE data!
    Array get_most_common(int number_of_entries);

  private:
    GeoRaster *raster;

    Ref<Image> image;

    Ref<Image> normalmap;

    Ref<Mutex> normalmap_load_mutex;

    int interpolation;

    bool validity = false;
};

} // namespace godot

#endif // __RASTER_H__