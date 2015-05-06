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
    m_lightBuffer->destroy();
    delete m_instance;
}
//----------------------------------------------------------------------------------------------------------------------
void LightManager::createGUI(){
    // Create a widget to work with
    QWidget* lightWidget = new QWidget(this);
    this->setWidget(lightWidget);

    // Apply layout to that widget
    QGridLayout *gridLayout = new QGridLayout(this);
    lightWidget->setLayout(gridLayout);

    // Combo box for selecting light to use
    m_lightIndexListWidget = new QListWidget(this);
    gridLayout->addWidget(m_lightIndexListWidget, 0, 0, 1, 4);

    QPushButton *addLightBtn = new QPushButton("Add Light", this);
    gridLayout->addWidget(addLightBtn, 1, 0, 1, 4);

    // Some double spin boxes for light parameters
    QLabel *translate = new QLabel("Translate", this);
    translate->setHidden(true);
    gridLayout->addWidget(translate, 2, 0, 1, 1);
    m_translateX = new QDoubleSpinBox(this);
    m_translateY = new QDoubleSpinBox(this);
    m_translateZ = new QDoubleSpinBox(this);
    m_translateX->setMinimum(-10.0);
    m_translateY->setMinimum(-10.0);
    m_translateZ->setMinimum(-10.0);
    m_translateX->setHidden(true);
    m_translateY->setHidden(true);
    m_translateZ->setHidden(true);
    gridLayout->addWidget(m_translateX, 2, 1, 1, 1);
    gridLayout->addWidget(m_translateY, 2, 2, 1, 1);
    gridLayout->addWidget(m_translateZ, 2, 3, 1, 1);

    QLabel *scale = new QLabel("Scale", this);
    scale->setHidden(true);
    gridLayout->addWidget(scale, 3, 0, 1, 1);
    m_scaleX = new QDoubleSpinBox(this);
    m_scaleY = new QDoubleSpinBox(this);
    m_scaleZ = new QDoubleSpinBox(this);
    m_scaleX->setMinimum(-10.0);
    m_scaleY->setMinimum(-10.0);
    m_scaleZ->setMinimum(-10.0);
    m_scaleX->setHidden(true);
    m_scaleY->setHidden(true);
    m_scaleZ->setHidden(true);
    m_scaleX->setValue(1.0);
    m_scaleY->setValue(1.0);
    m_scaleZ->setValue(1.0);
    gridLayout->addWidget(m_scaleX, 3, 1, 1, 1);
    gridLayout->addWidget(m_scaleY, 3, 2, 1, 1);
    gridLayout->addWidget(m_scaleZ, 3, 3, 1, 1);

    QLabel *rotate = new QLabel("Rotate", this);
    rotate->setHidden(true);
    gridLayout->addWidget(rotate, 4, 0, 1, 1);
    m_rotateX = new QDoubleSpinBox(this);
    m_rotateY = new QDoubleSpinBox(this);
    m_rotateZ = new QDoubleSpinBox(this);
    m_rotateX->setMinimum(-10.0);
    m_rotateY->setMinimum(-10.0);
    m_rotateZ->setMinimum(-10.0);
    m_rotateX->setHidden(true);
    m_rotateY->setHidden(true);
    m_rotateZ->setHidden(true);
    gridLayout->addWidget(m_rotateX, 4, 1, 1, 1);
    gridLayout->addWidget(m_rotateY, 4, 2, 1, 1);
    gridLayout->addWidget(m_rotateZ, 4, 3, 1, 1);

    QLabel *emission = new QLabel("Emission", this);
    emission->setHidden(true);
    gridLayout->addWidget(emission, 5, 0, 1, 1);
    m_emissionX = new QDoubleSpinBox(this);
    m_emissionY = new QDoubleSpinBox(this);
    m_emissionZ = new QDoubleSpinBox(this);
    m_emissionX->setHidden(true);
    m_emissionY->setHidden(true);
    m_emissionZ->setHidden(true);
    m_emissionX->setValue(5.0);
    m_emissionY->setValue(5.0);
    m_emissionZ->setValue(5.0);
    gridLayout->addWidget(m_emissionX, 5, 1, 1, 1);
    gridLayout->addWidget(m_emissionY, 5, 2, 1, 1);
    gridLayout->addWidget(m_emissionZ, 5, 3, 1, 1);

    m_button = new QPushButton("hello", this);
    gridLayout->addWidget(m_button, 6, 0, 1, 4);

    // A spacer to push everything to the top
    QSpacerItem* spacer = new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    gridLayout->addItem(spacer, 7, 0, 1, 4);

    // Connections
    connect(addLightBtn, SIGNAL(clicked()), this, SLOT(addLight()));
    connect(addLightBtn, SIGNAL(clicked()), translate, SLOT(show()));
    connect(addLightBtn, SIGNAL(clicked()), m_translateX, SLOT(show()));
    connect(addLightBtn, SIGNAL(clicked()), m_translateY, SLOT(show()));
    connect(addLightBtn, SIGNAL(clicked()), m_translateZ, SLOT(show()));
    connect(addLightBtn, SIGNAL(clicked()), scale, SLOT(show()));
    connect(addLightBtn, SIGNAL(clicked()), m_scaleX, SLOT(show()));
    connect(addLightBtn, SIGNAL(clicked()), m_scaleY, SLOT(show()));
    connect(addLightBtn, SIGNAL(clicked()), m_scaleZ, SLOT(show()));
    connect(addLightBtn, SIGNAL(clicked()), rotate, SLOT(show()));
    connect(addLightBtn, SIGNAL(clicked()), m_rotateX, SLOT(show()));
    connect(addLightBtn, SIGNAL(clicked()), m_rotateY, SLOT(show()));
    connect(addLightBtn, SIGNAL(clicked()), m_rotateZ, SLOT(show()));
    connect(addLightBtn, SIGNAL(clicked()), emission, SLOT(show()));
    connect(addLightBtn, SIGNAL(clicked()), m_emissionX, SLOT(show()));
    connect(addLightBtn, SIGNAL(clicked()), m_emissionY, SLOT(show()));
    connect(addLightBtn, SIGNAL(clicked()), m_emissionZ, SLOT(show()));

    connect(m_lightIndexListWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(updateGUI(QModelIndex)));
    connect(m_button, SIGNAL(clicked()), this, SLOT(updateLight()));
}
//----------------------------------------------------------------------------------------------------------------------
void LightManager::initialise(){
    createGUI();

    m_lightBuffer = PathTracerScene::getInstance()->getContext()->createBuffer( RT_BUFFER_INPUT);
    m_lightBuffer->setFormat( RT_FORMAT_USER);
    m_lightBuffer->setElementSize(sizeof(ParallelogramLight));
    m_lightBuffer->setSize(0u);

}
//----------------------------------------------------------------------------------------------------------------------
void LightManager::createParollelogramLight(){
    // Add our light to the buffer of lights
    Light *tmpLight = new Light();
    m_lights.push_back(tmpLight->getParallelogromLight());

    m_lightBuffer->setSize((unsigned int)m_numLights+1);
    memcpy(m_lightBuffer->map(), &(m_lights[0]), m_lights.size()*sizeof(ParallelogramLight));
    m_lightBuffer->unmap();

    // Add the lights geometry
    m_lightGeometry.push_back(tmpLight->getGeometryInstance());

    m_lightGeometry.back()->addMaterial(tmpLight->getMaterial());
    m_lightGeometry.back()["emission_color"]->setFloat(tmpLight->getParallelogromLight().emission);

    m_lightIndexListWidget->addItem(tr((std::string("light") + std::to_string(m_numLights+1)).c_str()));
    m_lightIndexListWidget->item(m_numLights)->setSelected(true);
    m_selectedLight = m_numLights;

    m_numLights++;
}
//------------------------------------------------------------------------------------------------------------------------------------
void LightManager::addLight(){
    PathTracerScene::getInstance()->addLight();
}
//------------------------------------------------------------------------------------------------------------------------------------
void LightManager::updateLight(){
    std::cout<<"selection light index: "<<m_selectedLight<<std::endl;
    // Changes the light parameters for the actual light
    // A pointer to the start of the buffer of lights
    ParallelogramLight* lightBuffer = (ParallelogramLight*)m_lightBuffer->map();
    lightBuffer[m_selectedLight].emission.x = m_emissionX->value();
    lightBuffer[m_selectedLight].emission.y = m_emissionY->value();
    lightBuffer[m_selectedLight].emission.z = m_emissionZ->value();
    m_lightBuffer->unmap();
    // Changes the emission paramters for the light
    m_lightGeometry[m_selectedLight]["emission_color"]->setFloat(make_float3(m_emissionX->value(), m_emissionY->value(), m_emissionZ->value()));
}
//------------------------------------------------------------------------------------------------------------------------------------
void LightManager::updateGUI(QModelIndex _index){
    std::cout<<"Index selected: "<<_index.row()<<std::endl;
    int row = _index.row();
    m_selectedLight = row;
    // A pointer to the start of the buffer of lights
    ParallelogramLight* lightBuffer = (ParallelogramLight*)m_lightBuffer->map();

    m_emissionX->setValue(lightBuffer[m_selectedLight].emission.x);
    m_emissionY->setValue(lightBuffer[m_selectedLight].emission.y);
    m_emissionZ->setValue(lightBuffer[m_selectedLight].emission.z);

    m_lightBuffer->unmap();
}
