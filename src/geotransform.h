#ifndef GEODOT_H
#define GEODOT_H

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/node.hpp>

#include <godot_cpp/core/binder_common.hpp>

#include "VectorExtractor.h"
#include "defines.h"
#include "geodata.h"

namespace godot {

class EXPORT GeoTransform : public Object {
    GDCLASS(GeoTransform, Object)

  protected:
    static void _bind_methods();

  public:
    GeoTransform() = default;
    ~GeoTransform();

    void set_transform(int from, int to);
    
    /// Interprets the given vector as a point in the coordinate system defined by "from" in a previous `set_transform` call and converts
    /// it to the coordinate system described by "to" in a previous `set_transform` call.
    Vector3 transform_coordinates(Vector3 coordinates);
  
  private:
    CoordinateTransform *transform;
};

} // namespace godot

#endif
