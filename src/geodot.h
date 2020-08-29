#ifndef GEODOT_H
#define GEODOT_H

#include <Godot.hpp>
#include <Node.hpp>
#include <Mutex.hpp>
#include <Array.hpp>

#include "RasterTileExtractor.h"
#include "VectorExtractor.h"
#include "defines.h"
#include "geoimage.h"
#include "geofeatures.h"
#include "geodata.h"

namespace godot {

class EXPORT Geodot : public Node {
    GODOT_CLASS(Geodot, Node)

public:
    /// Automatically called by Godot
    void _init();
    static void _register_methods();

    /// Return a GeoDataset wrapping the georeferenced dataset at the given path.
    Ref<GeoDataset> get_dataset(String path);

    /// Return a GeoRasterLayer (with no parent GeoDataset) wrapping the slippy tilename
    ///  pyramid at the given path.
    /// This is a special case / workaround for this type of data, as it is not encapsulated
    ///  by a dataset.
    /// Will possibly become deprecated.
    Ref<GeoRasterLayer> get_raster_layer_for_pyramid(String root_folder, String image_ending);

    Geodot() = default;
    ~Geodot() = default;
};

}

#endif
