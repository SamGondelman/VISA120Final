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
#include "CubeMesh.h"
#include "gl/datatype/VAO.h"
#include "FullScreenQuad.h"

#include <QApplication>
#include <QKeyEvent>
#include <iostream>

View::View(QWidget *parent) : QGLWidget(ViewFormat(), parent),
    m_time(), m_timer(), m_drawMode(DrawMode::DEFAULT), m_exposure(1.0f), m_useAdaptiveExposure(true)
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    m_sphere = std::make_unique<SphereMesh>(10, 10);
    m_cube = std::make_unique<CubeMesh>(1);
    m_lightSphere = std::make_unique<SphereMesh>(15, 15);
    m_fullscreenQuad = std::make_unique<FullScreenQuad>();

    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/shader.vert");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/shader.frag");
    m_deferredProgram = std::make_unique<CS123Shader>(vertexSource, fragmentSource);

    vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/lighting.vert");
    fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/lighting.frag");
    m_lightingProgram = std::make_unique<CS123Shader>(vertexSource, fragmentSource);

    vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/fullscreenQuad.vert");
    fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/bright.frag");
    m_brightProgram = std::make_unique<CS123Shader>(vertexSource, fragmentSource);

    glGenVertexArrays(1, &m_fullscreenQuadVAO);

    vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/fullscreenQuad.vert");
    fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/horizontalBlur.frag");
    m_horizontalBlurProgram = std::make_shared<CS123Shader>(vertexSource, fragmentSource);

    vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/fullscreenQuad.vert");
    fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/verticalBlur.frag");
    m_verticalBlurProgram = std::make_shared<CS123Shader>(vertexSource, fragmentSource);

    vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/fullscreenQuad.vert");
    fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/texture.frag");
    m_textureProgram = std::make_unique<CS123Shader>(vertexSource, fragmentSource);

    vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/fullscreenQuad.vert");
    fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/bloom.frag");
    m_bloomProgram = std::make_unique<CS123Shader>(vertexSource, fragmentSource);

    m_lights.push_back(Light(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 1.5f, 15.8f)));
    m_lights.push_back(Light(glm::vec3(-1.0f), glm::vec3(0.7f)));

}

