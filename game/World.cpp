#include "World.h"

#include "ResourceLoader.h"
#include "gl/shaders/CS123Shader.h"

World::World(std::string vert, std::string frag)
{
    std::string vertexSource = ResourceLoader::loadResourceFileToString(vert);
    std::string fragmentSource = ResourceLoader::loadResourceFileToString(frag);
    m_program = std::make_shared<CS123Shader>(vertexSource, fragmentSource);
}

