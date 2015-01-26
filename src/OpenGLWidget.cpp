#include <QGuiApplication>

#include <optixu/optixpp_namespace.h>

#include "OpenGLWidget.h"
#include <iostream>

const static float INCREMENT=0.01;
const static int RESOLOUTION_SCALE = 2;
//------------------------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//------------------------------------------------------------------------------------------------------------------------------------
const static float ZOOM=0.1;
OpenGLWidget::OpenGLWidget(const QGLFormat _format, QWidget *_parent) : QGLWidget(_format,_parent){
    // set this widget to have the initial keyboard focus
    setFocus();
    // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
    m_rotate=false;
    // mouse rotation values set to 0
    m_spinXFace=0;
    m_spinYFace=0;
    // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
    this->resize(_parent->size());
}
//----------------------------------------------------------------------------------------------------------------------
OpenGLWidget::~OpenGLWidget(){
    delete m_pathTracer;
    delete m_cam;
    delete m_shaderProgram;
}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::initializeGL(){
#ifdef LINUX
    glewExperimental = GL_TRUE;
    GLenum error = glewInit();
    if(error != GLEW_OK){
        std::cerr<<"GLEW IS NOT OK!!! "<<std::endl;
    }
#endif
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    // enable depth testing for drawing
    glEnable(GL_DEPTH_TEST);
    // enable multisampling for smoother drawing
    glEnable(GL_MULTISAMPLE);

    // as re-size is not explicitly called we need to do this.
    glViewport(0,0,width(),height());

//    m_sampleScene = new OptixSample();
//    m_sampleScene->setSize(512u,512u);
//    m_sampleScene->init();

//    m_sampleSceneTwo = new OptixSampleTwo();
//    m_sampleSceneTwo->setSize(1080,1080);
//    m_sampleSceneTwo->setNumBoxes(10);
//    m_sampleSceneTwo->init();
//    m_sampleSceneTwo->trace();

    m_pathTracer = new PathTracerScene();
    m_pathTracer->setNumSamples(2u);
    m_pathTracer->setSize(width(),height());
    std::cout<<width()<<","<<height()<<std::endl;
    m_pathTracer->init();

    //create our plane to project our scene onto
    float vertex[]={
        //bottom left
        0.0f,0.0f,
        //top left
        0.0f,1.0f,
        //bottom right
        1.0f,0.0f,
        //top right
        1.0f,1.0f
    };
    //notice our tex coords are flipped upside down becuase
    //the resulting image from a pinhole camera will be upside down
    float texCoords[]={
        //bottom left
        0.0,0.0f,
        //top left
        0.0f,1.0f,
        //bottom right
        1.0f,0.0f,
        //top right
        1.0f,1.0f

    };
    //gen our VAO
    glGenVertexArrays(1,&m_VAO);
    glBindVertexArray(m_VAO);
    //put our data into our vbo's
    glGenBuffers(2,m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER,m_VBO[0]);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertex),vertex,GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(float)*2.0,(GLvoid*)(0*sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,m_VBO[1]);
    glBufferData(GL_ARRAY_BUFFER,sizeof(texCoords),texCoords,GL_STATIC_DRAW);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(float)*2.0,(GLvoid*)(0*sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    //now lets generate our texture for later use
    glGenTextures(1,&m_texID);
    glBindTexture(GL_TEXTURE_2D, m_texID);

    // Change these to GL_LINEAR for super- or sub-sampling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // GL_CLAMP_TO_EDGE for linear filtering, not relevant for nearest.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture( GL_TEXTURE_2D, 0);
    //all good!


    // Create a shader program
    m_shaderProgram = new ShaderProgram();
    m_vertexShader = new Shader("shaders/pathTraceVert.vert", GL_VERTEX_SHADER);
    m_fragmentShader = new Shader("shaders/pathTraceFrag.frag", GL_FRAGMENT_SHADER);

    m_shaderProgram->attachShader(m_vertexShader);
    m_shaderProgram->attachShader(m_fragmentShader);
    m_shaderProgram->bindFragDataLocation(0, "fragColour");
    m_shaderProgram->link();
    m_shaderProgram->use();

    delete m_vertexShader;
    delete m_fragmentShader;

    m_modelViewProjectionLoc = m_shaderProgram->getUniformLoc("MVP");
    m_texLoc = m_shaderProgram->getUniformLoc("pathTraceTex");
    glUniform1i(m_texLoc,0);

    // Initialize the camera
    m_cam = new Camera(glm::vec3(0.001, 0.0001, 1.0));

    startTimer(0);

}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::resizeGL(const int _w, const int _h){
    // set the viewport for openGL
    glViewport(0,0,_w,_h);
    m_pathTracer->resize(_w/RESOLOUTION_SCALE,_h/RESOLOUTION_SCALE);

}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::timerEvent(QTimerEvent *){

//    m_pathTracer->trace();
    updateGL();
}