void View::paintGL() {
    // Camera properties
    float fieldOfViewY = 0.8f;                                  // Vertical field of view angle, in radians.
    float aspectRatio = static_cast<float>(m_width) / m_height; // Aspect ratio of the window.
    float nearClipPlane = 0.1f;                                 // Near clipping plane.
    float farClipPlane = 100.f;                                 // Far clipping plane.
    glm::vec3 eye = glm::vec3(6.0f * glm::sin(m_globalTime/3.0f), 1.0f, 6.0f * glm::cos(m_globalTime/3.0f));                // Camera position.
    glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);             // Where camera is looking.
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);                 // Up direction.

    // Draw to deferred buffer
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    m_deferredProgram->bind();
    m_deferredBuffer->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 V = glm::lookAt(eye, center, up);
    m_deferredProgram->setUniform("V", V);
    glm::mat4 P = glm::perspective(fieldOfViewY, aspectRatio, nearClipPlane, farClipPlane);
    m_deferredProgram->setUniform("P", P);

    if (m_drawMode != DrawMode::LIGHTS) {
        drawGeometry();
    } else {
        // Draw point lights as geometry
        for (auto& light : m_lights) {
            if (light.type == LightType::LIGHT_POINT) {
                glm::mat4 M = glm::translate(light.pos) * glm::scale(glm::vec3(2.0f * light.radius));
                m_deferredProgram->setUniform("M", M);
                CS123SceneMaterial mat;
                mat.cAmbient = glm::vec4(light.col, 1);
                m_deferredProgram->applyMaterial(mat);
                m_lightSphere->draw();
            }
        }
    }

    m_deferredBuffer->unbind();
    m_deferredProgram->unbind();

    // Lighting and drawing to screen
    if (m_drawMode == DrawMode::POSITION || m_drawMode == DrawMode::NORMAL || m_drawMode == DrawMode::AMBIENT ||
            m_drawMode == DrawMode::LIGHTS) {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_deferredBuffer->getId());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        int offset = m_drawMode != DrawMode::LIGHTS ? m_drawMode : DrawMode::AMBIENT;
        glReadBuffer(GL_COLOR_ATTACHMENT0 + offset);
        glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    } else {
        m_lightingBuffer->bind();
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        bool useLighting = m_drawMode == DrawMode::DEFAULT || m_drawMode == DrawMode::NO_HDR ||
                m_drawMode == DrawMode::BRIGHT || m_drawMode == DrawMode::BRIGHT_BLUR;

        // Ambient term
        if (m_drawMode != DrawMode::DIFFUSE && m_drawMode != DrawMode::SPECULAR) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_deferredBuffer->getId());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_lightingBuffer->getId());
            glReadBuffer(GL_COLOR_ATTACHMENT0 + DrawMode::AMBIENT);
            glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        }

        m_lightingProgram->bind();

        m_lightingProgram->setUniform("useDiffuse",(useLighting || m_drawMode == DrawMode::DIFFUSE) ? 1.0f : 0.0f);
        m_lightingProgram->setUniform("useSpecular", (useLighting || m_drawMode == DrawMode::SPECULAR) ? 1.0f : 0.0f);
        m_lightingProgram->setUniform("screenSize", glm::vec2(m_width, m_height));
        m_lightingProgram->setUniform("V", V);
        m_lightingProgram->setUniform("P", P);

        m_lightingProgram->setTexture("pos", m_deferredBuffer->getColorAttachment(0));
        m_lightingProgram->setTexture("nor", m_deferredBuffer->getColorAttachment(1));
        m_lightingProgram->setTexture("diff", m_deferredBuffer->getColorAttachment(3));
        m_lightingProgram->setTexture("spec", m_deferredBuffer->getColorAttachment(4));
        for (auto& light : m_lights) {
            // Light uniforms
            m_lightingProgram->setUniform("light.type", static_cast<int>(light.type));
            m_lightingProgram->setUniform("light.col", light.col);
            if (light.type == LightType::LIGHT_POINT) {
                m_lightingProgram->setUniform("light.posDir", light.pos);
                m_lightingProgram->setUniform("light.func", light.func);
                m_lightingProgram->setUniform("light.radius", light.radius);
            } else if (light.type == LightType::LIGHT_DIRECTIONAL) {
                m_lightingProgram->setUniform("light.posDir", light.dir);
            }

            // Draw light
            glm::vec3 dist = light.pos - eye;
            if (light.type == LightType::LIGHT_POINT && glm::dot(dist, dist) > light.radius * light.radius) {
                m_lightingProgram->setUniform("worldSpace", 1.0f);

                glm::mat4 M = glm::translate(light.pos) * glm::scale(glm::vec3(2.0f * light.radius));
                m_lightingProgram->setUniform("M", M);
                m_lightSphere->draw();
            } else {
                m_lightingProgram->setUniform("worldSpace", 0.0f);

                m_fullscreenQuad->draw();
            }
        }

        m_lightingBuffer->unbind();
        m_lightingProgram->unbind();
        glActiveTexture(GL_TEXTURE0);

        if (m_drawMode == DrawMode::DIFFUSE || m_drawMode == DrawMode::SPECULAR || m_drawMode == DrawMode::NO_HDR) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_lightingBuffer->getId());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        } else {
            m_brightProgram->bind();
            m_vblurBuffer->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            m_brightProgram->setTexture("color", m_lightingBuffer->getColorAttachment(0));

            glBindVertexArray(m_fullscreenQuadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);

            m_vblurBuffer->unbind();
            m_brightProgram->unbind();

            if (m_drawMode == DrawMode::BRIGHT) {
                m_textureProgram->bind();
                glViewport(0, 0, m_width, m_height);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                m_textureProgram->setTexture("tex", m_vblurBuffer->getColorAttachment(0));

                glBindVertexArray(m_fullscreenQuadVAO);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                glBindVertexArray(0);

                m_textureProgram->unbind();
            } else {
                bool horizontal = true;
                for (int i = 0; i < 2; i++) {
                    auto from = horizontal ? m_vblurBuffer : m_hblurBuffer;
                    auto to = !horizontal ? m_vblurBuffer : m_hblurBuffer;
                    auto program = horizontal ? m_horizontalBlurProgram : m_verticalBlurProgram;

                    program->bind();
                    to->bind();
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    program->setTexture("tex", from->getColorAttachment(0));

                    glBindVertexArray(m_fullscreenQuadVAO);
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                    glBindVertexArray(0);

                    to->unbind();
                    program->unbind();

                    horizontal = !horizontal;
                }

                if (m_drawMode == DrawMode::BRIGHT_BLUR) {
                    m_textureProgram->bind();
                    glViewport(0, 0, m_width, m_height);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    m_textureProgram->setTexture("tex", m_vblurBuffer->getColorAttachment(0));

                    glBindVertexArray(m_fullscreenQuadVAO);
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                    glBindVertexArray(0);

                    m_textureProgram->unbind();
                } else {
                    if (m_useAdaptiveExposure) {
                        m_lightingBuffer->getColorAttachment(0).bind();
                        glGenerateMipmap(GL_TEXTURE_2D);
                        int highestMipMapLevel = std::floor(std::log2(std::max(m_width, m_height)));
                        float averageLuminance;
                        glGetTexImage(GL_TEXTURE_2D, highestMipMapLevel, GL_RGBA, GL_FLOAT, &averageLuminance);
                        m_lightingBuffer->getColorAttachment(0).unbind();

                        float averageLuminanceClamped = std::fmaxf(0.2f, std::fminf(averageLuminance, 0.8f));
                        float exposureAdjustmentRate = 0.1f;
                        m_exposure = m_exposure + (0.5f/averageLuminanceClamped - m_exposure) * exposureAdjustmentRate;
                    } else {
                        m_exposure = 1.0f;
                    }

                    m_bloomProgram->bind();
                    glViewport(0, 0, m_width, m_height);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    m_bloomProgram->setTexture("color", m_lightingBuffer->getColorAttachment(0));
                    m_bloomProgram->setTexture("bloom", m_vblurBuffer->getColorAttachment(0));
                    m_bloomProgram->setUniform("exposure", m_exposure);

                    glBindVertexArray(m_fullscreenQuadVAO);
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                    glBindVertexArray(0);

                    m_bloomProgram->unbind();
                }
            }
        }
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
    m_lightingBuffer = std::make_unique<FBO>(1, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, w, h,
                                             TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                             TextureParameters::FILTER_METHOD::LINEAR,
                                             GL_FLOAT);

    m_vblurBuffer = std::make_shared<FBO>(1, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, w/2, h/2,
                                          TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                          TextureParameters::FILTER_METHOD::LINEAR,
                                          GL_FLOAT);
    m_hblurBuffer = std::make_shared<FBO>(1, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, w/2, h/2,
                                          TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                          TextureParameters::FILTER_METHOD::LINEAR,
                                          GL_FLOAT);
}

