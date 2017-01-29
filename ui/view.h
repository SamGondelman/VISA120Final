#ifndef VIEW_H
#define VIEW_H

#include "GL/glew.h"
#include <qgl.h>
#include <QTime>
#include <QTimer>
#include <memory>

#include "glm/glm.hpp"

class CS123Shader;
class FBO;
class SphereMesh;
class CubeMesh;
class FullScreenQuad;
class Light;
class Player;
class ParticleSystem;

enum DrawMode {
    POSITION = 0,
    NORMAL,
    AMBIENT,
    DIFFUSE,
    SPECULAR,
    LIGHTS,
    NO_HDR,
    BRIGHT,
    BRIGHT_BLUR,
    DEFAULT
};

enum World {
    WORLD_DEMO = 0,
    WORLD_1,
    WORLD_2
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
    float m_globalTime;

    std::unique_ptr<Player> m_player;
    std::shared_ptr<ParticleSystem> m_lightParticles;
    void drawCube();

    // Light World
    glm::vec3 m_lightOrigin;
    float m_lightTime;

    // OpenGL drawing stuff
    DrawMode m_drawMode;
    World m_world;

    std::shared_ptr<CS123Shader> m_deferredProgram;
    std::shared_ptr<CS123Shader> m_lightWorldProgram;
    std::shared_ptr<CS123Shader> m_waterWorldProgram;
    std::shared_ptr<FBO> m_deferredBuffer;

    std::unique_ptr<CS123Shader> m_lightingProgram;
    std::unique_ptr<FBO> m_lightingBuffer;

    GLuint m_fullscreenQuadVAO;
    std::unique_ptr<CS123Shader> m_brightProgram;

    // https://software.intel.com/en-us/blogs/2014/07/15/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms
    std::unique_ptr<CS123Shader> m_textureProgram;
    std::shared_ptr<CS123Shader> m_horizontalBlurProgram;
    std::shared_ptr<CS123Shader> m_verticalBlurProgram;
    std::shared_ptr<FBO> m_vblurBuffer;
    std::shared_ptr<FBO> m_hblurBuffer;

    std::unique_ptr<CS123Shader> m_bloomProgram;
    float m_exposure;
    bool m_useAdaptiveExposure;

    // Shapes
    std::unique_ptr<SphereMesh> m_sphere;
    std::unique_ptr<CubeMesh> m_cube;

    // Lights
    std::unique_ptr<SphereMesh> m_lightSphere;
    std::unique_ptr<FullScreenQuad> m_fullscreenQuad;

    std::vector<Light> m_lights;

    void drawGeometry(std::shared_ptr<CS123Shader> program, std::shared_ptr<FBO> deferredBuffer,
                      glm::mat4 &V, glm::mat4 &P);
    void worldUpdate(float dt);
    void setWorld();
    std::shared_ptr<CS123Shader> getWorldProgram();

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
