#include "TextureUtils.h"
#include <QGLWidget>

TextureUtils::~TextureUtils(){
    delete m_image;
}

GLuint TextureUtils::createTexture(QString path){

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    m_image = new QImage(path);
    QImage tex = QGLWidget::convertToGLFormat(*m_image);
    glTexImage2D(GL_TEXTURE_2D,0, GL_RGBA, tex.width(), tex.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, texture);

    return texture;
}

GLuint TextureUtils::createTexture(QImage _image){
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    m_image = new QImage(_image);
    QImage tex = QGLWidget::convertToGLFormat(*m_image);
    glTexImage2D(GL_TEXTURE_2D,0, GL_RGBA, tex.width(), tex.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.bits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, texture);

    return texture;
}
