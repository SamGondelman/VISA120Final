#include "ResourceLoader.h"
#include <QFile>
#include <QString>
#include <QTextStream>

std::string ResourceLoader::loadResourceFileToString(const std::string &resourcePath)
{
    QString vertFilePath = QString::fromStdString(resourcePath);
    QFile vertFile(vertFilePath);
    if (vertFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream vertStream(&vertFile);
        QString contents = vertStream.readAll();
        return contents.toStdString();
    }
    throw CS123::IOException("Could not open file: " + resourcePath);
}

void ResourceLoader::initializeGlew() {
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    fprintf(stdout, "Using GLEW %s\n", glewGetString(GLEW_VERSION));
}
