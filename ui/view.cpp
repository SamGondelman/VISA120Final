#include "view.h"

#include "viewformat.h"
#include "ResourceLoader.h"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "gl/shaders/CS123Shader.h"
#include "CS123SceneData.h"
#include "gl/datatype/FBO.h"
#include "gl/textures/Texture2D.h"
#include "gl/textures/TextureParametersBuilder.h"
#include "SphereMesh.h"
#include "CubeMesh.h"
#include "ConeMesh.h"
#include "CylinderMesh.h"
#include "gl/datatype/VAO.h"
#include "FullScreenQuad.h"
#include "Player.h"
#include "ParticleSystem.h"

#include "Entity.h"
#include "World.h"
#include "DemoWorld.h"
#include "LightWorld.h"
#include "WaterWorld.h"
#include "RockWorld.h"
#include "PhysicsWorld.h"

#include <QApplication>
#include <QKeyEvent>
#include <iostream>

#include <openvr.h>

float View::m_globalTime = 0.0f;
float View::m_rockTime = 0.0f;
std::unique_ptr<Player> View::m_player = nullptr;
std::unique_ptr<SphereMesh> View::m_sphere = nullptr;
std::unique_ptr<CubeMesh> View::m_cube = nullptr;
std::unique_ptr<ConeMesh> View::m_cone = nullptr;
std::unique_ptr<CylinderMesh> View::m_cylinder = nullptr;
unsigned int View::m_fullscreenQuadVAO = 0;
std::set<int> View::m_pressedKeys = std::set<int>();

View::View(QWidget *parent) : QGLWidget(ViewFormat(), parent),
    m_fps(0.0f), m_leftShield(nullptr), m_rightShield(nullptr),
    m_drawMode(DrawMode::DEFAULT), m_world(WORLD_DEMO),
    m_exposure(1.0f), m_useAdaptiveExposure(true)
{
    // View needs all mouse move events, not just mouse drag events
    setMouseTracking(true);

    // View needs keyboard focus
    setFocusPolicy(Qt::StrongFocus);

    // The update loop is implemented using a timer
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));

    // Print FPS
    connect(&m_FPStimer, SIGNAL(timeout()), this, SLOT(printFPS()));
}

View::~View()
{
    // Must remove shields here to ensure entity deletion happens in right order
    m_worlds[m_world]->getPhysWorld()->removeRigidBody(m_leftShield->m_rigidBody.get());
    m_worlds[m_world]->getPhysWorld()->removeRigidBody(m_rightShield->m_rigidBody.get());
    glDeleteVertexArrays(1, &m_fullscreenQuadVAO);
}

void View::initializeGL() {
    ResourceLoader::initializeGlew();

    m_time.start();
    // TODO: change to / 90
    m_timer.start(1000 / 60);

    m_FPStimer.start(1000);

    // OpenGL setup
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glGenVertexArrays(1, &m_fullscreenQuadVAO);

    m_sphere = std::make_unique<SphereMesh>(10, 10);
    m_cube = std::make_unique<CubeMesh>(1);
    m_cone = std::make_unique<ConeMesh>(20, 20);
    m_cylinder = std::make_unique<CylinderMesh>(20, 20);
    m_lightSphere = std::make_unique<SphereMesh>(15, 15);
    m_fullscreenQuad = std::make_unique<FullScreenQuad>();

    // Shader setup
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/lighting.vert");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/lighting.frag");
    m_lightingProgram = std::make_unique<CS123Shader>(vertexSource, fragmentSource);

    vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/rock.vert");
    fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/rock.frag");
    m_rockProgram = std::make_unique<CS123Shader>(vertexSource, fragmentSource);

    vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/fullscreenQuad.vert");
    fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/bright.frag");
    m_brightProgram = std::make_unique<CS123Shader>(vertexSource, fragmentSource);

    vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/shader.vert");
    fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/distortionStencil.frag");
    m_distortionStencilProgram = std::make_unique<CS123Shader>(vertexSource, fragmentSource);

    vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/fullscreenQuad.vert");
    fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/distortion.frag");
    m_distortionProgram = std::make_unique<CS123Shader>(vertexSource, fragmentSource);

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

    // Player setup
    m_player = std::make_unique<Player>(m_width, m_height);

    m_lightParticles = std::make_shared<ParticleSystem>(5000,
                                                        ":/shaders/lightParticlesDraw.frag",
                                                        ":/shaders/lightParticlesDraw.vert",
                                                        ":/shaders/lightParticlesUpdate.frag");
    m_fireParticles = std::make_shared<ParticleSystem>(1000,
                                                       ":/shaders/fireParticlesDraw.frag",
                                                       ":/shaders/fireParticlesDraw.vert",
                                                       ":/shaders/fireParticlesUpdate.frag");

    QImage shieldMapImage = QImage(":/images/shieldNormalMap.png");
    m_shieldMap = std::make_unique<Texture2D>(shieldMapImage.bits(),
                                              shieldMapImage.width(),
                                              shieldMapImage.height());
    // TODO: move these into Texture2D
    TextureParametersBuilder builder;
    builder.setFilter(TextureParameters::FILTER_METHOD::LINEAR);
    builder.setWrap(TextureParameters::WRAP_METHOD::REPEAT);
    TextureParameters parameters = builder.build();
    parameters.applyTo(*m_shieldMap);

    // World setup
    m_worlds.push_back(std::make_shared<DemoWorld>());
    m_worlds.push_back(std::make_shared<LightWorld>());
    m_worlds.push_back(std::make_shared<WaterWorld>());
    m_worlds.push_back(std::make_shared<RockWorld>());
    m_worlds.push_back(std::make_shared<PhysicsWorld>());
    switchWorld();
}

