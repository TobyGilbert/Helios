#ifndef PINHOLECAMERA_H
#define PINHOLECAMERA_H

/// @class PinholeCamera
/// @brief this in a simple pinhole camera for path tracing. This is converted code from NVidia's demo's
/// @author Declan Russell
/// @date 27/01/2015

#include <optixu/optixpp_namespace.h>
#include <cmath>

class PinholeCamera
{
public:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief type def for easier use of our optix float3
    //----------------------------------------------------------------------------------------------------------------------
    typedef optix::float3 float3;
    typedef optix::float4 float4;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our default contructor that does all our calculations
    /// @param _eye - the eye of our pinhole camera
    /// @param _lookat - the direction that our camera is facing
    /// @param _up - which direction up is
    /// @param _hfov - the horizontal field of view of our camera
    /// @param _vfov - the virtical field of view of our camera
    //----------------------------------------------------------------------------------------------------------------------
    PinholeCamera(float3 _eye, float3 _lookat, float3 _up, float _hfov, float _vfov);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief a mutator for our paramters
    /// @param _eye - the eye of our pinhole camera
    /// @param _lookat - the direction that our camera is facing
    /// @param _up - which direction up is
    /// @param _hfov - the horizontal field of view of our camera
    /// @param _vfov - the virtical field of view of our camera
    //----------------------------------------------------------------------------------------------------------------------
    void setParameters(float3 _eye, float3 _lookat, float3 _up, float _hfov, float _vfov);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief a function to return
    /// @param _eye call by reference to set the eye postion of our camera
    /// @param _U call by reference to set the U vector of of our camera
    /// @param _V call by reference to set the V vector of of our camera
    /// @param _W call by reference to set the W vector of of our camera
    //----------------------------------------------------------------------------------------------------------------------
    void getEyeUVW(float3 &_eye, float3 &_U, float3 &_V, float3 &_W);
    //----------------------------------------------------------------------------------------------------------------------
protected:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief basic constructor, we dont want this to be used.
    //----------------------------------------------------------------------------------------------------------------------
    PinholeCamera();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief calculates our Eye U V W vectors
    /// @brief this is called by our defualt constructor and setParamiters functions
    //----------------------------------------------------------------------------------------------------------------------
    void calcVectors(float3 _eye, float3 _lookat, float3 _up, float _hfov, float _vfov);
    //----------------------------------------------------------------------------------------------------------------------
private:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief convertion from degrees to radians
    //----------------------------------------------------------------------------------------------------------------------
    inline float DtoR(float d){return d*(static_cast<float>(M_PI)/180.f);}
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our eye location
    //----------------------------------------------------------------------------------------------------------------------
    float3 m_eye;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our U vector of our camera
    //----------------------------------------------------------------------------------------------------------------------
    float3 m_U;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our V vector of our camera
    //----------------------------------------------------------------------------------------------------------------------
    float3 m_V;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our W vector of our camera
    //----------------------------------------------------------------------------------------------------------------------
    float3 m_W;
    //----------------------------------------------------------------------------------------------------------------------

};

#endif // PINHOLECAMERA_H
