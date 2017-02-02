#ifndef PHYSICSWORLD_H
#define PHYSICSWORLD_H

#include "World.h"

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
    std::unique_ptr<btDiscreteDynamicsWorld> m_physWorld;

    std::vector<std::shared_ptr<btCollisionShape>> m_collShapes;
    std::vector<std::shared_ptr<btDefaultMotionState>> m_motionStates;
    std::vector<std::shared_ptr<btRigidBody>> m_rigidBodies;

    void addRigidBody(std::shared_ptr<btCollisionShape> colShape, btVector3 pos,
                      btQuaternion rot, btScalar mass);
};

#endif // PHYSICSWORLD_H
