#include "UI/MeshWidget.h"
#include "Core/pathtracerscene.h"
#include "Core/MaterialLibrary.h"
#include <QMessageBox>
#include <iostream>
#include <QMetaType>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//decare our static instance of our widget
MeshWidget* MeshWidget::m_instance;
//----------------------------------------------------------------------------------------------------------------------
MeshWidget* MeshWidget::getInstance(QWidget *parent){
    if(!m_instance){
        m_instance = new MeshWidget(parent);
    }
    else if(parent){
        m_instance->setParent(parent);
    }
    return m_instance;
}
//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::destroy(){
    delete m_instance;
}
//----------------------------------------------------------------------------------------------------------------------
MeshWidget::MeshWidget(QWidget *parent) :
    QDockWidget(parent)
{
    this->setWindowTitle("Model Library");
    m_curMeshName.clear();
    //add our grid layout to our widget
    QGroupBox * gb = new QGroupBox(this);
    this->setWidget(gb);
    QGridLayout *meshGridLayout = new QGridLayout(gb);
    gb->setLayout(meshGridLayout);

    //add our model list
    m_modelList = new QListWidget(gb);
    meshGridLayout->addWidget(m_modelList,0,0,1,4);
    connect(m_modelList,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(modelSelected(QListWidgetItem*)));

    //add a button to import a model
    QPushButton *importBtn = new QPushButton("Import Model",gb);
    meshGridLayout->addWidget(importBtn,1,0,1,1);
    connect(importBtn,SIGNAL(pressed()),this,SLOT(importModel()));

    //add our transform controls
    meshGridLayout->addWidget(new QLabel("Translate",gb), 2, 0, 1, 1);
    m_meshTranslateXDSpinBox = new QDoubleSpinBox(gb);
    m_meshTranslateXDSpinBox->setMaximum(INFINITY);
    m_meshTranslateXDSpinBox->setMinimum(-INFINITY);
    meshGridLayout->addWidget(m_meshTranslateXDSpinBox, 2, 1, 1, 1);
    m_meshTranslateYDSpinBox = new QDoubleSpinBox(gb);
    m_meshTranslateYDSpinBox->setMaximum(INFINITY);
    m_meshTranslateYDSpinBox->setMinimum(-INFINITY);
    meshGridLayout->addWidget(m_meshTranslateYDSpinBox, 2, 2, 1, 1);
    m_meshTranslateZDSpinBox = new QDoubleSpinBox(gb);
    m_meshTranslateZDSpinBox->setMaximum(INFINITY);
    m_meshTranslateZDSpinBox->setMinimum(-INFINITY);
    meshGridLayout->addWidget(m_meshTranslateZDSpinBox, 2, 3, 1, 1);
    meshGridLayout->addWidget(new QLabel("Rotate",gb), 3, 0, 1, 1);
    m_meshRotateXDSpinBox = new QDoubleSpinBox(gb);
    m_meshRotateXDSpinBox->setMaximum(INFINITY);
    m_meshRotateXDSpinBox->setMinimum(-INFINITY);
    meshGridLayout->addWidget(m_meshRotateXDSpinBox, 3, 1, 1, 1);
    m_meshRotateYDSpinBox = new QDoubleSpinBox(gb);
    m_meshRotateYDSpinBox->setMaximum(INFINITY);
    m_meshRotateYDSpinBox->setMinimum(-INFINITY);
    meshGridLayout->addWidget(m_meshRotateYDSpinBox, 3, 2, 1, 1);
    m_meshRotateZDSpinBox = new QDoubleSpinBox(gb);
    m_meshRotateZDSpinBox->setMaximum(INFINITY);
    m_meshRotateZDSpinBox->setMinimum(-INFINITY);
    meshGridLayout->addWidget(m_meshRotateZDSpinBox, 3, 3, 1, 1);
    meshGridLayout->addWidget(new QLabel("Scale",gb), 4, 0, 1, 1);
    m_meshScaleXDSpinBox = new QDoubleSpinBox(gb);
    m_meshScaleXDSpinBox->setMaximum(INFINITY);
    m_meshScaleXDSpinBox->setMinimum(-INFINITY);
    m_meshScaleXDSpinBox->setValue(1.0);
    meshGridLayout->addWidget(m_meshScaleXDSpinBox, 4, 1, 1, 1);
    m_meshScaleYDSpinBox = new QDoubleSpinBox(gb);
    m_meshScaleYDSpinBox->setMaximum(INFINITY);
    m_meshScaleYDSpinBox->setMinimum(-INFINITY);
    m_meshScaleYDSpinBox->setValue(1.0);
    meshGridLayout->addWidget(m_meshScaleYDSpinBox, 4, 2, 1, 1);
    m_meshScaleZDSpinBox = new QDoubleSpinBox(gb);
    m_meshScaleZDSpinBox->setMaximum(INFINITY);
    m_meshScaleZDSpinBox->setMinimum(-INFINITY);
    m_meshScaleZDSpinBox->setValue(1.0);
    meshGridLayout->addWidget(m_meshScaleZDSpinBox, 4, 3, 1, 1);

    QPushButton *openOSLHyperShaderBtn = new QPushButton("Open OSL hypershader",this);
    connect(openOSLHyperShaderBtn,SIGNAL(clicked()),AbstractMaterialWidget::getInstance(),SLOT(show()));
    meshGridLayout->addWidget(openOSLHyperShaderBtn,5,0,1,1);

    QPushButton *applyShaderBtn = new QPushButton("Apply shader to mesh",this);
    connect(applyShaderBtn,SIGNAL(clicked()),this,SLOT(applyOSLMaterial()));
    meshGridLayout->addWidget(applyShaderBtn,5,1,1,1);

    QPushButton *openMatLibBtn = new QPushButton("Select Material From Library",this);
    connect(openMatLibBtn,SIGNAL(clicked()),this,SLOT(applyMatFromLib()));
    meshGridLayout->addWidget(openMatLibBtn,5,2,1,2);

    m_meshSpacer = new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    meshGridLayout->addItem(m_meshSpacer, children().size(), 0, 2, 4);

    //-------------------------------------------------------------------------------------------------
    //-------------------------Connect up our signals and slots----------------------------------------
    //-------------------------------------------------------------------------------------------------

    connect(m_meshTranslateXDSpinBox,SIGNAL(valueChanged(double)),this,SLOT(signalTransformChange(double)));
    connect(m_meshTranslateYDSpinBox,SIGNAL(valueChanged(double)),this,SLOT(signalTransformChange(double)));
    connect(m_meshTranslateZDSpinBox,SIGNAL(valueChanged(double)),this,SLOT(signalTransformChange(double)));
    connect(m_meshRotateXDSpinBox,SIGNAL(valueChanged(double)),this,SLOT(signalTransformChange(double)));
    connect(m_meshRotateYDSpinBox,SIGNAL(valueChanged(double)),this,SLOT(signalTransformChange(double)));
    connect(m_meshRotateZDSpinBox,SIGNAL(valueChanged(double)),this,SLOT(signalTransformChange(double)));
    connect(m_meshScaleXDSpinBox,SIGNAL(valueChanged(double)),this,SLOT(signalTransformChange(double)));
    connect(m_meshScaleYDSpinBox,SIGNAL(valueChanged(double)),this,SLOT(signalTransformChange(double)));
    connect(m_meshScaleZDSpinBox,SIGNAL(valueChanged(double)),this,SLOT(signalTransformChange(double)));
}
//----------------------------------------------------------------------------------------------------------------------
MeshWidget::~MeshWidget(){
    std::map <QString,modelProp*>::const_iterator mp;
    for(mp = m_modelProperties.begin();mp!=m_modelProperties.end();mp++){
        delete mp->second;
    }

}
//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::importModel(){
    //Lets get the location of a mesh that we wish to import
    QString location = QFileDialog::getOpenFileName(this,tr("Import Mesh"), "models/", tr("Mesh Files (*.obj)"));
    //if nothing selected then we dont want to do anything
    if(location.isEmpty()){
        std::cerr<<"Model Import: Nothing Selected"<<std::endl;
        return;
    }
    //Get some file information
    QFileInfo fileInfo(location);
    QString name = fileInfo.baseName();

    //check to see if our model name is taken
    bool nameOk = false;
    QString tempName = name;
    int nameIncrement = 1;
    while(!nameOk){
        bool nameTake = false;
        for(int i=0;i<m_modelList->count();i++){
            if (m_modelList->item(i)->text()==tempName){
                tempName = name+QString("%1").arg(nameIncrement);
                nameIncrement++;
                nameTake = true;
            }
        }
        if(nameTake == false) nameOk = true;
    }
    name = tempName;

    m_curModelProp = new modelProp;

    m_curModelProp->meshHandle = PathTracerScene::getInstance()->importMesh(name.toStdString(),location.toStdString());

    if(m_curModelProp->meshHandle){
        m_curModelProp->transX = m_curModelProp->transY = m_curModelProp->transZ = m_curModelProp->rotX = m_curModelProp->rotY = m_curModelProp->rotZ = 0;
        m_curModelProp->scaleX = m_curModelProp->scaleY = m_curModelProp->scaleZ = 1;
        m_modelList->addItem(name);
        m_modelList->item(m_modelList->count()-1)->setSelected(true);
        m_curMeshName = name;
        m_modelProperties[name] = m_curModelProp;
        updateScene();
    }
    else{
        QMessageBox::warning(this,"Import Model","Failed to import model");
        return;
    }
}
//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::modelSelected(QListWidgetItem *_item){
    QString modelName = _item->text();
    std::map <QString, modelProp *>::const_iterator model=m_modelProperties.find(modelName);
    if(model==m_modelProperties.end()){
        QMessageBox::warning(this,"Model Library","Something went wrong with selecting model");
        return;
    }
    m_curMeshName = model->first;
    m_curModelProp = model->second;
    m_meshTranslateXDSpinBox->setValue(m_curModelProp->transX);
    m_meshTranslateYDSpinBox->setValue(m_curModelProp->transY);
    m_meshTranslateZDSpinBox->setValue(m_curModelProp->transZ);
    m_meshRotateXDSpinBox->setValue(m_curModelProp->rotX);
    m_meshRotateYDSpinBox->setValue(m_curModelProp->rotY);
    m_meshRotateZDSpinBox->setValue(m_curModelProp->rotZ);
    m_meshScaleXDSpinBox->setValue(m_curModelProp->scaleX);
    m_meshScaleYDSpinBox->setValue(m_curModelProp->scaleY);
    m_meshScaleZDSpinBox->setValue(m_curModelProp->scaleZ);
}

