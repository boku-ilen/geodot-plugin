#ifndef GEODOT_H
#define GEODOT_H

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/node.hpp>

#include <godot_cpp/core/binder_common.hpp>

#include "defines.h"
#include "geodata.h"

namespace godot {

class EXPORT GeoTransform : public Object {
    GDCLASS(GeoTransform, Object)

  protected:
    static void _bind_methods();

  public:
    /// Interprets the given vector as a point in the coordinate system defined by the given "from" EPSG code
    ///  and converts it to the coordinate system defined by the given "to" EPSG code.
    static Vector3 transform_coordinates(Vector3 coordinates, int from, int to);

    GeoTransform() = default;
    ~GeoTransform() = default;
};

} // namespace godot

#endif
