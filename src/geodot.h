#ifndef GEODOT_H
#define GEODOT_H

#include <Array.hpp>
#include <Godot.hpp>
#include <Mutex.hpp>
#include <Node.hpp>

#include "defines.h"
#include "geodata.h"

namespace godot {

class EXPORT Geodot : public Node {
    GODOT_CLASS(Geodot, Node)

  public:
    /// Automatically called by Godot
    void _init();
    static void _register_methods();

    /// Return a GeoDataset wrapping the georeferenced dataset at the given
    /// path.
    Ref<GeoDataset> get_dataset(String path);

    /// Return a GeoRasterLayer wrapping the georeferenced image at the given
    /// path. To be used only for individual images -- for datasets such as
    /// GeoPackages which contain raster data, use `get_dataset(path).get_raster_layer(name)`
    /// instead.
    Ref<GeoRasterLayer> get_raster_layer(String path);

    /// Return a GeoRasterLayer (with no parent GeoDataset) wrapping the slippy
    /// tilename
    ///  pyramid at the given path.
    /// This is a special case / workaround for this type of data, as it is not
    /// encapsulated
    ///  by a dataset.
    /// Will possibly become deprecated.
    Ref<GeoRasterLayer> get_raster_layer_for_pyramid(String root_folder, String image_ending);

    Vector3 transform_coordinates(Vector3 coordinates, String from, String to);

    Geodot() = default;
    ~Geodot() = default;
};

} // namespace godot

#endif
