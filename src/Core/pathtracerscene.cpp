#include "Core/pathtracerscene.h"
#include <QColor>
#include <iostream>
#include "HDRLoader.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Core/TextureLoader.h"
#include "Lights/LightManager.h"
#include <cuda_runtime.h>
#include <glm/gtc/matrix_inverse.hpp>

//Declare our static instance variable
PathTracerScene* PathTracerScene::m_instance;

//----------------------------------------------------------------------------------------------------------------------
PathTracerScene* PathTracerScene::getInstance(){
    if(!m_instance){
        m_instance = new PathTracerScene();
    }
    return m_instance;
}

//----------------------------------------------------------------------------------------------------------------------
PathTracerScene::PathTracerScene()  : m_cameraChanged(false),
                                    m_rr_begin_depth(1u),
                                    m_sqrt_num_samples( 4u ),
                                    m_frame(0),
                                    m_width(512),
                                    m_height(512),
                                    m_translateEnviroment(false)
{
    // create an instance of our OptiX engine
    m_context = optix::Context::create();
}
//----------------------------------------------------------------------------------------------------------------------
PathTracerScene::~PathTracerScene()
{
    delete m_camera;
    std::map<std::string,OptiXModel*>::iterator models;
    for(models=m_meshArray.begin();models!=m_meshArray.end();models++)
    {
        delete models->second;
//        m_meshArray.erase(models);
    }
    m_outputBuffer->destroy();
    m_enviSampler->destroy();
    m_context->destroy();
}

