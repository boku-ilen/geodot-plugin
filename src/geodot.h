#ifndef GEODOT_H
#define GEODOT_H

#include <Godot.hpp>
#include <Node.hpp>

namespace godot {

class Geodot : public Node {
    GODOT_CLASS(Geodot, Node)

private:
    float time_passed;

public:
    static void _register_methods();

    Geodot();
    ~Geodot();

    void _init(); // our initializer called by Godot

    void _process(float delta);
    
    float get_time_passed();
};

}

#endif
