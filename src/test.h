#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/mutex.hpp>

#include <godot_cpp/core/binder_common.hpp>

using namespace godot;

class GeoTest : public RefCounted {
    GDCLASS(GeoTest, RefCounted)

  protected:
    static void _bind_methods();

  public:
    void is_valid();
};