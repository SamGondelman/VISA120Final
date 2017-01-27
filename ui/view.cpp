#include "view.h"

#include "viewformat.h"
#include "ResourceLoader.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "gl/shaders/CS123Shader.h"
#include "CS123SceneData.h"
#include "gl/datatype/FBO.h"
#include "gl/textures/Texture2D.h"
#include "SphereMesh.h"

#include <QApplication>
#include <QKeyEvent>
#include <iostream>

View::View(QWidget *parent) : QGLWidget(ViewFormat(), parent),
    m_time(), m_timer(), m_drawMode(DrawMode::DEFAULT)
{
    // View needs all mouse move events, not just mouse drag events
    setMouseTracking(true);

    // View needs keyboard focus
    setFocusPolicy(Qt::StrongFocus);

    // The update loop is implemented using a timer
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));
}

View::~View()
{
    glDeleteVertexArrays(1, &m_fullscreenQuadVAO);
}

void View::initializeGL() {
    ResourceLoader::initializeGlew();

    m_time.start();
    // TODO: change to / 90
    m_timer.start(1000 / 60);

    // OpenGL setup
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    m_sphere = std::make_unique<SphereMesh>(10, 10);
    m_lightSphere = std::make_unique<SphereMesh>(15, 15);

    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/shader.vert");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/shader.frag");
    m_deferredProgram = std::make_unique<CS123Shader>(vertexSource, fragmentSource);

    vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/fullscreenQuad.vert");
    fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/fullscreenQuad.frag");
    m_fullscreenQuadProgram = std::make_unique<CS123Shader>(vertexSource, fragmentSource);

    glGenVertexArrays(1, &m_fullscreenQuadVAO);

    m_lights.push_back(PointLight(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(5.0f, 5.7f, 20.8f)));
    std::cout << m_lights[0].radius << std::endl;

}

void View::paintGL() {
    // Camera properties
    float fieldOfViewY = 0.8f;                                  // Vertical field of view angle, in radians.
    float aspectRatio = static_cast<float>(m_width) / m_height; // Aspect ratio of the window.
    float nearClipPlane = 0.1f;                                 // Near clipping plane.
    float farClipPlane = 100.f;                                 // Far clipping plane.
    glm::vec3 eye = glm::vec3(0.0f, 1.0f, 6.0f);                // Camera position.
    glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);             // Where camera is looking.
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);                 // Up direction.

    // Draw to deferred buffer
    glDisable(GL_BLEND);
    m_deferredProgram->bind();
    m_deferredBuffer->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 V = glm::lookAt(eye, center, up);
    m_deferredProgram->setUniform("V", V);
    glm::mat4 P = glm::perspective(fieldOfViewY, aspectRatio, nearClipPlane, farClipPlane);
    m_deferredProgram->setUniform("P", P);

    if (m_drawMode != DrawMode::LIGHTS) {
        // Draw geometry
        // sphere 1
        glm::mat4 M = glm::translate(glm::vec3(glm::sin(m_totalTime), 0.0f, 0.0f));
        m_deferredProgram->setUniform("M", M);
        CS123SceneMaterial mat;
        mat.cAmbient = glm::vec4(1, 0, 0, 1);
        mat.cDiffuse = glm::vec4(0, 1, 0, 1);
        mat.cSpecular = glm::vec4(0, 0, 1, 1);
        mat.shininess = 1.0f;
        m_deferredProgram->applyMaterial(mat);
        m_sphere->draw();

        // sphere 2
        M = glm::translate(glm::vec3(0.0f, 0.0f, glm::cos(m_totalTime)));
        m_deferredProgram->setUniform("M", M);
        mat.cAmbient = glm::vec4(1, 1, 0, 1);
        mat.cDiffuse = glm::vec4(1, 0, 1, 1);
        mat.cSpecular = glm::vec4(0, 1, 1, 1);
        mat.shininess = 1.0f;
        m_deferredProgram->applyMaterial(mat);
        m_sphere->draw();
    } else {
        // Draw lights as geometry
        for (auto& light : m_lights) {
            glm::mat4 M = glm::translate(light.pos) * glm::scale(glm::vec3(2.0f * light.radius));
            m_deferredProgram->setUniform("M", M);
            CS123SceneMaterial mat;
            mat.cAmbient = glm::vec4(light.col, 1);
            m_deferredProgram->applyMaterial(mat);
            m_lightSphere->draw();
        }
    }

    m_deferredBuffer->unbind();
    m_deferredProgram->unbind();

    // Lighting and drawing to screen
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_drawMode == DrawMode::POSITION || m_drawMode == DrawMode::NORMAL || m_drawMode == DrawMode::AMBIENT ||
            m_drawMode == DrawMode::LIGHTS) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_deferredBuffer->getId());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        int offset = m_drawMode != DrawMode::LIGHTS ? m_drawMode : DrawMode::AMBIENT;
        glReadBuffer(GL_COLOR_ATTACHMENT0 + offset);
        glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    } else {
        m_fullscreenQuadProgram->bind();
        glViewport(0, 0, m_width, m_height);
        m_fullscreenQuadProgram->setTexture("pos", m_deferredBuffer->getColorAttachment(0));
        m_fullscreenQuadProgram->setTexture("nor", m_deferredBuffer->getColorAttachment(1));
        m_fullscreenQuadProgram->setTexture("amb", m_deferredBuffer->getColorAttachment(2));
        m_fullscreenQuadProgram->setTexture("diff", m_deferredBuffer->getColorAttachment(3));
        m_fullscreenQuadProgram->setTexture("spec", m_deferredBuffer->getColorAttachment(4));
        glBindVertexArray(m_fullscreenQuadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        m_fullscreenQuadProgram->unbind();

        glActiveTexture(GL_TEXTURE0);
    }
}

