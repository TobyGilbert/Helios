#include "pathtracerscene.h"
#include "pinholecamera.h"
#include <QColor>
#include <iostream>
#include "HDRLoader.h"

PathTracerScene::PathTracerScene()  : m_rr_begin_depth(1u)
                                    , m_sqrt_num_samples( 2u )
                                    , m_width(512)
                                    , m_height(512)
                                    , m_frame(0)
{
    // create an instance of our OptiX engine
    m_context = optix::Context::create();
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
//    m_outputBuffer = m_context->createBuffer(RT_BUFFER_OUTPUT,RT_FORMAT_FLOAT4, m_width, m_height);
    output_buffer->set(m_outputBuffer);

    PinholeCamera camera(optix::make_float3( 278.0f, 273.0f, -800.0f ),  //eye
                         optix::make_float3( 278.0f, 273.0f, 0.0f ),     //lookat
                         optix::make_float3( 0.0f, 1.0f,  0.0f ),        //up
                         35.0f,                                          //hfov
                         35.0f);                                         //vfov

    float3 eye,U,V,W;
    camera.getEyeUVW(eye,U,V,W);

    //set up our camera in our engine
    m_context["eye"]->setFloat( eye );
    m_context["U"]->setFloat( U );
    m_context["V"]->setFloat( V );
    m_context["W"]->setFloat( W);

//    std::cout<<"Camera paramiters: "<<std::endl;
//    std::cout<<"eye "<<eye.x<<","<<eye.y<<","<<eye.z<<std::endl;
//    std::cout<<"U "<<U.x<<","<<U.y<<","<<U.z<<std::endl;
//    std::cout<<"V "<<V.x<<","<<V.y<<","<<V.z<<std::endl;
//    std::cout<<"W "<<W.x<<","<<W.y<<","<<W.z<<std::endl;

    m_context["sqrt_num_samples"]->setUint( m_sqrt_num_samples );
    m_context["bad_color"]->setFloat( 0.0f, 1.0f, 0.0f );
    m_context["bg_color"]->setFloat( optix::make_float3(0.0f) );
    const float3 default_color = make_float3(1.0f, 1.0f, 1.0f);
    m_context["envmap"]->setTextureSampler(loadHDRTexture( m_context, "./textures/CedarCity.hdr", default_color) );

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
      light.corner   = make_float3( 343.0f, 548.6f, 227.0f);
      light.v1       = make_float3( -130.0f, 0.0f, 0.0f);
      light.v2       = make_float3( 0.0f, 0.0f, 105.0f);
      light.normal   = normalize( cross(light.v1, light.v2) );
      light.emission = make_float3( 15.0f, 15.0f, 15.0f );

      Buffer light_buffer = m_context->createBuffer( RT_BUFFER_INPUT );
      light_buffer->setFormat( RT_FORMAT_USER );
      light_buffer->setElementSize( sizeof( ParallelogramLight ) );
      light_buffer->setSize( 1u );
      memcpy( light_buffer->map(), &light, sizeof( light ) );
      light_buffer->unmap();
      m_context["lights"]->setBuffer( light_buffer );

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

      // Reflective Material
      Material reflective_material = m_context->createMaterial();
      Program reflective_ch = m_context->createProgramFromPTXFile( ptx_path, "reflections");
      Program reflective_ah = m_context->createProgramFromPTXFile( ptx_path, "shadow");
      reflective_material->setClosestHitProgram(0, reflective_ch);
      reflective_material->setAnyHitProgram(1, reflective_ah);

      // Procedural floor material
      Material procedural_floor_material = m_context->createMaterial();
      Program procedural_ch = m_context->createProgramFromPTXFile(ptx_path, "procedural_floor");
      Program procedural_ah = m_context->createProgramFromPTXFile(ptx_path, "shadow");
      procedural_floor_material->setClosestHitProgram(0, procedural_ch);
      procedural_floor_material->setAnyHitProgram(1, procedural_ah);

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
      const float3 light_em = make_float3( 15.0f, 15.0f, 15.0f );

      // Sphere
      gis.push_back( createSphere(make_float4(100.0, 250.0, 250.0, 100.0)));
      gis.back()->addMaterial(reflective_material);
      gis.back()["diffuse_color"]->setFloat(white);
      gis.back()["reflectivity"]->setFloat(1.0);
      gis.back()["max_depth"]->setInt(3);

      // Floor
      gis.push_back( createParallelogram( make_float3( -1000.0f, 0.0f, 0.0f ),
                                          make_float3( 0.0f, 0.0f, 2000.2f ),
                                          make_float3( 2000.0f, 0.0f, 0.0f ) ) );
//      setMaterial(gis.back(), diffuse, "diffuse_color", white);
      gis.back()->addMaterial(reflective_material);
      gis.back()["diffuse_color"]->setFloat(white);
      gis.back()["reflectivity"]->setFloat(1.0);
      gis.back()["max_depth"]->setInt(3);

//      // Ceiling
//      gis.push_back( createParallelogram( make_float3( 0.0f, 548.8f, 0.0f ),
//                                          make_float3( 556.0f, 0.0f, 0.0f ),
//                                          make_float3( 0.0f, 0.0f, 559.2f ) ) );
//      setMaterial(gis.back(), diffuse, "diffuse_color", white);

////      // Back wall
//      gis.push_back( createParallelogram( make_float3( 0.0f, 0.0f, 559.2f),
//                                          make_float3( 0.0f, 548.8f, 0.0f),
//                                          make_float3( 556.0f, 0.0f, 0.0f) ) );
//      setMaterial(gis.back(), diffuse, "diffuse_color", white);

//      // Right wall
//      gis.push_back( createParallelogram( make_float3( 0.0f, 0.0f, 0.0f ),
//                                          make_float3( 0.0f, 548.8f, 0.0f ),
//                                          make_float3( 0.0f, 0.0f, 559.2f ) ) );
//      setMaterial(gis.back(), diffuse, "diffuse_color", green);

//      // Left wall
//      gis.push_back( createParallelogram( make_float3( 556.0f, 0.0f, 0.0f ),
//                                          make_float3( 0.0f, 0.0f, 559.2f ),
//                                          make_float3( 0.0f, 548.8f, 0.0f ) ) );
//      setMaterial(gis.back(), diffuse, "diffuse_color", red);

//      // Short block
//      gis.push_back( createParallelogram( make_float3( 130.0f, 165.0f, 65.0f),
//                                          make_float3( -48.0f, 0.0f, 160.0f),
//                                          make_float3( 160.0f, 0.0f, 49.0f) ) );
//      setMaterial(gis.back(), diffuse, "diffuse_color", white);
//      gis.push_back( createParallelogram( make_float3( 290.0f, 0.0f, 114.0f),
//                                          make_float3( 0.0f, 165.0f, 0.0f),
//                                          make_float3( -50.0f, 0.0f, 158.0f) ) );
//      setMaterial(gis.back(), diffuse, "diffuse_color", white);
//      gis.push_back( createParallelogram( make_float3( 130.0f, 0.0f, 65.0f),
//                                          make_float3( 0.0f, 165.0f, 0.0f),
//                                          make_float3( 160.0f, 0.0f, 49.0f) ) );
//      setMaterial(gis.back(), diffuse, "diffuse_color", white);
//      gis.push_back( createParallelogram( make_float3( 82.0f, 0.0f, 225.0f),
//                                          make_float3( 0.0f, 165.0f, 0.0f),
//                                          make_float3( 48.0f, 0.0f, -160.0f) ) );
//      setMaterial(gis.back(), diffuse, "diffuse_color", white);
//      gis.push_back( createParallelogram( make_float3( 240.0f, 0.0f, 272.0f),
//                                          make_float3( 0.0f, 165.0f, 0.0f),
//                                          make_float3( -158.0f, 0.0f, -47.0f) ) );
//      setMaterial(gis.back(), diffuse, "diffuse_color", white);

      // Tall block
      gis.push_back( createParallelogram( make_float3( 423.0f, 330.0f, 247.0f),
                                          make_float3( -158.0f, 0.0f, 49.0f),
                                          make_float3( 49.0f, 0.0f, 159.0f) ) );
//      setMaterial(gis.back(), diffuse, "diffuse_color", white);
      gis.back()->addMaterial(reflective_material);
      gis.back()["diffuse_color"]->setFloat(white);
      gis.back()["reflectivity"]->setFloat(1.0);
      gis.back()["max_depth"]->setInt(5);

      gis.push_back( createParallelogram( make_float3( 423.0f, 0.0f, 247.0f),
                                          make_float3( 0.0f, 330.0f, 0.0f),
                                          make_float3( 49.0f, 0.0f, 159.0f) ) );
//      setMaterial(gis.back(), diffuse, "diffuse_color", white);
      gis.back()->addMaterial(reflective_material);
      gis.back()["diffuse_color"]->setFloat(white);
      gis.back()["reflectivity"]->setFloat(1.0);
      gis.back()["max_depth"]->setInt(5);
      gis.push_back( createParallelogram( make_float3( 472.0f, 0.0f, 406.0f),
                                          make_float3( 0.0f, 330.0f, 0.0f),
                                          make_float3( -158.0f, 0.0f, 50.0f) ) );
//      setMaterial(gis.back(), diffuse, "diffuse_color", white);
      gis.back()->addMaterial(reflective_material);
      gis.back()["diffuse_color"]->setFloat(white);
      gis.back()["reflectivity"]->setFloat(1.0);
      gis.back()["max_depth"]->setInt(5);
      gis.push_back( createParallelogram( make_float3( 314.0f, 0.0f, 456.0f),
                                          make_float3( 0.0f, 330.0f, 0.0f),
                                          make_float3( -49.0f, 0.0f, -160.0f) ) );
//      setMaterial(gis.back(), diffuse, "diffuse_color", white);
      gis.back()->addMaterial(reflective_material);
      gis.back()["diffuse_color"]->setFloat(white);
      gis.back()["reflectivity"]->setFloat(1.0);
      gis.back()["max_depth"]->setInt(5);
      gis.push_back( createParallelogram( make_float3( 265.0f, 0.0f, 296.0f),
                                          make_float3( 0.0f, 330.0f, 0.0f),
                                          make_float3( 158.0f, 0.0f, -49.0f) ) );
//      setMaterial(gis.back(), diffuse, "diffuse_color", white);
      gis.back()->addMaterial(reflective_material);
      gis.back()["diffuse_color"]->setFloat(white);
      gis.back()["reflectivity"]->setFloat(1.0);
      gis.back()["max_depth"]->setInt(5);

      // Create shadow group (no light)
      GeometryGroup shadow_group = m_context->createGeometryGroup(gis.begin(), gis.end());
      shadow_group->setAcceleration( m_context->createAcceleration("Bvh","Bvh") );
      m_context["top_shadower"]->set( shadow_group );

      // Light
//      gis.push_back( createParallelogram( make_float3(348.0f, 548.6f, 250.0f),
//                                          make_float3( -130.0f, 0.0f, 0.0f),
//                                          make_float3( 0.0f, 0.0f, 105.0f) ) );
//      setMaterial(gis.back(), diffuse_light, "emission_color", light_em);

      // Create geometry group
      GeometryGroup geometry_group = m_context->createGeometryGroup(gis.begin(), gis.end());
      geometry_group->setAcceleration( m_context->createAcceleration("Bvh","Bvh") );
      m_context["top_object"]->set( geometry_group );
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
void PathTracerScene::trace(){
    //launch it

    m_context["frame_number"]->setUint( m_frame++ );
    m_context->launch(0,m_width,m_height);

//    QImage img(m_width,m_height,QImage::Format_RGB32);
//    QColor color;
//    int idx;
//    void* data = m_outputBuffer->map();
//    typedef struct { float r; float g; float b; float a;} rgb;
//    rgb* rgb_data = (rgb*)data;
//    if(rgb_data[0].r>0||rgb_data[0].g>0||rgb_data[0].b>0)
////    std::cout<<rgb_data[0].r<<","<<rgb_data[0].g<<","<<rgb_data[0].b<<std::endl;

//    for(unsigned int i=0; i<m_width*m_height; ++i){
////        std::cout<<rgb_data[i].r<<","<<rgb_data[i].g<<","<<rgb_data[i].b<<std::endl;
//        float red = rgb_data[i].r; if(red>1.0) red=1.0;
//        float green = rgb_data[i].g; if(green>1.0) green=1.0;
//        float blue = rgb_data[i].b; if(blue>1.0) blue=1.0;
//        float alpha = rgb_data[i].a; if(alpha>1.0) alpha=1.0;
//        color.setRgbF(red,green,blue,alpha);
//        idx = floor((float)i/m_height);

//        img.setPixel(i-(idx*m_height), idx, color.rgb());

//    }
//    m_outputBuffer->unmap();
//    img.save("pathTraceTest.png","PNG");

//    return img;
}

//----------------------------------------------------------------------------------------------------------------------
void PathTracerScene::resize(int _width, int _height){
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
