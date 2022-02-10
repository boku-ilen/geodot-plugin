#ifndef GEODOT_H
#define GEODOT_H

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/node.hpp>

#include <godot_cpp/core/binder_common.hpp>

#include "defines.h"
#include "geodata.h"

namespace godot {

class EXPORT GeoUtil : public Object {
    GDCLASS(GeoUtil, Object)

  protected:
    static void _bind_methods();

  public:
    /// Return a GeoRasterLayer (with no parent GeoDataset) wrapping the slippy
    /// tilename
    ///  pyramid at the given path.
    /// This is a special case / workaround for this type of data, as it is not
    /// encapsulated
    ///  by a dataset.
    /// Will possibly become deprecated.
    Ref<GeoRasterLayer> get_raster_layer_for_pyramid(String root_folder, String image_ending);

    Vector3 transform_coordinates(Vector3 coordinates, String from, String to);

    GeoUtil() = default;
    ~GeoUtil() = default;
};

} // namespace godot

#endif
