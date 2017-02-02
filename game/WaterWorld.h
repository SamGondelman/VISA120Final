#ifndef WATERWORLD_H
#define WATERWORLD_H

#include "World.h"

class WaterWorld : public World {
public:
    WaterWorld();

    void makeCurrent() override;
    void update(float dt) override;
    void drawGeometry() override;
};

#endif // WATERWORLD_H
