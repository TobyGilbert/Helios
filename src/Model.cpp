#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>

Model::Model(std::string _path){
#ifdef LINUX
    glewExperimental = GL_TRUE;
    GLenum error = glewInit();
    if(error != GLEW_OK){
        std::cerr<<"GLEW IS NOT OK!!!"<<std::endl;
    }
#endif
    loadModel(_path);
}
Model::Model(){
#ifdef LINUX
    glewExperimental = GL_TRUE;
    GLenum error = glewInit();
    if(error != GLEW_OK){
        std::cerr<<"GLEW IS NOT OK!!!"<<std::endl;
    }
#endif
}

Model::~Model(){
   glDeleteBuffers(1, &m_vertexVBO);
   glDeleteBuffers(1, &m_normalsVBO);
   glDeleteBuffers(1, &m_cubeVBO);

   glDeleteBuffers(1, &m_textureVBO);
   glDeleteVertexArrays(1, &m_VAO);
}

void Model::loadCube(){
   // create and add data to the vbo
   GLfloat bufferData[] = {

      //  X     Y     Z       U     V        Normals
      // bottom
      -1.0f,-1.0f,-1.0f,   0.0f, 0.0f,       0.0, -1.0, 0.0,
      1.0f,-1.0f,-1.0f,   1.0f, 0.0f,        0.0, -1.0, 0.0,
      -1.0f,-1.0f, 1.0f,   0.0f, 1.0f,       0.0, -1.0, 0.0,
      1.0f,-1.0f,-1.0f,   1.0f, 0.0f,        0.0, -1.0, 0.0,
      1.0f,-1.0f, 1.0f,   1.0f, 1.0f,        0.0, -1.0, 0.0,
      -1.0f,-1.0f, 1.0f,   0.0f, 1.0f,       0.0, -1.0, 0.0,

      // top
      -1.0f, 1.0f,-1.0f,   1.0f, 0.0f,       0.0, 1.0, 0.0,
      -1.0f, 1.0f, 1.0f,   1.0f, 1.0f,       0.0, 1.0, 0.0,
      1.0f, 1.0f,-1.0f,   0.0f, 0.0f,        0.0, 1.0, 0.0,
      1.0f, 1.0f,-1.0f,   0.0f, 0.0f,        0.0, 1.0, 0.0,
      -1.0f, 1.0f, 1.0f,   1.0f, 1.0f,       0.0, 1.0, 0.0,
      1.0f, 1.0f, 1.0f,   0.0f, 1.0f,        0.0, 1.0, 0.0,

      // front
      -1.0f,-1.0f, 1.0f,   0.0f, 0.0f,       0.0, 0.0, 1.0,
      1.0f,-1.0f, 1.0f,   1.0f, 0.0f,        0.0, 0.0, 1.0,
      -1.0f, 1.0f, 1.0f,   0.0f, 1.0f,       0.0, 0.0, 1.0,
      1.0f,-1.0f, 1.0f,   1.0f, 0.0f,        0.0, 0.0, 1.0,
      1.0f, 1.0f, 1.0f,   1.0f, 1.0f,        0.0, 0.0, 1.0,
      -1.0f, 1.0f, 1.0f,   0.0f, 1.0f,       0.0, 0.0, 1.0,

      // back
      -1.0f,-1.0f,-1.0f,   1.0f, 0.0f,       0.0, 0.0, -1.0,
      -1.0f, 1.0f,-1.0f,   1.0f, 1.0f,       0.0, 0.0, -1.0,
      1.0f,-1.0f,-1.0f,   0.0f, 0.0f,        0.0, 0.0, -1.0,
      1.0f,-1.0f,-1.0f,   0.0f, 0.0f,        0.0, 0.0, -1.0,
      -1.0f, 1.0f,-1.0f,   1.0f, 1.0f,       0.0, 0.0, -1.0,
      1.0f, 1.0f,-1.0f,   0.0f, 1.0f,        0.0, 0.0, -1.0,

      // left
      -1.0f,-1.0f, 1.0f,   1.0f, 0.0f,       -1.0, 0.0, 0.0,
      -1.0f, 1.0f,-1.0f,   0.0f, 1.0f,       -1.0, 0.0, 0.0,
      -1.0f,-1.0f,-1.0f,   0.0f, 0.0f,       -1.0, 0.0, 0.0,
      -1.0f,-1.0f, 1.0f,   1.0f, 0.0f,       -1.0, 0.0, 0.0,
      -1.0f, 1.0f, 1.0f,   1.0f, 1.0f,       -1.0, 0.0, 0.0,
      -1.0f, 1.0f,-1.0f,   0.0f, 1.0f,       -1.0, 0.0, 0.0,

      // right
      1.0f,-1.0f, 1.0f,   0.0f, 0.0f,        1.0, 0.0, 0.0,
      1.0f,-1.0f,-1.0f,   1.0f, 0.0f,        1.0, 0.0, 0.0,
      1.0f, 1.0f,-1.0f,   1.0f, 1.0f,        1.0, 0.0, 0.0,
      1.0f,-1.0f, 1.0f,   0.0f, 0.0f,        1.0, 0.0, 0.0,
      1.0f, 1.0f,-1.0f,   1.0f, 1.0f,        1.0, 0.0, 0.0,
      1.0f, 1.0f, 1.0f,   0.0f, 1.0f,        1.0, 0.0, 0.0
   };

   m_numVerts = 6*3*2;
   glGenBuffers(1, &m_cubeVBO);
   glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(bufferData), bufferData, GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, NULL);

   // create a vao
   glGenVertexArrays(1,&m_VAO);
   glBindVertexArray(m_VAO);

   // find the position of the shader input "position"
   GLint positionLoc = 0;//m_program->getAttribLoc("position");
   GLint normalLoc = 1;// m_program->getAttribLoc("normals");
   GLint texCoordLoc = 2;//m_program->getAttribLoc("texCoord");


   // connect the data to the shader input
   glEnableVertexAttribArray(positionLoc);
   glEnableVertexAttribArray(texCoordLoc);
   glEnableVertexAttribArray(normalLoc);

   glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
   glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GL_FLOAT), (GLvoid*)(0*sizeof(GL_FLOAT)));
   glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 8*sizeof(GL_FLOAT), (GLvoid*)(3*sizeof(GL_FLOAT)));
   glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GL_FLOAT), (GLvoid*)(5*sizeof(GL_FLOAT)));

   glBindBuffer(GL_ARRAY_BUFFER, NULL);
   glBindVertexArray(NULL);

}

