#include "PhysicsWorld.h"

#include "view.h"
#include "gl/shaders/CS123Shader.h"
#include "Player.h"

PhysicsWorld::PhysicsWorld() : World(":/shaders/shader.vert", ":/shaders/shader.frag")
{
}

void PhysicsWorld::makeCurrent() {
    View::m_player->setEye(glm::vec3(4, 2, 4));
    View::m_player->setCenter(glm::vec3(0));
    m_lights.clear();
    m_lights.emplace_back(glm::vec3(-1.0f), glm::vec3(0.7f));

    World::makeCurrent();

    CS123SceneMaterial mat;
    mat.cAmbient = glm::vec4(0.05, 0.05, 0.05, 1);
    mat.cDiffuse = glm::vec4(0.5, 0.5, 0.5, 1);
    mat.cSpecular = glm::vec4(0.7, 0.7, 0.7, 1);
    mat.shininess = 20.0f;
//    m_entities.emplace_back(m_physWorld, ShapeType::SPHERE, 1.0f,
//                            btVector3(1.5, 2, 0), btVector3(1.0, 1.0, 1.0), mat);
//    m_entities.emplace_back(m_physWorld, ShapeType::CONE, 1.0f,
//                            btVector3(-1.5, 2, 0), btVector3(1.0, 1.0, 1.0), mat);
//    m_entities.emplace_back(m_physWorld, ShapeType::CUBE, 1.0f,
//                            btVector3(0, 2, -1.5), btVector3(1.0, 1.0, 1.0), mat);
//    m_entities.emplace_back(m_physWorld, ShapeType::CYLINDER, 1.0f,
//                            btVector3(0, 2, 1.5), btVector3(1.0, 1.0, 1.0), mat);

    // Walls
    m_entities.emplace_back(m_physWorld, ShapeType::CUBE, 0.0f,
                            btVector3(0, -0.25, 0), btVector3(2.7, 0.5, 1.7), mat);
    m_entities.emplace_back(m_physWorld, ShapeType::CUBE, 0.0f,
                            btVector3(0, 2.65, 0), btVector3(2.7, 0.5, 1.7), mat);
    m_entities.emplace_back(m_physWorld, ShapeType::CUBE, 0.0f,
                            btVector3(-1.60, 1.2, 0), btVector3(0.5, 2.6, 1.7), mat);
    m_entities.emplace_back(m_physWorld, ShapeType::CUBE, 0.0f,
                            btVector3(1.60, 1.2, 0), btVector3(0.5, 2.6, 1.7), mat);
    m_entities.emplace_back(m_physWorld, ShapeType::CUBE, 0.0f,
                            btVector3(0, 1.2, 1), btVector3(2.7, 2.6, 0.5), mat);

    // Wall with window
    m_entities.emplace_back(m_physWorld, ShapeType::CUBE, 0.0f,
                            btVector3(0, 0.5, -1), btVector3(2.7, 1.0, 0.3), mat);
    m_entities.emplace_back(m_physWorld, ShapeType::CUBE, 0.0f,
                            btVector3(0, 2.4, -1), btVector3(2.7, 0.5, 0.3), mat);
    m_entities.emplace_back(m_physWorld, ShapeType::CUBE, 0.0f,
                            btVector3(0.9, 1.2, -1), btVector3(1.0, 2.6, 0.3), mat);
    m_entities.emplace_back(m_physWorld, ShapeType::CUBE, 0.0f,
                            btVector3(-0.9, 1.2, -1), btVector3(1.0, 2.6, 0.3), mat);
}

void PhysicsWorld::update(float dt) {
    World::update(dt);
}

void PhysicsWorld::drawGeometry() {
    glm::mat4 m;
    for (auto& e : m_entities) {
        if (e.m_draw) {
            e.getModelMatrix(m);
            m_program->setUniform("M", m);
            m_program->applyMaterial(e.getMaterial());
            e.draw();
        }
    }
}
