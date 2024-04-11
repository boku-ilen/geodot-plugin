#include "geofeatures.h"

using namespace godot;

// GeoFeature

void GeoFeature::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_attribute", "name"), &GeoFeature::get_attribute);
    ClassDB::bind_method(D_METHOD("set_attribute", "name", "value"), &GeoFeature::set_attribute);
    ClassDB::bind_method(D_METHOD("get_attributes"), &GeoFeature::get_attributes);
    ClassDB::bind_method(D_METHOD("get_id"), &GeoFeature::get_id);
    ClassDB::bind_method(D_METHOD("intersects_with", "other"), &GeoFeature::intersects_with);

    ADD_SIGNAL(MethodInfo("feature_changed"));
}

String GeoFeature::get_attribute(String name) const {
    return String::utf8(gdal_feature->get_attribute(name.utf8().get_data()));
}

void GeoFeature::set_attribute(String name, String value) {
    gdal_feature->set_attribute(name.utf8().get_data(), value.utf8().get_data());
}

int GeoFeature::get_id() const {
    return gdal_feature->get_id();
}

Dictionary GeoFeature::get_attributes() const {
    Dictionary attributes = Dictionary();

    std::map<std::string, std::string> attribute_map = gdal_feature->get_attributes();

    for (const auto &attribute : attribute_map) {
        attributes[attribute.first.c_str()] = String::utf8(attribute.second.c_str());
    }

    return attributes;
}

void GeoFeature::set_gdal_feature(std::shared_ptr<Feature> gdal_feature) {
    this->gdal_feature = gdal_feature;
}

void GeoFeature::set_deleted(bool is_deleted) {
    this->gdal_feature->is_deleted = is_deleted;
}

bool GeoFeature::intersects_with(Ref<GeoFeature> other) {
    return this->gdal_feature->intersects_with(other->gdal_feature);
}

// GeoPoint

void GeoPoint::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_vector3"), &GeoPoint::get_vector3);
    ClassDB::bind_method(D_METHOD("get_float_offset_vector3", "offset_x", "offset_y", "offset_z"),
                         &GeoPoint::get_float_offset_vector3);
    ClassDB::bind_method(D_METHOD("get_offset_vector3", "offset_x", "offset_y", "offset_z"),
                         &GeoPoint::get_offset_vector3);
    ClassDB::bind_method(D_METHOD("set_vector3", "vector"), &GeoPoint::set_vector3);
    ClassDB::bind_method(
        D_METHOD("set_float_offset_vector3", "vector", "offset_x", "offset_y", "offset_z"),
        &GeoPoint::set_float_offset_vector3);
    ClassDB::bind_method(
        D_METHOD("set_offset_vector3", "vector", "offset_x", "offset_y", "offset_z"),
        &GeoPoint::set_offset_vector3);
}

Vector3 GeoPoint::get_float_offset_vector3(double offset_x, double offset_y, double offset_z) {
    std::shared_ptr<PointFeature> point = std::dynamic_pointer_cast<PointFeature>(gdal_feature);

    return Vector3(point->get_x() + offset_x, point->get_z() + offset_y,
                   -point->get_y() - offset_z);
}

Vector3 GeoPoint::get_offset_vector3(int offset_x, int offset_y, int offset_z) {
    return get_float_offset_vector3(offset_x, offset_y, offset_z);
}

void GeoPoint::set_float_offset_vector3(Vector3 vector, double offset_x, double offset_y, double offset_z) {
    std::shared_ptr<PointFeature> point = std::dynamic_pointer_cast<PointFeature>(gdal_feature);

    // Internally, a different coordinate system is used (Z up and reversed), which is why the
    // coordinates are passed in a different order here.
    point->set_vector(offset_x + vector.x, offset_z - vector.z, offset_y + vector.y);

    emit_signal("feature_changed");
}

void GeoPoint::set_offset_vector3(Vector3 vector, int offset_x, int offset_y, int offset_z) {
    set_float_offset_vector3(vector, offset_x, offset_y, offset_z);
}

