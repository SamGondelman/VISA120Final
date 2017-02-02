#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include <memory>

#include <btBulletCollisionCommon.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

#include "CS123SceneData.h"

class CS123Shader;

class World {
public:
    World() {}
    World(std::string vert, std::string frag);
    virtual ~World() {}

    virtual void makeCurrent() = 0;
    virtual void update(float dt) = 0;
    virtual void drawGeometry() = 0;

    std::vector<Light>& getLights() { return m_lights; }
    std::shared_ptr<CS123Shader> getWorldProgram() { return m_program; }

protected:
    std::vector<Light> m_lights;
    std::shared_ptr<CS123Shader> m_program;
};

#endif // WORLD_H
