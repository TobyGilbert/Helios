#ifndef BRDFS_H_
#define BRDFS_H_
#include <optixu/optixu_math_namespace.h>

inline __device__ float fresnel_dielectric(float cosi, float eta) {
    // special case: ignore fresnel
    if (eta == 0){
        return 1;
    }
    // compute fresnel reflectance without explicitly computing the refracted direction
    if (cosi < 0.0f){
        eta = 1.0f / eta;
    }
    float c = fabsf(cosi);
    float g = eta * eta - 1 + c * c;
    if (g > 0) {
        g = sqrtf(g);
        float A = (g - c) / (g + c);
        float B = (c * (g + c) - 1) / (c * (g - c) + 1);
        return 0.5f * A * A * (1 + B * B);
    }
    return 1.0f; // TIR (no refracted component)
}
#endif
