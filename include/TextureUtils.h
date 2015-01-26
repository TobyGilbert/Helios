#ifndef __TEXTUREUTILS_H_
#define __TEXTUREUTILS_H_

#ifdef DARWIN
    #include <OpenGL/gl3.h>
    #include <GLFW/glfw3.h>
#else
    #include <GL/glew.h>
    #include <GL/gl.h>
#endif
#include <iostream>
#include <QImage>

class TextureUtils{
public:
   GLuint createTexture(QString path);
   GLuint createTexture(QImage _image);
   inline QImage* getImage(){return m_image;}
   ~TextureUtils();
private:
    QImage *m_image;
};

#endif