//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::init()
{
    // how many ray types we have
    // we have our light ray, shadow ray and a bsdf shadow ray
    m_context->setRayTypeCount( 3 );
    // we only have one entry point which is our output buffer
    m_context->setEntryPointCount( 1 );
    // sets the stack size important for recursion
    // we want this to be as big as our hardware will allow us
    // so that we can send as many rays as the user desires
    m_context->setStackSize( 4000 );
    // set some variables
    m_context["scene_epsilon"]->setFloat( 1.e-3f );
    // set our ray types
    m_context["pathtrace_ray_type"]->setUint(0u);
    m_context["pathtrace_shadow_ray_type"]->setUint(1u);
    m_context["rr_begin_depth"]->setUint(m_rr_begin_depth);

    // Enable printing on the GPU
    rtContextSetPrintEnabled(m_context->get(), 1);

    // create our output buffer and set it in our engine
    optix::Variable output_buffer = m_context["output_buffer"];
    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * m_width * m_height, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,0);

    m_outputBuffer = m_context->createBufferFromGLBO(RT_BUFFER_OUTPUT,vbo);
    m_outputBuffer->setFormat(RT_FORMAT_FLOAT4);
    m_outputBuffer->setSize(m_width/m_devicePixelRatio,m_height/m_devicePixelRatio);
    output_buffer->set(m_outputBuffer);

    m_camera = new PathTraceCamera(optix::make_float3( 0.0f, 0.0f, -25.0f ),      //eye
                                 optix::make_float3( 0.0f, 0.0f, 0.0f ),        //lookat
                                 optix::make_float3( 0.0f, 1.0f,  0.0f ),       //up
                                 35.0f,                                         //hfov
                                 35.0f);                                        //vfov

    float3 eye,U,V,W;
    m_camera->getEyeUVW(eye,U,V,W);
    //set up our camera in our engine
    m_context["eye"]->setFloat( eye );
    m_context["U"]->setFloat( U );
    m_context["V"]->setFloat( V );
    m_context["W"]->setFloat( W);
    m_context["aperture_radius"]->setFloat(0.0);
    m_context["focal_point"]->setFloat(0.0, 0.0, 0.0);

    //set our max ray depth
    m_context["maxDepth"]->setUint(5);
    m_maxRayDepth = 5;
    m_context["sqrt_num_samples"]->setUint(m_sqrt_num_samples );
    m_context["bad_color"]->setFloat( 0.0f, 1.0f, 0.0f );
    m_context["bg_color"]->setFloat( make_float3(0.0f) );
    const float3 default_color = make_float3(1.0f, 1.0f, 1.0f);
    m_enviSampler = loadHDRTexture(m_context, "./HDRMaps/CedarCity.hdr", default_color);
    m_context["envmap"]->setTextureSampler(m_enviSampler);
    m_context["strength"]->setFloat(1.0);
    float m[16];
    m[ 0] = 1.0f;  m[ 1] = 0.0f;  m[ 2] = 0.0f;  m[ 3] = 0.0f;
    m[ 4] = 0.0f;  m[ 5] = 1.0f;  m[ 6] = 0.0f;  m[ 7] = 0.0f;
    m[ 8] = 0.0f;  m[ 9] = 0.0f;  m[10] = 1.0f;  m[11] = 0.0f;
    m[12] = 0.0f;  m[13] = 0.0f;  m[14] = 0.0f;  m[15] = 1.0f;
    m_context["cameraMatrix"]->setMatrix4x4fv(false, m);

    // Setup programs
    std::string ptx_path = "ptx/path_tracer.cu.ptx";
    optix::Program ray_gen_program = m_context->createProgramFromPTXFile( ptx_path, "depth_of_field_camera" );

    m_context->setRayGenerationProgram( 0, ray_gen_program );
    optix::Program exception_program = m_context->createProgramFromPTXFile( ptx_path, "exception" );
    m_context->setExceptionProgram( 0, exception_program );
    m_context->setMissProgram( 0, m_context->createProgramFromPTXFile( ptx_path, "envi_miss" ) );

    //init our frame number
    m_context["frame_number"]->setUint(1);

    // Index of sampling_stategy (BSDF, light, MIS)
    m_sampling_strategy = 0;
    m_context["sampling_stategy"]->setInt(m_sampling_strategy);

    // Lights buffer
    m_context["lights"]->setBuffer( LightManager::getInstance()->getLightsBuffer() );


    //create our top group and set it in our engine and add a matrix for our global translation
    m_globalTrans = m_context->createTransform();
    m_globalTrans->setMatrix(false,m,0);
    m_globalTransGroup = m_context->createGroup();
    m_globalTrans->setChild(m_globalTransGroup);
    m_globalTransGroup->setAcceleration(m_context->createAcceleration("Sbvh","Bvh"));
    m_topGroup = m_context->createGroup();
    m_topGroup->setAcceleration(m_context->createAcceleration("NoAccel","NoAccel"));
    m_topGroup->addChild(m_globalTrans);
    m_context["top_object"]->set(m_topGroup);

    // Finalize
    m_context->validate();
    m_context->compile();
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::addLight()
{
    m_context["lights"]->setBuffer( LightManager::getInstance()->getLightsBuffer() );

    // Only add the last one on the vector to avoid duplicates
    m_globalTransGroup->addChild(LightManager::getInstance()->getGeomAndTrans().back());
    m_globalTransGroup->getAcceleration()->markDirty();
    m_frame = 0;
}
//----------------------------------------------------------------------------------------------------------------------
OptiXModel* PathTracerScene::importMesh(std::string _id, std::string _path)
{
    /// @todo maybe have all this stuff in a model management class rather than the scene
    /// @todo meshes are all set with detault diffuse texture, we need some sort of material management
    //import mesh
    OptiXModel* model = new OptiXModel(_path);
    Material defaultMaterial = m_context->createMaterial();

    std::string ptx_path = "ptx/path_tracer.cu.ptx";
    Program default_ch = m_context->createProgramFromPTXFile( ptx_path, "defaultMaterial" );
    Program default_ah = m_context->createProgramFromPTXFile( ptx_path, "shadow" );
    defaultMaterial->setClosestHitProgram( 0, default_ch );
    defaultMaterial->setAnyHitProgram( 1, default_ah );
    model->setMaterial(defaultMaterial);
    //add to our scene
    m_globalTransGroup->addChild(model->getGeomAndTrans());
    m_globalTransGroup->getAcceleration()->markDirty();
    m_meshArray[_id] = model;
    m_frame = 0;

    return model;
}
//----------------------------------------------------------------------------------------------------------------------
OptiXModel* PathTracerScene::createInstance(std::string _geomId, std::string _instanceName)
{
    std::map<std::string,OptiXModel*>::iterator model = m_meshArray.find(_geomId);
    if(model!=m_meshArray.end())
    {
        OptiXModel* instance = new OptiXModel(model->second);
        m_globalTransGroup->addChild(instance->getGeomAndTrans());
        m_globalTransGroup->getAcceleration()->markDirty();
        m_meshArray[_instanceName] = instance;
        m_frame = 0;
        return instance;
    }
    else
    {
        std::cerr<<"Error: Could not find model in path tracer! Doing nothing"<<std::endl;
        return 0;
    }
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::removeGeomtry(std::string _id)
{
    std::map<std::string,OptiXModel*>::iterator model = m_meshArray.find(_id);
    if(model!=m_meshArray.end())
    {
        m_globalTransGroup->removeChild(model->second->getGeomAndTrans());
        m_globalTransGroup->getAcceleration()->markDirty();
        delete model->second;
        m_meshArray.erase(model);

    }
    else
    {
        std::cerr<<"Error: Could not find model to delete in path tracer"<<std::endl;
    }
}
void PathTracerScene::removeLight(int _id){
    m_globalTransGroup->removeChild(LightManager::getInstance()->getGeomAndTrans()[_id]);
    m_globalTransGroup->getAcceleration()->markDirty();
}

//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::transformModel(std::string _id, glm::mat4 _trans)
{
    std::map<std::string,OptiXModel*>::iterator it = m_meshArray.find(_id);
    OptiXModel* mdl = it->second;
    mdl->setTrans(_trans);
    m_globalTransGroup->getAcceleration()->markDirty();
    m_frame = 0;
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::setModelMaterial(std::string _id, Material _mat){
    std::map<std::string,OptiXModel*>::iterator it = m_meshArray.find(_id);
    OptiXModel* mdl = it->second;
    mdl->setMaterial(_mat);
    m_frame=0;
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::trace()
{
    //if our camera has changed then update it in our engine
    if(m_cameraChanged) updateCamera();

    //launch it
    m_context["frame_number"]->setUint( m_frame++ );
    m_context->launch(0,m_width,m_height);
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::resize(int _width, int _height)
{
//    m_width = _width/m_devicePixelRatio;
    m_width = _width/m_devicePixelRatio;
    m_height = _height/m_devicePixelRatio;

    unsigned int elementSize = m_outputBuffer->getElementSize();
    GLuint handleID = m_outputBuffer->getGLBOId();

    m_outputBuffer->setSize(m_width,m_height);
    m_outputBuffer->unregisterGLBuffer();
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, handleID);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, elementSize * m_width * m_height, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    m_outputBuffer->registerGLBuffer();

    m_frame = 0;
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::updateCamera()
{
    float3 eye,U,V,W;
    m_camera->getEyeUVW(eye,U,V,W);

    //set up our camera in our engine
    m_context["eye"]->setFloat( eye );
    m_context["U"]->setFloat( U );
    m_context["V"]->setFloat( V );
    m_context["W"]->setFloat( W);
    m_frame = 0;
    m_cameraChanged = false;
}
//----------------------------------------------------------------------------------------------------------------------
QImage PathTracerScene::saveImage()
{
    QImage img(m_width,m_height,QImage::Format_RGB32);
    QColor color;
    // as we're using a openGL buffer rather than optix we must map it with openGL calls
    GLint vboId = m_outputBuffer->getGLBOId();
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    void* data = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
    typedef struct { float r; float g; float b; float a;} rgb;
    rgb* rgb_data = (rgb*)data;

    int x;
    int y;
    int h = m_width * m_height;
    for(unsigned int i=0; i<m_width*m_height; i++)
    {
        float red = rgb_data[h-i].r; if(red>1.0) red=1.0;
        float green = rgb_data[h-i].g; if(green>1.0) green=1.0;
        float blue = rgb_data[h-i].b; if(blue>1.0) blue=1.0;
        float alpha = rgb_data[h-i].a; if(alpha>1.0) alpha=1.0;
        color.setRgbF(red,green,blue,alpha);
        y = floor((float)i/m_width);
        x = m_width - i + y*m_width - 1;
        img.setPixel(x, y, color.rgb());

    }
    //remember to unmap the buffer after or you're going to have a bad time!
    glUnmapBuffer(GL_ARRAY_BUFFER);

    return img;
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::setGlobalTrans(glm::mat4 _trans)
{
    // identity matrix to init our transformation
    float m[16];
    m[ 0] = _trans[0][0];  m[ 1] = _trans[1][0];  m[ 2] = _trans[2][0];  m[ 3] = _trans[3][0];
    m[ 4] = _trans[0][1];  m[ 5] = _trans[1][1];  m[ 6] = _trans[2][1];  m[ 7] = _trans[3][1];
    m[ 8] = _trans[0][2];  m[ 9] = _trans[1][2];  m[ 10] = _trans[2][2];  m[ 11] = _trans[3][2];
    m[ 12] = _trans[0][3];  m[ 13] = _trans[1][3];  m[ 14] = _trans[2][3];  m[ 15] = _trans[3][3];
    // create our inverse transform
    float invM[16];
    glm::mat4 inv = glm::inverse(_trans);
    invM[ 0] = inv[0][0];  invM[ 1] = inv[1][0];  invM[ 2] = inv[2][0];  invM[ 3] = inv[3][0];
    invM[ 4] = inv[0][1];  invM[ 5] = inv[1][1];  invM[ 6] = inv[2][1];  invM[ 7] = inv[3][1];
    invM[ 8] = inv[0][2];  invM[ 9] = inv[1][2];  invM[ 10] = inv[2][2];  invM[ 11] = inv[3][2];
    invM[ 12] = inv[0][3];  invM[ 13] = inv[1][3];  invM[ 14] = inv[2][3];  invM[ 15] = inv[3][3];
    // set our transform
    m_globalTrans->setMatrix(false,m,invM);

    // Update the lights buffer used for direct lighting
    LightManager::getInstance()->transformLights(_trans);

    //update our scene
    cleanTopAcceleration();
}
void PathTracerScene::setGlobalTransEnvironment(glm::mat4 _trans)
{
    // identity matrix to init our transformation
    glm::mat4 trans = glm::inverseTranspose(_trans);
    float m[16];
    m[ 0] = trans[0][0];  m[ 1] = trans[1][0];  m[ 2] = trans[2][0];  m[ 3] = trans[3][0];
    m[ 4] = trans[0][1];  m[ 5] = trans[1][1];  m[ 6] = trans[2][1];  m[ 7] = trans[3][1];
    m[ 8] = trans[0][2];  m[ 9] = trans[1][2];  m[ 10] = trans[2][2];  m[ 11] = trans[3][2];
    m[ 12] = trans[0][3];  m[ 13] = trans[1][3];  m[ 14] = trans[2][3];  m[ 15] = trans[3][3];

    m_context["cameraMatrix"]->setMatrix4x4fv(false, m);

    //update our scene
    cleanTopAcceleration();
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::setEnvironmentMap(std::string _environmentMap)
{
    const float3 default_color = make_float3(1.0f, 1.0f, 1.0f);
    m_enviSampler->destroy();
    m_enviSampler = loadHDRTexture(m_context, _environmentMap, default_color);
    m_context["envmap"]->setTextureSampler(m_enviSampler);
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::cleanTopAcceleration()
{
    m_globalTransGroup->getAcceleration()->markDirty();
    m_frame = 0;
}
//----------------------------------------------------------------------------------------------------------------------

