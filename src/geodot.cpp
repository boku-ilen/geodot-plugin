#include "geodot.h"
#include "RasterTileExtractor.h"

#include <algorithm> // For std::clamp
#include <functional> // For std::hash
#include <mutex> // For std::mutex

std::mutex resource_creation_mutex;


using namespace godot;

void Geodot::_init() {
    RasterTileExtractor::initialize();
    VectorExtractor::initialize();
    load_mutex = Ref<Mutex>(Mutex::_new());
    image_cache = Dictionary();
}

void Geodot::_register_methods() {
    register_method("get_dataset", &Geodot::get_dataset);
}

GeoDataset *Geodot::get_dataset(String path) {
    GeoDataset *dataset = GeoDataset::_new();

    dataset->load_from_file(path);

    return dataset;
}

/* Ref<GeoImage> Geodot::get_image(String path, String file_ending,
                                double top_left_x, double top_left_y, double size_meters,
                                int img_size, int interpolation_type) {
    load_mutex->lock();

    int image_hash = std::hash<std::string>()(std::string(path.utf8().get_data()) + std::string(file_ending.utf8().get_data()) + std::to_string(top_left_x) + std::to_string(top_left_y) + std::to_string(size_meters) + std::to_string(img_size) + std::to_string(interpolation_type));

    if (image_cache.has(image_hash)) {
        load_mutex->unlock();
        return image_cache[image_hash];
    } else {
        resource_creation_mutex.lock();

        // This strange __internal_constructor call is required to prevent a memory leak
        // See https://github.com/GodotNativeTools/godot-cpp/issues/215
        // TODO: Should be fixed according to that issue!
        Ref<GeoImage> image = Ref<GeoImage>::__internal_constructor(GeoImage::_new());

        GeoRaster *raster = RasterTileExtractor::get_raster_at_position(
            path.utf8().get_data(),
            file_ending.utf8().get_data(),
            top_left_x, top_left_y, size_meters,
            img_size, interpolation_type);

        if (raster == nullptr) {
            resource_creation_mutex.unlock();
            load_mutex->unlock();
            Godot::print_error("No valid data was available for the requested path and position!", "Geodot::get_image", "geodot.cpp", 26);
            return image;
        }

        image->set_raster(raster, interpolation_type);

        image_cache[image_hash] = image;

        resource_creation_mutex.unlock();
        load_mutex->unlock();

        return image;
    }
}

Array Geodot::get_lines_near_position(String path, double pos_x, double pos_y, double radius, int max_lines) {
    Array lines = Array();

    std::list<LineFeature *> linefeatures = VectorExtractor::get_lines_near_position(path.utf8().get_data(), pos_x, pos_y, radius, max_lines);

    resource_creation_mutex.lock();

    for (LineFeature *linefeature : linefeatures) {
        Ref<GeoLine> line = GeoLine::_new();
        line->set_gdal_feature(linefeature);

        lines.push_back(line);
    }

    resource_creation_mutex.unlock();

    return lines;
}

Array Geodot::get_points_near_position(String path, double pos_x, double pos_y, double radius, int max_points) {
    Array points = Array();

    std::list<PointFeature *> pointfeatures = VectorExtractor::get_points_near_position(path.utf8().get_data(), pos_x, pos_y, radius, max_points);

    resource_creation_mutex.lock();

    for (PointFeature *pointfeature : pointfeatures) {
        Ref<GeoPoint> point = GeoPoint::_new();
        point->set_gdal_feature(pointfeature);

        points.push_back(point);
    }

    resource_creation_mutex.unlock();

    return points;
}

Array Geodot::crop_lines_to_square(String path, double top_left_x, double top_left_y, double size_meters, int max_lines) {
    Array lines = Array();

    std::list<LineFeature *> linefeatures = VectorExtractor::crop_lines_to_square(path.utf8().get_data(), top_left_x, top_left_y, size_meters, max_lines);

    resource_creation_mutex.lock();

    for (LineFeature *linefeature : linefeatures) {
        Ref<GeoLine> line = GeoLine::_new();
        line->set_gdal_feature(linefeature);

        lines.push_back(line);
    }

    resource_creation_mutex.unlock();

    return lines;
}
 */