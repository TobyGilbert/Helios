#ifndef __TEXTURE_H_
#define __TEXTURE_H_


#ifdef DARWIN
    #include <GLFW/glfw3.h>
    #include <OpenGL/gl3.h>
#else
    #include <GL/glew.h>
    #include <GL/gl.h>
#endif
#include <iostream>
#include <glm/glm.hpp>
#include <QImage>

class Texture{
public:
   Texture(QString _path);
   Texture(QImage _image);
   ~Texture();
   void bind(GLuint _unit);
   GLuint getTextureID();
   void setParamater(GLenum _pname, GLenum _param);
   inline QImage* getImage(){return m_image;}
private:
   GLuint m_textureID;
   QImage *m_image;

};
#endif
