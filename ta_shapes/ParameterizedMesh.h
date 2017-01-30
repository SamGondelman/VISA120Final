#ifndef __PARAMETERIZED_MESH_H__
#define __PARAMETERIZED_MESH_H__

#include "Mesh.h"

#include <memory>
#include <vector>

#include "glm/glm.hpp"

class VAO;

class ParameterizedMesh : public Mesh {
public:
    /**
     * This templated class takes a special argument called a functor
     * Functors are regular C++ classes that have operator() implemented
     * Functors are "function objects" because you can call them like functions,
     * but they're actually objects. Their major benefit over function pointers
     * it that they can hold state, which you can use to modify the functor's behavior
     *
     * Func f's operator() signature must look like:
     * void operator()(float s, float t, Vector *position, Vector *normal, Vector* texture)
     *
     * Template functions must be defined in the header file in order for the class
     * to compile. This often looks ugly, so the solution is to place the template
     * functions in a separate file, but then include it at the bottom of the header file.
     * This fulfills the template compilation requirements while leaving the header
     * looking nice and clean.
     *
     * sCount and tCount are the number of patches to subdivide the surface into
     * the surface is drawn using all squares from sStart to sStop and tStart to tStop
     * where 0 <= sStart < sStop <= sCount and 0 <= tStart < tStop <= tCount
     */
    template<class Func>
    ParameterizedMesh(Func f,
                      unsigned int sCount, unsigned int tCount,
                      unsigned int sStart, unsigned int tStart,
                      unsigned int sStop, unsigned int tStop,
                      bool doubleTriangles = true);
    virtual ~ParameterizedMesh();

    virtual void draw(int num = 1) override;
    void drawNormals(void (*drawNormal)(const glm::vec4 &, const glm::vec4 &));

private:
    void insertVector3(float *data, int &index, std::vector<glm::vec3> &vectorList, unsigned int vectorIndex);
    void insertVector2(float *data, int &index, std::vector<glm::vec2> &vectorList, unsigned int vectorIndex);

    void addTriangles(float *vertexData);
    void addTriangle(float *vertexData, int &vertexIndex, const Triangle &triangle);

    void buildVAO();


    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoords;
    std::vector<Triangle> triangles;
    std::unique_ptr<VAO> m_VAO;
};

#include "ParameterizedMesh.tpp"

#endif
