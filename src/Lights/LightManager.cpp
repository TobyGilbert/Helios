/// @brief A class to manage all the lights in our scene
/// @author Toby Gilbert

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
//    m_lights.push_back(tmpLight);
    m_parallelogramLights.push_back(tmpLight->getParallelogromLight());


    // Add the default information for our gui defaults vector, resize and reset the gui
    m_lightTransforms.resize((unsigned int)m_numLights+1);
    m_lightTransforms.back().m_translate = glm::vec3(0.0, 0.0, 0.0);
    m_lightTransforms.back().m_rotate = glm::vec3(0.0, 0.0, 0.0);
    m_lightTransforms.back().m_scale = glm::vec3(1.0, 1.0, 1.0);
    setGuiDefaults();

    // Resize the light information buffer and copy back the data
    m_lightBuffer->setSize((unsigned int)m_numLights+1);
    memcpy(m_lightBuffer->map(), &(m_parallelogramLights[0]), m_parallelogramLights.size()*sizeof(ParallelogramLight));
    m_lightBuffer->unmap();

    // Add the transform-geometry node to the vector for use when drawing the light geometry
    m_geoAndTrans.push_back(tmpLight->getGeomAndTrans());

    m_lightIndexListWidget->addItem(tr((std::string("light") + std::to_string(m_numLights+1)).c_str()));
    m_lightIndexListWidget->item(m_numLights)->setSelected(true);
    m_selectedLight = m_numLights;

    m_numLights++;

    delete tmpLight;
}
//------------------------------------------------------------------------------------------------------------------------------------
void LightManager::setGuiDefaults(){
    m_translateX->setValue(0.0);
    m_translateY->setValue(0.0);
    m_translateZ->setValue(0.0);
    m_rotateX->setValue(0.0);
    m_rotateY->setValue(0.0);
    m_rotateZ->setValue(0.0);
    m_scaleX->setValue(1.0);
    m_scaleY->setValue(1.0);
    m_scaleZ->setValue(1.0);
    m_emissionX->setValue(5.0);
    m_emissionY->setValue(5.0);
    m_emissionZ->setValue(5.0);
}
//------------------------------------------------------------------------------------------------------------------------------------
void LightManager::addLight(){
    PathTracerScene::getInstance()->addLight();
}
//------------------------------------------------------------------------------------------------------------------------------------
void LightManager::updateLight(){
    // Changes the light parameters for the actual light
    // A pointer to the start of the buffer of lights
    ParallelogramLight* lightBuffer = (ParallelogramLight*)m_lightBuffer->map();
    lightBuffer[m_selectedLight].emission.x = m_emissionX->value();
    lightBuffer[m_selectedLight].emission.y = m_emissionY->value();
    lightBuffer[m_selectedLight].emission.z = m_emissionZ->value();

    // Scale by the default values no need to scale in as the light is flat
    // Scale in x
//    lightBuffer[m_selectedLight].corner.x = 0.5 * m_scaleX->value();
//    lightBuffer[m_selectedLight].v1.x = -1.0 * m_scaleX->value();
//    // Scale in z
//    lightBuffer[m_selectedLight].corner.z = -0.5 * m_scaleZ->value();
//    lightBuffer[m_selectedLight].v2.z = 1.0 * m_scaleZ->value();

    // Translate using the scaled values
    lightBuffer[m_selectedLight].corner.x = (0.5 * m_scaleX->value()) + m_translateX->value();
    lightBuffer[m_selectedLight].v1.x = (-1.0 * m_scaleX->value()) + m_translateX->value();

    lightBuffer[m_selectedLight].corner.y = m_translateY->value();

    lightBuffer[m_selectedLight].corner.z = (-0.5 * m_scaleZ->value()) + m_translateZ->value();
    lightBuffer[m_selectedLight].v2.z = (1.0 * m_scaleZ->value()) + m_translateZ->value();


    m_lightBuffer->unmap();

    // Update the vector to store transforms
    LightTransforms lightTrans;
    lightTrans.m_translate = glm::vec3(m_translateX->value(), m_translateY->value(), m_translateZ->value());
    lightTrans.m_rotate = glm::vec3(m_rotateX->value(), m_rotateY->value(), m_rotateZ->value());
    lightTrans.m_scale = glm::vec3(m_scaleX->value(), m_scaleY->value(), m_scaleZ->value());
    m_lightTransforms[m_selectedLight]= lightTrans;

    glm::mat4 transform = glm::mat4();

    // Rotate
    float DtoR = 3.14159265359/180.0;
    glm::mat4 rotx = glm::mat4();
    glm::mat4 roty = glm::mat4();
    glm::mat4 rotz = glm::mat4();
    rotx = glm::rotate(rotx, (float)m_rotateX->value()*DtoR, glm::vec3(1.0, 0.0, 0.0));
    roty = glm::rotate(roty, (float)m_rotateY->value()*DtoR, glm::vec3(0.0, 1.0, 0.0));
    rotz = glm::rotate(rotz, (float)m_rotateZ->value()*DtoR, glm::vec3(0.0, 0.0, 1.0));
    transform = rotx * roty * rotz;

    // Scale
    transform[0][0] = m_scaleX->value();
    transform[1][1] = m_scaleY->value();
    transform[2][2] = m_scaleZ->value();

    // Translate
    transform[3][0] = m_translateX->value();
    transform[3][1] = m_translateY->value();
    transform[3][2] = m_translateZ->value();

    // Translate the optix::Translate node using the new transform matrix
    setTrans(m_geoAndTrans[m_selectedLight], transform);

    // Changes the emission paramters for the light
    m_geoAndTrans[m_selectedLight]->getChild<optix::GeometryGroup>()->getChild(0)["emission_color"]->setFloat(m_emissionX->value(), m_emissionY->value(), m_emissionZ->value());

    // Update accelleration structure
    PathTracerScene::getInstance()->cleanTopAcceleration();
}
//------------------------------------------------------------------------------------------------------------------------------------
void LightManager::updateGUI(QModelIndex _index){
    std::cout<<"Index selected: "<<_index.row()<<std::endl;
    int row = _index.row();
    m_selectedLight = row;
    // A pointer to the start of the buffer of lights
    ParallelogramLight* lightBuffer = (ParallelogramLight*)m_lightBuffer->map();

    m_translateX->setValue(m_lightTransforms[m_selectedLight].m_translate.x);
    m_translateY->setValue(m_lightTransforms[m_selectedLight].m_translate.y);
    m_translateZ->setValue(m_lightTransforms[m_selectedLight].m_translate.z);

    m_rotateX->setValue(m_lightTransforms[m_selectedLight].m_rotate.x);
    m_rotateY->setValue(m_lightTransforms[m_selectedLight].m_rotate.y);
    m_rotateZ->setValue(m_lightTransforms[m_selectedLight].m_rotate.z);

    m_scaleX->setValue(m_lightTransforms[m_selectedLight].m_scale.x);
    m_scaleY->setValue(m_lightTransforms[m_selectedLight].m_scale.y);
    m_scaleZ->setValue(m_lightTransforms[m_selectedLight].m_scale.z);

    m_emissionX->setValue(lightBuffer[m_selectedLight].emission.x);
    m_emissionY->setValue(lightBuffer[m_selectedLight].emission.y);
    m_emissionZ->setValue(lightBuffer[m_selectedLight].emission.z);

    m_lightBuffer->unmap();
}
//------------------------------------------------------------------------------------------------------------------------------------
void LightManager::setTrans(optix::Transform _transform, glm::mat4 _trans, bool _transpose){
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
    _transform->setMatrix(_transpose,m,invM);
}
//------------------------------------------------------------------------------------------------------------------------------------
