#ifndef __SPHERE_MESH_H__
#define __SPHERE_MESH_H__

#include "ParameterizedMesh.h"

class SphereMesh : public ParameterizedMesh {
public:
    SphereMesh(int slices, int stacks, float repeatU = 1, float repeatV = 1);
    virtual ~SphereMesh() override;
};

#endif
