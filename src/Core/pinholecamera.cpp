#include "Core/pinholecamera.h"

#include <optixu/optixu_math_namespace.h>
#include <cmath>

PinholeCamera::PinholeCamera()
{
}
//----------------------------------------------------------------------------------------------------------------------
PinholeCamera::PinholeCamera(float3 _eye, float3 _lookat, float3 _up, float _hfov, float _vfov){
    calcVectors(_eye,_lookat, _up, _hfov,_vfov);
}
//----------------------------------------------------------------------------------------------------------------------
void PinholeCamera::setParameters(float3 _eye, float3 _lookat, float3 _up, float _hfov, float _vfov){
    calcVectors(_eye,_lookat, _up, _hfov,_vfov);
}
//----------------------------------------------------------------------------------------------------------------------
void PinholeCamera::calcVectors(float3 _eye, float3 _lookat, float3 _up, float _hfov, float _vfov){
    m_lookat = _lookat;
    m_eye = _eye;
    m_up= _up;
    m_W = _lookat - _eye;
    float lookdir_len = optix::length(m_W);
    m_U = optix::normalize(optix::cross(m_W,_up));
    m_V = optix::normalize(optix::cross(m_U,m_W));
    m_U*= lookdir_len * tanf(DtoR(_hfov*0.5f));
    m_V*= lookdir_len * tanf(DtoR(_vfov*0.5f));
    m_hfov = _hfov;
    m_vfov = _vfov;
}
//----------------------------------------------------------------------------------------------------------------------
void PinholeCamera::getEyeUVW(float3 &_eye, float3 &_U, float3 &_V, float3 &_W){
    _eye = m_eye;
    _U = m_U;
    _V = m_V;
    _W = m_W;
}
//----------------------------------------------------------------------------------------------------------------------
void PinholeCamera::translate(float _x, float _y){
    float3 trans = m_U*_x + m_V*_y;
    m_eye+=trans;
    m_lookat+=trans;
}
//----------------------------------------------------------------------------------------------------------------------
void PinholeCamera::dolly(float _scale)
{
  // Better make sure the scale isn't exactly one.
  if (_scale == 1.0f) return;
  float3 d = (m_lookat - m_eye) * _scale;
  m_eye  = m_eye - d;

  calcVectors(m_eye,m_lookat,m_up,m_hfov,m_vfov);
}
//----------------------------------------------------------------------------------------------------------------------
void PinholeCamera::rotate(glm::mat4 _trans){
    float m[16];
    m[ 0] = _trans[0][0];  m[ 1] = _trans[1][0];  m[ 2] = _trans[2][0];  m[ 3] = _trans[3][0];
    m[ 4] = _trans[0][1];  m[ 5] = _trans[1][1];  m[ 6] = _trans[2][1];  m[ 7] = _trans[3][1];
    m[ 8] = _trans[0][2];  m[ 9] = _trans[1][2];  m[ 10] = _trans[2][2];  m[ 11] = _trans[3][2];
    m[ 12] = _trans[0][3];  m[ 13] = _trans[1][3];  m[ 14] = _trans[2][3];  m[ 15] = _trans[3][3];
    Matrix4x4 trans(m);
    float3 cen = m_lookat;
    Matrix4x4 frame = initWithBasis( normalize(m_U),normalize(m_V),normalize(-m_W),cen );
    glm::mat4 glmFrame;
    glmFrame[0][0] = frame[0]; glmFrame[1][0] = frame[1]; glmFrame[2][0] = frame[2]; glmFrame[3][0] = frame[3];
    glmFrame[0][1] = frame[4]; glmFrame[1][1] = frame[5]; glmFrame[2][1] = frame[6]; glmFrame[3][1] = frame[7];
    glmFrame[0][2] = frame[8]; glmFrame[1][2] = frame[9]; glmFrame[2][2] = frame[10]; glmFrame[3][2] = frame[11];
    glmFrame[0][3] = frame[12]; glmFrame[1][3] = frame[13]; glmFrame[2][3] = frame[14]; glmFrame[3][3] = frame[15];
    glm::mat4 inverse = glm::inverse(glmFrame);
    m[ 0] = inverse[0][0];  m[ 1] = inverse[1][0];  m[ 2] = inverse[2][0];  m[ 3] = inverse[3][0];
    m[ 4] = inverse[0][1];  m[ 5] = inverse[1][1];  m[ 6] = inverse[2][1];  m[ 7] = inverse[3][1];
    m[ 8] = inverse[0][2];  m[ 9] = inverse[1][2];  m[ 10] = inverse[2][2];  m[ 11] = inverse[3][2];
    m[ 12] = inverse[0][3];  m[ 13] = inverse[1][3];  m[ 14] = inverse[2][3];  m[ 15] = inverse[3][3];
    Matrix4x4 frame_inv(m);
    Matrix4x4 final_trans = frame * trans * frame_inv;

    float4 up4     = make_float4( m_up );
    float4 eye4    = make_float4( m_eye );
    eye4.w         = 1.0f;
    float4 lookat4 = make_float4( m_lookat );
    lookat4.w      = 1.0f;

    calcVectors(make_float3(final_trans*eye4), make_float3(final_trans*lookat4), make_float3(final_trans*up4),m_hfov,m_vfov);

}

//----------------------------------------------------------------------------------------------------------------------
Matrix4x4 PinholeCamera::initWithBasis(const float3 &u, const float3 &v, const float3 &w, const float3 &t){
    float m[16];
    m[0] = u.x;
    m[1] = v.x;
    m[2] = w.x;
    m[3] = t.x;

    m[4] = u.y;
    m[5] = v.y;
    m[6] = w.y;
    m[7] = t.y;

    m[8] = u.z;
    m[9] = v.z;
    m[10] = w.z;
    m[11] = t.z;

    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
    return Matrix4x4( m );
}
//----------------------------------------------------------------------------------------------------------------------
