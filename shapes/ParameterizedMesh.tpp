//#include "ParameterizedMesh.h"

template<class Func>
ParameterizedMesh::ParameterizedMesh(Func vertexGeneratingFunctor,
                   unsigned int sCount, unsigned int tCount,
                   unsigned int sStart, unsigned int tStart,
                   unsigned int sStop, unsigned int tStop,
                   bool doubleTriangles)
{
    unsigned int s, t, index = vertices.size();
    float sPercent, tPercent;

    for (s = sStart, sPercent = static_cast<float>(s) / sCount; s <= sStop; s++, sPercent = static_cast<float>(s) / static_cast<float>(sCount)) {
        for (t = tStart, tPercent = static_cast<float>(t) / tCount; t <= tStop; t++, tPercent = static_cast<float>(t) / static_cast<float>(tCount)) {
            glm::vec3 vertex, normal;
            glm::vec2 texture;
            vertexGeneratingFunctor(sPercent, tPercent, vertex, normal, texture);
            vertices.push_back(vertex);
            normals.push_back(normal);
            texcoords.push_back(texture * glm::vec2(m_repeatU, m_repeatV));
        }
    }

    // only index across the vertices we generate above
    sCount = sStop - sStart;
    tCount = tStop - tStart;

    for (s = 0; s < sCount; s++, index++) {
        for (t = 0; t < tCount; t++, index++) {
            if (doubleTriangles) triangles.push_back(Triangle(index, index + 1, index + (tCount + 1)));
            triangles.push_back(Triangle(index + (tCount + 1), index + 1, index + (tCount + 1) + 1));
        }
    }

    buildVAO();
}
