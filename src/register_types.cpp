#include "register_types.h"

#include <godot/gdnative_interface.h>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>

#include "geodata.h"
#include "geoimage.h"
#include "geoutil.h"

using namespace godot;

void register_geodot_types(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) { return; }
    ClassDB::register_class<GeoImage>();
    ClassDB::register_class<GeoFeature>();
    ClassDB::register_class<GeoLine>();
    ClassDB::register_class<GeoPoint>();
    ClassDB::register_class<GeoPolygon>();
    ClassDB::register_class<GeoDataset>();
    ClassDB::register_class<GeoFeatureLayer>();
    ClassDB::register_class<GeoRasterLayer>();
    ClassDB::register_class<PyramidGeoRasterLayer>();
    ClassDB::register_class<GeoUtil>();
}

void unregister_geodot_types(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) { return; }
}

extern "C" {

GDNativeBool GDN_EXPORT geodot_library_init(const GDNativeInterface *p_interface,
                                            const GDNativeExtensionClassLibraryPtr p_library,
                                            GDNativeInitialization *r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_interface, p_library, r_initialization);

    // Initialize the custom libraries
    RasterTileExtractor::initialize();
    VectorExtractor::initialize();

    init_obj.register_initializer(register_geodot_types);
    init_obj.register_terminator(unregister_geodot_types);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}
