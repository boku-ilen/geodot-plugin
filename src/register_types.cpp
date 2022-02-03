#include "register_types.h"

#include <godot/gdnative_interface.h>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>

#include "geodot.h"

using namespace godot;

void register_geodot_types() {
    ClassDB::register_class<GeoImage>();
    ClassDB::register_class<GeoFeature>();
    ClassDB::register_class<GeoLine>();
    ClassDB::register_class<GeoPoint>();
    ClassDB::register_class<GeoPolygon>();
    ClassDB::register_class<GeoDataset>();
    ClassDB::register_class<GeoFeatureLayer>();
    ClassDB::register_class<GeoRasterLayer>();
    ClassDB::register_class<PyramidGeoRasterLayer>();
    ClassDB::register_class<Geodot>();
}

void unregister_geodot_types() {}

extern "C" {

GDNativeBool GDN_EXPORT geodot_library_init(const GDNativeInterface *p_interface,
                                            const GDNativeExtensionClassLibraryPtr p_library,
                                            GDNativeInitialization *r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_interface, p_library, r_initialization);

    // Initialize the custom libraries
    RasterTileExtractor::initialize();
    VectorExtractor::initialize();

    init_obj.register_scene_initializer(register_geodot_types);
    init_obj.register_scene_terminator(unregister_geodot_types);

    return init_obj.init();
}
}
