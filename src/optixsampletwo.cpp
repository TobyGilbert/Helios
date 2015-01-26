#include "include/optixsampletwo.h"
#include <iostream>
#include <QColor>
#include <math.h>
#include "commonStructs.h"

//----------------------------------------------------------------------------------------------------------------------
OptixSampleTwo::OptixSampleTwo()
{
    m_numBoxes = 6;
    m_context = optix::Context::create();
}
//----------------------------------------------------------------------------------------------------------------------
OptixSampleTwo::~OptixSampleTwo(){
    //clean up
    m_outputBuffer->destroy();
    m_context->destroy();
    m_context = 0;
}

//----------------------------------------------------------------------------------------------------------------------
void OptixSampleTwo::init(){
    //set how many ray types we have
    //in this sample we have 2 shadow and radiance
    m_context->setRayTypeCount(2);
    //set how many entry points we have
    m_context->setEntryPointCount(1);

    //set our variables, this also creates our variables *gasps*
    m_context["max_depth"]->setInt(10u);
    m_context["radiance_ray_type"]->setUint(0u);
    m_context["shadow_ray_type"]->setUint(1u);
    m_context["scene_epsilon"]->setFloat(1.e-4f);

    //create our buffere object
    m_outputBuffer = m_context->createBuffer(RT_BUFFER_OUTPUT,RT_FORMAT_FLOAT4,m_width,m_height);
    //link our buffer object to our variable
    m_context["output_buffer"]->set(m_outputBuffer);

    //set up our light
    BasicLight light;
    light.color.x = 0.9f;
    light.color.y = 0.9f;
    light.color.z = 0.9f;
    light.pos.x   = 0.0f;
    light.pos.y   = 20.0f;
    light.pos.z   = 20.0f;
    light.casts_shadow = 1;
    light.padding      = 0u;

    //create our light buffer
    m_light_buffer_obj = m_context->createBuffer(RT_BUFFER_INPUT,RT_FORMAT_USER,1);
    m_light_buffer_obj->setElementSize(sizeof(BasicLight));
    //input our light buffer data to our buffer
    void *light_buffer_data = m_light_buffer_obj->map();
    ((BasicLight*)light_buffer_data)[0] = light;
    //release our pointer free into the wild
    m_light_buffer_obj->unmap();
    //attach our light variable to our buffer
    m_context["lights"]->set(m_light_buffer_obj);

    //create our ray gen program
    optix::Program ray_gen_program = m_context->createProgramFromPTXFile("ptx/pinhole_camera.cu.ptx","pinhole_camera");
    //set the variables in our ray gen program
    ray_gen_program["eye"]->setFloat(0.0f,0.0f,5.0f);
    ray_gen_program["U"]->setFloat(1.0f,0.0f,0.0f);
    ray_gen_program["V"]->setFloat(0.0f,1.0f,0.0f);
    ray_gen_program["W"]->setFloat(0.0f,0.0f,-1.0f);
    //set it in our context as our ray gen program
    m_context->setRayGenerationProgram(0,ray_gen_program);

    //set our miss program
    optix::Program miss_program = m_context->createProgramFromPTXFile("ptx/constantbg.cu.ptx","miss");
    //set our miss colour
    miss_program["bg_color"]->setFloat(0.3f,0.1f,0.2f);
    m_context->setMissProgram(0,miss_program);

    //get our box geomtetry
    optix::Geometry box = createGeomtry();

    //get our material
    optix::Material material = createMaterial();

    //create our geomtry instaces
    createGeometryInstances(box,material);

    //validate our context
    m_context->validate();
    //compile our context
    m_context->compile();

}
//----------------------------------------------------------------------------------------------------------------------
optix::Geometry OptixSampleTwo::createGeomtry(){
    optix::Program box_intersection_program;
    optix::Program box_bounding_box_program;

    //create our geometry in our engine
    optix::Geometry box = m_context->createGeometry();
    //set our how many primatives we have
    box->setPrimitiveCount(1u);

    //create our intersection and bourding programs
    box_intersection_program = m_context->createProgramFromPTXFile("ptx/box.cu.ptx", "box_intersect");
    box_bounding_box_program = m_context->createProgramFromPTXFile("ptx/box.cu.ptx", "box_bounds");

    //set our intersection and bounding programs
    box->setIntersectionProgram(box_intersection_program);
    box->setBoundingBoxProgram(box_bounding_box_program);

    //set our variables in our box program
    box->declareVariable("boxmin");
    box->declareVariable("boxmax");
    box["boxmin"]->setFloat(-0.5f,-0.5f,-0.5f);
    box["boxmax"]->setFloat(0.5f,0.5f,0.5f);

    //now we're done return our geometry
    return box;
}


