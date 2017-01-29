#ifndef PLAYER_H
#define PLAYER_H

#include "glm.hpp"

class Player {
public:
    Player(int width, int height);

    void setLook(glm::vec3 look);
    void setEye(glm::vec3 eye);
    void setCenter(glm::vec3 center);
    void setAspectRatio(int width, int height);

    glm::vec3& getEye();
    glm::mat4 getView();
    glm::mat4 getPerspective();

private:
    // Camera properties
    float m_FOVY;               // Vertical field of view angle, in radians.
    float m_aspectRatio;        // Aspect ratio of the window.
    float m_near;               // Near clipping plane.
    float m_far;                // Far clipping plane.
    glm::vec3 m_eye;            // Camera position.
    glm::vec3 m_center;         // Camera viewing point.
    glm::vec3 m_up;             // Up direction.

};

#endif // PLAYER_H
