
#define GLM_FORCE_RADIANS
#include "Core/pathtracerscene.h"
#include <QColor>
#include <iostream>
#include "HDRLoader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Core/TextureLoader.h"
#include "Lights/LightManager.h"

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
PathTracerScene::PathTracerScene()  : m_rr_begin_depth(1u)
                                    , m_sqrt_num_samples( 2u )
                                    , m_width(512)
                                    , m_height(512)
                                    , m_frame(0)
                                    , m_cameraChanged(false)
{
    // create an instance of our OptiX engine
    m_context = optix::Context::create();
}
//----------------------------------------------------------------------------------------------------------------------
PathTracerScene::~PathTracerScene(){
    delete m_camera;
    m_outputBuffer->destroy();
    m_lightBuffer->destroy();
    m_enviSampler->destroy();
    m_context->destroy();
}

//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::init(){
    // how many ray types we have
    // we have our light ray, shadow ray and a bsdf shadow ray
    m_context->setRayTypeCount( 3 );
    // we only have one entry point which is our output buffer
    m_context->setEntryPointCount( 1 );
    // sets the stack size important for recuerssion
    m_context->setStackSize( 4000 );
    // set some variables
    m_context["scene_epsilon"]->setFloat( 1.e-3f );
    // set our ray types
    m_context["pathtrace_ray_type"]->setUint(0u);
    m_context["pathtrace_shadow_ray_type"]->setUint(1u);
    m_context["pathtrace_bsdf_shadow_ray_type"]->setUint(2u);
    m_context["rr_begin_depth"]->setUint(m_rr_begin_depth);

    // Enable printing
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

    m_camera = new PinholeCamera(optix::make_float3( 0.0f, 0.0f, -25.0f ),      //eye
                                 optix::make_float3( 0.0f, 0.0f, 0.0f ),        //lookat
                                 optix::make_float3( 0.0f, 1.0f,  0.0f ),        //up
                                 35.0f,                                          //hfov
                                 35.0f);                                         //vfov

    float3 eye,U,V,W;
    m_camera->getEyeUVW(eye,U,V,W);

    //set up our camera in our engine
    m_context["eye"]->setFloat( eye );
    m_context["U"]->setFloat( U );
    m_context["V"]->setFloat( V );
    m_context["W"]->setFloat( W);

    m_context["sqrt_num_samples"]->setUint( m_sqrt_num_samples );
    m_context["bad_color"]->setFloat( 0.0f, 1.0f, 0.0f );
    m_context["bg_color"]->setFloat( make_float3(0.0f) );
    const float3 default_color = make_float3(1.0f, 1.0f, 1.0f);
    m_enviSampler = loadHDRTexture(m_context, "./HDRMaps/Refmap.hdr", default_color);
    m_context["envmap"]->setTextureSampler(m_enviSampler);

    // Load normalmap
    optix::TextureSampler normalMap = loadTexture(m_context, "./textures/normalMap2.jpg");
    m_context["normalMap"]->setTextureSampler(normalMap);

    // Setup programs
    std::string ptx_path = "ptx/path_tracer.cu.ptx";
    optix::Program ray_gen_program = m_context->createProgramFromPTXFile( ptx_path, "pathtrace_camera" );

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


    //create our top group and set it in our engine
    m_topGroup = m_context->createGroup();
    m_context["top_object"]->set(m_topGroup);
    m_topGroup->setAcceleration(m_context->createAcceleration("Bvh","Bvh"));

    // Finalize
    m_context->validate();
    m_context->compile();
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::addLight(){
    LightManager::getInstance()->createParollelogramLight();
    m_context["lights"]->setBuffer( LightManager::getInstance()->getLightsBuffer() );

    // Only add the last one on the vector to avoid duplicates
    m_topGroup->addChild(LightManager::getInstance()->getGeomAndTrans().back());

    m_topGroup->getAcceleration()->markDirty();
    m_frame = 0;
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::importMesh(std::string _id, std::string _path){
    /// @todo maybe have all this stuff in a model management class rather than the scene
    /// @todo meshes are all set with detault diffuse texture, we need some sort of material management
    //import mesh
    OptiXModel* model = new OptiXModel(_path,m_context);
    Material defaultMaterial = m_context->createMaterial();

    std::string ptx_path = "ptx/path_tracer.cu.ptx";
    Program default_ch = m_context->createProgramFromPTXFile( ptx_path, "defaultMaterial" );
    Program default_ah = m_context->createProgramFromPTXFile( ptx_path, "shadow" );
    defaultMaterial->setClosestHitProgram( 0, default_ch );
    defaultMaterial->setAnyHitProgram( 1, default_ah );
    model->setMaterial(defaultMaterial);
    //add to our scene
    std::cout<<"has been called path: "<<_path<<std::endl;
    m_topGroup->addChild(model->getGeomAndTrans());
    m_topGroup->getAcceleration()->markDirty();
    m_meshArray[_id] = model;
    m_frame = 0;
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::transformModel(std::string _id, glm::mat4 _trans){
    std::map<std::string,OptiXModel*>::iterator it = m_meshArray.find(_id);
    OptiXModel* mdl = it->second;
    mdl->setTrans(_trans);
    m_topGroup->getAcceleration()->markDirty();
    m_frame = 0;
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::setModelMaterial(std::string _id, Material &_mat){
    std::map<std::string,OptiXModel*>::iterator it = m_meshArray.find(_id);
    OptiXModel* mdl = it->second;
    mdl->setMaterial(_mat);
    m_frame=0;
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::trace(){
    //if our camera has changed then update it in our engine
    if(m_cameraChanged) updateCamera();

    //launch it
    m_context["frame_number"]->setUint( m_frame++ );
    m_context->launch(0,m_width,m_height);
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::resize(int _width, int _height){
//    m_width = _width/m_devicePixelRatio;
    m_width = _height/m_devicePixelRatio;
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
void PathTracerScene::updateCamera(){
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
QImage PathTracerScene::saveImage(){
    QImage img(m_width,m_height,QImage::Format_RGB32);
    QColor color;
    int idx;
    // as we're using a openGL buffer rather than optix we must map it with openGL calls
    GLint vboId = m_outputBuffer->getGLBOId();
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    void* data = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
    typedef struct { float r; float g; float b; float a;} rgb;
    rgb* rgb_data = (rgb*)data;
    if(rgb_data[0].r>0||rgb_data[0].g>0||rgb_data[0].b>0)

    for(unsigned int i=0; i<m_width*m_height; i++){
        int h = m_width * m_height;
        float red = rgb_data[h-i].r; if(red>1.0) red=1.0;
        float green = rgb_data[h-i].g; if(green>1.0) green=1.0;
        float blue = rgb_data[h-i].b; if(blue>1.0) blue=1.0;
        float alpha = rgb_data[h-i].a; if(alpha>1.0) alpha=1.0;
        color.setRgbF(red,green,blue,alpha);
        idx = floor((float)i/m_width);

        img.setPixel(i-(idx*m_width), idx, color.rgb());

    }
    //remember to unmap the buffer after or you're going to have a bad time!
    glUnmapBuffer(GL_ARRAY_BUFFER);

    return img;
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::setEnvironmentMap(std::string _environmentMap){
    const float3 default_color = make_float3(1.0f, 1.0f, 1.0f);
    m_enviSampler->destroy();
    m_enviSampler = loadHDRTexture(m_context, _environmentMap, default_color);
    m_context["envmap"]->setTextureSampler(m_enviSampler);
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::cleanTopAcceleration(){
    m_topGroup->getAcceleration()->markDirty();
    m_frame = 0;
}
