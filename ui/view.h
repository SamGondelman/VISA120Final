#ifndef VIEW_H
#define VIEW_H

#include "GL/glew.h"
#include <qgl.h>
#include <QTime>
#include <QTimer>
#include <memory>

class CS123Shader;
class FBO;
class SphereMesh;
class PointLight;

enum DrawMode {
    POSITION = 0,
    NORMAL,
    AMBIENT,
    DIFFUSE,
    SPECULAR,
    LIGHTS,
    DEFAULT
};

class View : public QGLWidget {
    Q_OBJECT

public:
    View(QWidget *parent);
    ~View();

private:
    int m_width;
    int m_height;

    QTime m_time;
    QTimer m_timer;
    float m_totalTime;

    // OpenGL drawing stuff
    DrawMode m_drawMode;

    std::unique_ptr<CS123Shader> m_deferredProgram;
    std::unique_ptr<FBO> m_deferredBuffer;

    GLuint m_fullscreenQuadVAO;
    std::unique_ptr<CS123Shader> m_fullscreenQuadProgram;

    // Shapes
    std::unique_ptr<SphereMesh> m_sphere;
    std::unique_ptr<SphereMesh> m_lightSphere;
    std::vector<PointLight> m_lights;

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private slots:
    void tick();
};

#endif // VIEW_H
