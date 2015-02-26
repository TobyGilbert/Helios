#include "pathtracerscene.h"
#include <QColor>
#include <iostream>
#include "HDRLoader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "TextureLoader.h"
#include "ShaderGlobals.h"


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
PathTracerScene::~PathTracerScene(){
    delete m_camera;
    delete m_model;
    delete m_model2;
    delete m_model3;
    m_outputBuffer->destroy();
    m_lightBuffer->destroy();
    m_mapTexSample->destroy();
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
    // for printing?
    m_context->setStackSize( 1800 );
    // set some variables
    m_context["scene_epsilon"]->setFloat( 1.e-3f );
    // set our ray types
    m_context["pathtrace_ray_type"]->setUint(0u);
    m_context["pathtrace_shadow_ray_type"]->setUint(1u);
    m_context["pathtrace_bsdf_shadow_ray_type"]->setUint(2u);
    m_context["rr_begin_depth"]->setUint(m_rr_begin_depth);

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

    m_camera = new PinholeCamera(optix::make_float3( 0.0f, 25.0f, -50.0f ),      //eye
                                 optix::make_float3( 0.0f, 25.0f, 0.0f ),        //lookat
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
    m_enviSampler = loadHDRTexture(m_context, "./textures/CedarCity.hdr", default_color);
    m_context["envmap"]->setTextureSampler(m_enviSampler);

    // Setup programs
    std::string ptx_path = "ptx/path_tracer.cu.ptx";
    optix::Program ray_gen_program = m_context->createProgramFromPTXFile( ptx_path, "pathtrace_camera" );

    m_context->setRayGenerationProgram( 0, ray_gen_program );
    optix::Program exception_program = m_context->createProgramFromPTXFile( ptx_path, "exception" );
    m_context->setExceptionProgram( 0, exception_program );
    m_context->setMissProgram( 0, m_context->createProgramFromPTXFile( ptx_path, "envi_miss" ) );
//    m_context->setMissProgram(0, m_context->createProgramFromPTXFile(ptx_path, "miss"));

    //init our frame number
    m_context["frame_number"]->setUint(1);

    // Index of sampling_stategy (BSDF, light, MIS)
    m_sampling_strategy = 0;
    m_context["sampling_stategy"]->setInt(m_sampling_strategy);

    //create our top group and set it in our engine
    m_topGroup = m_context->createGroup();
    m_context["top_object"]->set(m_topGroup);
    m_topGroup->setAcceleration(m_context->createAcceleration("Bvh","Bvh"));

    // Our our map texture sample
    m_mapTexSample = loadTexture(m_context, "textures/map.png");

    // Create scene geometry
    createGeometry();

    // Finalize
    m_context->validate();
    m_context->compile();
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::createGeometry(){
    // Light buffer
      ParallelogramLight light;
      light.corner   = make_float3( 10.0f, 60.9f, 70.0f);
      light.v1       = make_float3( -20.0f, 0.0f, 0.0f);
      light.v2       = make_float3( 0.0f, 0.0f, 10.0f);
      light.normal   = normalize( cross(light.v1, light.v2) );
      light.emission = make_float3( 25.0f, 25.0f, 25.0f );

      m_lightBuffer = m_context->createBuffer( RT_BUFFER_INPUT );
      m_lightBuffer->setFormat( RT_FORMAT_USER );
      m_lightBuffer->setElementSize( sizeof( ParallelogramLight ) );
      m_lightBuffer->setSize( 1u );
      memcpy( m_lightBuffer->map(), &light, sizeof( light ) );
      m_lightBuffer->unmap();
      m_context["lights"]->setBuffer( m_lightBuffer );

      // Set up diffuse material
      Material diffuse = m_context->createMaterial();
      std::string ptx_path = "ptx/path_tracer.cu.ptx";
      Program diffuse_ch = m_context->createProgramFromPTXFile( ptx_path, "diffuse" );
      Program diffuse_ah = m_context->createProgramFromPTXFile( ptx_path, "shadow" );
      diffuse->setClosestHitProgram( 0, diffuse_ch );
      diffuse->setAnyHitProgram( 1, diffuse_ah );

      Material diffuse_light = m_context->createMaterial();
      Program diffuse_em = m_context->createProgramFromPTXFile( ptx_path, "diffuseEmitter" );
      diffuse_light->setClosestHitProgram( 0, diffuse_em );

      // Shader Globals material
      Material shader_globals = m_context->createMaterial();
      Program shader_globals_ch = m_context->createProgramFromPTXFile(ptx_path, "constructShaderGlobals");
      Program shader_globals_ah = m_context->createProgramFromPTXFile(ptx_path, "shadow");
      shader_globals->setClosestHitProgram(0, shader_globals_ch);
      shader_globals->setAnyHitProgram(1, shader_globals_ah);

      // Reflective Material
      Material reflective_material = m_context->createMaterial();
      Program reflective_ch = m_context->createProgramFromPTXFile( ptx_path, "reflections");
      Program reflective_ah = m_context->createProgramFromPTXFile( ptx_path, "shadow");
      reflective_material->setClosestHitProgram(0, reflective_ch);
      reflective_material->setAnyHitProgram(1, reflective_ah);

      // Glass material
      Material glass_material = m_context->createMaterial();
      Program glass_ch = m_context->createProgramFromPTXFile( ptx_path, "glass_refract");
      Program glass_ah = m_context->createProgramFromPTXFile( ptx_path, "shadow");
      glass_material->setClosestHitProgram(0, glass_ch);
      glass_material->setAnyHitProgram(1, glass_ah);

      // Set up parallelogram programs
      ptx_path = "ptx/parallelogram.cu.ptx";
      m_pgram_bounding_box = m_context->createProgramFromPTXFile( ptx_path, "bounds" );
      m_pgram_intersection = m_context->createProgramFromPTXFile( ptx_path, "intersect" );

      // Set up sphere programs
      ptx_path = "ptx/sphere.cu.ptx";
      m_pgram_bounding_sphere = m_context->createProgramFromPTXFile(ptx_path, "bounds_sphere");
      m_pgram_sphereIntersection = m_context->createProgramFromPTXFile(ptx_path, "intersect_sphere");

      // create geometry instances
      std::vector<GeometryInstance> gis;

      const float3 white = make_float3( 1.0f, 1.0f, 1.0f );
      const float3 green = make_float3( 0.05f, 0.8f, 0.05f );
      const float3 red   = make_float3( 0.8f, 0.05f, 0.05f );
      const float3 light_em = make_float3( 5.0f, 5.0f, 5.0f );
      const float3 blue = make_float3( 0.0, 0.0, 1.0);

      // Diffuse Sphere
//      gis.push_back(createSphere(make_float4(0.0, 30.0, 70.0, 8.0)));
//      gis.back()->addMaterial(diffuse);
//      gis.back()["diffuse_color"]->setFloat(white);
//      gis.back()["map_texture"]->setTextureSampler(m_mapTexSample);

//      // Glass Sphere
//      gis.push_back( createSphere(make_float4(20.0, 30.0, 70.0, 8.0)));
//      gis.back()->addMaterial(glass_material);
//      gis.back()["glass_color"]->setFloat(white);
//      gis.back()["index_of_refraction"]->setFloat(1.5);

//      // Metal Sphere
//      gis.push_back( createSphere(make_float4(0.0, 30.0, 70.0, 8.0)));
//      gis.back()->addMaterial(reflective_material);
//      gis.back()["diffuse_color"]->setFloat(white);
//      gis.back()["max_depth"]->setInt(3);


      // Floor
      gis.push_back( createParallelogram( make_float3( -50.0f, 5.0f, 100.0f ),
                                          make_float3( 0.0f, 0.0f, -200.0f ),
                                          make_float3( 100.0f, 0.0f, 0.0f ) ) );
      gis.back()->addMaterial(reflective_material);

////      // Ceiling
//      gis.push_back( createParallelogram( make_float3( 25.0f, 50.0f, 75.0f ),
//                                          make_float3( -50.0f, 0.0f, 0.0f ),
//                                          make_float3( 0.0f, 0.0f, -50.0f ) ) );
//      gis.back()->addMaterial(reflective_material);

      // Back wall
      gis.push_back( createParallelogram( make_float3( -25.0f, 0.0f, 75.0f),
                                          make_float3( 0.0f, 50.0f, 0.0f),
                                          make_float3( 50.0f, 0.0f, 0.0f) ) );
      gis.back()->addMaterial(reflective_material);

//      // Right wall
//      gis.push_back( createParallelogram( make_float3( -25.0f, 0.0f, 25.0f ),
//                                          make_float3( 0.0f, 50.0f, 0.0f ),
//                                          make_float3( 0.0f, 0.0f, 50.0f ) ) );
////      setMaterial(gis.back(), diffuse, "diffuse_color", green);
//      gis.back()->addMaterial(reflective_material);


//      // Left wall
//      gis.push_back( createParallelogram( make_float3( 25.0f, 50.0f, 75.0f ),
//                                          make_float3( 0.0f, 0.0f, -50.0f ),
//                                          make_float3( 0.0f, -50.0f, 0.0f ) ) );
////      setMaterial(gis.back(), diffuse, "diffuse_color", blue);
//      gis.back()->addMaterial(reflective_material);

      // Create shadow group (no light)
      GeometryGroup shadow_group = m_context->createGeometryGroup(gis.begin(), gis.end());
      shadow_group->setAcceleration( m_context->createAcceleration("Bvh","Bvh") );
      m_context["top_shadower"]->set( shadow_group );

      gis.push_back( createParallelogram( make_float3( 10.0f, 49.99f, 50.0f),
                                          make_float3( -20.0f, 0.0f, 0.0f),
                                          make_float3( 0.0f, 0.0f, 10.0f) ) );
      setMaterial(gis.back(), diffuse_light, "emission_color", light_em);

      // Create geometry group
      GeometryGroup geometry_group = m_context->createGeometryGroup(gis.begin(), gis.end());

      // Metal teapot
      m_model = new OptiXModel("models/newteapot.obj",m_context);
      m_model->addMaterial(reflective_material);
      glm::mat4 trans;
      trans = glm::scale(trans,glm::vec3(13.0));
      trans[3][0] = -23;
      trans[3][1] = 10;
      trans[3][2] = 50;
      m_model->setTrans(trans);

      // Glass Teapot
      m_model2 = new OptiXModel(m_model,m_context);
//      m_model2->createGeometry("models/newteapot.obj",m_context);
      m_model2->addMaterial(glass_material);
      m_model2->getGeometryInstance()["glass_color"]->setFloat(white);
      m_model2->getGeometryInstance()["index_of_refraction"]->setFloat(1.5);
      trans = glm::mat4(1.0);
      trans = glm::scale(trans,glm::vec3(13.0));
      trans[3][0] = 23;
      trans[3][1] = 10;
      trans[3][2] = 50;
      m_model2->setTrans(trans);

      // Diffuse teapot
      m_model3 = new OptiXModel("models/teapot.obj",m_context);
      m_model3->addMaterial(diffuse);
      m_model3->getGeometryInstance()["diffuse_color"]->setFloat(white);
      m_model3->getGeometryInstance()["map_texture"]->setTextureSampler(loadTexture( m_context, "textures/map.png") );
      trans = glm::mat4(1.0);
      trans = glm::scale(trans,glm::vec3(13.0));
      trans[3][0] = 0;
      trans[3][1] = 10;
      trans[3][2] = 50;
      m_model3->setTrans(trans);

      geometry_group->setAcceleration( m_context->createAcceleration("Bvh","Bvh") );

      m_topGroup->addChild(geometry_group);
      m_topGroup->addChild(m_model->getGeomAndTrans());
      m_topGroup->addChild(m_model2->getGeomAndTrans());
      m_topGroup->addChild(m_model3->getGeomAndTrans());
      m_topGroup->setAcceleration(m_context->createAcceleration("Bvh","Bvh"));
      m_topGroup->getAcceleration()->markDirty();
}
//----------------------------------------------------------------------------------------------------------------------
optix::GeometryInstance PathTracerScene::createParallelogram(const float3 &anchor, const float3 &offset1, const float3 &offset2){
    optix::Geometry parallelogram = m_context->createGeometry();
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

    optix::GeometryInstance gi = m_context->createGeometryInstance();
    gi->setGeometry(parallelogram);
    return gi;
}
//----------------------------------------------------------------------------------------------------------------------
optix::GeometryInstance PathTracerScene::createLightParallelogram(const float3 &anchor, const float3 &offset1, const float3 &offset2, int lgt_instance){
    optix::Geometry parallelogram = m_context->createGeometry();
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
    parallelogram["lgt_instance"]->setInt( lgt_instance );

    optix::GeometryInstance gi = m_context->createGeometryInstance();
    gi->setGeometry(parallelogram);
    return gi;
}
//----------------------------------------------------------------------------------------------------------------------
optix::GeometryInstance PathTracerScene::createSphere(const float4 &sphereLoc){
    Geometry sphere = m_context->createGeometry();

    sphere->setPrimitiveCount( 1u );
    sphere->setIntersectionProgram( m_pgram_sphereIntersection );
    sphere->setBoundingBoxProgram( m_pgram_bounding_sphere );

    sphere["sphere"]->setFloat( sphereLoc );

    GeometryInstance gi = m_context->createGeometryInstance();
    gi->setGeometry(sphere);

    return gi;
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::setMaterial(optix::GeometryInstance &gi, optix::Material material, const std::string &color_name, const float3 &color){
    gi->addMaterial(material);
    gi[color_name]->setFloat(color);
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::importMesh(int _id, std::string _path){
    /// @todo maybe have all this stuff in a model management class rather than the scene
    /// @todo meshes are all set with detault diffuse texture, we need some sort of material management
    //import mesh
    OptiXModel* model = new OptiXModel(_path,m_context);
    Material diffuse = m_context->createMaterial();
    std::string ptx_path = "ptx/path_tracer.cu.ptx";
    Program diffuse_ch = m_context->createProgramFromPTXFile( ptx_path, "diffuse" );
    Program diffuse_ah = m_context->createProgramFromPTXFile( ptx_path, "shadow" );
    diffuse->setClosestHitProgram( 0, diffuse_ch );
    diffuse->setAnyHitProgram( 1, diffuse_ah );
    diffuse["diffuse_color"]->setFloat(1.0,1.0,1.0);
    diffuse["map_texture"]->setTextureSampler(loadTexture( m_context, "textures/map.png") );
    model->addMaterial(diffuse);
    //add to our scene
    std::cout<<"has been called path: "<<_path<<std::endl;
    m_topGroup->addChild(model->getGeomAndTrans());
    m_topGroup->getAcceleration()->markDirty();
    m_meshArray[_id] = model;
    m_frame = 0;
}
//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::transformModel(int _id, glm::mat4 _trans){
    std::map<int,OptiXModel*>::iterator it = m_meshArray.find(_id);
    OptiXModel* mdl = it->second;
    mdl->setTrans(_trans);
    m_topGroup->getAcceleration()->markDirty();
    m_frame = 0;
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