//----------------------------------------------------------------------------------------------------------------------
optix::Material OptixSampleTwo::createMaterial(){
    //create our closest hit and any hit programs
    optix::Program closest_hit_program = m_context->createProgramFromPTXFile("ptx/phong.cu.ptx","closest_hit_radiance");
    optix::Program any_hit_program = m_context->createProgramFromPTXFile("ptx/phong.cu.ptx","any_hit_shadow");

    //create a material
    optix::Material material = m_context->createMaterial();

    //set our closest hit and any hit programs in our material
    //some nvidia notes
    /* Note that we are leaving anyHitProgram[0] and closestHitProgram[1] as NULL.
     * This is because our radiance rays only need closest_hit and shadow rays only
     * need any_hit */
    material->setClosestHitProgram(0,closest_hit_program);
    material->setAnyHitProgram(1,any_hit_program);

    //now we're all done lets return our material
    return material;
}
//----------------------------------------------------------------------------------------------------------------------
void OptixSampleTwo::createGeometryInstances(optix::Geometry _box, optix::Material _material){

    // our transforms
    optix::Transform *transforms = new optix::Transform[m_numBoxes];
    // geomtry group of geomtrygroups.... groupception if you will
    optix::Group top_level_group;

    // identity matrix, pretty sure we could use glm or eigen for this shit
    float m[16];
    m[ 0] = 1.0f;  m[ 1] = 0.0f;  m[ 2] = 0.0f;  m[ 3] = 0.0f;
    m[ 4] = 0.0f;  m[ 5] = 1.0f;  m[ 6] = 0.0f;  m[ 7] = 0.0f;
    m[ 8] = 0.0f;  m[ 9] = 0.0f;  m[10] = 1.0f;  m[11] = 0.0f;
    m[12] = 0.0f;  m[13] = 0.0f;  m[14] = 0.0f;  m[15] = 1.0f;

    for(int i=0; i<m_numBoxes; i++){

        //create our instance
        optix::GeometryInstance instance = m_context->createGeometryInstance();
        //set the geomtry, and material programs
        instance->setGeometry(_box);
        instance->setMaterialCount(1);
        instance->setMaterial(0,_material);

        //vary the diffuse accross the instances that we create
        float kd_slider = (float)i/ (float)m_numBoxes;

        //set the variables in our material for this instance
        instance->declareVariable("Kd");
        instance->declareVariable("Ks");
        instance->declareVariable("Ka");
        instance->declareVariable("phong_exp");
        instance->declareVariable("reflectivity");
        instance->declareVariable("ambient_light_color");
        //set our variables
        instance["Kd"]->setFloat(kd_slider, 0.0f, 1.0f-kd_slider);
        instance["Ks"]->setFloat(0.5f, 0.5f, 0.5f);
        instance["Ka"]->setFloat(0.8f, 0.8f, 0.8f);
        instance["reflectivity"]->setFloat(0.8f, 0.8f, 0.8f);
        instance["phong_exp"]->setFloat(10.0f);
        instance["ambient_light_color"]->setFloat(0.2f, 0.2f, 0.2f);

        //create a groupt to hold the instances
        optix::GeometryGroup geometrygroup = m_context->createGeometryGroup();
        geometrygroup->setChildCount(1);
        //stick our instance in our group
        geometrygroup->setChild(0,instance);

        //create our acceleration
        optix::Acceleration acceleration = m_context->createAcceleration("NoAccel","NoAccel");
        //set this acceleration in our geometry group
        geometrygroup->setAcceleration(acceleration);

        //make a acceleration dirty
        acceleration->markDirty();

        //add a transform to our geomtry group node
        transforms[i] = m_context->createTransform();
        transforms[i]->setChild(geometrygroup);
        //edit our tranform matrix
        m[3] = i*1.5f - (m_numBoxes-1)*0.75f;
        transforms[i]->setMatrix(false,m,0);
    }

    //place our geomtry groups as children of our top level group
    top_level_group = m_context->createGroup();
    top_level_group->setChildCount(m_numBoxes);
    for(int i=0; i<m_numBoxes;++i){
        top_level_group->setChild(i,transforms[i]);
    }

    //add our group to our engine
    optix::Variable top_object = m_context->declareVariable("top_object");
    top_object->set(top_level_group);
    optix::Variable top_shadower = m_context->declareVariable("top_shadower");
    top_shadower->set(top_level_group);

    //create an acceleratio for our top level group
    optix::Acceleration top_level_acceleration = m_context->createAcceleration("NoAccel","NoAccel");
    top_level_group->setAcceleration(top_level_acceleration);

    //make our top level acceleration dirty
    top_level_acceleration->markDirty();

    delete [] transforms;

}

//----------------------------------------------------------------------------------------------------------------------
QImage OptixSampleTwo::trace(){
    //launch it
    m_context->launch(0,m_width,m_height);
    std::cout<<"here"<<std::endl;

    QImage img(m_width,m_height,QImage::Format_RGB32);
    QColor color;
    int idx;
    void* data = m_outputBuffer->map();
    typedef struct { float r; float g; float b; float a;} rgb;
    rgb* rgb_data = (rgb*)data;
    for(unsigned int i=0; i<m_width*m_height; ++i){
//        std::cout<<rgb_data[i].r<<","<<rgb_data[i].g<<","<<rgb_data[i].b<<std::endl;
        float red = rgb_data[i].r; if(red>1.0) red=1.0;
        float green = rgb_data[i].g; if(green>1.0) green=1.0;
        float blue = rgb_data[i].b; if(blue>1.0) blue=1.0;
        float alpha = rgb_data[i].a; if(alpha>1.0) alpha=1.0;
        color.setRgbF(red,green,blue,alpha);
        idx = floor((float)i/m_height);

        img.setPixel(i-(idx*m_height), idx, color.rgb());

    }
    m_outputBuffer->unmap();
    img.save("test.png","PNG");

    return img;
}

//----------------------------------------------------------------------------------------------------------------------
