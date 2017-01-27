#ifndef __OBJ_MESH_H__
#define __OBJ_MESH_H__

#include "Mesh.h"

#include "glm.h"
#include "glm/glm.hpp"

namespace CS123 { namespace GL {

class VAO;

}}

#include <memory>
#include <vector>

class ObjMesh : public Mesh {
public:
    ObjMesh(const char *file);
    virtual ~ObjMesh();

    //virtual void transformByMatrix(const Matrix4x4 &matrix);
    virtual void draw() override;

private:
    void buildVAO();
    void addTriangles(GLfloat *vertexData);
    void addTriangle(float *vertexData, int &vertexIndex, const Triangle &triangle);
    void insertVector3(float *data, int &index, std::vector<glm::vec3> &vectorList, unsigned int vectorIndex);
    void insertVector2(float *data, int &index, std::vector<glm::vec2> &vectorList, unsigned int vectorIndex);

    glm::mat4 m_matrix;

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<Triangle> triangles;
    std::unique_ptr<CS123::GL::VAO> m_VAO;
    GLuint m_geometryVAO;
    GLuint m_vbo;
};

#endif
