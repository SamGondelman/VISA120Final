#ifndef VIEW_H
#define VIEW_H

#include "GL/glew.h"
#include <qgl.h>
#include <QTime>
#include <QTimer>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <QMutex>

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <glm/gtx/norm.hpp>

#include <openvr.h>

#include <CS123SceneData.h>
#include <LinearMath/btVector3.h>

class CS123Shader;
class FBO;
class SphereMesh;
class CubeMesh;
class ConeMesh;
class CylinderMesh;
class FullScreenQuad;
struct Light;
class Player;
class Texture2D;
class PhysicsWorld;
class QNetworkAccessManager;
class QNetworkReply;

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
    NO_DISTORTION,
    DEFAULT
};

enum Hand {
    LEFT = 0,
    RIGHT
};

enum ViveButton {
    LEFT_MENU = 0,
    LEFT_GRIP,
    LEFT_TOUCHPAD,
    LEFT_TOUCHPAD_TOUCH,

    RIGHT_MENU,
    RIGHT_GRIP,
    RIGHT_TOUCHPAD,
    RIGHT_TOUCHPAD_TOUCH,
};

enum ViveAxis {
    LEFT_X = 0,
    LEFT_Y,
    LEFT_TRIGGER,

    RIGHT_X,
    RIGHT_Y,
    RIGHT_TRIGGER,
};

enum Mode {
    CREATE = 0,
    PAINT
};

struct GrabbedEntity {
    btVector3 p;
    btVector3 s;
    glm::quat r;
    CS123SceneMaterial mat;
    btVector3 v;
    btVector3 av;
} typedef GrabbedEntity;

class View : public QGLWidget {
    Q_OBJECT

public:
    View(QWidget *parent);
    ~View();

    static float m_globalTime;
    static std::unique_ptr<Player> m_player;

    // Shapes
    static std::unique_ptr<SphereMesh> m_sphere;
    static std::unique_ptr<CubeMesh> m_cube;
    static std::unique_ptr<ConeMesh> m_cone;
    static std::unique_ptr<CylinderMesh> m_cylinder;
    static GLuint m_fullscreenQuadVAO;

    static std::unordered_set<int> m_pressedKeys;

    static std::unordered_map<std::string, Texture2D> m_textureMap;
    static QMutex m_textureMutex;
    QString m_currentTextureString { "" };

    static glm::vec3 m_viewDir;

private:
    QTime m_time;
    QTimer m_timer;

    float m_dt;
    float m_fps;
    QTimer m_FPStimer;

    std::unique_ptr<PhysicsWorld> m_world;

    Mode m_mode { CREATE };
    Mode m_prevMode { CREATE };
    bool m_prevRightTouch { false };
    bool m_prevLeftTouch { false };
    bool m_prevGrabbing { false };
    GrabbedEntity m_grabbedEntity;
    bool m_didGrab { false };
    bool m_prevScaling { false };
    glm::vec3 m_scalingVec;
    btVector3 m_objectScale;
    glm::vec3 m_paintLeft { glm::vec3(NAN) };
    glm::vec3 m_paintRight { glm::vec3(NAN) };
    glm::vec3 m_paintColor { glm::vec3(1.0f) };

    // Element effects
    float m_createTimeLeft;
    float m_createTimeRight;

    // Game state
    DrawMode m_drawMode;

    // FBOs and shaders
    std::unique_ptr<FBO> m_deferredBuffer;

    std::unique_ptr<FBO> m_lightingBuffer;
    std::unique_ptr<CS123Shader> m_lightingProgram;

    std::unique_ptr<FBO> m_distortionBuffer;
    std::unique_ptr<CS123Shader> m_distortionStencilProgram;
    std::unique_ptr<CS123Shader> m_distortionProgram;
    std::unique_ptr<Texture2D> m_shieldMap;

    std::unique_ptr<CS123Shader> m_textureProgram;

    std::unique_ptr<CS123Shader> m_brightProgram;

    // https://software.intel.com/en-us/blogs/2014/07/15/an-investigation-of-fast-real-time-gpu-based-image-blur-algorithms
    std::shared_ptr<FBO> m_vblurBuffer;
    std::shared_ptr<FBO> m_hblurBuffer;
    std::shared_ptr<CS123Shader> m_horizontalBlurProgram;
    std::shared_ptr<CS123Shader> m_verticalBlurProgram;

    std::unique_ptr<CS123Shader> m_bloomProgram;
    float m_exposure;
    bool m_useAdaptiveExposure;

    // Lights
    std::unique_ptr<SphereMesh> m_lightSphere;
    std::unique_ptr<FullScreenQuad> m_fullscreenQuad;

    void drawHands(glm::mat4 &V, glm::mat4 &P);
    void drawAction(glm::mat4& V, glm::mat4& P);
    void drawDistortionObjects();

    void initializeGL();
    void initVR();
    void handleInput(const vr::VRControllerState_t &state, bool isLeftHand);
    void paintGL();
    void renderEye(vr::EVREye eye);
    void resizeGL(int w, int h);

    void updatePoses();
    void updateInputs();
    void updateActions();
    int findClosestObject(glm::vec3 &p);
    void placeGrabbedObject();

    void addImage(QString &s, QImage &img);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    // VR stuff
    vr::IVRSystem *m_hmd;
    uint32_t m_eyeWidth, m_eyeHeight;
    std::shared_ptr<FBO> m_leftEyeBuffer;
    std::shared_ptr<FBO> m_rightEyeBuffer;

    vr::TrackedDevicePose_t m_trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
    vr::TrackedDevicePose_t m_trackedHandPoses[2];

    glm::mat4 m_leftProjection, m_leftPose;
    glm::mat4 m_rightProjection, m_rightPose;
    glm::mat4 m_hmdPose;

    std::unordered_set<ViveButton> _buttonStates;
    std::unordered_map<ViveAxis, float> _axisStates;

    glm::mat4 vrMatrixToQt(const vr::HmdMatrix34_t &mat) {
        return glm::mat4x4(
            mat.m[0][0], mat.m[1][0], mat.m[2][0], 0.0f,
            mat.m[0][1], mat.m[1][1], mat.m[2][1], 0.0f,
            mat.m[0][2], mat.m[1][2], mat.m[2][2], 0.0f,
            mat.m[0][3], mat.m[1][3], mat.m[2][3], 1.0f
        );
    }

    glm::mat4 vrMatrixToQt(const vr::HmdMatrix44_t &mat) {
        return glm::mat4x4(
            mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
            mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
            mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
            mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
        );
    }

    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    std::unique_ptr<QNetworkAccessManager> m_networkManagerImg;

private slots:
    void tick();
    void printFPS();
    void getImageLinkFromURL(QNetworkReply *reply);
    void getImageFromLink(QNetworkReply *reply);
};

#endif // VIEW_H
