#include "Mesh.h"

#include "gl/datatype/VAO.h"
#include "gl/datatype/VBO.h"
#include "gl/datatype/VBOAttribMarker.h"

using namespace CS123::GL;

Mesh::Mesh() :
    m_repeatU(1),
    m_repeatV(1)
{
}

Mesh::~Mesh()
{
}
