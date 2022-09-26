#include "geofeatures.h"

using namespace godot;

// GeoFeature

GeoFeature::GeoFeature() {}

GeoFeature::~GeoFeature() {
    // FIXME: Decrease the GDAL feature's reference count?
}

void GeoFeature::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_attribute"), &GeoFeature::get_attribute);
    ClassDB::bind_method(D_METHOD("set_attribute"), &GeoFeature::set_attribute);
    ClassDB::bind_method(D_METHOD("get_attributes"), &GeoFeature::get_attributes);
    ClassDB::bind_method(D_METHOD("get_id"), &GeoFeature::get_id);
    ClassDB::bind_method(D_METHOD("intersects_with"), &GeoFeature::intersects_with);
}

String GeoFeature::get_attribute(String name) const {
    return gdal_feature->get_attribute(name.utf8().get_data());
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
        attributes[attribute.first.c_str()] = attribute.second.c_str();
    }

    return attributes;
}

void GeoFeature::set_gdal_feature(Feature *gdal_feature) {
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
    ClassDB::bind_method(D_METHOD("get_offset_vector3"), &GeoPoint::get_offset_vector3);
    ClassDB::bind_method(D_METHOD("set_vector3"), &GeoPoint::set_vector3);
    ClassDB::bind_method(D_METHOD("set_offset_vector3"), &GeoPoint::set_offset_vector3);

    ADD_SIGNAL(MethodInfo("point_changed"));
}

Vector3 GeoPoint::get_offset_vector3(int offset_x, int offset_y, int offset_z) {
    PointFeature *point = dynamic_cast<PointFeature *>(gdal_feature);

    return Vector3(point->get_x() + offset_x, point->get_z() + offset_y,
                   -(point->get_y() + offset_z));
}

void GeoPoint::set_offset_vector3(Vector3 vector, int offset_x, int offset_y, int offset_z) {
    PointFeature *point = dynamic_cast<PointFeature *>(gdal_feature);

    // Internally, a different coordinate system is used (Z up and reversed), which is why the
    // coordinates are passed in a different order here.
    point->set_vector(offset_x + vector.x, offset_z - vector.z, offset_y + vector.y);

    emit_signal("point_changed");
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
    ClassDB::bind_method(D_METHOD("get_offset_curve3d"), &GeoLine::get_offset_curve3d);
    ClassDB::bind_method(D_METHOD("set_curve3d"), &GeoLine::set_curve3d);
    ClassDB::bind_method(D_METHOD("set_offset_curve3d"), &GeoLine::set_offset_curve3d);
    ClassDB::bind_method(D_METHOD("add_point"), &GeoLine::add_point);

    ADD_SIGNAL(MethodInfo("line_changed"));
}

Ref<Curve3D> GeoLine::get_offset_curve3d(int offset_x, int offset_y, int offset_z) {
    LineFeature *line = (LineFeature *)gdal_feature;

    Ref<Curve3D> curve;
    curve.instantiate();

    int point_count = line->get_point_count();

    for (int i = 0; i < point_count; i++) {
        // Note: y and z are swapped because of differences in the coordinate
        // system!
        double x = line->get_line_point_x(i) + static_cast<double>(offset_x);
        double y = line->get_line_point_z(i) + static_cast<double>(offset_y);
        double z = -(line->get_line_point_y(i) + static_cast<double>(offset_z));

        curve->add_point(Vector3(x, y, z));
    }

    return curve;
}

void GeoLine::set_offset_curve3d(Ref<Curve3D> curve, int offset_x, int offset_y, int offset_z) {
    LineFeature *line = (LineFeature *)gdal_feature;
    int point_count = curve->get_point_count();

    // Update the number of points to free up space or create new space, depending on the difference
    line->set_point_count(point_count);

    // Set all points in the LineFeature according to the Curve3D
    for (int i = 0; i < point_count; i++) {
        Vector3 position = curve->get_point_position(i);
        line->set_line_point(i, position.x - static_cast<double>(offset_x),
                             position.z - static_cast<double>(offset_x),
                             -position.y - static_cast<double>(offset_x));
    }

    emit_signal("line_changed");
}

Ref<Curve3D> GeoLine::get_curve3d() {
    return get_offset_curve3d(0, 0, 0);
}

void GeoLine::set_curve3d(Ref<Curve3D> curve) {
    set_offset_curve3d(curve, 0, 0, 0);

    emit_signal("line_changed");
}

void GeoLine::add_point(Vector3 point) {
    LineFeature *line = (LineFeature *)gdal_feature;
    line->set_point_count(line->get_point_count() + 1);
    line->set_line_point(line->get_point_count() - 1, point.x, point.y, point.z);

    emit_signal("line_changed");
}

// GeoPolygon

void GeoPolygon::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_outer_vertices"), &GeoPolygon::get_outer_vertices);
    ClassDB::bind_method(D_METHOD("set_outer_vertices"), &GeoPolygon::set_outer_vertices);
    ClassDB::bind_method(D_METHOD("get_holes"), &GeoPolygon::get_holes);
    ClassDB::bind_method(D_METHOD("add_hole"), &GeoPolygon::add_hole);
}

PackedVector2Array GeoPolygon::get_outer_vertices() {
    PackedVector2Array vertices = PackedVector2Array();
    PolygonFeature *polygon = (PolygonFeature *)gdal_feature;

    std::list<std::vector<double>> raw_outer_vertices = polygon->get_outer_vertices();

    for (const std::vector<double> vertex : raw_outer_vertices) {
        vertices.push_back(Vector2(vertex[0], vertex[1]));
    }

    return vertices;
}

void GeoPolygon::set_outer_vertices(PackedVector2Array vertices) {
    // TODO: Implement
}

Array GeoPolygon::get_holes() {
    Array holes = Array();
    PolygonFeature *polygon = (PolygonFeature *)gdal_feature;

    std::list<std::list<std::vector<double>>> raw_holes = polygon->get_holes();

    for (const std::list<std::vector<double>> raw_hole_vertices : raw_holes) {
        PackedVector2Array hole_vertices = PackedVector2Array();

        for (const std::vector<double> vertex : raw_hole_vertices) {
            hole_vertices.push_back(Vector2(vertex[0], vertex[1]));
        }

        holes.push_back(hole_vertices);
    }

    return holes;
}

void GeoPolygon::add_hole(PackedVector2Array hole) {
    // TODO: Implement
}