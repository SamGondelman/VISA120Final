#ifndef DEMOWORLD_H
#define DEMOWORLD_H

#include "World.h"

class DemoWorld : public World {
public:
    DemoWorld();

    void makeCurrent() override;
    void update(float dt) override;
    void drawGeometry() override;
};

#endif // DEMOWORLD_H
