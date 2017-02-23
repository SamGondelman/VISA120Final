#ifndef CS123SHADER_H
#define CS123SHADER_H

#include "Shader.h"
#include "CS123SceneData.h"

class CS123SceneLightData;

class CS123Shader : public Shader {
public:
    CS123Shader(const std::string &vertexSource, const std::string &fragmentSource);
    CS123Shader(const std::string &vertexSource, const std::string &geometrySource, const std::string &fragmentSource);

    void applyMaterial(const CS123SceneMaterial &material) {
        setUniform("ambient_color", material.cAmbient.rgb);
        setUniform("diffuse_color", material.cDiffuse.rgb);
        setUniform("specular_color", material.cSpecular.rgb);
        setUniform("shininess", material.shininess);
    }
    void setLight(const CS123SceneLightData &light);
};

#endif // CS123SHADER_H
