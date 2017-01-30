#include "ParameterizedMesh.h"

#include "gl/datatype/VAO.h"
#include "gl/datatype/VBO.h"
#include "gl/datatype/VBOAttribMarker.h"
#include "gl/shaders/ShaderAttribLocations.h"

ParameterizedMesh::~ParameterizedMesh()
{
}

void ParameterizedMesh::buildVAO() {
    int numVertices = triangles.size() * 3;

    const int numberOfFloats = 8 * numVertices;
    std::vector<GLfloat> vertexData;
    vertexData.reserve(numberOfFloats);
    addTriangles(&vertexData[0]);

    std::vector<VBOAttribMarker> markers;
    markers.push_back(VBOAttribMarker(ShaderAttrib::POSITION, 3, 0));
    markers.push_back(VBOAttribMarker(ShaderAttrib::NORMAL, 3, 3*sizeof(float)));
    markers.push_back(VBOAttribMarker(ShaderAttrib::TEXCOORD0, 2, (3+3)*sizeof(float)));

    VBO vbo = VBO(&vertexData[0], numberOfFloats, markers);
    m_VAO = std::make_unique<VAO>(vbo, numVertices);

}

void ParameterizedMesh::addTriangles(float *vertexData) {
    int vertexIndex = 0;
    for (unsigned int i = 0; i < triangles.size(); i++) {
        Triangle &triangle = triangles[i];
        addTriangle(vertexData, vertexIndex, triangle);
    }
}

void ParameterizedMesh::addTriangle(float *vertexData, int &vertexIndex, const Triangle &triangle) {
    insertVector3(vertexData, vertexIndex, vertices, triangle.a_vertex);
    insertVector3(vertexData, vertexIndex, normals, triangle.a_normal);
    insertVector2(vertexData, vertexIndex, texcoords, triangle.a_tex);

    insertVector3(vertexData, vertexIndex, vertices, triangle.b_vertex);
    insertVector3(vertexData, vertexIndex, normals, triangle.b_normal);
    insertVector2(vertexData, vertexIndex, texcoords, triangle.b_tex);

    insertVector3(vertexData, vertexIndex, vertices, triangle.c_vertex);
    insertVector3(vertexData, vertexIndex, normals, triangle.c_normal);
    insertVector2(vertexData, vertexIndex, texcoords, triangle.c_tex);
}

void ParameterizedMesh::insertVector2(float *data, int &index, std::vector<glm::vec2> &vectorList, unsigned int vectorIndex) {
    glm::vec2 vector;
    if (vectorIndex >= vectorList.size()) {
        vector = glm::vec2();
    } else {
        vector = vectorList[vectorIndex];
    }
    data[index++] = vector.x;
    data[index++] = vector.y;
}

void ParameterizedMesh::insertVector3(float *data, int &index, std::vector<glm::vec3> &vectorList, unsigned int vectorIndex) {
    glm::vec3 vector;
    if (vectorIndex >= vectorList.size()) {
        vector = glm::vec3();
    } else {
        vector = vectorList[vectorIndex];
    }
    data[index++] = vector.x;
    data[index++] = vector.y;
    data[index++] = vector.z;
}

void ParameterizedMesh::draw(int num) {
    m_VAO->bind();
    m_VAO->draw(num);
    m_VAO->unbind();
}
