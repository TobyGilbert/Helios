#include <optix_world.h>

using namespace optix;

rtDeclareVariable(float4, sphere, , );

rtDeclareVariable(float3, texcoord, attribute texcoord, );
rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(int, lgt_idx, attribute lgt_idx, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

RT_PROGRAM void intersect_sphere(int primIdx){
    float3 center = make_float3( sphere.x, sphere.y, sphere.z );
    float radius = sphere.w;
    float3 O = ray.origin - center;
    float a = dot( ray.direction, ray.direction);
    float b = 2.0 * dot( ray.direction, O );
    float c = dot(O , O) - (radius*radius);
    float disc = b*b - (4 * a * c);
    if( disc > 0.0f ) {
        float sdisc = sqrtf( disc );
        float root1 = (-b - sdisc) / (2 * a);
        bool check_second = true;
        if( rtPotentialIntersection( root1 ) ) {
            shading_normal = geometric_normal = ((O + root1*ray.direction))/radius;
            if( rtReportIntersection( 0 ) ) check_second = false;
        }
        if( check_second ) {
            float root2 = (-b + sdisc) / ( 2 * a);
            if( rtPotentialIntersection( root2 ) ) {
                shading_normal = geometric_normal =((O + root2*ray.direction))/radius;
                rtReportIntersection( 0 );
            }
        }
    }
}

RT_PROGRAM void bounds_sphere(int, float result[6])
{
    float3 center = make_float3(sphere.x,sphere.y,sphere.z);
    float3 radiusV3 = make_float3(sphere.w,sphere.w,sphere.w);
    float3 min = center - radiusV3;
    float3 max = center + radiusV3;

    result[0] = min.x;
    result[1] = min.y;
    result[2] = min.z;
    result[3] = max.x;
    result[4] = max.y;
    result[5] = max.z;
}

