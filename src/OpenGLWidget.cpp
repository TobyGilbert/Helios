#include <QGuiApplication>
#include <QFileDialog>
#include <QBuffer>
#include <QImageWriter>

#include <optixu/optixpp_namespace.h>

#include "OpenGLWidget.h"
#include <iostream>

#include "Shading.h"

const static float INCREMENT=0.02;
#define PI 3.14159265359f
//------------------------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//------------------------------------------------------------------------------------------------------------------------------------
const static float ZOOM=0.1;
OpenGLWidget::OpenGLWidget(const QGLFormat _format, QWidget *_parent) : QGLWidget(_format,_parent){
    // set this widget to have the initial keyboard focus
    setFocus();
    setFocusPolicy(Qt::StrongFocus);
    // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
    m_rotate=false;
    // mouse rotation values set to 0
    m_spinXFace=0;
    m_spinYFace=0;
    m_zoom = 1.0;
    m_resolutionScale = 1;
    m_moveRenderReduction = 4;
    // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
    this->resize(_parent->size());
}
//----------------------------------------------------------------------------------------------------------------------
OpenGLWidget::~OpenGLWidget(){
    delete m_pathTracer;
    delete m_shaderProgram;
    delete m_cam;
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(2, m_VBO);
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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // as re-size is not explicitly called we need to do this.
    glViewport(0,0,width(),height());

    m_pathTracer = new PathTracerScene();
    m_pathTracer->setNumSamples(2u);
    m_pathTracer->setSize(width(),height());
    m_pathTracer->setDevicePixelRatio(devicePixelRatio());
    m_pathTracer->init();

    //create our plane to project our scene onto
    float vertex[]={
        //bottom left
        -1.0f,-1.0f,
        //top left
        -1.0f,1.0f,
        //bottom right
        1.0f,-1.0f,
        //top right
        1.0f,1.0f
    };
    //notice our tex coords are flipped upside down becuase
    //the resulting image from a pinhole camera will be upside down
    float texCoords[]={
        //bottom left
        1.0,1.0f,
        //top left
        1.0f,0.0f,
        //bottom right
        0.0f,1.0f,
        //top right
        0.0f,0.0f
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

    m_cam = new Camera(glm::vec3(0.0, 0.0, -20.0));

//    Shading shade;
//    shade.compileOSL(QString("shaders/OSL/checkerboard.osl"));

    startTimer(0);

}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::resizeGL(const int _w, const int _h){
    // set the viewport for openGL
    glViewport(0,0,_w,_h);
    m_pathTracer->resize(_w/m_resolutionScale,_h/m_resolutionScale);
    m_cam->setShape(width(), height());

}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::timerEvent(QTimerEvent *){
//    m_pathTracer->trace();
    updateGL();
}

//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::paintGL(){
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_pathTracer->trace();
    GLuint vboId = m_pathTracer->getOutputBuffer()->getGLBOId();

    glBindTexture( GL_TEXTURE_2D, m_texID);
    // send pbo to texture
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, vboId);

    RTsize elementSize = m_pathTracer->getOutputBuffer()->getElementSize();
    if      ((elementSize % 8) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
    else if ((elementSize % 4) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    else if ((elementSize % 2) == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    else                             glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, width()/RESOLOUTION_SCALE, height()/RESOLOUTION_SCALE, 0, GL_RGBA, GL_FLOAT, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, m_pathTracer->getWidth(), m_pathTracer->getHeight(), 0, GL_RGBA, GL_FLOAT, 0);

    loadMatricesToShader(glm::mat4(1.0), m_cam->getViewMatrix(), m_cam->getProjectionMatrix());

    glBindBuffer( GL_PIXEL_UNPACK_BUFFER, 0 );
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
void OpenGLWidget::loadMatricesToShader(glm::mat4 _modelMatrix, glm::mat4 _viewMatrix, glm::mat4 _perspectiveMatrix){
    GLuint MVPLoc = m_shaderProgram->getUniformLoc("MVP");

    glm::mat4 MVP = _perspectiveMatrix;

    glUniformMatrix4fv(MVPLoc, 1, GL_FALSE, glm::value_ptr(MVP));
}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::meshTransform(int _id, float _transX, float _transY, float _transZ, float _rotX, float _rotY, float _rotZ, float _scaleX, float _scaleY, float _scaleZ){
    glm::mat4 rotX,rotY,rotZ,finalRot;
    float DtoR = PI/180.0;
    rotX = glm::rotate(rotX,_rotX*DtoR,glm::vec3(1,0,0));
    rotY = glm::rotate(rotY,_rotY*DtoR,glm::vec3(0,1,0));
    rotZ = glm::rotate(rotZ,_rotZ*DtoR,glm::vec3(0,0,1));
    finalRot = rotX * rotY * rotZ;
    glm::mat4 finalTrans = finalRot;
    finalTrans[3][0] = _transX;
    finalTrans[3][1] = _transY;
    finalTrans[3][2] = _transZ;
    finalTrans[0][0] = _scaleX;
    finalTrans[1][1] = _scaleY;
    finalTrans[2][2] = _scaleZ;
    m_pathTracer->transformModel(_id,finalTrans);
}

//------------------------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::mouseMoveEvent (QMouseEvent *_event){
  // Sourced from Jon Macey's NGL library
  // note the method buttons() is the button state when event was called
  // this is different from button() which is used to check which button was
  // pressed when the mousePress/Release event is generated
  if(m_rotate && _event->buttons() == Qt::LeftButton){
    float diffx=_event->x()-m_origX;
    float diffy=_event->y()-m_origY;
    glm::mat4 rotx,roty,finalRot;
    rotx = glm::rotate(rotx, 0.002f * diffy,glm::vec3(1.0,0.0,0.0));
    roty = glm::rotate(roty, 0.002f * diffx,glm::vec3(0.0,1.0,0.0));
    finalRot = rotx*roty;
    m_pathTracer->getCamera()->rotate(finalRot);
    m_pathTracer->signalCameraChanged();
    m_origX = _event->x();
    m_origY = _event->y();
  }
        // right mouse translate code
  else if(m_translate && _event->buttons() == Qt::RightButton){
    float diffX = (_event->x() - m_origXPos) * INCREMENT;
    float diffY = (_event->y() - m_origYPos) * INCREMENT;
    diffX*=-1.0;
    m_origXPos=_event->x();
    m_origYPos=_event->y();
    m_pathTracer->getCamera()->translate(diffX,diffY);
    m_pathTracer->signalCameraChanged();
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
    m_pathTracer->resize(width()/m_moveRenderReduction,height()/m_moveRenderReduction);
  }
  // right mouse translate mode
  else if(_event->button() == Qt::RightButton)
  {
    m_origXPos = _event->x();
    m_origYPos = _event->y();
    m_translate = true;
    m_pathTracer->resize(width()/m_moveRenderReduction,height()/m_moveRenderReduction);
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
    m_pathTracer->resize(width(),height());
  }
        // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_translate=false;
    m_pathTracer->resize(width(),height());
  }
}
//------------------------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::wheelEvent(QWheelEvent *_event){
    // Sourced from Jon Macey's NGL library
    // check the diff of the wheel position (0 means no change)
    if(_event->delta() > 0)
    {
        m_zoom=ZOOM;
        m_pathTracer->getCamera()->dolly(-m_zoom);
        m_pathTracer->signalCameraChanged();
    }
    else if(_event->delta() <0 )
    {
        m_zoom= ZOOM;
        m_pathTracer->getCamera()->dolly(m_zoom);
        m_pathTracer->signalCameraChanged();
    }
}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::keyPressEvent(QKeyEvent *_event){
    switch(_event->key()){
        case Qt::Key_Escape:
            QGuiApplication::exit();
            break;
        default:
            break;
    }
}
//----------------------------------------------------------------------------------------------------------------------
void OpenGLWidget::saveImage(){

   QImage image = m_pathTracer->saveImage();
   QFileDialog fileDialog(this);
   fileDialog.setDefaultSuffix(".png");
   QString saveFile = fileDialog.getSaveFileName(this, tr("Save Image File"));

   image.save(saveFile+QString(".png"), "PNG");
}