void View::resizeGL(int w, int h) {
    m_width = w;
    m_height = h;
//    float ratio = static_cast<QGuiApplication *>(QCoreApplication::instance())->devicePixelRatio();
    //w = static_cast<int>(w / ratio);
    //h = static_cast<int>(h / ratio);
    glViewport(0, 0, w, h);

    m_player->setAspectRatio(w, h);

    // Resize buffers
    // contains view space positions/normals and ambient/diffuse/specular colors
    // reused for distortion objects to take advantage of depth buffer
    m_deferredBuffer = std::make_unique<FBO>(5, FBO::DEPTH_STENCIL_ATTACHMENT::DEPTH_ONLY, w, h,
                                             TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                             TextureParameters::FILTER_METHOD::LINEAR,
                                             GL_FLOAT);
    // collects lighting, contains whole scene minus distortion objects
    m_lightingBuffer = std::make_unique<FBO>(1, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, w, h,
                                             TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                             TextureParameters::FILTER_METHOD::LINEAR,
                                             GL_FLOAT);
    // contains whole scene + distortion objects
    m_distortionBuffer = std::make_unique<FBO>(1, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, w, h,
                                               TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                               TextureParameters::FILTER_METHOD::LINEAR,
                                               GL_FLOAT);

    // half-sized buffers for collecting/blurring bright areas
    m_vblurBuffer = std::make_shared<FBO>(1, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, w/2, h/2,
                                          TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                          TextureParameters::FILTER_METHOD::LINEAR,
                                          GL_FLOAT);
    m_hblurBuffer = std::make_shared<FBO>(1, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, w/2, h/2,
                                          TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                          TextureParameters::FILTER_METHOD::LINEAR,
                                          GL_FLOAT);
}

