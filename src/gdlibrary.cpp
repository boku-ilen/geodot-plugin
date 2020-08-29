#include "geodot.h"

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *o) {
    godot::Godot::gdnative_init(o);
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *o) {
    godot::Godot::gdnative_terminate(o);
}

extern "C" void GDN_EXPORT godot_nativescript_init(void *handle) {
    godot::Godot::nativescript_init(handle);

    godot::register_class<godot::GeoImage>();
    godot::register_class<godot::GeoFeature>();
    godot::register_class<godot::GeoLine>();
    godot::register_class<godot::GeoPoint>();
    godot::register_class<godot::GeoPolygon>();
    godot::register_class<godot::GeoDataset>();
    godot::register_class<godot::GeoFeatureLayer>();
    godot::register_class<godot::GeoRasterLayer>();
    godot::register_class<godot::PyramidGeoRasterLayer>();
    godot::register_class<godot::Geodot>();
}
