#include "test.h"

void GeoTest::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_valid"), &GeoTest::is_valid);
}

void GeoTest::is_valid() {}