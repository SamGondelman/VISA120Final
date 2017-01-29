#include "view.h"

#include "viewformat.h"
#include "ResourceLoader.h"
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
#include "Player.h"
#include "ParticleSystem.h"

#include <QApplication>
#include <QKeyEvent>
#include <iostream>

View::View(QWidget *parent) : QGLWidget(ViewFormat(), parent),
    m_time(), m_timer(), m_drawMode(DrawMode::DEFAULT), m_world(WORLD_DEMO),
    m_exposure(1.0f), m_useAdaptiveExposure(true),
    m_lightOrigin(glm::vec3(0)), m_lightTime(INFINITY)
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
    m_deferredProgram = std::make_shared<CS123Shader>(vertexSource, fragmentSource);

    vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/shader.vert");
    fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/lightWorld.frag");
    m_lightWorldProgram = std::make_shared<CS123Shader>(vertexSource, fragmentSource);

    vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/shader.vert");
    fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/waterWorld.frag");
    m_waterWorldProgram = std::make_shared<CS123Shader>(vertexSource, fragmentSource);

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

    m_player = std::make_unique<Player>(m_width, m_height);
    m_lightParticles = std::make_shared<ParticleSystem>(5000,
                                                        ":/shaders/lightParticlesDraw.frag",
                                                        ":/shaders/lightParticlesDraw.vert",
                                                        ":/shaders/lightParticlesUpdate.frag");

    setWorld();
}

