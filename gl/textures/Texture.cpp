#include "Texture.h"

#include <cassert>
#include <utility>

#include <GL/glew.h>
#include "gl/GLDebug.h"

Texture::Texture() :
    m_handle(0)
{
    // TODO [Task 2] Generate the texture
    // begin ta code
    glGenTextures(1, &m_handle);
    // end ta code
}

Texture::Texture(Texture &&that) :
    m_handle(that.m_handle)
{
    that.m_handle = 0;
}

Texture& Texture::operator=(Texture &&that) {
    this->~Texture();
    m_handle = that.m_handle;
    that.m_handle = 0;
    return *this;
}

Texture::~Texture()
{
    // TODO Don't forget to delete!
    // begin ta code
    glDeleteTextures(1, &m_handle);
    // end ta code
}

unsigned int Texture::id() const {
    return m_handle;
}