void View::paintGL() {
    // Draw to deferred buffer
    glDisable(GL_BLEND);

    auto worldProgram = m_worlds[m_world]->getWorldProgram();

    worldProgram->bind();
    m_deferredBuffer->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 V = m_player->getView();
    worldProgram->setUniform("V", V);
    glm::mat4 P = m_player->getPerspective();
    worldProgram->setUniform("P", P);

    if (m_drawMode != DrawMode::LIGHTS) {
        m_worlds[m_world]->drawGeometry();
        drawParticles(0, V, P);
        drawRocks(V, P);
    } else {
        // Draw point lights as geometry
        for (auto& light : m_worlds[m_world]->getLights()) {
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

    m_deferredBuffer->unbind();
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
        glClear(GL_COLOR_BUFFER_BIT);

        bool useLighting = m_drawMode > DrawMode::LIGHTS;

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
        for (auto& light : m_worlds[m_world]->getLights()) {
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
        glDisable(GL_BLEND);

        if (m_drawMode == DrawMode::DIFFUSE || m_drawMode == DrawMode::SPECULAR || m_drawMode == DrawMode::NO_HDR) {
            // For debugging, draw lighting buffer before HDR/bloom
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_lightingBuffer->getId());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        } else {
            if (m_drawMode != DrawMode::NO_DISTORTION) {
                // Distortion objects
                // Render distortion objects with deferred buffer depth attachment to get distortion stencil
                m_distortionStencilProgram->bind();
                m_deferredBuffer->bind();
                glClear(GL_COLOR_BUFFER_BIT);
                m_distortionStencilProgram->setUniform("V", V);
                m_distortionStencilProgram->setUniform("P", P);
                drawDistortionObjects();
                m_deferredBuffer->unbind();
                m_distortionStencilProgram->unbind();

                // Combine distortion with lighting buffer
                m_distortionProgram->bind();
                m_distortionBuffer->bind();
                glClear(GL_COLOR_BUFFER_BIT);
                m_distortionProgram->setTexture("offsetMap", m_deferredBuffer->getColorAttachment(0));
                m_distortionProgram->setTexture("color", m_lightingBuffer->getColorAttachment(0));
                glBindVertexArray(m_fullscreenQuadVAO);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                glBindVertexArray(0);
                m_distortionBuffer->unbind();
                m_distortionProgram->unbind();
            } else {
                glBindFramebuffer(GL_READ_FRAMEBUFFER, m_lightingBuffer->getId());
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_distortionBuffer->getId());
                glReadBuffer(GL_COLOR_ATTACHMENT0);
                glBlitFramebuffer(0, 0, m_width, m_height, 0, 0, m_width, m_height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
            }

            // HDR/bloom
            // Extract bright areas
            m_brightProgram->bind();
            m_vblurBuffer->bind();
            glClear(GL_COLOR_BUFFER_BIT);

            m_brightProgram->setTexture("color", m_distortionBuffer->getColorAttachment(0));

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
                    glClear(GL_COLOR_BUFFER_BIT);

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
                        m_distortionBuffer->getColorAttachment(0).bind();
                        glGenerateMipmap(GL_TEXTURE_2D);
                        int highestMipMapLevel = std::floor(std::log2(std::max(m_width, m_height)));
                        float averageLuminance;
                        glGetTexImage(GL_TEXTURE_2D, highestMipMapLevel, GL_RGBA, GL_FLOAT, &averageLuminance);
                        m_distortionBuffer->getColorAttachment(0).unbind();

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
                    m_bloomProgram->setTexture("color", m_distortionBuffer->getColorAttachment(0));
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

void drawCube(int num) {
    View::m_cube->draw(num);
}

void drawFire(int num) {
    glBindVertexArray(View::m_fullscreenQuadVAO);
    // Top cap
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 5, num);
    // Bottom cap
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 5, 5, num);
    glBindVertexArray(0);
}

void View::drawParticles(float dt, glm::mat4& V, glm::mat4& P) {
    // Bind the fullscreen VAO to update entire particle texture
    glBindVertexArray(m_fullscreenQuadVAO);
    m_lightParticles->update(dt, m_pressedKeys.find(Qt::Key_U) != m_pressedKeys.end());
    m_fireParticles->update(dt, m_pressedKeys.find(Qt::Key_I) != m_pressedKeys.end());
    glBindVertexArray(0);

    // Bind the deferred buffer to draw the particles
    m_deferredBuffer->bind();
    m_lightParticles->render(V, P, &drawCube);
    m_fireParticles->render(V, P, &drawFire);
}

void View::drawRocks(glm::mat4& V, glm::mat4& P) {
    glDisable(GL_CULL_FACE);
    m_rockProgram->bind();
    m_rockProgram->setUniform("V", V);
    m_rockProgram->setUniform("P", P);
    m_rockProgram->setUniform("time", m_rockTime);

    glm::mat4 M = glm::translate(glm::vec3(0.75, 0, 0.0)) *
            glm::rotate(static_cast<float>(M_PI)/2.0f, glm::vec3(0, 0, -1)) *
            glm::scale(glm::vec3(0.5f, 1.0f, 0.5f));
    m_rockProgram->setUniform("M", M);
    m_cone->draw();

    M = glm::translate(glm::vec3(-0.75, 0, 0.0)) *
            glm::rotate(static_cast<float>(M_PI)/2.0f, glm::vec3(0, 0, -1)) *
            glm::scale(glm::vec3(0.5f, 1.0f, 0.5f));
    m_rockProgram->setUniform("M", M);
    m_sphere->draw();

    m_rockProgram->unbind();
    glEnable(GL_CULL_FACE);
}

void View::drawDistortionObjects() {
    m_distortionStencilProgram->setTexture("normalMap", *m_shieldMap);

    glm::mat4 M;
    m_leftShield->getModelMatrix(M);
    m_distortionStencilProgram->setUniform("M", M);
    m_rightShield->draw();

    m_rightShield->getModelMatrix(M);
    m_distortionStencilProgram->setUniform("M", M);
    m_rightShield->draw();
}

void View::mousePressEvent(QMouseEvent *event) {

}

void View::mouseMoveEvent(QMouseEvent *event) {

}

void View::mouseReleaseEvent(QMouseEvent *event) {

}

void View::keyPressEvent(QKeyEvent *event) {
    if (!event->isAutoRepeat()) m_pressedKeys.insert(event->key());

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
    else if (event->key() == Qt::Key_0) m_drawMode = DrawMode::NO_DISTORTION;

    WorldState prevWorld = m_world;
    if (event->key() == Qt::Key_F1) m_world = WorldState::WORLD_DEMO;
    else if (event->key() == Qt::Key_F2) m_world = WorldState::WORLD_1;
    else if (event->key() == Qt::Key_F3) m_world = WorldState::WORLD_2;
    else if (event->key() == Qt::Key_F4) m_world = WorldState::WORLD_3;
    else if (event->key() == Qt::Key_F5) m_world = WorldState::WORLD_4;

    if (m_world != prevWorld) switchWorld(prevWorld);

    if (event->key() == Qt::Key_P) m_useAdaptiveExposure = !m_useAdaptiveExposure;
}

void View::keyReleaseEvent(QKeyEvent *event) {
    if (!event->isAutoRepeat()) m_pressedKeys.erase(event->key());
}

void View::tick() {
    // Get the number of seconds since the last tick (variable update rate)
    float dt = m_time.restart() * 0.001f;
    if (dt != 0.0f) m_fps = 0.02f / dt + 0.98f * m_fps;

    m_globalTime += dt;

    // Rock spawning
    if (m_pressedKeys.find(Qt::Key_O) != m_pressedKeys.end()) {
        m_rockTime += dt;
        m_rockTime = std::min(1.75f, m_rockTime);
        if (m_rockTime == 1.75f) {
            CS123SceneMaterial mat;
            mat.cAmbient = glm::vec4(0.25f*glm::vec3(0.137, 0.094, 0.118), 1);
            mat.cDiffuse = glm::vec4(0.25f*glm::vec3(0.443, 0.263, 0.2), 1);
            mat.cSpecular = glm::vec4(0.25f*glm::vec3(0.773, 0.561, 0.419), 1);
            mat.shininess = 1.0f;
            m_worlds[m_world]->getEntities().emplace_back(m_worlds[m_world]->getPhysWorld(),
                                                          ShapeType::SPHERE, 1.0f, btVector3(-0.75, 0, 0.0),
                                                          btVector3(0.5, 1.0, 0.5), mat, btQuaternion(0, 0, -M_PI/2.0f),
                                                          btVector3(20, 0, 0));
            m_worlds[m_world]->getEntities().emplace_back(m_worlds[m_world]->getPhysWorld(),
                                                          ShapeType::CONE, 1.0f, btVector3(0.75, 0, 0.0),
                                                          btVector3(0.5, 1.0, 0.5), mat, btQuaternion(0, 0, -M_PI/2.0f),
                                                          btVector3(20, 0, 0));
            m_rockTime = 0.0f;
            m_pressedKeys.erase(Qt::Key_O);
        }
    } else {
        m_rockTime = std::max(0.0f, m_rockTime - 5 * dt);
    }

    // Shield movement
    btTransform t;
    t.setIdentity();
    t.setOrigin(btVector3(2.2f * std::sin(m_globalTime/2.0f), 0, 2.2f * std::cos(m_globalTime/2.0f)));
    t.setRotation(btQuaternion(m_globalTime/2.0f, 0, 0));
    m_leftShield->m_rigidBody->setWorldTransform(t);
    m_leftShield->m_rigidBody->getMotionState()->setWorldTransform(t);

    m_worlds[m_world]->update(dt);

    // Flag this view for repainting (Qt will call paintGL() soon after)
    update();
}

void View::switchWorld(WorldState prevWorld) {
    m_worlds[m_world]->makeCurrent();

    if (!m_leftShield && !m_rightShield) {
        m_leftShield = std::make_unique<Entity>(m_worlds[m_world]->getPhysWorld(), ShapeType::CUBE, 0.0f,
                                                btVector3(0.0f, 0.0f, 1.5f), btVector3(1.5f, 1.5f, 0.05f));
        m_leftShield->m_rigidBody->setCollisionFlags(m_leftShield->m_rigidBody->getCollisionFlags() |
                                                     btCollisionObject::CF_KINEMATIC_OBJECT);
        m_leftShield->m_rigidBody->setActivationState(DISABLE_DEACTIVATION);
        m_rightShield = std::make_unique<Entity>(m_worlds[m_world]->getPhysWorld(), ShapeType::CUBE, 0.0f,
                                                 btVector3(0.0f, 0.5f, 0.0f), btVector3(0.5f, 0.5f, 0.5f));
        m_leftShield->m_rigidBody->setCollisionFlags(m_leftShield->m_rigidBody->getCollisionFlags() |
                                                     btCollisionObject::CF_KINEMATIC_OBJECT);
        m_leftShield->m_rigidBody->setActivationState(DISABLE_DEACTIVATION);
    } else {
        m_worlds[prevWorld]->getPhysWorld()->removeRigidBody(m_leftShield->m_rigidBody.get());
        m_worlds[prevWorld]->getPhysWorld()->removeRigidBody(m_rightShield->m_rigidBody.get());
        m_worlds[m_world]->getPhysWorld()->addRigidBody(m_leftShield->m_rigidBody.get());
        m_worlds[m_world]->getPhysWorld()->addRigidBody(m_rightShield->m_rigidBody.get());
    }
}

void View::printFPS() {
    std::cout << m_fps << std::endl;
}
