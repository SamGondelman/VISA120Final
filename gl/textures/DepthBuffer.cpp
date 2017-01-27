#include "DepthBuffer.h"

#include "GL/glew.h"

DepthBuffer::DepthBuffer(int width, int height) :
    m_width(width),
    m_height(height)
{
    // TODO [Task 8] bind() the render buffer and call glRenderbufferStorageEXT
    // begin ta code
    bind();
    glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    // end ta code

    // TODO Don't forget to unbind()!
    // begin ta code
    unbind();
    // end ta code
}
