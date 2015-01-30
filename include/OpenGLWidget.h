#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#ifdef DARWIN
    #include <OpenGL/gl3.h>
#else
    #include <GL/glew.h>
    #include <GL/gl.h>
#endif

#include "Camera.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "pathtracerscene.h"

#include <QGLWidget>
#include <QEvent>
#include <QResizeEvent>
#include <QMessageBox>
#include <glm/glm.hpp>

class OpenGLWidget : public QGLWidget
{
    Q_OBJECT //must include to gain access to qt stuff

public:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief ctor for our NGL drawing class
    /// @param [in] parent the parent window to the class
    //----------------------------------------------------------------------------------------------------------------------
    explicit OpenGLWidget(const QGLFormat _format, QWidget *_parent=0);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief dtor must close down and release OpenGL resources
    //----------------------------------------------------------------------------------------------------------------------
    ~OpenGLWidget();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the virtual initialize class is called once when the window is created and we have a valid GL context
    /// use this to setup any default GL stuff
    //----------------------------------------------------------------------------------------------------------------------
    void initializeGL();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief this is called everytime we want to draw the scene
    //----------------------------------------------------------------------------------------------------------------------
    void paintGL();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief called to resize the window
    //----------------------------------------------------------------------------------------------------------------------
    void resizeGL(const int _w, const int _h );
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief mouse move
    //----------------------------------------------------------------------------------------------------------------------
    void mouseMoveEvent(QMouseEvent *_event);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief a timer event function from the Q_object
    //----------------------------------------------------------------------------------------------------------------------
    void timerEvent(QTimerEvent *);//
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief A function called when a mouse button is pressed
    //----------------------------------------------------------------------------------------------------------------------
    void mousePressEvent(QMouseEvent *_event);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Called when the mouse button is released
    //----------------------------------------------------------------------------------------------------------------------
    void mouseReleaseEvent(QMouseEvent *_event);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Scroll wheel event
    //----------------------------------------------------------------------------------------------------------------------
    void wheelEvent(QWheelEvent *_event);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Key press event
    //----------------------------------------------------------------------------------------------------------------------
    void keyPressEvent(QKeyEvent *_event);
    //----------------------------------------------------------------------------------------------------------------------
private:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief out vao for our plane to project our texture on to
    //----------------------------------------------------------------------------------------------------------------------
    GLuint m_VAO;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our buffer object for our vao, one for vertex postions and one for texture coordinates
    //----------------------------------------------------------------------------------------------------------------------
    GLuint m_VBO[2];
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief handle to the texture we will project onto our plane
    //----------------------------------------------------------------------------------------------------------------------
    GLuint m_texID;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief out path tracing scene
    //----------------------------------------------------------------------------------------------------------------------
    PathTracerScene *m_pathTracer;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Shader Program
    //----------------------------------------------------------------------------------------------------------------------
    ShaderProgram *m_shaderProgram;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Vertex Shader
    //----------------------------------------------------------------------------------------------------------------------
    Shader *m_vertexShader;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Fragment Shader
    //----------------------------------------------------------------------------------------------------------------------
    Shader *m_fragmentShader;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the location of our texture uniform in our shader
    //----------------------------------------------------------------------------------------------------------------------
    GLuint m_texLoc;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief ModelViewProjection matrix location
    //----------------------------------------------------------------------------------------------------------------------
    GLuint m_modelViewProjectionLoc;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Height of the window
    //----------------------------------------------------------------------------------------------------------------------
    int m_height;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Width of the window
    //----------------------------------------------------------------------------------------------------------------------
    int m_width;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Mouse transforms
    //----------------------------------------------------------------------------------------------------------------------
    glm::mat4 m_mouseGlobalTX;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief model pos
    //----------------------------------------------------------------------------------------------------------------------
    glm::vec3 m_modelPos;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Spin face x
    //----------------------------------------------------------------------------------------------------------------------
    float m_spinXFace;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Sping face y
    //----------------------------------------------------------------------------------------------------------------------
    float m_spinYFace;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the zoom of our camera
    //----------------------------------------------------------------------------------------------------------------------
    float m_zoom;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief rotate bool
    //----------------------------------------------------------------------------------------------------------------------
    bool m_rotate;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief translate bool
    //----------------------------------------------------------------------------------------------------------------------
    bool m_translate;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief
    //----------------------------------------------------------------------------------------------------------------------
    int m_origX;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief
    //----------------------------------------------------------------------------------------------------------------------
    int m_origY;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief
    //----------------------------------------------------------------------------------------------------------------------
    int m_origXPos;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief
    //----------------------------------------------------------------------------------------------------------------------
    int m_origYPos;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Our opengl camera
    //----------------------------------------------------------------------------------------------------------------------
    Camera *m_cam;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Load our perspective matrix to the shader
    //----------------------------------------------------------------------------------------------------------------------
    void loadMatricesToShader(glm::mat4 _modelMatrix, glm::mat4 _viewMatrix, glm::mat4 _perspectiveMatrix);
    //----------------------------------------------------------------------------------------------------------------------

};

#endif // OPENGLWIDGET_H
