#include "optixsample.h"
#include <iostream>
#include <QColor>
#include <math.h>

//----------------------------------------------------------------------------------------------------------------------
OptixSample::OptixSample()
{
    m_context = optix::Context::create();
}
//----------------------------------------------------------------------------------------------------------------------
OptixSample::~OptixSample(){
    // clean up
    m_outputBuffer->destroy();
    m_context->destroy();
    m_context = 0;
}

//----------------------------------------------------------------------------------------------------------------------
void OptixSample::init(){
    // set how many ray types we have
    m_context->setRayTypeCount(1);
    // set our many entry points to our engine we have
    m_context->setEntryPointCount(1);
    // create our buffer
    m_outputBuffer = m_context->createBuffer(RT_BUFFER_OUTPUT,RT_FORMAT_FLOAT4,m_width,m_height);
    // create our output buffer in the engine
    m_context->declareVariable("result_buffer");
    //link it to our buffer
//    optix::Variable outputBuffer = m_context["result_buffer"];
    m_context["result_buffer"]->set(m_outputBuffer);

    // create our ptx path to our program
    std::string ptx_path = std::string("ptx/draw_color.cu.ptx");
    //create our program
    optix::Program ray_gen_program = m_context->createProgramFromPTXFile(ptx_path,"draw_solid_color");
    //set our color in our program
    ray_gen_program["draw_color"]->setFloat(0.462f, 0.725f, 0.0f);

    //set our ray generation program
    m_context->setRayGenerationProgram(0,ray_gen_program);

    //validate our program
    m_context->validate();
    //compile it
    m_context->compile();

}
//----------------------------------------------------------------------------------------------------------------------
QImage OptixSample::trace(){
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
        std::cout<<rgb_data[i].r<<","<<rgb_data[i].g<<","<<rgb_data[i].b<<std::endl;
        color.setRgbF(rgb_data[i].r,rgb_data[i].g,rgb_data[i].b,rgb_data[i].a);
        idx = floor((float)i/m_height);

        img.setPixel(i-(idx*m_height), idx, color.rgb());

    }
    m_outputBuffer->unmap();
    img.save("test","PNG");

    return img;
}

//----------------------------------------------------------------------------------------------------------------------

