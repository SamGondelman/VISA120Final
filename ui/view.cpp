#include "view.h"

#include "viewformat.h"
#include "ResourceLoader.h"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/norm.hpp>

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

#include "Entity.h"
#include "PhysicsWorld.h"

#include <QApplication>
#include <QKeyEvent>
#include <QMessageBox>
#include <QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <iostream>
#include <time.h>
#include <QLoggingCategory>

#include "voce/src/c++/voce.h"

float View::m_globalTime = 0.0f;
std::unique_ptr<Player> View::m_player = nullptr;
std::unique_ptr<SphereMesh> View::m_sphere = nullptr;
std::unique_ptr<CubeMesh> View::m_cube = nullptr;
std::unique_ptr<ConeMesh> View::m_cone = nullptr;
std::unique_ptr<CylinderMesh> View::m_cylinder = nullptr;
unsigned int View::m_fullscreenQuadVAO = 0;
std::unordered_set<int> View::m_pressedKeys = std::unordered_set<int>();
std::unordered_map<std::string, Texture2D> View::m_textureMap = std::unordered_map<std::string, Texture2D>();
QMutex View::m_textureMutex;

View::View(QWidget *parent) : QGLWidget(ViewFormat(), parent),
    m_fps(0.0f), m_createTimeLeft(0.0f), m_createTimeRight(0.0f),
    m_drawMode(DrawMode::DEFAULT), m_exposure(1.0f),
    m_useAdaptiveExposure(true)
{
    // View needs all mouse move events, not just mouse drag events
    setMouseTracking(true);

    // View needs keyboard focus
    setFocusPolicy(Qt::StrongFocus);

    // The update loop is implemented using a timer
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(tick()));

    // Print FPS
    connect(&m_FPStimer, SIGNAL(timeout()), this, SLOT(printFPS()));

    // Setup network managers
    srand(time(NULL));
    QLoggingCategory::setFilterRules("qt.network.ssl.warning=false");
    m_networkManager = std::make_unique<QNetworkAccessManager>();
    m_networkManagerImg = std::make_unique<QNetworkAccessManager>();
    connect(m_networkManager.get(), SIGNAL(finished(QNetworkReply*)),
            this, SLOT(getImageLinkFromURL(QNetworkReply*)));
    connect(m_networkManagerImg.get(), SIGNAL(finished(QNetworkReply*)),
            this, SLOT(getImageFromLink(QNetworkReply*)));

    // Start voce
    voce::init("libraries/voce/lib", true, true, "", "visa120grammar");
}

View::~View()
{
    if (m_hmd) vr::VR_Shutdown();
    glDeleteVertexArrays(1, &m_fullscreenQuadVAO);
    voce::destroy();
}

void View::initializeGL() {
    ResourceLoader::initializeGlew();

    m_time.start();
    m_timer.start(1000 / 90);

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
    m_cone = std::make_unique<ConeMesh>(1, 20);
    m_cylinder = std::make_unique<CylinderMesh>(1, 5);
    m_lightSphere = std::make_unique<SphereMesh>(15, 15);
    m_fullscreenQuad = std::make_unique<FullScreenQuad>();

    // Shader setup
    std::string vertexSource = ResourceLoader::loadResourceFileToString(":/shaders/lighting.vert");
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(":/shaders/lighting.frag");
    m_lightingProgram = std::make_unique<CS123Shader>(vertexSource, fragmentSource);

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
    m_player = std::make_unique<Player>(m_eyeWidth, m_eyeHeight);

    QString helpImagePath = ":/images/vivewand.png";
    QImage helpImage = QImage(helpImagePath);
    addImage(helpImagePath, helpImage);

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
    m_world = std::make_unique<PhysicsWorld>();
    m_world->makeCurrent();

    initVR();

    m_player->setAspectRatio(m_eyeWidth, m_eyeHeight);

    // Initialize screen buffers
    // contains view space positions/normals and ambient/diffuse/specular colors
    // reused for distortion objects to take advantage of depth buffer
    m_deferredBuffer = std::make_unique<FBO>(5, FBO::DEPTH_STENCIL_ATTACHMENT::DEPTH_ONLY, m_eyeWidth, m_eyeHeight,
                                             TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                             TextureParameters::FILTER_METHOD::LINEAR,
                                             GL_FLOAT);
    // collects lighting, contains whole scene minus distortion objects
    m_lightingBuffer = std::make_unique<FBO>(1, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, m_eyeWidth, m_eyeHeight,
                                             TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                             TextureParameters::FILTER_METHOD::LINEAR,
                                             GL_FLOAT);
    // contains whole scene + distortion objects
    m_distortionBuffer = std::make_unique<FBO>(1, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, m_eyeWidth, m_eyeHeight,
                                               TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                               TextureParameters::FILTER_METHOD::LINEAR,
                                               GL_FLOAT);

    // half-sized buffers for collecting/blurring bright areas
    m_vblurBuffer = std::make_shared<FBO>(1, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, m_eyeWidth/2, m_eyeHeight/2,
                                          TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                          TextureParameters::FILTER_METHOD::LINEAR,
                                          GL_FLOAT);
    m_hblurBuffer = std::make_shared<FBO>(1, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, m_eyeWidth/2, m_eyeHeight/2,
                                          TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                          TextureParameters::FILTER_METHOD::LINEAR,
                                          GL_FLOAT);
}

