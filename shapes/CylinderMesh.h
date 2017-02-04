#ifndef __CYLINDER_MESH_H__
#define __CYLINDER_MESH_H__

#include "Mesh.h"
#include "ParameterizedMesh.h"

//ParameterizedMesh
class CylinderMesh : public Mesh {
public:
    CylinderMesh(int stacks, int slices, float repeatU = 1, float repeatV = 1);
    virtual ~CylinderMesh() override;

    virtual void draw(int num = 1) override;

private:
    ParameterizedMesh m_topCap;
    ParameterizedMesh m_barrel;
    ParameterizedMesh m_bottomCap;
};

#endif
