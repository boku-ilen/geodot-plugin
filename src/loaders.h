#pragma once


#include "godot_cpp/variant/packed_string_array.hpp"
#include <godot_cpp/classes/resource_format_loader.hpp>

namespace godot {

class GeoDatasetLoader : public ResourceFormatLoader {

    GDCLASS(GeoDatasetLoader, ResourceFormatLoader);

protected:
    static void _bind_methods();

public:
    
    PackedStringArray _get_recognized_extensions() const override;

    bool _handles_type(const StringName &type) const override;

    String _get_resource_script_class(const String &path) const override;

    String _get_resource_type(const String &path) const override;

    Variant _load(const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const override;

};

class GeoRasterLayerLoader : public ResourceFormatLoader {

    GDCLASS(GeoRasterLayerLoader, ResourceFormatLoader);

protected:
    static void _bind_methods();

public:
    
    PackedStringArray _get_recognized_extensions() const override;

    bool _handles_type(const StringName &type) const override;

    String _get_resource_script_class(const String &path) const override;

    String _get_resource_type(const String &path) const override;

    Variant _load(const String &p_path, const String &p_original_path, bool p_use_sub_threads, int32_t p_cache_mode) const override;

};

}