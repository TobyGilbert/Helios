#include "Camera.h"

Camera::Camera(glm::vec3 _pos){
   m_position = _pos;
   m_up = glm::vec3(0.0, 1.0, 0.0);
   m_fov = 45.0;
   m_aspect = 720.0/576.0;
//   m_projectionMatrix = glm::perspective(m_fov, m_aspect, 0.1f, 350.0f);
   m_projectionMatrix = glm::ortho(0,1,0,1);
   lookAt(m_position, glm::vec3(0.0, 0.0, 0.0), m_up);
}

Camera::~Camera(){
}

void Camera::lookAt(glm::vec3 _position, glm::vec3 _center, glm::vec3 _up){
    m_viewMatrix = glm::lookAt(_position, _center, _up);
}

void Camera::setPosition(glm::vec3 _position){
   m_position = _position;
}

void Camera::setProjectionMatrix(float _fov, float _aspect, float _near, float _far){
    m_projectionMatrix = glm::perspective(_fov, _aspect, _near, _far);
}

void Camera::setShape(float aspect){
    setProjectionMatrix(m_fov, m_aspect, 0.1f, 350.0f);
}

void Camera::setShape(float _w, float _h){
    setProjectionMatrix(m_fov, _w/_h, 0.1f, 350.0f);
}