void View::addImage(QString &s, QImage &img) {
    img = QGLWidget::convertToGLFormat(img);
    Texture2D tex(img.bits(), img.width(), img.height());
    TextureParametersBuilder builder;
    builder.setFilter(TextureParameters::FILTER_METHOD::LINEAR);
    builder.setWrap(TextureParameters::WRAP_METHOD::REPEAT);
    TextureParameters parameters = builder.build();
    parameters.applyTo(tex);

    m_textureMutex.lock();
    m_textureMap.emplace(std::pair<std::string, Texture2D>(s.toStdString(), std::move(tex)));
    m_textureMutex.unlock();
}

void View::initVR() {
    vr::EVRInitError error = vr::VRInitError_None;
    m_hmd = vr::VR_Init(&error, vr::VRApplication_Scene);

    if (error != vr::VRInitError_None) {
        m_hmd = nullptr;
        QString message = vr::VR_GetVRInitErrorAsEnglishDescription(error);
        qCritical() << message;
        QMessageBox::critical(this, "Unable to init VR", message);
        return;
    }

    // get eye matrices
    m_rightProjection = vrMatrixToQt(m_hmd->GetProjectionMatrix(vr::Eye_Right, m_player->getNear(), m_player->getFar()));
    m_rightPose = vrMatrixToQt(m_hmd->GetEyeToHeadTransform(vr::Eye_Right));

    m_leftProjection = vrMatrixToQt(m_hmd->GetProjectionMatrix(vr::Eye_Left, m_player->getNear(), m_player->getFar()));
    m_leftPose = vrMatrixToQt(m_hmd->GetEyeToHeadTransform(vr::Eye_Left));

    // setup frame buffers for eyes
    m_hmd->GetRecommendedRenderTargetSize(&m_eyeWidth, &m_eyeHeight);

    if (!vr::VRCompositor()) {
        QString message = "Compositor initialization failed. See log file for details";
        qCritical() << message;
        QMessageBox::critical(this, "Unable to init VR", message);
    }

    // VR buffers
    m_leftEyeBuffer = std::make_shared<FBO>(1, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, m_eyeWidth, m_eyeHeight,
                                            TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                            TextureParameters::FILTER_METHOD::LINEAR,
                                            GL_UNSIGNED_BYTE);
    m_rightEyeBuffer = std::make_shared<FBO>(1, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, m_eyeWidth, m_eyeHeight,
                                             TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                             TextureParameters::FILTER_METHOD::LINEAR,
                                             GL_UNSIGNED_BYTE);

    // Make sure hands are disabled initially
    m_trackedHandPoses[Hand::LEFT].bDeviceIsConnected = false;
    m_trackedHandPoses[Hand::RIGHT].bDeviceIsConnected = false;
}

void View::resizeGL(int w, int h) {

}

