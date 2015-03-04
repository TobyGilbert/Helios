#include "Texture.h"
#include "TextureUtils.h"

Texture::Texture(QString _path){
   TextureUtils utils;
   m_textureID = utils.createTexture(_path);
   m_image = new QImage();
   m_image = utils.getImage();
}

Texture::Texture(QImage _image){
    TextureUtils utils;
    m_textureID = utils.createTexture(_image);
    m_image = new QImage(_image);
}

Texture::~Texture(){
   glDeleteTextures(1, &m_textureID);
}

void Texture::bind(GLuint _unit){
   glActiveTexture(GL_TEXTURE0 + _unit);
   glBindTexture(GL_TEXTURE_2D, m_textureID);
}

GLuint Texture::getTextureID(){
   return m_textureID;
}

void Texture::setParamater(GLenum _pname, GLenum _param){
   glBindTexture(GL_TEXTURE_2D, m_textureID);
   glTexParameteri(GL_TEXTURE_2D, _param, _param);
   glBindTexture(GL_TEXTURE_2D, 0);
}
