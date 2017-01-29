#include "ParticleSystem.h"

#include <gl/datatype/FBO.h>
#include "gl/shaders/CS123Shader.h"
#include "ResourceLoader.h"

ParticleSystem::ParticleSystem(int numParticles, std::string drawFrag, std::string drawVert,
                               std::string updateFrag, std::string updateVert, int numColorAttachments) :
    m_FBO1(nullptr), m_FBO2(nullptr),
    m_firstPass(true), m_evenPass(true), m_numParticles(numParticles)
{
    std::string vertexSource = ResourceLoader::loadResourceFileToString(drawVert);
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(drawFrag);
    m_drawProgram = std::make_unique<CS123Shader>(vertexSource, fragmentSource);

    vertexSource = ResourceLoader::loadResourceFileToString(updateVert);
    fragmentSource = ResourceLoader::loadResourceFileToString(updateFrag);
    m_updateProgram = std::make_unique<CS123Shader>(vertexSource, fragmentSource);

    m_FBO1 = std::make_shared<FBO>(numColorAttachments, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, m_numParticles, 1,
                                   TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                   TextureParameters::FILTER_METHOD::NEAREST, GL_FLOAT);
    m_FBO2 = std::make_shared<FBO>(numColorAttachments, FBO::DEPTH_STENCIL_ATTACHMENT::NONE, m_numParticles, 1,
                                   TextureParameters::WRAP_METHOD::CLAMP_TO_EDGE,
                                   TextureParameters::FILTER_METHOD::NEAREST, GL_FLOAT);

    glGenVertexArrays(1, &m_emptyVAO);
}

ParticleSystem::~ParticleSystem() {
    glDeleteVertexArrays(1, &m_emptyVAO);
}

void ParticleSystem::updateAndDraw(float dt, int width, int height, glm::mat4& V, glm::mat4& P,
                                   std::shared_ptr<FBO> deferredBuffer) {
    auto prevFBO = m_evenPass ? m_FBO1 : m_FBO2;
    auto nextFBO = !m_evenPass ? m_FBO1 : m_FBO2;
    float firstPass = m_firstPass ? 1.0f : 0.0f;

    // Update
    m_updateProgram->bind();
    nextFBO->bind();

    // Setup update uniforms
    m_updateProgram->setUniform("firstPass", firstPass);
    m_updateProgram->setUniform("numParticles", m_numParticles);
    m_updateProgram->setTexture("prevPos", prevFBO->getColorAttachment(0));
    m_updateProgram->setTexture("prevVel", prevFBO->getColorAttachment(1));

    // Draw fullscreen quad
    glBindVertexArray(m_emptyVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    nextFBO->unbind();
    m_updateProgram->unbind();

    // Draw particles to deferred buffer
    m_drawProgram->bind();
    deferredBuffer->bind();
    glViewport(0, 0, width, height);

    // Setup draw uniforms
    m_drawProgram->setTexture("pos", nextFBO->getColorAttachment(0));
    m_drawProgram->setTexture("vel", nextFBO->getColorAttachment(1));
    m_drawProgram->setUniform("numParticles", m_numParticles);
    m_drawProgram->setUniform("V", V);
    m_drawProgram->setUniform("P", P);

    // Draw one triangle per particle
    glBindVertexArray(m_emptyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3 * m_numParticles);
    glBindVertexArray(0);

    m_firstPass = false;
    m_evenPass = !m_evenPass;
}