void View::paintGL() {
    renderEye(vr::Eye_Left);
    renderEye(vr::Eye_Right);

    vr::Texture_t left = { (void*)m_leftEyeBuffer->getColorAttachment(0).id(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
    vr::Texture_t right = { (void*)m_rightEyeBuffer->getColorAttachment(0).id(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
    vr::VRCompositor()->Submit(vr::Eye_Left, &left);
    vr::VRCompositor()->Submit(vr::Eye_Right, &right);
}

void View::renderEye(vr::EVREye eye) {
    // Draw to deferred buffer
    glDisable(GL_BLEND);

    auto eyeBuffer = eye == vr::Eye_Left ? m_leftEyeBuffer : m_rightEyeBuffer;
    auto worldProgram = m_world->getWorldProgram();

    worldProgram->bind();
    m_deferredBuffer->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 V = (eye == vr::Eye_Right ? m_leftPose : m_rightPose) * m_hmdPose;
    worldProgram->setUniform("V", V);
    glm::mat4 P = eye == vr::Eye_Left ? m_leftProjection : m_rightProjection;
    worldProgram->setUniform("P", P);

    if (m_drawMode != DrawMode::LIGHTS) {
        m_world->drawGeometry();
        drawHands(V, P);
        drawAction(V, P);
    } else {
        // Draw point lights as geometry
        for (auto& light : m_world->getLights()) {
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
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, eyeBuffer->getId());
        int offset = m_drawMode != DrawMode::LIGHTS ? m_drawMode : DrawMode::AMBIENT;
        glReadBuffer(GL_COLOR_ATTACHMENT0 + offset);
        glBlitFramebuffer(0, 0, m_eyeWidth, m_eyeHeight, 0, 0, m_eyeWidth, m_eyeHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
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
            glBlitFramebuffer(0, 0, m_eyeWidth, m_eyeHeight, 0, 0, m_eyeWidth, m_eyeHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        }

        // Diffuse and/or specular terms
        m_lightingProgram->setUniform("useDiffuse",(useLighting || m_drawMode == DrawMode::DIFFUSE) ? 1.0f : 0.0f);
        m_lightingProgram->setUniform("useSpecular", (useLighting || m_drawMode == DrawMode::SPECULAR) ? 1.0f : 0.0f);
        m_lightingProgram->setUniform("screenSize", glm::vec2(m_eyeWidth, m_eyeHeight));
        m_lightingProgram->setUniform("V", V);
        m_lightingProgram->setUniform("P", P);

        m_lightingProgram->setTexture("pos", m_deferredBuffer->getColorAttachment(0));
        m_lightingProgram->setTexture("nor", m_deferredBuffer->getColorAttachment(1));
        m_lightingProgram->setTexture("diff", m_deferredBuffer->getColorAttachment(3));
        m_lightingProgram->setTexture("spec", m_deferredBuffer->getColorAttachment(4));
        for (auto& light : m_world->getLights()) {
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
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, eyeBuffer->getId());
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            glBlitFramebuffer(0, 0, m_eyeWidth, m_eyeHeight, 0, 0, m_eyeWidth, m_eyeHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
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
                glBlitFramebuffer(0, 0, m_eyeWidth, m_eyeHeight, 0, 0, m_eyeWidth, m_eyeHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
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
                eyeBuffer->bind();
                glViewport(0, 0, m_eyeWidth, m_eyeHeight);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                m_textureProgram->setTexture("tex", m_vblurBuffer->getColorAttachment(0));

                glBindVertexArray(m_fullscreenQuadVAO);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                glBindVertexArray(0);

                eyeBuffer->unbind();
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
                    eyeBuffer->bind();
                    glViewport(0, 0, m_eyeWidth, m_eyeHeight);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    m_textureProgram->setTexture("tex", m_vblurBuffer->getColorAttachment(0));

                    glBindVertexArray(m_fullscreenQuadVAO);
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                    glBindVertexArray(0);

                    eyeBuffer->unbind();
                    m_textureProgram->unbind();
                } else {
                    // Calculate adaptive exposure
                    if (m_useAdaptiveExposure) {
                        m_distortionBuffer->getColorAttachment(0).bind();
                        glGenerateMipmap(GL_TEXTURE_2D);
                        int highestMipMapLevel = std::floor(std::log2(std::max(m_eyeWidth, m_eyeHeight)));
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
                    eyeBuffer->bind();
                    glViewport(0, 0, m_eyeWidth, m_eyeHeight);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    m_bloomProgram->setTexture("color", m_distortionBuffer->getColorAttachment(0));
                    m_bloomProgram->setTexture("bloom", m_vblurBuffer->getColorAttachment(0));
                    m_bloomProgram->setUniform("exposure", m_exposure);

                    glBindVertexArray(m_fullscreenQuadVAO);
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                    glBindVertexArray(0);

                    eyeBuffer->unbind();
                    m_bloomProgram->unbind();
                }
            }
        }
    }
}

void View::drawHands(glm::mat4 &V, glm::mat4 &P) {
    auto program = m_world->getWorldProgram();
    program->bind();
    program->setUniform("V", V);
    program->setUniform("P", P);

    CS123SceneMaterial mat;
    mat.cAmbient = glm::vec4(0.5, 0, 0, 1);
    mat.cDiffuse = glm::vec4(0.5, 0.5, 0.5, 1);
    mat.cSpecular = glm::vec4(0.5, 0.5, 0.5, 1);
    mat.shininess = 10.0;
    program->applyMaterial(mat);

    if (m_trackedHandPoses[Hand::LEFT].bPoseIsValid && m_trackedHandPoses[Hand::LEFT].bDeviceIsConnected) {
        program->setUniform("M", vrMatrixToQt(m_trackedHandPoses[Hand::LEFT].mDeviceToAbsoluteTracking) * glm::rotate(glm::radians(90.0f), glm::vec3(1, 0, 0)) *
                glm::translate(glm::vec3(0, 0.05, 0)) * glm::scale(glm::vec3(0.05, 0.20, 0.05)));
        if (m_mode == CREATE) m_cube->draw();
        else m_cylinder->draw();
    }

    mat.cAmbient = glm::vec4(0, 0, 0.5, 1);
    program->applyMaterial(mat);

    if (m_trackedHandPoses[Hand::RIGHT].bPoseIsValid && m_trackedHandPoses[Hand::RIGHT].bDeviceIsConnected) {
        program->setUniform("M", vrMatrixToQt(m_trackedHandPoses[Hand::RIGHT].mDeviceToAbsoluteTracking) * glm::rotate(glm::radians(90.0f), glm::vec3(1, 0, 0)) *
                glm::translate(glm::vec3(0, 0.05, 0)) * glm::scale(glm::vec3(0.05, 0.20, 0.05)));
        if (m_mode == CREATE) m_cube->draw();
        else m_cylinder->draw();
    }

    program->unbind();
}

void View::drawAction(glm::mat4& V, glm::mat4& P) {
    auto program = m_world->getWorldProgram();
    program->bind();
    program->setUniform("V", V);
    program->setUniform("P", P);

    if (m_mode == CREATE) {
        CS123SceneMaterial mat;
        mat.cAmbient = glm::vec4(0.05, 0.05, 0.05, 1);
        mat.cDiffuse = glm::vec4(0.5, 0.5, 0.5, 1);
        mat.cSpecular = glm::vec4(0.7, 0.7, 0.7, 1);
        mat.shininess = 20.0f;
        program->applyMaterial(mat);
        m_textureMutex.lock();

        float st = 4.0f;
        float sr = 12.0f;
        if (m_createTimeLeft > 0.0f && m_trackedHandPoses[Hand::LEFT].bPoseIsValid && m_trackedHandPoses[Hand::LEFT].bDeviceIsConnected) {
            float s = m_createTimeLeft < M_PI/st ? glm::sin(m_createTimeLeft*st) +  m_createTimeLeft*st : glm::sin(m_createTimeLeft*st - M_PI_2)/2.0f + 2.64f;
            glm::mat4 M = vrMatrixToQt(m_trackedHandPoses[Hand::LEFT].mDeviceToAbsoluteTracking) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::scale(glm::vec3(s/sr));
            program->setUniform("M", M);
            int useTexture = m_textureMap.find(m_currentTextureString.toStdString()) != m_textureMap.end() ? 1 : 0;
            program->setUniform("useTexture", useTexture);
            if (useTexture) program->setTexture("tex", m_textureMap[m_currentTextureString.toStdString()]);
            m_cube->draw();
        }

        if (m_createTimeRight > 0.0f && m_trackedHandPoses[Hand::RIGHT].bPoseIsValid && m_trackedHandPoses[Hand::RIGHT].bDeviceIsConnected) {
            float s = m_createTimeRight < M_PI/st ? glm::sin(m_createTimeRight*st) +  m_createTimeRight*st : glm::sin(m_createTimeRight*st - M_PI_2)/2.0f + 2.64f;
            glm::mat4 M = vrMatrixToQt(m_trackedHandPoses[Hand::RIGHT].mDeviceToAbsoluteTracking) * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0)) * glm::scale(glm::vec3(s/sr));
            program->setUniform("M", M);
            int useTexture = m_textureMap.find(m_currentTextureString.toStdString()) != m_textureMap.end() ? 1 : 0;
            program->setUniform("useTexture", useTexture);
            if (useTexture) program->setTexture("tex", m_textureMap[m_currentTextureString.toStdString()]);
            m_cube->draw();
        }
        m_textureMutex.unlock();
        program->setUniform("useTexture", 0);
    } else {
        CS123SceneMaterial mat;
        mat.cAmbient.xyz = m_paintColor * 0.05f;
        mat.cDiffuse.xyz = m_paintColor * 0.5f;
        mat.cSpecular.xyz = m_paintColor * 0.7f;
        mat.shininess = 20.0f;
        program->applyMaterial(mat);

        if (!glm::isnan(m_paintLeft.x) && m_trackedHandPoses[Hand::LEFT].bPoseIsValid) {
            glm::mat4 M = vrMatrixToQt(m_trackedHandPoses[Hand::LEFT].mDeviceToAbsoluteTracking);
            glm::vec3 pos = glm::vec3(M[3]);
            glm::vec3 d = pos - m_paintLeft;
            float dist = glm::length(d);
            float yaw = atan2(d.x, d.z);
            float pitch = M_PI_2 - asin(d.y/glm::length(d));
            glm::mat4 r = glm::mat4_cast(glm::quat(glm::vec3(pitch, yaw, 0.0f)));
            glm::mat4 m = glm::translate((pos + m_paintLeft)/2.0f) * r * glm::scale(glm::vec3(0.02, dist, 0.02));
            program->setUniform("M", m);
            m_cylinder->draw();
        }

        if (!glm::isnan(m_paintRight.x) && m_trackedHandPoses[Hand::RIGHT].bPoseIsValid) {
            glm::mat4 M = vrMatrixToQt(m_trackedHandPoses[Hand::RIGHT].mDeviceToAbsoluteTracking);
            glm::vec3 pos = glm::vec3(M[3]);
            glm::vec3 d = pos - m_paintRight;
            float dist = glm::length(d);
            float yaw = atan2(d.x, d.z);
            float pitch = M_PI_2 - asin(d.y/glm::length(d));
            glm::mat4 r = glm::mat4_cast(glm::quat(glm::vec3(pitch, yaw, 0.0f)));
            glm::mat4 m = glm::translate((pos + m_paintRight)/2.0f) * r * glm::scale(glm::vec3(0.02, dist, 0.02));
            program->setUniform("M", m);
            m_cylinder->draw();
        }
    }

    program->unbind();
}

void View::drawDistortionObjects() {
    m_distortionStencilProgram->setTexture("normalMap", *m_shieldMap);

    glm::mat4 M = glm::translate(glm::vec3(0, 1.5, -1)) * glm::scale(glm::vec3(1, 1.4, 0.05));
    m_distortionStencilProgram->setUniform("M", M);
    m_cube->draw();
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

    if (event->key() == Qt::Key_P) m_useAdaptiveExposure = !m_useAdaptiveExposure;

    // VISA 120 Final
    if (event->key() == Qt::Key_T) m_mode = CREATE;
    if (event->key() == Qt::Key_Y) m_mode = PAINT;
}

void View::keyReleaseEvent(QKeyEvent *event) {
    if (!event->isAutoRepeat()) m_pressedKeys.erase(event->key());
}

void View::updatePoses() {
    vr::VRCompositor()->WaitGetPoses(m_trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

    if (m_trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid) {
        m_hmdPose = glm::inverse(vrMatrixToQt(m_trackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].mDeviceToAbsoluteTracking));
    }

    if (m_hmd->IsTrackedDeviceConnected(m_hmd->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand))) {
        m_trackedHandPoses[Hand::LEFT] = m_trackedDevicePose[m_hmd->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand)];
    }
    if (m_hmd->IsTrackedDeviceConnected(m_hmd->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand))) {
        m_trackedHandPoses[Hand::RIGHT] = m_trackedDevicePose[m_hmd->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand)];
    }
}

