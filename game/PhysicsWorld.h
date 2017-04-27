#ifndef PHYSICSWORLD_H
#define PHYSICSWORLD_H

#include "World.h"

struct Paint {
    Paint(std::pair<glm::vec3, glm::vec3> p, glm::vec3 c) : points(p), col(c) {}
    std::pair<glm::vec3, glm::vec3> points;
    glm::vec3 col;
};

class PhysicsWorld : public World {
public:
    PhysicsWorld();

    void makeCurrent() override;
    void update(float dt) override;
    void drawGeometry() override;

    void addPaint(glm::vec3 p1, glm::vec3 p2, glm::vec3 col) { m_paint.emplace_back(std::pair<glm::vec3, glm::vec3>(p1, p2), col); }

private:
    std::vector<Paint> m_paint;
};

#endif // PHYSICSWORLD_H
