#ifndef LIGHTWORLD_H
#define LIGHTWORLD_H

#include "World.h"

class LightWorld : public World {
public:
    LightWorld();

    void makeCurrent() override;
    void update(float dt) override;
    void drawGeometry() override;

private:
    glm::vec3 m_lightOrigin;
    float m_lightTime;
};

#endif // LIGHTWORLD_H