void View::updateInputs() {
    _buttonStates.clear();

    if (m_hmd->IsTrackedDeviceConnected(m_hmd->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand))) {
        vr::VRControllerState_t state;
        if (m_hmd->GetControllerState(m_hmd->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand), &state, sizeof(state))) {
            handleInput(state, true);
        }
    }

    if (m_hmd->IsTrackedDeviceConnected(m_hmd->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand))) {
        vr::VRControllerState_t state;
        if (m_hmd->GetControllerState(m_hmd->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand), &state, sizeof(state))) {
            handleInput(state, false);
        }
    }
}

void View::handleInput(const vr::VRControllerState_t &state, bool isLeftHand) {
    // Buttons
    for (uint32_t button = 0; button < vr::k_EButton_Max; button++) {
        auto mask = vr::ButtonMaskFromId((vr::EVRButtonId)button);
        bool pressed = 0 != (state.ulButtonPressed & mask);
        bool touched = 0 != (state.ulButtonTouched & mask);
        if (pressed) {
            if (button == vr::k_EButton_ApplicationMenu) {
                _buttonStates.insert(isLeftHand ? LEFT_MENU : RIGHT_MENU);
            } else if (button == vr::k_EButton_Grip) {
                _buttonStates.insert(isLeftHand ? LEFT_GRIP : RIGHT_GRIP);
            } else if (button == vr::k_EButton_SteamVR_Touchpad) {
                _buttonStates.insert(isLeftHand ? LEFT_TOUCHPAD : RIGHT_TOUCHPAD);
            }
        }
        if (touched) {
             if (button == vr::k_EButton_SteamVR_Touchpad) {
                 _buttonStates.insert(isLeftHand ? LEFT_TOUCHPAD_TOUCH : RIGHT_TOUCHPAD_TOUCH);
            }
        }
    }

    // Axes
    for (uint32_t i = 0; i < vr::k_unControllerStateAxisCount; i++) {
        float x = state.rAxis[i].x;
        float y = state.rAxis[i].y;
        int axis = i + vr::k_EButton_Axis0;

        if (axis == vr::k_EButton_SteamVR_Touchpad) {
            _axisStates[isLeftHand ? LEFT_X : RIGHT_X] = x;
            _axisStates[isLeftHand ? LEFT_Y : RIGHT_Y] = y;
        } else if (axis == vr::k_EButton_SteamVR_Trigger) {
            _axisStates[isLeftHand ? LEFT_TRIGGER : RIGHT_TRIGGER] = x;
        }
    }
}

