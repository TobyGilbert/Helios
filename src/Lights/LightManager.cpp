#include "Lights/LightManager.h"
#include "Core/pathtracerscene.h"
#include <iostream>



// Declare our static instance of our class
LightManager* LightManager::m_instance;

//----------------------------------------------------------------------------------------------------------------------
LightManager* LightManager::getInstance(QWidget *parent){
    if(!m_instance){
        m_instance = new LightManager(parent);
    }
    else{
        if(parent!=0) std::cerr<<"LightManager already has a parent class"<<std::endl;
    }
    return m_instance;
}
//----------------------------------------------------------------------------------------------------------------------
LightManager::LightManager(QWidget *parent) : QDockWidget("Light Manager", parent), m_numLights(0){
    initialise();
}
//----------------------------------------------------------------------------------------------------------------------
LightManager::~LightManager(){
    for (unsigned int i=0; i<m_guiWidgets.size(); i++){
        delete m_guiWidgets[i];
    }
    m_lightBuffer->destroy();
    delete m_instance;
}
//----------------------------------------------------------------------------------------------------------------------
void LightManager::createGUI(){
    // Create a widget to work with
    QWidget* lightWidget = new QWidget(this);
    m_guiWidgets.push_back(lightWidget);
    this->setWidget(lightWidget);

    // Apply layout to that widget
    QGridLayout *gridLayout = new QGridLayout(this);
    lightWidget->setLayout(gridLayout);
    m_guiWidgets.push_back(gridLayout);

    // Some double spin boxes for light parameters
    QLabel *corner = new QLabel("Corner", this);
    m_guiWidgets.push_back(corner);
    gridLayout->addWidget(corner, 0, 0, 1, 1);
    m_cornerX = new QDoubleSpinBox(this);
    m_cornerY = new QDoubleSpinBox(this);
    m_cornerZ = new QDoubleSpinBox(this);
    m_cornerX->setMinimum(-10.0);
    m_cornerY->setMinimum(-10.0);
    m_cornerZ->setMinimum(-10.0);

    m_guiWidgets.push_back(m_cornerX);
    m_guiWidgets.push_back(m_cornerY);
    m_guiWidgets.push_back(m_cornerZ);
    gridLayout->addWidget(m_cornerX, 0, 1, 1, 1);
    gridLayout->addWidget(m_cornerY, 0, 2, 1, 1);
    gridLayout->addWidget(m_cornerZ, 0, 3, 1, 1);

    QLabel *v1 = new QLabel("V1", this);
    m_guiWidgets.push_back(v1);
    gridLayout->addWidget(v1, 1, 0, 1, 1);
    m_v1X = new QDoubleSpinBox(this);
    m_v1Y = new QDoubleSpinBox(this);
    m_v1Z = new QDoubleSpinBox(this);
    m_v1X->setMinimum(-10.0);
    m_v1Y->setMinimum(-10.0);
    m_v1Z->setMinimum(-10.0);
    m_guiWidgets.push_back(m_v1X);
    m_guiWidgets.push_back(m_v1Y);
    m_guiWidgets.push_back(m_v1Z);
    gridLayout->addWidget(m_v1X, 1, 1, 1, 1);
    gridLayout->addWidget(m_v1Y, 1, 2, 1, 1);
    gridLayout->addWidget(m_v1Z, 1, 3, 1, 1);

    QLabel *v2 = new QLabel("V2", this);
    m_guiWidgets.push_back(v2);
    gridLayout->addWidget(v2, 2, 0, 1, 1);
    m_v2X = new QDoubleSpinBox(this);
    m_v2Y = new QDoubleSpinBox(this);
    m_v2Z = new QDoubleSpinBox(this);
    m_guiWidgets.push_back(m_v2X);
    m_guiWidgets.push_back(m_v2Y);
    m_guiWidgets.push_back(m_v2Z);
    m_v2X->setMinimum(-10.0);
    m_v2Y->setMinimum(-10.0);
    m_v2Z->setMinimum(-10.0);
    gridLayout->addWidget(m_v2X, 2, 1, 1, 1);
    gridLayout->addWidget(m_v2Y, 2, 2, 1, 1);
    gridLayout->addWidget(m_v2Z, 2, 3, 1, 1);

    QLabel *emission = new QLabel("Emission", this);
    m_guiWidgets.push_back(emission);
    gridLayout->addWidget(emission, 3, 0, 1, 1);
    m_emissionX = new QDoubleSpinBox(this);
    m_emissionY = new QDoubleSpinBox(this);
    m_emissionZ = new QDoubleSpinBox(this);
    m_guiWidgets.push_back(m_emissionX);
    m_guiWidgets.push_back(m_emissionY);
    m_guiWidgets.push_back(m_emissionZ);
    gridLayout->addWidget(m_emissionX, 3, 1, 1, 1);
    gridLayout->addWidget(m_emissionY, 3, 2, 1, 1);
    gridLayout->addWidget(m_emissionZ, 3, 3, 1, 1);

    QPushButton *addLightBtn = new QPushButton("Add Light");
    m_guiWidgets.push_back(addLightBtn);
    gridLayout->addWidget(addLightBtn, 4, 0, 1, 4);

    // A spacer to push everything to the top
    QSpacerItem* spacer = new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    gridLayout->addItem(spacer, 5, 0, 1, 4);

    // Connections
    connect(addLightBtn, SIGNAL(clicked()), this, SLOT(addLight()));

}
//----------------------------------------------------------------------------------------------------------------------
void LightManager::initialise(){
    createGUI();

    m_lightBuffer = PathTracerScene::getInstance()->getContext()->createBuffer( RT_BUFFER_INPUT);
    m_lightBuffer->setFormat( RT_FORMAT_USER);
    m_lightBuffer->setElementSize(sizeof(ParallelogramLight));

    std::string ptx_path = "ptx/path_tracer.cu.ptx";
    m_lightMaterial = PathTracerScene::getInstance()->getContext()->createMaterial();
    Program diffuse_em = PathTracerScene::getInstance()->getContext()->createProgramFromPTXFile( ptx_path, "diffuseEmitter" );
    m_lightMaterial->setClosestHitProgram( 0, diffuse_em );

    // Set up our intersection programs
    ptx_path = "ptx/parallelogram.cu.ptx";
    m_pgram_bounding_box = PathTracerScene::getInstance()->getContext()->createProgramFromPTXFile( ptx_path, "bounds" );
    m_pgram_intersection = PathTracerScene::getInstance()->getContext()->createProgramFromPTXFile( ptx_path, "intersect" );
}
//----------------------------------------------------------------------------------------------------------------------
void LightManager::createParollelogramLight(float3 _corner, float3 _v1, float3 _v2, float3 _emission){
    ParallelogramLight light;
    light.corner = _corner;
    light.v1 = _v1;
    light.v2 = _v2;
    light.normal = normalize(cross(light.v1, light.v2));
    light.emission = _emission;

    // Add our light to the buffer of lights
    m_lightBuffer->setSize((unsigned int)m_numLights+1);
    memcpy(m_lightBuffer->map(), &light, sizeof(light));
    m_lightBuffer->unmap();

    // Add the lights geometry
    m_lightGeometry.push_back(createParallelogram(light.corner, light.v1, light.v2));

    m_lightGeometry.back()->addMaterial(m_lightMaterial);
    m_lightGeometry.back()["emission_color"]->setFloat(light.emission);

    m_numLights++;

}
//----------------------------------------------------------------------------------------------------------------------
optix::GeometryInstance LightManager::createParallelogram(const float3 &anchor, const float3 &offset1, const float3 &offset2){
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
//------------------------------------------------------------------------------------------------------------------------------------
void LightManager::addLight(){
    float3 corner = make_float3(m_cornerX->value(), m_cornerY->value(), m_cornerZ->value());
    float3 v1 = make_float3(m_v1X->value(), m_v1Y->value(), m_v1Z->value());
    float3 v2 = make_float3(m_v2X->value(), m_v2Y->value(), m_v2Z->value());
    float3 emission = make_float3(m_emissionX->value(), m_emissionY->value(), m_emissionZ->value());
    PathTracerScene::getInstance()->addLight(corner, v1, v2, emission);
}
