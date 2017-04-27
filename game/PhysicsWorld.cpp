#include "PhysicsWorld.h"

#include "view.h"
#include "gl/shaders/CS123Shader.h"
#include "Player.h"

#include "SphereMesh.h"
#include "CylinderMesh.h"

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

    glm::vec3 p1(0, 0.5, 0.0);
    glm::vec3 p2(-0.25, 0.75, 0.3);
    m = glm::translate(p1) * glm::scale(glm::vec3(0.025));
    m_program->setUniform("M", m);
    View::m_sphere->draw();

    m = glm::translate(p2) * glm::scale(glm::vec3(0.025));
    m_program->setUniform("M", m);
    View::m_sphere->draw();

    glm::vec3 d = p1 - p2;
    float dist = glm::length(d);
    float yaw = glm::degrees(atan2(d.x, d.z));
    float pitch = glm::degrees(atan2(d.y, sqrt(d.x*d.x + d.z*d.z)));
    glm::mat4 r = glm::mat4_cast(glm::quat(glm::vec3(pitch, yaw, 0.0f)));
    m = glm::translate((p1 + p2)/2.0f) * r * glm::scale(glm::vec3(0.02, dist, 0.02));
    m_program->setUniform("M", m);
    View::m_cylinder->draw();
}