//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::signalTransformChange(double _val){

    if(m_curMeshName.isEmpty()) return;

    // get our transform values
    m_curModelProp->transX = m_meshTranslateXDSpinBox->value();
    m_curModelProp->transY = m_meshTranslateYDSpinBox->value();
    m_curModelProp->transZ = m_meshTranslateZDSpinBox->value();
    m_curModelProp->rotX = m_meshRotateXDSpinBox->value();
    m_curModelProp->rotY = m_meshRotateYDSpinBox->value();
    m_curModelProp->rotZ = m_meshRotateZDSpinBox->value();
    m_curModelProp->scaleX = m_meshScaleXDSpinBox->value();
    m_curModelProp->scaleY = m_meshScaleYDSpinBox->value();
    m_curModelProp->scaleZ = m_meshScaleZDSpinBox->value();

    //create our transform matrix
    glm::mat4 rotXMat,rotYMat,rotZMat,finalRot;
    float DtoR = 3.14159265359/180.0;
    rotXMat = glm::rotate(rotXMat,m_curModelProp->rotX*DtoR,glm::vec3(1,0,0));
    rotYMat = glm::rotate(rotYMat,m_curModelProp->rotY*DtoR,glm::vec3(0,1,0));
    rotZMat = glm::rotate(rotZMat,m_curModelProp->rotZ*DtoR,glm::vec3(0,0,1));
    finalRot = rotXMat * rotYMat * rotZMat;
    glm::mat4 finalTrans = finalRot;
    finalTrans[3][0] = m_curModelProp->transX;
    finalTrans[3][1] = m_curModelProp->transY;
    finalTrans[3][2] = m_curModelProp->transZ;
    finalTrans[0][0] = m_curModelProp->scaleX;
    finalTrans[1][1] = m_curModelProp->scaleY;
    finalTrans[2][2] = m_curModelProp->scaleZ;
    PathTracerScene::getInstance()->transformModel(m_curMeshName.toStdString(),finalTrans);
    updateScene();

}
//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::applyOSLMaterial(){
    AbstractMaterialWidget::getInstance()->applyMaterialToMesh(m_curMeshName.toStdString());
    updateScene();
}
//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::applyMatFromLib()
{
    MaterialLibrary::getInstance()->applyLibMatToMesh(m_curMeshName.toStdString());
    updateScene();
}
//----------------------------------------------------------------------------------------------------------------------

