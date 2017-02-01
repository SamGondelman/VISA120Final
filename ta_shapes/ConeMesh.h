#ifndef __CONE_MESH_H__
#define __CONE_MESH_H__

#include "ParameterizedMesh.h"

class ConeMesh : public Mesh {
public:
    ConeMesh(int stacks, int slices, float repeatU = 1, float repeatV = 1);
    virtual ~ConeMesh() override;

    virtual void draw(int num = 1) override;

private:
    ParameterizedMesh m_body;
    ParameterizedMesh m_tip;
    ParameterizedMesh m_bottomCap;

};

#endif
