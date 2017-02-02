#ifndef __CUBE_MESH_H__
#define __CUBE_MESH_H__

#include "Mesh.h"
#include "ParameterizedMesh.h"

class CubeMesh : public Mesh {
public:
    CubeMesh(int squaresPerEdge, float repeatU = 1, float repeatV = 1);
    virtual ~CubeMesh() override;

    virtual void draw(int num = 1) override;

private:
    ParameterizedMesh m_top;
    ParameterizedMesh m_bottom;
    ParameterizedMesh m_left;
    ParameterizedMesh m_right;
    ParameterizedMesh m_front;
    ParameterizedMesh m_back;
};

#endif
