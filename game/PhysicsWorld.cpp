#include "PhysicsWorld.h"

#include "view.h"
#include "gl/shaders/CS123Shader.h"
#include "Player.h"
#include "SphereMesh.h"
#include "CubeMesh.h"

PhysicsWorld::PhysicsWorld() : World(":/shaders/shader.vert", ":/shaders/shader.frag")
{

    m_configuration = std::make_unique<btDefaultCollisionConfiguration>();
    m_dispatcher = std::make_unique<btCollisionDispatcher>(m_configuration.get());
    m_broadphase = std::make_unique<btDbvtBroadphase>();
    m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();
    m_physWorld = std::make_unique<btDiscreteDynamicsWorld>(m_dispatcher.get(),
                                                            m_broadphase.get(),
                                                            m_solver.get(),
                                                            m_configuration.get());
    m_physWorld->setGravity(btVector3(0,-9.81,0));
}

void PhysicsWorld::makeCurrent() {
    View::m_player->setEye(glm::vec3(4, 2, 4));
    View::m_player->setCenter(glm::vec3(0));
    m_lights.clear();
    m_lights.push_back(Light(glm::vec3(-1.0f), glm::vec3(0.7f)));


    for (auto c : m_rigidBodies) m_physWorld->removeRigidBody(c.get());
    m_collShapes.clear();
    m_motionStates.clear();
    m_rigidBodies.clear();

    std::shared_ptr<btCollisionShape> colShape = std::make_shared<btSphereShape>(btScalar(0.5f));
    addRigidBody(colShape, btVector3(0.51, 2, 0), btQuaternion(), 1.0);

    std::shared_ptr<btCollisionShape> colShape2 = std::make_shared<btBoxShape>(btVector3(0.5f, 0.5f, 0.5f));
    addRigidBody(colShape2, btVector3(0, 0, 0), btQuaternion(), 0.0);
}

void PhysicsWorld::addRigidBody(std::shared_ptr<btCollisionShape> colShape, btVector3 pos,
                                btQuaternion rot, btScalar mass) {
    m_collShapes.push_back(colShape);

    btVector3 localInertia = btVector3(0,0,0);
    if (mass != 0.f) colShape->calculateLocalInertia(mass, localInertia);

    std::shared_ptr<btDefaultMotionState> motionState =
            std::make_shared<btDefaultMotionState>(btTransform(rot, pos));
    m_motionStates.push_back(motionState);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,
                                                    motionState.get(),
                                                    colShape.get(),
                                                    localInertia);
    std::shared_ptr<btRigidBody> body = std::make_shared<btRigidBody>(rbInfo);
    m_rigidBodies.push_back(body);

    m_physWorld->addRigidBody(body.get());
}

void PhysicsWorld::update(float dt) {
    m_physWorld->stepSimulation(dt);
}

void PhysicsWorld::drawGeometry() {
    btTransform t;
    btScalar m[16];
    CS123SceneMaterial mat;

    m_rigidBodies[0]->getMotionState()->getWorldTransform(t);
    t.getOpenGLMatrix(m);
    m_program->setUniformMat4("M", m);
    mat.cAmbient = glm::vec4(0.05, 0, 0, 1);
    mat.cDiffuse = glm::vec4(0.2, 0.6, 0.5, 1);
    mat.cSpecular = glm::vec4(0.5, 0.4, 0.7, 1);
    mat.shininess = 20.0f;
    m_program->applyMaterial(mat);
    View::m_sphere->draw();

    m_rigidBodies[1]->getMotionState()->getWorldTransform(t);
    t.getOpenGLMatrix(m);
    m_program->setUniformMat4("M", m);
    mat.cAmbient = glm::vec4(0.05, 0, 0, 1);
    mat.cDiffuse = glm::vec4(0.2, 0.6, 0.5, 1);
    mat.cSpecular = glm::vec4(0.5, 0.4, 0.7, 1);
    mat.shininess = 20.0f;
    m_program->applyMaterial(mat);
    View::m_cube->draw();
}
