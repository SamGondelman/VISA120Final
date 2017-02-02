#include "WaterWorld.h"

#include "glm.h"
#include "glm/gtx/transform.hpp"

#include "view.h"
#include "gl/shaders/CS123Shader.h"
#include "Player.h"
#include "SphereMesh.h"
#include "CubeMesh.h"

WaterWorld::WaterWorld() : World(":/shaders/shader.vert", ":/shaders/waterWorld.frag")
{

}

void WaterWorld::makeCurrent() {
    m_lights.clear();
    m_lights.push_back(Light(glm::vec3(-1.0f), glm::vec3(0.7f)));
}

void WaterWorld::update(float dt) {
//    View::m_player->setEye(glm::vec3(6.0f * glm::sin(View::m_globalTime/3.0f), 1.0f,
//                           6.0f * glm::cos(View::m_globalTime/3.0f)));
    View::m_player->setEye(glm::vec3(0.0f, 1.5f + 0.5f * glm::sin(View::m_globalTime/3.0f), 6.0f));
    View::m_player->setCenter(glm::vec3(0));
}

void WaterWorld::drawGeometry() {
    // sphere 1
    glm::mat4 M = glm::translate(glm::vec3(glm::sin(View::m_globalTime), 0.0f, 0.0f));
    m_program->setUniform("M", M);
    CS123SceneMaterial mat;
    mat.cAmbient = glm::vec4(0.1, 0, 0, 1);
    mat.cDiffuse = glm::vec4(0, 1, 0, 1);
    mat.cSpecular = glm::vec4(0, 0, 1, 1);
    mat.shininess = 20.0f;
    m_program->applyMaterial(mat);
    View::m_sphere->draw();

    // sphere 2
    M = glm::translate(glm::vec3(0.0f, 0.0f, glm::cos(View::m_globalTime)));
    m_program->setUniform("M", M);
    mat.cAmbient = glm::vec4(0.07, 0.07, 0, 1);
    mat.cDiffuse = glm::vec4(1, 0, 1, 1);
    mat.cSpecular = glm::vec4(0, 1, 1, 1);
    mat.shininess = 100.0f;
    m_program->applyMaterial(mat);
    View::m_sphere->draw();

    // sphere 3
    M = glm::translate(glm::vec3(0.0f, 1.5f, 0.0f));
    m_program->setUniform("M", M);
    mat.cAmbient = glm::vec4(0.2, 0.2, 0.4, 1);
    mat.cDiffuse = glm::vec4(1.3, 2, 1.2, 1);
    mat.cSpecular = glm::vec4(2, 1, 1, 1);
    mat.shininess = 50.0f;
    m_program->applyMaterial(mat);
    View::m_sphere->draw();

    // cube
    M = glm::translate(glm::vec3(0.0f, -1.0f, 0.0f)) * glm::scale(glm::vec3(3.0f, 1.0f, 3.0f));
    m_program->setUniform("M", M);
    mat.cAmbient = glm::vec4(0, 0, 0, 1);
    mat.cDiffuse = glm::vec4(0.25, 0.25, 0.25, 1);
    mat.cSpecular = glm::vec4(1, 0, 1, 1);
    mat.shininess = 10.0f;
    m_program->applyMaterial(mat);
    View::m_cube->draw();
}