Vector3 GeoPoint::get_vector3() {
    return get_offset_vector3(0, 0, 0);
}

void GeoPoint::set_vector3(Vector3 vector) {
    set_offset_vector3(vector, 0, 0, 0);
}

// GeoLine

void GeoLine::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_curve3d"), &GeoLine::get_curve3d);
    ClassDB::bind_method(D_METHOD("get_offset_curve3d", "offset_x", "offset_y", "offset_z"),
                         &GeoLine::get_offset_curve3d);
    ClassDB::bind_method(D_METHOD("get_float_offset_curve3d", "offset_x", "offset_y", "offset_z"),
                         &GeoLine::get_float_offset_curve3d);
    ClassDB::bind_method(D_METHOD("set_curve3d", "curve"), &GeoLine::set_curve3d);
    ClassDB::bind_method(
        D_METHOD("set_float_offset_curve3d", "curve", "offset_x", "offset_y", "offset_z"),
        &GeoLine::set_float_offset_curve3d);
    ClassDB::bind_method(
        D_METHOD("set_offset_curve3d", "curve", "offset_x", "offset_y", "offset_z"),
        &GeoLine::set_offset_curve3d);
    ClassDB::bind_method(D_METHOD("add_point", "point"), &GeoLine::add_point);
}

Ref<Curve3D> GeoLine::get_float_offset_curve3d(double offset_x, double offset_y, double offset_z) {
    std::shared_ptr<LineFeature> line = std::dynamic_pointer_cast<LineFeature>(gdal_feature);

    Ref<Curve3D> curve;
    curve.instantiate();

    int point_count = line->get_point_count();

    for (int i = 0; i < point_count; i++) {
        // Note: y and z are swapped because of differences in the coordinate
        // system!
        double x = line->get_line_point_x(i) + offset_x;
        double y = line->get_line_point_z(i) + offset_y;
        double z = -line->get_line_point_y(i) - offset_z;

        curve->add_point(Vector3(x, y, z));
    }

    return curve;
}

Ref<Curve3D> GeoLine::get_offset_curve3d(int offset_x, int offset_y, int offset_z) {
    return get_float_offset_curve3d(offset_x, offset_y, offset_z);
}

void GeoLine::set_float_offset_curve3d(Ref<Curve3D> curve, double offset_x, double offset_y, double offset_z) {
    std::shared_ptr<LineFeature> line = std::dynamic_pointer_cast<LineFeature>(gdal_feature);
    int point_count = curve->get_point_count();

    // Update the number of points to free up space or create new space, depending on the difference
    line->set_point_count(point_count);

    // Set all points in the LineFeature according to the Curve3D
    for (int i = 0; i < point_count; i++) {
        Vector3 position = curve->get_point_position(i);
        line->set_line_point(i, position.x + offset_x,
                             position.z + offset_z,
                             -position.y - offset_y);
    }

    emit_signal("feature_changed");
}

void GeoLine::set_offset_curve3d(Ref<Curve3D> curve, int offset_x, int offset_y, int offset_z) {
    set_float_offset_curve3d(curve, offset_x, offset_y, offset_z);
}

Ref<Curve3D> GeoLine::get_curve3d() {
    return get_offset_curve3d(0, 0, 0);
}

void GeoLine::set_curve3d(Ref<Curve3D> curve) {
    set_offset_curve3d(curve, 0, 0, 0);

    emit_signal("feature_changed");
}

void GeoLine::add_point(Vector3 point) {
    std::shared_ptr<LineFeature> line = std::dynamic_pointer_cast<LineFeature>(gdal_feature);
    line->set_point_count(line->get_point_count() + 1);
    line->set_line_point(line->get_point_count() - 1, point.x, point.y, point.z);

    emit_signal("feature_changed");
}

// GeoPolygon

