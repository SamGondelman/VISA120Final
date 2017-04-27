#ifndef ENTITY_H
#define ENTITY_H

#include <memory>
#include "glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <btBulletCollisionCommon.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

#include "CS123SceneData.h"

enum ShapeType {
    CUBE = 0,
    SPHERE,
    CYLINDER,
    CONE,
    CONVEX_MESH,
    CONCAVE_MESH
};

class Entity {
public:
    Entity() {}
    Entity(std::shared_ptr<btDiscreteDynamicsWorld> physWorld, ShapeType shapeType, btScalar mass,
           const btVector3 &pos, const btVector3 &scale, CS123SceneMaterial mat, const btQuaternion &rot = btQuaternion(0, 0, 0, 1),
           const btVector3 &vel = btVector3(0, 0, 0));
    Entity(std::shared_ptr<btDiscreteDynamicsWorld> physWorld, ShapeType shapeType, btScalar mass,
           const btVector3 &pos, const btVector3 &scale, const btQuaternion &rot = btQuaternion(0, 0, 0, 1), const btVector3 &vel = btVector3(0, 0, 0));
    void setupRigidBody(std::shared_ptr<btDiscreteDynamicsWorld> physWorld, btScalar& mass,
          const btVector3 &pos, const btVector3 &scale, const btQuaternion &rot, const btVector3 &vel);

    void getModelMatrix(glm::mat4& m);
    void draw();
    CS123SceneMaterial& getMaterial() { return m_mat; }

    std::unique_ptr<btRigidBody> m_rigidBody;
    bool m_draw;

private:
    ShapeType m_shapeType;
    std::unique_ptr<btCollisionShape> m_collShape;
    std::unique_ptr<btDefaultMotionState> m_motionState;
    glm::mat4 m_scale;
    CS123SceneMaterial m_mat;
};

#endif // ENTITY_H