//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::paintGL(){

//    m_sampleScene->trace();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //Initialise the model matrix

    m_pathTracer->trace();
    GLuint vboId = m_pathTracer->getOutputBuffer()->getGLBOId();

    glBindTexture( GL_TEXTURE_2D, m_texID);
    // send pbo to texture
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vboId);

    Buffer buffer = m_pathTracer->getOutputBuffer();
    RTformat buffer_format = buffer->getFormat();

    RTsize elementSize = m_pathTracer->getOutputBuffer()->getElementSize();
    if      ((elementSize % 8) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
    else if ((elementSize % 4) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    else if ((elementSize % 2) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    else                             glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    {
      if(buffer_format == RT_FORMAT_UNSIGNED_BYTE4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width(), height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
      } else if(buffer_format == RT_FORMAT_FLOAT4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, width()/RESOLOUTION_SCALE, height()/RESOLOUTION_SCALE, 0, GL_RGBA, GL_FLOAT, 0);
      } else if(buffer_format == RT_FORMAT_FLOAT3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F_ARB, width(), height(),0, GL_RGB, GL_FLOAT, 0);
      } else if(buffer_format == RT_FORMAT_FLOAT) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE32F_ARB, width(), height(), 0, GL_LUMINANCE, GL_FLOAT, 0);
      } else {
        assert(0 && "Unknown buffer format");
      }
    }
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, width(), height(), 0, GL_RGBA, GL_FLOAT, 0);
    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );

//    glUniform1i(m_texLoc,0);

    loadMatricesToShader();

    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::loadMatricesToShader(){

    glm::mat4 projectionMatrix = m_cam->getProjectionMatrix();

    glm::mat4 ViewMatrix = m_cam->getViewMatrix();

    glm::mat4 modelViewProjectionMatrix = projectionMatrix * ViewMatrix;

    glUniformMatrix4fv(m_modelViewProjectionLoc, 1, false, glm::value_ptr(modelViewProjectionMatrix));
}
//------------------------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::mouseMoveEvent (QMouseEvent *_event){
  // Sourced from Jon Macey's NGL library
  // note the method buttons() is the button state when event was called
  // this is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if(m_rotate && _event->buttons() == Qt::LeftButton){
    int diffx=_event->x()-m_origX;
    int diffy=_event->y()-m_origY;
    m_spinXFace += (float) 0.5f * diffy;
    m_spinYFace += (float) 0.5f * diffx;
    m_origX = _event->x();
    m_origY = _event->y();
  }
        // right mouse translate code
  else if(m_translate && _event->buttons() == Qt::RightButton){
    int diffX = (int)(_event->x() - m_origXPos);
    int diffY = (int)(_event->y() - m_origYPos);
    m_origXPos=_event->x();
    m_origYPos=_event->y();
    m_modelPos.x += INCREMENT * diffX;
    m_modelPos.y -= INCREMENT * diffY;
   }
}
//------------------------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::mousePressEvent ( QMouseEvent * _event){
    // Sourced from Jon Macey's NGL library
  // this method is called when the mouse button is pressed in this case we
  // store the value where the mouse was clicked (x,y) and set the Rotate flag to true
  if(_event->button() == Qt::LeftButton)
  {
    m_origX = _event->x();
    m_origY = _event->y();
    m_rotate = true;
  }
  // right mouse translate mode
  else if(_event->button() == Qt::RightButton)
  {
    m_origXPos = _event->x();
    m_origYPos = _event->y();
    m_translate = true;
  }

}
//------------------------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::mouseReleaseEvent ( QMouseEvent * _event ){
    // Sourced from Jon Macey's NGL library
  // this event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_rotate=false;
  }
        // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_translate=false;
  }
}
//------------------------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::wheelEvent(QWheelEvent *_event){
    // Sourced from Jon Macey's NGL library
    // check the diff of the wheel position (0 means no change)
    if(_event->delta() > 0)
    {
        m_modelPos.z+=ZOOM;
    }
    else if(_event->delta() <0 )
    {
        m_modelPos.z-=ZOOM;
    }
}

