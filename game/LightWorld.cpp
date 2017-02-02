#include "LightWorld.h"

#include "glm.h"
#include "glm/gtx/transform.hpp"

#include "view.h"
#include "gl/shaders/CS123Shader.h"
#include "Player.h"
#include "SphereMesh.h"
#include "CubeMesh.h"

LightWorld::LightWorld() : World(":/shaders/shader.vert", ":/shaders/lightWorld.frag"),
    m_lightOrigin(glm::vec3(0)), m_lightTime(INFINITY)
{

}

void LightWorld::makeCurrent() {
    View::m_player->setEye(glm::vec3(-4, 2, -4));
    View::m_player->setCenter(glm::vec3(0));
}

void LightWorld::update(float dt) {
    const float RING_DURATION = 5.0f;
    if (m_lightTime == INFINITY || m_lightTime > RING_DURATION) {
        m_lightTime = 0.0f;
        m_lightOrigin = View::m_player->getEye();
    } else {
        m_lightTime += dt;
    }
}

void LightWorld::drawGeometry() {
    m_program->setUniform("origin", m_lightOrigin);
    m_program->setUniform("time", m_lightTime);

    // floor
    glm::mat4 M = glm::translate(glm::vec3(0.0f, -1.0f, 0.0f)) * glm::scale(glm::vec3(30.0f, 0.3f, 30.0f));
    m_program->setUniform("M", M);
    View::m_cube->draw();

    // box
    M = glm::translate(glm::vec3(0.0f, -0.5f, 0.0f)) * glm::scale(glm::vec3(1.5f, 2.0f, 1.5f));
    m_program->setUniform("M", M);
    View::m_cube->draw();

    // walls
    M = glm::translate(glm::vec3(7.0f, 2.5f, 0.0f)) * glm::scale(glm::vec3(1.0f, 7.0f, 7.0f));
    m_program->setUniform("M", M);
    View::m_cube->draw();

    M = glm::translate(glm::vec3(0.0f, 2.5f, 7.0f)) * glm::scale(glm::vec3(7.0f, 7.0f, 1.0f));
    m_program->setUniform("M", M);
    View::m_cube->draw();
}