void GeoPolygon::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_outer_vertices"), &GeoPolygon::get_outer_vertices);
    ClassDB::bind_method(D_METHOD("get_float_offset_outer_vertices"), &GeoPolygon::get_float_offset_outer_vertices);
    ClassDB::bind_method(D_METHOD("get_offset_outer_vertices"), &GeoPolygon::get_offset_outer_vertices);
    ClassDB::bind_method(D_METHOD("set_outer_vertices", "vertices"),
                         &GeoPolygon::set_outer_vertices);
    ClassDB::bind_method(D_METHOD("set_float_offset_outer_vertices", "offset_x", "offset_y", "vertices"),
                         &GeoPolygon::set_float_offset_outer_vertices);
    ClassDB::bind_method(D_METHOD("set_offset_outer_vertices", "offset_x", "offset_y", "vertices"),
                         &GeoPolygon::set_offset_outer_vertices);
    ClassDB::bind_method(D_METHOD("get_holes"), &GeoPolygon::get_holes);
    ClassDB::bind_method(D_METHOD("get_float_offset_holes", "offset_x", "offset_y"), &GeoPolygon::get_float_offset_holes);
    ClassDB::bind_method(D_METHOD("get_offset_holes", "offset_x", "offset_y"), &GeoPolygon::get_offset_holes);
    ClassDB::bind_method(D_METHOD("add_hole", "hole"), &GeoPolygon::add_hole);
}

PackedVector2Array GeoPolygon::get_float_offset_outer_vertices(double offset_x, double offset_y) {
    PackedVector2Array vertices = PackedVector2Array();
    std::shared_ptr<PolygonFeature> polygon = std::dynamic_pointer_cast<PolygonFeature>(gdal_feature);

    std::list<std::vector<double>> raw_outer_vertices = polygon->get_outer_vertices();

    for (const std::vector<double> vertex : raw_outer_vertices) {
        vertices.push_back(Vector2(vertex[0] + offset_x, vertex[1] + offset_y));
    }

    return vertices;
}

PackedVector2Array GeoPolygon::get_offset_outer_vertices(int offset_x, int offset_y) {
    return get_float_offset_outer_vertices(offset_x, offset_y);
}

PackedVector2Array GeoPolygon::get_outer_vertices() {
    return get_offset_outer_vertices(0.0, 0.0);
}

void GeoPolygon::set_outer_vertices(PackedVector2Array vertices) {
    set_float_offset_outer_vertices(0.0, 0.0, vertices);
}

void GeoPolygon::set_float_offset_outer_vertices(double offset_x, double offset_y, PackedVector2Array vertices) {
    std::list<std::vector<double>> new_outer_vertices;

    for (int i = 0; i < vertices.size(); i += 1) {
        new_outer_vertices.emplace_back(std::vector<double>{vertices[i].x + offset_x, vertices[i].y + offset_y});
    }

    std::shared_ptr<PolygonFeature> polygon = std::dynamic_pointer_cast<PolygonFeature>(gdal_feature);
    polygon->set_outer_vertices(new_outer_vertices);
}

void GeoPolygon::set_offset_outer_vertices(int offset_x, int offset_y, PackedVector2Array vertices) {
    set_float_offset_outer_vertices(offset_x, offset_y, vertices);
}

Array GeoPolygon::get_holes() {
    return get_offset_holes(0.0, 0.0);
}

Array GeoPolygon::get_float_offset_holes(double offset_x, double offset_y) {
    Array holes = Array();
    std::shared_ptr<PolygonFeature> polygon = std::dynamic_pointer_cast<PolygonFeature>(gdal_feature);

    std::list<std::list<std::vector<double>>> raw_holes = polygon->get_holes();

    for (const std::list<std::vector<double>> raw_hole_vertices : raw_holes) {
        PackedVector2Array hole_vertices = PackedVector2Array();

        for (const std::vector<double> vertex : raw_hole_vertices) {
            hole_vertices.push_back(Vector2(vertex[0] + offset_x, vertex[1] + offset_y));
        }

        holes.push_back(hole_vertices);
    }

    return holes;
}

Array GeoPolygon::get_offset_holes(int offset_x, int offset_y) {
    return get_float_offset_holes(offset_x, offset_y);
}

void GeoPolygon::add_hole(PackedVector2Array hole) {
    // TODO: Implement
}
