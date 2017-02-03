#include "PhysicsWorld.h"

#include "view.h"
#include "gl/shaders/CS123Shader.h"
#include "Player.h"

PhysicsWorld::PhysicsWorld() : World(":/shaders/shader.vert", ":/shaders/shader.frag")
{

    m_configuration = std::make_unique<btDefaultCollisionConfiguration>();
    m_dispatcher = std::make_unique<btCollisionDispatcher>(m_configuration.get());
    m_broadphase = std::make_unique<btDbvtBroadphase>();
    m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();
    m_physWorld = std::make_shared<btDiscreteDynamicsWorld>(m_dispatcher.get(),
                                                            m_broadphase.get(),
                                                            m_solver.get(),
                                                            m_configuration.get());
    m_physWorld->setGravity(btVector3(0,-9.81,0));
}

void PhysicsWorld::makeCurrent() {
    View::m_player->setEye(glm::vec3(4, 2, 4));
    View::m_player->setCenter(glm::vec3(0));
    m_lights.clear();
    m_lights.emplace_back(glm::vec3(-1.0f), glm::vec3(0.7f));

    for (auto& e : m_entities) m_physWorld->removeRigidBody(e.m_rigidBody.get());
    m_entities.clear();
    m_solver->reset();
    m_physWorld->clearForces();
    m_broadphase->resetPool(m_dispatcher.get());

    m_entities.emplace_back(m_physWorld, ShapeType::SPHERE, 1.0f,
                            btVector3(0.51, 2, 0), btVector3(1.5, 0.2, 1.5));
    m_entities.emplace_back(m_physWorld, ShapeType::CONE, 1.0f,
                            btVector3(-0.51, 2, 0), btVector3(1.0, 2.0, 1.0));
    m_entities.emplace_back(m_physWorld, ShapeType::CUBE, 0.0f,
                            btVector3(0, 0, 0), btVector3(1.0, 1.5, 1.0));
}

void PhysicsWorld::update(float dt) {
    m_physWorld->stepSimulation(dt);
}

void PhysicsWorld::drawGeometry() {
    CS123SceneMaterial mat;
    mat.cAmbient = glm::vec4(0.05, 0, 0, 1);
    mat.cDiffuse = glm::vec4(0.2, 0.6, 0.5, 1);
    mat.cSpecular = glm::vec4(0.5, 0.4, 0.7, 1);
    mat.shininess = 20.0f;
    m_program->applyMaterial(mat);

    glm::mat4 m;
    for (auto& e : m_entities) {
        e.getModelMatrix(m);
        m_program->setUniform("M", m);
        e.draw();
    }
}