void View::resizeGL(int w, int h) {
    m_width = w;
    m_height = h;
    float ratio = static_cast<QGuiApplication *>(QCoreApplication::instance())->devicePixelRatio();
    //w = static_cast<int>(w / ratio);
    //h = static_cast<int>(h / ratio);
    glViewport(0, 0, w, h);

    // Resize deferred buffer
    m_deferredBuffer = std::make_unique<FBO>(5, FBO::DEPTH_STENCIL_ATTACHMENT::DEPTH_ONLY, w, h,
                                             TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                             TextureParameters::FILTER_METHOD::LINEAR,
                                             GL_FLOAT);
}

void View::mousePressEvent(QMouseEvent *event) {

}

void View::mouseMoveEvent(QMouseEvent *event) {

}

void View::mouseReleaseEvent(QMouseEvent *event) {

}

void View::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) QApplication::quit();

    if (event->key() == Qt::Key_1) m_drawMode = DrawMode::DEFAULT;
    else if (event->key() == Qt::Key_2) m_drawMode = DrawMode::POSITION;
    else if (event->key() == Qt::Key_3) m_drawMode = DrawMode::NORMAL;
    else if (event->key() == Qt::Key_4) m_drawMode = DrawMode::AMBIENT;
    else if (event->key() == Qt::Key_5) m_drawMode = DrawMode::LIGHTS;
    else if (event->key() == Qt::Key_6) m_drawMode = DrawMode::DIFFUSE;
    else if (event->key() == Qt::Key_7) m_drawMode = DrawMode::SPECULAR;


    // TODO: Handle keyboard presses here
}

void View::keyReleaseEvent(QKeyEvent *event) {

}

void View::tick() {
    // Get the number of seconds since the last tick (variable update rate)
    float dt = m_time.restart() * 0.001f;

    // TODO: Implement the demo update here
    m_totalTime += dt;

    // Flag this view for repainting (Qt will call paintGL() soon after)
    update();
}
