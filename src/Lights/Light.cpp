#include "Lights/Light.h"
#include "Core/pathtracerscene.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace optix;
//----------------------------------------------------------------------------------------------------------------------
Light::Light(){
    std::string ptx_path = "ptx/path_tracer.cu.ptx";
    m_lightMaterial = PathTracerScene::getInstance()->getContext()->createMaterial();
    Program diffuse_em = PathTracerScene::getInstance()->getContext()->createProgramFromPTXFile( ptx_path, "diffuseEmitter" );
    m_lightMaterial->setClosestHitProgram( 0, diffuse_em );

    // Set up our intersection programs
    ptx_path = "ptx/parallelogram.cu.ptx";
    m_pgram_bounding_box = PathTracerScene::getInstance()->getContext()->createProgramFromPTXFile( ptx_path, "bounds" );
    m_pgram_intersection = PathTracerScene::getInstance()->getContext()->createProgramFromPTXFile( ptx_path, "intersect" );

    // identity matrix to init our transformation
    float m[16];
    m[ 0] = 1.0f;  m[ 1] = 0.0f;  m[ 2] = 0.0f;  m[ 3] = 0.0f;
    m[ 4] = 0.0f;  m[ 5] = 1.0f;  m[ 6] = 0.0f;  m[ 7] = 0.0f;
    m[ 8] = 0.0f;  m[ 9] = 0.0f;  m[10] = 1.0f;  m[11] = 0.0f;
    m[12] = 0.0f;  m[13] = 0.0f;  m[14] = 0.0f;  m[15] = 1.0f;
    m_trans = PathTracerScene::getInstance()->getContext()->createTransform();
    m_trans->setMatrix(false, m, 0);

    createParollelogramLight();

}
//----------------------------------------------------------------------------------------------------------------------
Light::~Light(){

}
//----------------------------------------------------------------------------------------------------------------------
optix::GeometryInstance Light::createParallelogram(const float3 &anchor, const float3 &offset1, const float3 &offset2){
    optix::Geometry parallelogram = PathTracerScene::getInstance()->getContext()->createGeometry();
    parallelogram->setPrimitiveCount( 1u );
    parallelogram->setIntersectionProgram( m_pgram_intersection );
    parallelogram->setBoundingBoxProgram( m_pgram_bounding_box );

    float3 normal = normalize( cross( offset1, offset2 ) );
    float d = dot( normal, anchor );
    float4 plane = optix::make_float4( normal, d );

    float3 v1 = offset1 / dot( offset1, offset1 );
    float3 v2 = offset2 / dot( offset2, offset2 );

    parallelogram["plane"]->setFloat( plane );
    parallelogram["anchor"]->setFloat( anchor );
    parallelogram["v1"]->setFloat( v1 );
    parallelogram["v2"]->setFloat( v2 );

    optix::GeometryInstance gi = PathTracerScene::getInstance()->getContext()->createGeometryInstance();
    gi->setGeometry(parallelogram);
    return gi;
}
//----------------------------------------------------------------------------------------------------------------------
void Light::createParollelogramLight(){
    m_parallelogramLight.corner = make_float3(0.5, 0.0, -0.5);
    m_parallelogramLight.v1 = make_float3(-1.0, 0.0, 0.0);
    m_parallelogramLight.v2 = make_float3(0.0, 0.0, 1.0);
    m_parallelogramLight.normal = normalize(cross(m_parallelogramLight.v1, m_parallelogramLight.v2));
    m_parallelogramLight.emission = make_float3(5.0, 5.0, 5.0);

    m_geometryInstance = createParallelogram(m_parallelogramLight.corner, m_parallelogramLight.v1, m_parallelogramLight.v2);
    m_geometryInstance->addMaterial(m_lightMaterial);
    m_geometryInstance["emission_color"]->setFloat(5.0, 5.0, 5.0);

    m_geometryGroup = PathTracerScene::getInstance()->getContext()->createGeometryGroup();
    m_geometryGroup->setChildCount(1);
    m_geometryGroup->setChild(0, m_geometryInstance);

    m_acceleration = PathTracerScene::getInstance()->getContext()->createAcceleration("Bvh", "Bvh");
    m_geometryGroup->setAcceleration(m_acceleration);

    m_acceleration->markDirty();

    m_trans->setChild(m_geometryGroup);
}
//----------------------------------------------------------------------------------------------------------------------

