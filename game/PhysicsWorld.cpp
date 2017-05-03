#include "PhysicsWorld.h"

#include "view.h"
#include "gl/shaders/CS123Shader.h"
#include "Player.h"

#include "SphereMesh.h"
#include "CylinderMesh.h"
#include "gl/textures/Texture2D.h"

PhysicsWorld::PhysicsWorld() : World(":/shaders/shader.vert", ":/shaders/shader.frag")
{
}

void PhysicsWorld::makeCurrent() {
    m_lights.clear();
    m_lights.emplace_back(glm::vec3(-1.0f), glm::vec3(0.7f));
    m_lights.emplace_back(glm::vec3(-1.0f), glm::vec3(0.1f));

    World::makeCurrent();

    CS123SceneMaterial mat;
    mat.cAmbient = glm::vec4(0.05, 0.05, 0.05, 1);
    mat.cDiffuse = glm::vec4(0.5, 0.5, 0.5, 1);
    mat.cSpecular = glm::vec4(0.7, 0.7, 0.7, 1);
    mat.shininess = 20.0f;

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

    // Instructions
    mat.textureMap.isUsed = true;
    mat.textureMap.filename = ":/images/vivewand.png";
    m_entities.emplace_back(m_physWorld, ShapeType::CUBE, 0.0f,
                            btVector3(0.9, 1.7, -0.89), btVector3(0.5, 0.5, 0.1), mat);

    // Clear paint
    m_paint.clear();
}

void PhysicsWorld::update(float dt) {
    m_lights[m_lights.size() - 1].dir = View::m_viewDir;
    World::update(dt);
}

void PhysicsWorld::drawGeometry() {
    glm::mat4 m;
    View::m_textureMutex.lock();
    for (auto& e : m_entities) {
        if (e.m_draw) {
            e.getModelMatrix(m);
            m_program->setUniform("M", m);
            m_program->applyMaterial(e.getMaterial());
            auto &containsTex = View::m_textureMap.find(e.getMaterial().textureMap.filename);
            int useTexture = e.getMaterial().textureMap.isUsed && (containsTex != View::m_textureMap.end() ? 1 : 0);
            m_program->setUniform("useTexture", useTexture);
            if (useTexture) m_program->setTexture("tex", View::m_textureMap[e.getMaterial().textureMap.filename]);
            e.draw();
        }
    }
    View::m_textureMutex.unlock();
    m_program->setUniform("useTexture", 0);

    CS123SceneMaterial mat;
    mat.shininess = 20.0f;
    for (auto &p : m_paint) {
        glm::vec3 d = p.points.second - p.points.first;
        float dist = glm::length(d);
        float yaw = atan2(d.x, d.z);
        float pitch = M_PI_2 - asin(d.y/glm::length(d));
        glm::mat4 r = glm::mat4_cast(glm::quat(glm::vec3(pitch, yaw, 0.0f)));
        m = glm::translate((p.points.first + p.points.second)/2.0f) * r * glm::scale(glm::vec3(0.02, dist, 0.02));
        m_program->setUniform("M", m);
        mat.cAmbient.xyz = p.col * 0.05f;
        mat.cDiffuse.xyz = p.col * 0.5f;
        mat.cSpecular.xyz = p.col * 0.7f;
        m_program->applyMaterial(mat);
        View::m_cylinder->draw();
    }
}
