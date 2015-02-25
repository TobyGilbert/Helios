#ifndef SHADERGLOBALS_H_
#define SHADERGLOBALS_H_
#include <optixu/optixpp_namespace.h>

struct OSLShaderGlobals{
    float3 P;
    float3 I;
    float3 N;
    float3 Ng;
    float u, v;
};


#endif
