#include "RockWorld.h"

#include "glm.h"
#include "glm/gtx/transform.hpp"

#include "view.h"
#include "gl/shaders/CS123Shader.h"
#include "Player.h"
#include "CubeMesh.h"

RockWorld::RockWorld() : World(":/shaders/shader.vert", ":/shaders/shader.frag")
{

}

void RockWorld::makeCurrent() {
    m_lights.clear();
    m_lights.push_back(Light(glm::vec3(-1.0f), glm::vec3(0.7f)));
}

void RockWorld::update(float dt) {
    View::m_player->setEye(glm::vec3(0.0f, 1.5f + 0.5f * glm::sin(View::m_globalTime/3.0f), 6.0f));
    View::m_player->setCenter(glm::vec3(0));
    View::m_rockTime += dt;
}

void RockWorld::drawGeometry() {
    // floor
    glm::mat4 M = glm::translate(glm::vec3(0.0f, -1.0f, 0.0f)) * glm::scale(glm::vec3(3.0f, 0.25f, 3.0f));
    m_program->setUniform("M", M);
    CS123SceneMaterial mat;
    mat.cAmbient = glm::vec4(0.125, 0.125, 0.05, 1);
    mat.cDiffuse = glm::vec4(0.25, 0.125, 0.15, 1);
    mat.cSpecular = glm::vec4(0.25, 0.25, 0.5, 1);
    mat.shininess = 10.0f;
    m_program->applyMaterial(mat);
    View::m_cube->draw();
}
