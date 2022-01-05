#ifndef __FEATURES_H__
#define __FEATURES_H__

#include <Curve3D.hpp>
#include <Godot.hpp>

#include "Feature.h"
#include "LineFeature.h"
#include "PointFeature.h"
#include "PolygonFeature.h"
#include "defines.h"

namespace godot {

// Wrapper for any georeferenced feature from GDAL.
class EXPORT GeoFeature : public Resource {
    GODOT_CLASS(GeoFeature, Resource)

  public:
    GeoFeature();
    virtual ~GeoFeature();

    /// Automatically called by Godot
    void _init() {} // Must be here as Godot always calls this for Objects
    static void _register_methods();

    String get_attribute(String name) const;
    void set_attribute(String name, String value);

    int get_id() const;

    Dictionary get_attributes() const;

    void set_gdal_feature(Feature *gdal_feature);

  protected:
    Feature *gdal_feature;
};

// Wrapper for a PointFeature from the VectorExtractor.
class EXPORT GeoPoint : public GeoFeature {
    GODOT_SUBCLASS(GeoPoint, GeoFeature)

  public:
    GeoPoint() = default;
    ~GeoPoint() = default;

    /// Automatically called by Godot
    void _init() {} // Must be here as Godot always calls this for Objects
    static void _register_methods();

    Vector3 get_offset_vector3(int offset_x, int offset_y, int offset_z);
    void set_offset_vector3(Vector3 vector, int offset_x, int offset_y, int offset_z);

    Vector3 get_vector3();
    void set_vector3(Vector3 vector);
};

// Wrapper for a LineFeature from the VectorExtractor.
class EXPORT GeoLine : public GeoFeature {
    GODOT_SUBCLASS(GeoLine, GeoFeature)

  public:
    GeoLine() = default;
    ~GeoLine() = default;

    /// Automatically called by Godot
    void _init() {} // Must be here as Godot always calls this for Objects
    static void _register_methods();

    Ref<Curve3D> get_offset_curve3d(int offset_x, int offset_y, int offset_z);
    void set_offset_curve3d(Ref<Curve3D> curve, int offset_x, int offset_y, int offset_z);

    Ref<Curve3D> get_curve3d();
    void set_curve3d(Ref<Curve3D> curve);

    void add_point(Vector3 point);
};

// Wrapper for a PolygonFeature from the VectorExtractor.
class EXPORT GeoPolygon : public GeoFeature {
    GODOT_SUBCLASS(GeoPolygon, GeoFeature)

  public:
    GeoPolygon() = default;
    ~GeoPolygon() = default;

    /// Automatically called by Godot
    void _init() {} // Must be here as Godot always calls this for Objects
    static void _register_methods();

    /// Return the vertices making up the base polygon in a PoolVector2Array.
    PoolVector2Array get_outer_vertices();
    void set_outer_vertices(PoolVector2Array vertices);

    /// Return a list with any number of PoolVector2Arrays. These represent
    /// polygons that
    ///  should be cut out of the base polygon retreived by get_outer_vertices.
    Array get_holes();
    void add_hole(PoolVector2Array hole);
};

} // namespace godot

#endif // __FEATURES_H__