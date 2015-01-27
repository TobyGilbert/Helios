#include "pinholecamera.h"

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
    m_eye = _eye;
    m_W = _lookat - _eye;
    float lookdir_len = optix::length(m_W);
    m_U = optix::normalize(optix::cross(m_W,_up));
    m_V = optix::normalize(optix::cross(m_U,m_W));
    m_U*= lookdir_len * tanf(DtoR(_hfov*0.5f));
    m_V*= lookdir_len * tanf(DtoR(_vfov*0.5f));
}
//----------------------------------------------------------------------------------------------------------------------
void PinholeCamera::getEyeUVW(float3 &_eye, float3 &_U, float3 &_V, float3 &_W){
    _eye = m_eye;
    _U = m_U;
    _V = m_V;
    _W = m_W;
}
//----------------------------------------------------------------------------------------------------------------------
