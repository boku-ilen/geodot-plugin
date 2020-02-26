#include "geodot.h"

using namespace godot;

void Geodot::_register_methods() {
    register_method("_process", &Geodot::_process);
    register_method("get_time_passed", &Geodot::get_time_passed);
}

Geodot::Geodot() {
}

Geodot::~Geodot() {
    // add your cleanup here
}

void Geodot::_init() {
    // initialize any variables here
    time_passed = 0.0;
}

void Geodot::_process(float delta) {
    time_passed += delta;
}

float Geodot::get_time_passed() {
    return time_passed;
}
