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
    Vector3 transform_coordinates(Vector3 coordinates, String from, String to);

    GeoUtil() = default;
    ~GeoUtil() = default;
};

} // namespace godot

#endif
