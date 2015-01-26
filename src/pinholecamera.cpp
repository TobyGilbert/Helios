#include "pinholecamera.h"

#include <optixu/optixu_math_namespace.h>

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
    m_U = optix::normalize(optix::cross(_lookat,_up));
    m_V = optix::normalize(optix::cross(m_U,_lookat));

    float length = optix::length(_lookat);
    float height = 2.0*tan(_vfov/2.0) * length;
    float width = 2.0*tan(_hfov/2.0) * length;

    m_W = 0.5*width*m_U + 0.5*height*m_V - _eye;
    m_eye = _eye;
}
//----------------------------------------------------------------------------------------------------------------------
void PinholeCamera::getEyeUVW(float3 &_eye, float3 &_U, float3 &_V, float3 &_W){
    _eye = m_eye;
    _U = m_U;
    _V = m_V;
    _W = m_W;
}
//----------------------------------------------------------------------------------------------------------------------
