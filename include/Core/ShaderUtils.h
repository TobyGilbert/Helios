#ifndef __SHADERUTILS_H_
#define __SHADERUTILS_H_

#ifdef DARWIN
    #include <OpenGL/gl3.h>
#else
    #include <GL/glew.h>
    #include <GL/gl.h>
#endif
#include <iostream>
#include <fstream>
#include <streambuf>
#include <string>

class shaderUtils{
public:
   static GLuint createShaderFromFile(const GLchar* path, GLenum shaderType);
};

#endif