Entity *View::findClosestObject(glm::vec3 &p) {
    Entity *res = nullptr;
    float minDist = INFINITY;

    // There should be a way to do this with Bullet?
    for (auto &e : m_world->getEntities()) {
        glm::mat4 m;
        e.getModelMatrix(m);
        glm::vec3 pos = m[3].xyz;
        float dist = glm::distance2(pos, p);
        if (dist < 0.01f && dist < minDist && e.m_rigidBody->getInvMass() != 0.0f) {
            minDist = dist;
            res = &e;
        }
    }
    return res;
}

void View::updateActions() {
    // Box spawning
    if (m_mode == CREATE) {
        if (_buttonStates.find(LEFT_TOUCHPAD) != _buttonStates.end()) {
            m_createTimeLeft += m_dt;
            m_prevLeftTouch = true;
        } else if (m_mode == CREATE && m_prevLeftTouch){
            CS123SceneMaterial mat;
            mat.cAmbient = glm::vec4(0.05, 0.05, 0.05, 1);
            mat.cDiffuse = glm::vec4(0.5, 0.5, 0.5, 1);
            mat.cSpecular = glm::vec4(0.7, 0.7, 0.7, 1);
            mat.shininess = 20.0f;
            mat.textureMap.isUsed = !m_currentTextureString.isEmpty();
            mat.textureMap.filename = m_currentTextureString.toStdString();

            glm::mat4 M = vrMatrixToQt(m_trackedHandPoses[Hand::LEFT].mDeviceToAbsoluteTracking);
            glm::vec3 pos = glm::vec3(M[3]);
            glm::quat rot = glm::quat_cast(glm::mat3(M * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0))));
            auto v = m_trackedHandPoses[Hand::LEFT].vVelocity.v;
            auto av = m_trackedHandPoses[Hand::LEFT].vAngularVelocity.v;

            float st = 4.0f;
            float sr = 12.0f;
            float s = m_createTimeLeft < M_PI/st ? glm::sin(m_createTimeLeft*st) +  m_createTimeLeft*st : glm::sin(m_createTimeLeft*st - M_PI_2)/2.0f + 2.64f;
            m_world->getEntities().emplace_back(m_world->getPhysWorld(),
                                                ShapeType::CUBE, 1.0f, btVector3(pos.x, pos.y, pos.z),
                                                btVector3(s/sr, s/sr, s/sr), mat, btQuaternion(rot.x, rot.y, rot.z, rot.w),
                                                btVector3(v[0], v[1], v[2]), btVector3(av[0], av[1], av[2]));
            m_createTimeLeft = 0.0f;
            m_prevLeftTouch = false;
        }

        if (_buttonStates.find(RIGHT_TOUCHPAD) != _buttonStates.end()) {
            m_createTimeRight += m_dt;
            m_prevRightTouch = true;
        } else if (m_mode == CREATE && m_prevRightTouch){
            CS123SceneMaterial mat;
            mat.cAmbient = glm::vec4(0.05, 0.05, 0.05, 1);
            mat.cDiffuse = glm::vec4(0.5, 0.5, 0.5, 1);
            mat.cSpecular = glm::vec4(0.7, 0.7, 0.7, 1);
            mat.shininess = 20.0f;
            mat.textureMap.isUsed = !m_currentTextureString.isEmpty();
            mat.textureMap.filename = m_currentTextureString.toStdString();

            glm::mat4 M = vrMatrixToQt(m_trackedHandPoses[Hand::RIGHT].mDeviceToAbsoluteTracking);
            glm::vec3 pos = glm::vec3(M[3]);
            glm::quat rot = glm::quat_cast(glm::mat3(M * glm::rotate(glm::radians(-90.0f), glm::vec3(1, 0, 0))));
            auto v = m_trackedHandPoses[Hand::RIGHT].vVelocity.v;
            auto av = m_trackedHandPoses[Hand::RIGHT].vAngularVelocity.v;

            float st = 4.0f;
            float sr = 12.0f;
            float s = m_createTimeRight < M_PI/st ? glm::sin(m_createTimeRight*st) +  m_createTimeRight*st : glm::sin(m_createTimeRight*st - M_PI_2)/2.0f + 2.64f;
            m_world->getEntities().emplace_back(m_world->getPhysWorld(),
                                                ShapeType::CUBE, 1.0f, btVector3(pos.x, pos.y, pos.z),
                                                btVector3(s/sr, s/sr, s/sr), mat, btQuaternion(rot.x, rot.y, rot.z, rot.w),
                                                btVector3(v[0], v[1], v[2]), btVector3(av[0], av[1], av[2]));
            m_createTimeRight = 0.0f;
            m_prevRightTouch = false;
        }
        if (_axisStates[RIGHT_TRIGGER] > 0.95f && !m_prevGrabbing) {
            // start grabbing
            m_prevGrabbing = true;
            glm::mat4 M = vrMatrixToQt(m_trackedHandPoses[Hand::RIGHT].mDeviceToAbsoluteTracking);
            glm::vec3 pos = glm::vec3(M[3]);
            m_grabbedEntity = findClosestObject(pos);
            if (m_grabbedEntity) m_world->getPhysWorld()->removeRigidBody(m_grabbedEntity->m_rigidBody.get());
        } else if (_axisStates[RIGHT_TRIGGER] > 0.95f && m_prevGrabbing && m_grabbedEntity) {
            // move object around
            btTransform t;
            t.setIdentity();
            glm::mat4 t2 = vrMatrixToQt(m_trackedHandPoses[Hand::RIGHT].mDeviceToAbsoluteTracking);
            t.setFromOpenGLMatrix((btScalar *) &t2);
            m_grabbedEntity->m_rigidBody->setWorldTransform(t);
            m_grabbedEntity->m_rigidBody->getMotionState()->setWorldTransform(t);
            if (m_trackedHandPoses[Hand::RIGHT].bPoseIsValid) {
                auto v = m_trackedHandPoses[Hand::RIGHT].vVelocity.v;
                auto av = m_trackedHandPoses[Hand::RIGHT].vAngularVelocity.v;
                m_grabbedEntity->m_rigidBody->setLinearVelocity(btVector3(v[0], v[1], v[2]));
                m_grabbedEntity->m_rigidBody->setAngularVelocity(btVector3(av[0], av[1], av[2]));
            }
        } else if (_axisStates[RIGHT_TRIGGER] <= 0.95f) {
            m_prevGrabbing = false;
            if (m_grabbedEntity) {
                // place object back in world (something about this is buggy?)
                m_world->getPhysWorld()->addRigidBody(m_grabbedEntity->m_rigidBody.get());
                m_grabbedEntity = nullptr;
            }
        }
    } else {
        // Painting
        if (_axisStates[LEFT_TRIGGER] > 0.95f) {
            glm::mat4 M = vrMatrixToQt(m_trackedHandPoses[Hand::LEFT].mDeviceToAbsoluteTracking);
            glm::vec3 pos = glm::vec3(M[3]);
            if (glm::isnan(m_paintLeft.x)) {
                m_paintLeft = pos;
            } else if (glm::distance(m_paintLeft, pos) > 0.1f) {
                m_world->addPaint(m_paintLeft, pos, m_paintColor);
                m_paintLeft = pos;
            }
        }

        if (_axisStates[RIGHT_TRIGGER] > 0.95f) {
            glm::mat4 M = vrMatrixToQt(m_trackedHandPoses[Hand::RIGHT].mDeviceToAbsoluteTracking);
            glm::vec3 pos = glm::vec3(M[3]);
            if (glm::isnan(m_paintRight.x)) {
                m_paintRight = pos;
            } else if (glm::distance(m_paintRight, pos) > 0.1f) {
                m_world->addPaint(m_paintRight, pos, m_paintColor);
                m_paintRight = pos;
            }
        }

        if (_buttonStates.find(LEFT_GRIP) != _buttonStates.end()) {
            m_paintLeft = glm::vec3(NAN);
        }
        if (_buttonStates.find(RIGHT_GRIP) != _buttonStates.end()) {
            m_paintRight = glm::vec3(NAN);
        }
    }
}

