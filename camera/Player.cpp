#include "Player.h"

#include "glm/gtx/transform.hpp"

Player::Player(int width, int height) :
    m_FOVY(0.8f), m_near(0.1f), m_far(100.0f),
    m_eye(glm::vec3(0, 1.5, 0)), m_up(glm::vec3(0, 1, 0))
{
    setLook(glm::vec3(0, 0, -1));
    setAspectRatio(width, height);
}

void Player::setLook(glm::vec3 look) {
    m_center = m_eye + look;
}

void Player::setEye(glm::vec3 eye) {
    m_eye = eye;
}

void Player::setCenter(glm::vec3 center) {
    m_center = center;
}

void Player::setAspectRatio(int width, int height) {
    m_aspectRatio = static_cast<float>(width) / height;
}

glm::vec3& Player::getEye() {
    return m_eye;
}

glm::mat4 Player::getView() {
    return glm::lookAt(m_eye, m_center, m_up);
}

glm::mat4 Player::getPerspective() {
    return glm::perspective(m_FOVY, m_aspectRatio, m_near, m_far);
}