void View::drawGeometry() {
    // sphere 1
    glm::mat4 M = glm::translate(glm::vec3(glm::sin(m_globalTime), 0.0f, 0.0f));
    m_deferredProgram->setUniform("M", M);
    CS123SceneMaterial mat;
    mat.cAmbient = glm::vec4(0.1, 0, 0, 1);
    mat.cDiffuse = glm::vec4(0, 1, 0, 1);
    mat.cSpecular = glm::vec4(0, 0, 1, 1);
    mat.shininess = 20.0f;
    m_deferredProgram->applyMaterial(mat);
    m_sphere->draw();

    // sphere 2
    M = glm::translate(glm::vec3(0.0f, 0.0f, glm::cos(m_globalTime)));
    m_deferredProgram->setUniform("M", M);
    mat.cAmbient = glm::vec4(0.07, 0.07, 0, 1);
    mat.cDiffuse = glm::vec4(1, 0, 1, 1);
    mat.cSpecular = glm::vec4(0, 1, 1, 1);
    mat.shininess = 100.0f;
    m_deferredProgram->applyMaterial(mat);
    m_sphere->draw();

    // sphere 3
    M = glm::translate(glm::vec3(0.0f, 1.5f, 0.0f));
    m_deferredProgram->setUniform("M", M);
    mat.cAmbient = glm::vec4(0.2, 0.2, 0.4, 1);
    mat.cDiffuse = glm::vec4(1.3, 2, 1.2, 1);
    mat.cSpecular = glm::vec4(2, 1, 1, 1);
    mat.shininess = 50.0f;
    m_deferredProgram->applyMaterial(mat);
    m_sphere->draw();

    // cube
    M = glm::translate(glm::vec3(0.0f, -1.0f, 0.0f)) * glm::scale(glm::vec3(3.0f, 1.0f, 3.0f));
    m_deferredProgram->setUniform("M", M);
    mat.cAmbient = glm::vec4(0, 0, 0, 1);
    mat.cDiffuse = glm::vec4(0.25, 0.25, 0.25, 1);
    mat.cSpecular = glm::vec4(1, 0, 1, 1);
    mat.shininess = 10.0f;
    m_deferredProgram->applyMaterial(mat);
    m_cube->draw();
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
    else if (event->key() == Qt::Key_8) m_drawMode = DrawMode::NO_HDR;
    else if (event->key() == Qt::Key_9) m_drawMode = m_drawMode == DrawMode::BRIGHT ? DrawMode::BRIGHT_BLUR : DrawMode::BRIGHT;

    if (event->key() == Qt::Key_P) m_useAdaptiveExposure = !m_useAdaptiveExposure;

}

void View::keyReleaseEvent(QKeyEvent *event) {

}

void View::tick() {
    // Get the number of seconds since the last tick (variable update rate)
    float dt = m_time.restart() * 0.001f;

    m_globalTime += dt;

    // Flag this view for repainting (Qt will call paintGL() soon after)
    update();
}
