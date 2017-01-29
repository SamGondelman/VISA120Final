# -------------------------------------------------
# Project created by QtCreator 2010-08-22T14:12:19
# -------------------------------------------------
QT += opengl xml
TARGET = manus
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++14
CONFIG += c++14

win32 {
    DEFINES += GLEW_STATIC
    LIBS += -lopengl32 -lglu32
}

SOURCES += \
    camera/Player.cpp \
    gl/GLDebug.cpp \
    gl/datatype/VBOAttribMarker.cpp \
    gl/datatype/VBO.cpp \
    gl/datatype/IBO.cpp \
    gl/datatype/VAO.cpp \
    gl/datatype/FBO.cpp \
    gl/textures/Texture.cpp \
    gl/textures/Texture2D.cpp \
    gl/textures/TextureParameters.cpp \
    gl/textures/TextureParametersBuilder.cpp \
    gl/textures/RenderBuffer.cpp \
    gl/textures/DepthBuffer.cpp \
    gl/shaders/Shader.cpp \
    gl/shaders/CS123Shader.cpp \
    lib/ResourceLoader.cpp \
    particles/ParticleSystem.cpp \
    ta_shapes/FullScreenQuad.cpp \
    ta_shapes/SphereMesh.cpp \
    ta_shapes/CubeMesh.cpp \
    ta_shapes/ParameterizedMesh.tpp \
    ta_shapes/ParameterizedMesh.cpp \
    ta_shapes/Mesh.cpp \
    ui/view.cpp \
    ui/viewformat.cpp \
    ui/mainwindow.cpp \
    glew-1.10.0/src/glew.c \
    main.cpp

HEADERS += \
    camera/Player.h \
    gl/GLDebug.h \
    gl/datatype/VBOAttribMarker.h \
    gl/datatype/VBO.h \
    gl/datatype/IBO.h \
    gl/datatype/VAO.h \
    gl/datatype/FBO.h \
    gl/textures/Texture.h \
    gl/textures/Texture2D.h \
    gl/textures/TextureParameters.h \
    gl/textures/TextureParametersBuilder.h \
    gl/textures/RenderBuffer.h \
    gl/textures/DepthBuffer.h \
    gl/shaders/Shader.h \
    gl/shaders/ShaderAttribLocations.h \
    gl/shaders/CS123Shader.h \
    lib/ResourceLoader.h \
    lib/CS123SceneData.h \
    particles/ParticleSystem.h \
    ta_shapes/FullScreenQuad.h \
    ta_shapes/SphereMesh.h \
    ta_shapes/CubeMesh.h \
    ta_shapes/ParameterizedMesh.h \
    ta_shapes/Mesh.h \
    ui/view.h \
    ui/viewformat.h \
    ui/mainwindow.h \
    ui_mainwindow.h \
    glew-1.10.0/include/GL/glew.h

FORMS += ui/mainwindow.ui
INCLUDEPATH += camera glm lib particles ta_shapes ui glew-1.10.0/include
DEPENDPATH += camera glm lib particles ta_shapes ui glew-1.10.0/include

DEFINES += _USE_MATH_DEFINES
DEFINES += TIXML_USE_STL
DEFINES += GLM_SWIZZLE GLM_FORCE_RADIANS
OTHER_FILES += \
    shaders/shader.vert \
    shaders/shader.frag \
    shaders/lightWorld.frag \
    shaders/waterWorld.frag \
    shaders/lighting.vert \
    shaders/lighting.frag \
    shaders/bright.frag \
    shaders/horizontalBlur.frag \
    shaders/verticalBlur.frag \
    shaders/texture.frag \
    shaders/bloom.frag \
    shaders/fullscreenQuad.vert \
    shaders/lightParticlesDraw.vert \
    shaders/lightParticlesDraw.frag \
    shaders/lightParticlesUpdate.frag

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CXXFLAGS_WARN_ON -= -Wall
QMAKE_CXXFLAGS_WARN_ON += -Waddress -Warray-bounds -Wc++0x-compat -Wchar-subscripts -Wformat\
                          -Wmain -Wmissing-braces -Wparentheses -Wreorder -Wreturn-type \
                          -Wsequence-point -Wsign-compare -Wstrict-overflow=1 -Wswitch \
                          -Wtrigraphs -Wuninitialized -Wunused-label -Wunused-variable \
                          -Wvolatile-register-var -Wno-extra

QMAKE_CXXFLAGS += -g

RESOURCES += \
    resources.qrc