void View::paintGL() {
    // Draw to deferred buffer
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    auto worldProgram = getWorldProgram();

    worldProgram->bind();
    m_deferredBuffer->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 V = m_player->getView();
    worldProgram->setUniform("V", V);
    glm::mat4 P = m_player->getPerspective();
    worldProgram->setUniform("P", P);

    if (m_drawMode != DrawMode::LIGHTS) {
        drawGeometry(worldProgram, m_deferredBuffer, V, P);
    } else {
        // Draw point lights as geometry
        for (auto& light : m_lights) {
            if (light.type == LightType::LIGHT_POINT) {
                glm::mat4 M = glm::translate(light.pos) * glm::scale(glm::vec3(2.0f * light.radius));
                worldProgram->setUniform("M", M);
                CS123SceneMaterial mat;
                mat.cAmbient = glm::vec4(light.col, 1);
                worldProgram->applyMaterial(mat);
                m_lightSphere->draw();
            }
        }
    }

    worldProgram->unbind();

    // Drawing to screen
    if (m_drawMode == DrawMode::POSITION || m_drawMode == DrawMode::NORMAL || m_drawMode == DrawMode::AMBIENT ||
            m_drawMode == DrawMode::LIGHTS) {
        // No lighting
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_deferredBuffer->getId());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        int offset = m_drawMode != DrawMode::LIGHTS ? m_drawMode : DrawMode::AMBIENT;
        glReadBuffer(GL_COLOR_ATTACHMENT0 + offset);
        glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    } else {
        // Lighting
        m_lightingProgram->bind();
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

        // Diffuse and/or specular terms
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
            glm::vec3 dist = light.pos - m_player->getEye();
            if (light.type == LightType::LIGHT_POINT && glm::dot(dist, dist) > light.radius * light.radius) {
                // Outside point light
                m_lightingProgram->setUniform("worldSpace", 1.0f);

                glm::mat4 M = glm::translate(light.pos) * glm::scale(glm::vec3(2.0f * light.radius));
                m_lightingProgram->setUniform("M", M);
                m_lightSphere->draw();
            } else {
                // Inside point light or directional light
                m_lightingProgram->setUniform("worldSpace", 0.0f);

                m_fullscreenQuad->draw();
            }
        }

        m_lightingBuffer->unbind();
        m_lightingProgram->unbind();
        glActiveTexture(GL_TEXTURE0);

        if (m_drawMode == DrawMode::DIFFUSE || m_drawMode == DrawMode::SPECULAR || m_drawMode == DrawMode::NO_HDR) {
            // For debugging, draw lighting buffer before HDR/bloom
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_lightingBuffer->getId());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        } else {
            // HDR/bloom
            // Extract bright areas
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
                // Draw bright areas (already downscaled/upscaled but not blurred)
                m_textureProgram->bind();
                glViewport(0, 0, m_width, m_height);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                m_textureProgram->setTexture("tex", m_vblurBuffer->getColorAttachment(0));

                glBindVertexArray(m_fullscreenQuadVAO);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                glBindVertexArray(0);

                m_textureProgram->unbind();
            } else {
                // Blur bright areas
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
                    // Draw blurred bright areas
                    m_textureProgram->bind();
                    glViewport(0, 0, m_width, m_height);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    m_textureProgram->setTexture("tex", m_vblurBuffer->getColorAttachment(0));

                    glBindVertexArray(m_fullscreenQuadVAO);
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                    glBindVertexArray(0);

                    m_textureProgram->unbind();
                } else {
                    // Calculate adaptive exposure
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

                    // Recombine bloom, tonemapping
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

    m_player->setAspectRatio(w, h);

    // Resize deferred buffer
    m_deferredBuffer = std::make_shared<FBO>(5, FBO::DEPTH_STENCIL_ATTACHMENT::DEPTH_ONLY, w, h,
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

void View::drawGeometry(std::shared_ptr<CS123Shader> program, std::shared_ptr<FBO> deferredBuffer,
                        glm::mat4& V, glm::mat4& P) {
    glm::mat4 M;
    CS123SceneMaterial mat;
    if (m_world == World::WORLD_DEMO || m_world == World::WORLD_2) {
        // sphere 1
        M = glm::translate(glm::vec3(glm::sin(m_globalTime), 0.0f, 0.0f));
        program->setUniform("M", M);
        mat.cAmbient = glm::vec4(0.1, 0, 0, 1);
        mat.cDiffuse = glm::vec4(0, 1, 0, 1);
        mat.cSpecular = glm::vec4(0, 0, 1, 1);
        mat.shininess = 20.0f;
        program->applyMaterial(mat);
        m_sphere->draw();

        // sphere 2
        M = glm::translate(glm::vec3(0.0f, 0.0f, glm::cos(m_globalTime)));
        program->setUniform("M", M);
        mat.cAmbient = glm::vec4(0.07, 0.07, 0, 1);
        mat.cDiffuse = glm::vec4(1, 0, 1, 1);
        mat.cSpecular = glm::vec4(0, 1, 1, 1);
        mat.shininess = 100.0f;
        program->applyMaterial(mat);
        m_sphere->draw();

        // sphere 3
        M = glm::translate(glm::vec3(0.0f, 1.5f, 0.0f));
        program->setUniform("M", M);
        mat.cAmbient = glm::vec4(0.2, 0.2, 0.4, 1);
        mat.cDiffuse = glm::vec4(1.3, 2, 1.2, 1);
        mat.cSpecular = glm::vec4(2, 1, 1, 1);
        mat.shininess = 50.0f;
        program->applyMaterial(mat);
        m_sphere->draw();

        // cube
        M = glm::translate(glm::vec3(0.0f, -1.0f, 0.0f)) * glm::scale(glm::vec3(3.0f, 1.0f, 3.0f));
        program->setUniform("M", M);
        mat.cAmbient = glm::vec4(0, 0, 0, 1);
        mat.cDiffuse = glm::vec4(0.25, 0.25, 0.25, 1);
        mat.cSpecular = glm::vec4(1, 0, 1, 1);
        mat.shininess = 10.0f;
        program->applyMaterial(mat);
        m_cube->draw();
    } else if (m_world == World::WORLD_1) {
        program->setUniform("origin", m_lightOrigin);
        program->setUniform("time", m_lightTime);

        // floor
        M = glm::translate(glm::vec3(0.0f, -1.0f, 0.0f)) * glm::scale(glm::vec3(30.0f, 0.3f, 30.0f));
        program->setUniform("M", M);
        m_cube->draw();

        // box
        M = glm::translate(glm::vec3(0.0f, -0.5f, 0.0f)) * glm::scale(glm::vec3(1.5f, 2.0f, 1.5f));
        program->setUniform("M", M);
        m_cube->draw();

        // walls
        M = glm::translate(glm::vec3(7.0f, 2.5f, 0.0f)) * glm::scale(glm::vec3(1.0f, 7.0f, 7.0f));
        program->setUniform("M", M);
        m_cube->draw();

        M = glm::translate(glm::vec3(0.0f, 2.5f, 7.0f)) * glm::scale(glm::vec3(7.0f, 7.0f, 1.0f));
        program->setUniform("M", M);
        m_cube->draw();
    }

    // Bind the fullscreen VAO to update entire particle texture
    glBindVertexArray(m_fullscreenQuadVAO);
    m_lightParticles->update(0);
    glBindVertexArray(0);

    // Bind the deferred buffer to draw the particles
    deferredBuffer->bind();
    m_lightParticles->render(m_width, m_height, V, P, &drawCube, this);
    deferredBuffer->unbind();
}

void View::drawCube() {
    m_cube->draw();
}

void View::worldUpdate(float dt) {
    if (m_world == World::WORLD_DEMO || m_world == World::WORLD_2) {
        m_player->setEye(glm::vec3(6.0f * glm::sin(m_globalTime/3.0f), 1.0f, 6.0f * glm::cos(m_globalTime/3.0f)));
        m_player->setCenter(glm::vec3(0));

//        m_player->setEye(glm::vec3(0.0f, 1.0f, 6.0f));
    } else if (m_world == World::WORLD_1) {
        const float RING_DURATION = 5.0f;
        if (m_lightTime == INFINITY || m_lightTime > RING_DURATION) {
            m_lightTime = 0.0f;
            m_lightOrigin = m_player->getEye();
        } else {
            m_lightTime += dt;
        }
    }

}

void View::setWorld() {
    // Set camera
    if (m_world == World::WORLD_DEMO) {
        // camera gets set in update
    } else if (m_world == World::WORLD_1) {
        m_player->setEye(glm::vec3(-4, 2, -4));
        m_player->setCenter(glm::vec3(0));
    }

    // Set lights
    m_lights.clear();
    if (m_world == World::WORLD_DEMO) {
        m_lights.push_back(Light(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 1.5f, 15.8f)));
        m_lights.push_back(Light(glm::vec3(-1.0f), glm::vec3(0.7f)));
    } else if (m_world == World::WORLD_1) {
        // no lights
    } else if (m_world == World::WORLD_2) {
//        m_lights.push_back(Light(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f), glm::vec3(1.0f, 1.5f, 15.8f)));
        m_lights.push_back(Light(glm::vec3(-1.0f), glm::vec3(0.7f)));
    }
}

std::shared_ptr<CS123Shader> View::getWorldProgram() {
    if (m_world == World::WORLD_DEMO || m_drawMode == DrawMode::LIGHTS) {
        return m_deferredProgram;
    } else if (m_world == World::WORLD_1){
        return m_lightWorldProgram;
    } else {
        return m_waterWorldProgram;
    }
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

    World prevWorld = m_world;
    if (event->key() == Qt::Key_F1) m_world = World::WORLD_DEMO;
    else if (event->key() == Qt::Key_F2) m_world = World::WORLD_1;
    else if (event->key() == Qt::Key_F3) m_world = World::WORLD_2;

    if (m_world != prevWorld) setWorld();

}

void View::keyReleaseEvent(QKeyEvent *event) {

}

void View::tick() {
    // Get the number of seconds since the last tick (variable update rate)
    float dt = m_time.restart() * 0.001f;

    m_globalTime += dt;

    worldUpdate(dt);

    // Flag this view for repainting (Qt will call paintGL() soon after)
    update();
}
