#ifndef PHYSICSWORLD_H
#define PHYSICSWORLD_H

#include "World.h"

class PhysicsWorld : public World {
public:
    PhysicsWorld();

    void makeCurrent() override {}
    void update(float dt) override {}
    void drawGeometry() override {}
};

#endif // PHYSICSWORLD_H
