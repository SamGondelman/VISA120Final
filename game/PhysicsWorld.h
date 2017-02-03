#ifndef PHYSICSWORLD_H
#define PHYSICSWORLD_H

#include "World.h"

#include "Entity.h"

class PhysicsWorld : public World {
public:
    PhysicsWorld();

    void makeCurrent() override;
    void update(float dt) override;
    void drawGeometry() override;

private:
    std::unique_ptr<btDefaultCollisionConfiguration> m_configuration;
    std::unique_ptr<btCollisionDispatcher> m_dispatcher;
    std::unique_ptr<btBroadphaseInterface> m_broadphase;
    std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
    std::shared_ptr<btDiscreteDynamicsWorld> m_physWorld;

    // TODO: figure out how to make this a std::vector<std::shared_ptr<Entity>>
    std::vector<Entity> m_entities;
};

#endif // PHYSICSWORLD_H
