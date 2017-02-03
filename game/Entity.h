#ifndef ENTITY_H
#define ENTITY_H

#include <memory>
#include "glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <btBulletCollisionCommon.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

enum ShapeType {
    CUBE,
    SPHERE,
    CYLINDER,
    CONE,
    CONVEX_MESH,
    CONCAVE_MESH
};

class Entity {
public:
    Entity(std::shared_ptr<btDiscreteDynamicsWorld> physWorld, ShapeType shapeType, btScalar mass,
           btVector3 pos, btVector3 scale, btQuaternion rot = btQuaternion());

    void getModelMatrix(glm::mat4& m);
    void draw();

    std::unique_ptr<btRigidBody> m_rigidBody;

private:
    ShapeType m_shapeType;
    std::unique_ptr<btCollisionShape> m_collShape;
    std::unique_ptr<btDefaultMotionState> m_motionState;
    glm::mat4 m_scale;
};

#endif // ENTITY_H