void View::tick() {
    // Get the number of seconds since the last tick (variable update rate)
    m_dt = m_time.restart() * 0.001f;
    if (m_dt != 0.0f) m_fps = 0.02f / m_dt + 0.98f * m_fps;

    m_globalTime += m_dt;

    // VR updates
    updatePoses();
    updateInputs();

    // Rocks
    updateActions();

    // voce
    while (voce::getRecognizerQueueSize() > 0) {
        std::string s = voce::popRecognizedString();

        if (s.rfind("exit") != std::string::npos) {
//            exit(0);
        } else if (s.rfind("create") != std::string::npos) {
            QString c = QString::fromStdString(s);
            c.replace("create ", "");
            QNetworkRequest request;
            request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
            request.setUrl(QUrl("https://www.google.com/search?tbm=isch&q=" + c + "+transparent"));
            m_networkManager->get(request);
            m_mode = CREATE;
        } else if (s.rfind("paint") != std::string::npos) {
            QString c = QString::fromStdString(s);
            c.replace("paint ", "");
            if (c == "red") {
                m_paintColor = glm::vec3(1, 0, 0);
            } else if (c == "green") {
                m_paintColor = glm::vec3(0, 1, 0);
            } else if (c == "blue") {
                m_paintColor = glm::vec3(0, 0, 1);
            } else if (c == "yellow") {
                m_paintColor = glm::vec3(1, 1, 0);
            } else if (c == "cyan") {
                m_paintColor = glm::vec3(0, 1, 1);
            } else if (c == "magenta") {
                m_paintColor = glm::vec3(1, 0, 1);
            } else if (c == "black") {
                m_paintColor = glm::vec3(0);
            } else if (c == "white") {
                m_paintColor = glm::vec3(1);
            }
            m_mode = PAINT;
        }

        std::cout << "You said: " << s << std::endl;
    }

    if (m_prevMode != m_mode) {
        if (m_mode == PAINT) {
            m_createTimeLeft = 0.0f;
            m_createTimeRight = 0.0f;
            m_prevLeftTouch = false;
            m_prevRightTouch = false;
            m_currentTextureString = "";
            m_prevGrabbing = false;
            if (m_grabbedEntity) {
                m_world->getPhysWorld()->addRigidBody(m_grabbedEntity->m_rigidBody.get());
                m_grabbedEntity = nullptr;
            }
        } else {
            m_paintLeft = glm::vec3(NAN);
            m_paintRight = glm::vec3(NAN);
        }
        m_prevMode = m_mode;
    }

    // Update world
    m_world->update(m_dt);

    // Flag this view for repainting (Qt will call paintGL() soon after)
    update();
}

void View::printFPS() {
    std::cout << m_fps << std::endl;
}

void View::getImageLinkFromURL(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        std::cout << "getImageLinkFromURL error: " << reply->errorString().toStdString() << std::endl;
        return;
    }

    QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    if (!contentType.toLower().contains("charset=utf-8")) {
        std::cout << "getImageLinkFromURL contentType error" << std::endl;
        return;
    }

    QString html = QString(reply->readAll());
    QStringList sp = html.split("https://encrypted-tbn");
    QString s = sp.at((rand() % (sp.size() - 1)) + 1);

    QNetworkRequest request;
    request.setSslConfiguration(QSslConfiguration::defaultConfiguration());
    request.setUrl(QUrl("https://encrypted-tbn" + s.split("\"")[0]));
    m_networkManagerImg->get(request);

    delete reply;
}

void View::getImageFromLink(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        std::cout << "getImageFromLink error" << std::endl;
        return;
    }

    QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    if (!contentType.toLower().contains("image")) {
        std::cout << "getImageFromLink contentType error" << std::endl;
        return;
    }

    QImage img = QImage::fromData(reply->readAll());
//    img.save("test.png");

    m_currentTextureString = reply->url().toString();
    addImage(reply->url().toString(), img);

    delete reply;
}