void Model::loadModel(std::string _path){
   Assimp::Importer importer;
   const aiScene* scene = importer.ReadFile(_path.c_str(), aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

   aiMesh* mesh = scene->mMeshes[0];

   aiVector3D* vertices;
   aiVector3D* normals;
   aiVector3D* textureCoords[mesh->mNumVertices];
   aiVector3D* tangents;

   vertices = mesh->mVertices;
   m_numVerts = mesh->mNumVertices;
   normals = mesh->mNormals;
   for (int i = 0; i<mesh->mNumVertices; i++){
      textureCoords[i] = &(mesh->mTextureCoords[0][i]);
   }
   tangents = mesh->mTangents;

   glGenBuffers(1, &m_vertexVBO);
   glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(aiVector3D)*mesh->mNumVertices, &vertices[0], GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, NULL);

   glGenBuffers(1, &m_normalsVBO);
   glBindBuffer(GL_ARRAY_BUFFER, m_normalsVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(aiVector3D)*mesh->mNumVertices, &normals[0], GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, NULL);

   glGenBuffers(1, &m_textureVBO);
   glBindBuffer(GL_ARRAY_BUFFER, m_textureVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(aiVector3D)*mesh->mNumVertices, textureCoords[0], GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, NULL);

   glGenBuffers(1, &m_tangentsVBO);
   glBindBuffer(GL_ARRAY_BUFFER, m_tangentsVBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(aiVector3D)*mesh->mNumVertices, &tangents[0], GL_STATIC_DRAW);
   glBindBuffer(GL_ARRAY_BUFFER, NULL);

   // create a vao
   glGenVertexArrays(1,&m_VAO);
   glBindVertexArray(m_VAO);

   // find the position of the shader input "position"
   GLint positionLoc = 0;//m_program->getAttribLoc("position");
   GLint normalLoc = 1;// m_program->getAttribLoc("normals");
   GLint texCoordLoc = 2;
   GLint tangLoc = 3;

   // connect the data to the shader input
   glEnableVertexAttribArray(positionLoc);
   glEnableVertexAttribArray(normalLoc);
   glEnableVertexAttribArray(texCoordLoc);
   glEnableVertexAttribArray(tangLoc);

   glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);
   glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(aiVector3D), (GLvoid*)(0*sizeof(GL_FLOAT)));
   glBindBuffer(GL_ARRAY_BUFFER, NULL);

   glBindBuffer(GL_ARRAY_BUFFER, m_normalsVBO);
   glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, sizeof(aiVector3D), (GLvoid*)(0*sizeof(GL_FLOAT)));
   glBindBuffer(GL_ARRAY_BUFFER, NULL);

   glBindBuffer(GL_ARRAY_BUFFER, m_textureVBO);
   glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, sizeof(aiVector3D), (GLvoid*)(0*sizeof(GL_FLOAT)));
   glBindBuffer(GL_ARRAY_BUFFER, NULL);

   glBindBuffer(GL_ARRAY_BUFFER, m_tangentsVBO);
   glVertexAttribPointer(tangLoc, 3, GL_FLOAT, GL_FALSE, sizeof(aiVector3D), (GLvoid*)(0*sizeof(GL_FLOAT)));
   glBindBuffer(GL_ARRAY_BUFFER, NULL);

   glBindVertexArray(NULL);
}
