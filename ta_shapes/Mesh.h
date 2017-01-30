#ifndef __MESH_H__
#define __MESH_H__

struct Triangle {
    unsigned int a_vertex, b_vertex, c_vertex;
    unsigned int a_normal, b_normal, c_normal;
    unsigned int a_tex, b_tex, c_tex;

    Triangle(unsigned int _a = 0, unsigned int _b = 0, unsigned int _c = 0) :
        a_vertex(_a), b_vertex(_b), c_vertex(_c),
        a_normal(_a), b_normal(_b), c_normal(_c),
        a_tex(_a), b_tex(_b), c_tex(_c) {}
    Triangle(unsigned int _a_vertex, unsigned int _a_normal, unsigned int _b_vertex,
        unsigned int _b_normal, unsigned int _c_vertex, unsigned int _c_normal,
        unsigned int _a_tex = 0, unsigned int _b_tex = 0, unsigned int _c_tex = 0) :
        a_vertex(_a_vertex), b_vertex(_b_vertex), c_vertex(_c_vertex),
        a_normal(_a_normal), b_normal(_b_normal), c_normal(_c_normal),
        a_tex(_a_tex), b_tex(_b_tex), c_tex(_c_tex) {}
};

class Mesh {
public:
    Mesh();
    virtual ~Mesh();

    virtual void draw(int num = 1) = 0;

protected:

    float m_repeatU;
    float m_repeatV;

private:

};

#endif
