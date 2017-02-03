#include "Entity.h"

#include "view.h"
#include "CubeMesh.h"
#include "SphereMesh.h"
#include "ConeMesh.h"
#include <BulletDynamics/Dynamics/btRigidBody.h>

Entity::Entity(std::shared_ptr<btDiscreteDynamicsWorld> physWorld, ShapeType shapeType, btScalar mass,
               btVector3 pos, btVector3 scale, btQuaternion rot) :
    m_shapeType(shapeType)
{
    switch (m_shapeType) {
        case ShapeType::CUBE:
            m_collShape = std::make_unique<btBoxShape>(scale / 2.0f);
            break;
        case ShapeType::SPHERE:
            if (scale[0] == scale[1] && scale[0] == scale[2]) {
                m_collShape = std::make_unique<btSphereShape>(scale[0] / 2.0f);
            } else {
                btVector3 p(0, 0, 0);
                btScalar r(1.0f);
                m_collShape = std::make_unique<btMultiSphereShape>(&p, &r, 1);
                m_collShape->setLocalScaling(scale / 2.0f);
            }
            break;
        case ShapeType::CYLINDER:
            m_collShape = std::make_unique<btCylinderShape>(scale / 2.0f);
            break;
        case ShapeType::CONE:
            m_collShape = std::make_unique<btConeShape>(scale[0] / 2.0f, scale[1]);
            break;
        default:
            m_collShape = std::make_unique<btBoxShape>(scale / 2.0f);
            break;
    }

    m_scale = glm::scale(glm::vec3(scale[0], scale[1], scale[2]));

    btVector3 localInertia = btVector3(0,0,0);
    if (mass != 0.f) m_collShape->calculateLocalInertia(mass, localInertia);

    m_motionState = std::make_unique<btDefaultMotionState>(btTransform(rot, pos));
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,
                                                    m_motionState.get(),
                                                    m_collShape.get(),
                                                    localInertia);
    m_rigidBody = std::make_unique<btRigidBody>(rbInfo);

    physWorld->addRigidBody(m_rigidBody.get());
}

void Entity::getModelMatrix(glm::mat4& m) {
    btTransform t;
    m_rigidBody->getMotionState()->getWorldTransform(t);
    t.getOpenGLMatrix(glm::value_ptr(m));
    m = m * m_scale;
}

void Entity::draw() {
    switch (m_shapeType) {
        case ShapeType::CUBE:
            View::m_cube->draw();
            break;
        case ShapeType::SPHERE:
            View::m_sphere->draw();
            break;
        case ShapeType::CYLINDER:
            break;
        case ShapeType::CONE:
            View::m_cone->draw();
            break;
        default:
            break;
    }
}