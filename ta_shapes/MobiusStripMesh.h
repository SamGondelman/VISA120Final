#ifndef __MOBIUS_STRIP_MESH_H__
#define __MOBIUS_STRIP_MESH_H__

#include "ParameterizedMesh.h"

class MobiusStripMesh : public ParameterizedMesh {
public:
    MobiusStripMesh(int stacks, int slices);
    virtual ~MobiusStripMesh() override;
};

#endif
