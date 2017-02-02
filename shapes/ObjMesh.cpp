
#include <QMessageBox>

#include <vector>
#include <memory>

#include "gl/datatype/VAO.h"
#include "ObjMesh.h"
#include "gl/datatype/VBO.h"
#include "gl/datatype/VBOAttribMarker.h"
#include "gl/shaders/ShaderAttribLocations.h"

#include "glm.cpp"

///
///
/// THIS CLASS IS THE BROKEN
/// REPLACE THIS GLM CRAPOLA WITH ASSIMP
/// BE A MAN
/// DO THE RIGHT THING
///
///

using namespace CS123::GL;

#define invalid(indices, min, max) (indices[0] < min || indices [1] < min || indices[2] < min || \
    indices[0] > max || indices[1] > max || indices[2] > max)

ObjMesh::ObjMesh(const char *file)
{
    m_matrix = glm::mat4(1.0f);
    GLMmodel *model = glmReadOBJ(file);

    if (!model) {
        QMessageBox::critical(nullptr, "Error", "Could not load obj file \"" + QString(file) + "\"");
        return;
    }

    glmUnitize(model);

    for (unsigned int i = 3; i < model->numvertices*3 + 3; i+=3) {
        GLfloat* vertex = &model->vertices[i];
        vertices.push_back(glm::vec3(vertex[0], vertex[1], vertex[2]));
    }
    for (unsigned int i = 3; i < model->numnormals*3 + 3; i+=3) {
        GLfloat* normal = &model->normals[i];
        normals.push_back(glm::vec3(normal[0], normal[1], normal[2]));
    }
    if (model->numnormals == 0) {
        normals.push_back(glm::vec3(0, 0, 0));
    }

    for (unsigned int i = 2; i < model->numtexcoords*2 + 2; i+=2) {
        GLfloat* texcoord = &model->texcoords[i];
        texcoords.push_back(glm::vec2(texcoord[0], texcoord[1]));
    }
    if (model->numtexcoords == 0) {
        texcoords.push_back(glm::vec2(0, 0));
    }

    for (unsigned int i = 0; i < model->numtriangles; i++) {
        GLMtriangle *triangle = &model->triangles[i];
        GLuint *vindices = triangle->vindices;
        GLuint *nindices = triangle->nindices;
        GLuint ones[] = { 1, 1, 1};
        if (invalid(nindices, 1, normals.size())) {
            nindices = ones;
        }
        GLuint *tindices = triangle->tindices;
        if (invalid(tindices, 1, texcoords.size())) {
            tindices = ones;
        }
        triangles.push_back(Triangle(vindices[0]-1, nindices[0]-1, vindices[1]-1, nindices[1]-1,
                                     vindices[2]-1, nindices[2]-1, tindices[0]-1, tindices[1]-1, tindices[2]-1));
    }
    buildVAO();
    glmDelete(model);
}

ObjMesh::~ObjMesh()
{
}

void ObjMesh::buildVAO() {
    int numVertices = triangles.size() * 3;
    const int numberOfFloats = 8 * numVertices;
    GLfloat* vertexData = new GLfloat[numberOfFloats];
    addTriangles(vertexData);

    std::vector<VBOAttribMarker> markers;
    markers.push_back(VBOAttribMarker(ShaderAttrib::POSITION, 3, 0));
    markers.push_back(VBOAttribMarker(ShaderAttrib::NORMAL, 3, 3*sizeof(float)));
    markers.push_back(VBOAttribMarker(ShaderAttrib::TEXCOORD0, 2, (3+3)*sizeof(float)));

    CS123::GL::VBO vbo = VBO(vertexData, numberOfFloats, markers);
    m_VAO = std::make_unique<VAO>(vbo, numVertices);
}

void ObjMesh::addTriangles(GLfloat *vertexData) {
    int vertexIndex = 0;
    for (unsigned int i = 0; i < triangles.size(); i++)
    {
        Triangle &triangle = triangles[i];
        addTriangle(vertexData, vertexIndex, triangle);
    }
}

void ObjMesh::addTriangle(float *vertexData, int &vertexIndex, const Triangle &triangle) {
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

void ObjMesh::insertVector2(float *data, int &index, std::vector<glm::vec2> &vectorList, unsigned int vectorIndex) {
    glm::vec2 vector;
    if (vectorIndex >= vectorList.size()) {
        vector = glm::vec2();
    } else {
        vector = vectorList[vectorIndex];
    }
    data[index++] = vector.x;
    data[index++] = vector.y;
}

//Inserts the given glm::vec3 into the given array, and increments the index accordingly.
void ObjMesh::insertVector3(float *data, int &index, std::vector<glm::vec3> &vectorList, unsigned int vectorIndex) {
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

void ObjMesh::draw() {
    m_VAO->bind();
    m_VAO->draw();
    m_VAO->unbind();
}

//void ObjMesh::transformByMatrix(const Matrix4x4 &matrix) {
//    m_matrix *= matrix;
//}

//void ObjMesh::draw() {
//    // Save and restore the material properties so we don't mess up the next scene (and the rest of this one)
//    float oldAmbient[4];
//    float oldDiffuse[4];
//    float oldSpecular[4];
//    float oldEmission[4];
//    float oldShininess;

//    glGetMaterialfv(GL_FRONT, GL_AMBIENT, oldAmbient);
//    glGetMaterialfv(GL_FRONT, GL_DIFFUSE, oldDiffuse);
//    glGetMaterialfv(GL_FRONT, GL_SPECULAR, oldSpecular);
//    glGetMaterialfv(GL_FRONT, GL_EMISSION, oldEmission);
//    glGetMaterialfv(GL_FRONT, GL_SHININESS, &oldShininess);

//    // Draw the model with its transformation matrix (need GL_NORMALIZE to normalize the model normals in case m_matrix has a scale transformation)
//    Matrix4x4 bufferM = m_matrix;
//    double *buffer = bufferM.getTranspose().data;
//    glPushMatrix();
//    glMultMatrixd(buffer);
//    glEnable(GL_NORMALIZE);
//    glmDraw(m_model, GLM_SMOOTH | GLM_MATERIAL);
//    glDisable(GL_NORMALIZE);
//    glPopMatrix();

//    // cout << "oldAmbient = " << oldAmbient[0] << ", " << oldAmbient[1] << ", " << oldAmbient[2] << ", " << oldAmbient[3] << endl;
//    // cout << "oldDiffuse = " << oldDiffuse[0] << ", " << oldDiffuse[1] << ", " << oldDiffuse[2] << ", " << oldDiffuse[3] << endl;
//    // cout << "oldSpecular = " << oldSpecular[0] << ", " << oldSpecular[1] << ", " << oldSpecular[2] << ", " << oldSpecular[3] << endl;
//    // cout << "oldEmission = " << oldEmission[0] << ", " << oldEmission[1] << ", " << oldEmission[2] << ", " << oldEmission[3] << endl;
//    // cout << "oldShininess = " << oldShininess << endl;

//    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, oldAmbient);
//    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, oldDiffuse);
//    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, oldSpecular);
//    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, oldEmission);
//    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, oldShininess);
//}
