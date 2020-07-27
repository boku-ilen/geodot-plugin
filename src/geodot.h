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

private:
    Ref<Mutex> load_mutex;

    Dictionary image_cache;

public:
    /// Automatically called by Godot
    void _init();
    static void _register_methods();

    /// Returns a GeoDataset wrapping the georeferenced dataset at the given path.
    GeoDataset *get_dataset(String path);

    Geodot() = default;
    ~Geodot() = default;
};

}

#endif
