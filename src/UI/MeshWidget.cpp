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
MeshWidget* MeshWidget::getInstance(QWidget *parent)
{
    if(!m_instance)
    {
        m_instance = new MeshWidget(parent);
    }
    else if(parent)
    {
        m_instance->setParent(parent);
    }
    return m_instance;
}
//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::destroy()
{
    delete m_instance;
}
//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::save(QDataStream &ds)
{
    //iterate through all our our map
    modelProp * props;
    std::map<QString,modelProp*>::const_iterator mp;
    for(mp = m_modelProperties.begin();mp!=m_modelProperties.end();mp++)
    {
        //load our data into our file
        props = mp->second;
        ds<<mp->first;
        ds<<props->meshPath;
        ds<<props->transX;
        ds<<props->transY;
        ds<<props->transZ;
        ds<<props->rotX;
        ds<<props->rotY;
        ds<<props->rotZ;
        ds<<props->scaleX;
        ds<<props->scaleY;
        ds<<props->scaleZ;
        //say if our model has a material applied
        if(props->materialName.length()>0)
        {
            ds<<true;
            //write in our material name
            ds<<QString(props->materialName.c_str());
        }
        else{
            ds<<false;
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::load(QDataStream &ds)
{
    modelProp *tempProps;
    std::map<QString,QString> paths;
    std::map<QString,QString>::iterator it;
    while(!ds.atEnd())
    {
        //load in all our data from our file
        tempProps = new modelProp;
        bool hasMat;
        ds>>tempProps->name;
        ds>>tempProps->meshPath;
        ds>>tempProps->transX;
        ds>>tempProps->transY;
        ds>>tempProps->transZ;
        ds>>tempProps->rotX;
        ds>>tempProps->rotY;
        ds>>tempProps->rotZ;
        ds>>tempProps->scaleX;
        ds>>tempProps->scaleY;
        ds>>tempProps->scaleZ;
        ds>>hasMat;
        tempProps->materialName = "";
        if(hasMat)
        {
            QString matName;
            ds>>matName;
            tempProps->materialName = matName.toStdString();
        }

        //check to see if we can find our model
        QFileInfo pathCheck(tempProps->meshPath);
        if(!pathCheck.exists()){
            QString error = "Cannot find model "+tempProps->name+" from location "+tempProps->meshPath+". Skipping import of model.";
            QMessageBox::warning(this,"Mesh Load",error);
            delete tempProps;
            continue;
        }

        //check if the name of our object is already taken in our scene
        bool nameOk = false;
        QString tempName = tempProps->name;
        int nameIncrement = 1;
        while(!nameOk)
        {
            bool nameTake = false;
            for(int i=0;i<m_modelList->count();i++)
            {
                if (m_modelList->item(i)->text()==tempName)
                {
                    tempName = tempProps->name+QString("%1").arg(nameIncrement);
                    nameIncrement++;
                    nameTake = true;
                }
            }
            if(nameTake == false) nameOk = true;
        }
        tempProps->name = tempName;


        //If we already have this model in our scene then we may as well make the new one an instance
        it = paths.find(tempProps->meshPath);
        if(it==paths.end())
        {
            tempProps->meshHandle = PathTracerScene::getInstance()->importMesh(tempProps->name.toStdString(),tempProps->meshPath.toStdString());
            paths[tempProps->meshPath] = tempProps->name;
        }
        else
        {
            tempProps->meshHandle = PathTracerScene::getInstance()->createInstance(it->second.toStdString(),tempProps->name.toStdString());
        }

        //set the material of our model if its in our library
        optix::Material mat;
        if(MaterialLibrary::getInstance()->getMatFromLib(tempProps->materialName,mat))
        {
            tempProps->meshHandle->setMaterial(mat);
        }

        //create our transform matrix
        glm::mat4 rotXMat,rotYMat,rotZMat,finalRot;
        float DtoR = 3.14159265359/180.0;
        glm::mat4 finalTrans;
        finalTrans[0][0] = tempProps->scaleX;
        finalTrans[1][1] = tempProps->scaleY;
        finalTrans[2][2] = tempProps->scaleZ;
        rotXMat = glm::rotate(rotXMat,tempProps->rotX*DtoR,glm::vec3(1,0,0));
        rotYMat = glm::rotate(rotYMat,tempProps->rotY*DtoR,glm::vec3(0,1,0));
        rotZMat = glm::rotate(rotZMat,tempProps->rotZ*DtoR,glm::vec3(0,0,1));
        finalRot = rotXMat * rotYMat * rotZMat;
        finalTrans*=finalRot;
        finalTrans[3][0] = tempProps->transX;
        finalTrans[3][1] = tempProps->transY;
        finalTrans[3][2] = tempProps->transZ;
        //transform our model
        PathTracerScene::getInstance()->transformModel(tempProps->name.toStdString(),finalTrans);

        m_modelList->addItem(tempProps->name);
        //store in our map
        m_modelProperties[tempProps->name] = tempProps;
    }

}
//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::clearScene()
{
    //select all items in list
    m_modelList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_modelList->selectAll();
    //delete all that are selected
    removeSelected();
    m_modelList->setSelectionMode(QAbstractItemView::SingleSelection);
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

    QPushButton *instanceBtn = new QPushButton("Create Instance",this);
    connect(instanceBtn,SIGNAL(pressed()),this,SLOT(createInstance()));
    meshGridLayout->addWidget(instanceBtn,1,2,1,1);

    QPushButton *deleteMdlBtn = new QPushButton("Delete Model",this);
    connect(deleteMdlBtn,SIGNAL(pressed()),this,SLOT(removeSelected()));
    meshGridLayout->addWidget(deleteMdlBtn,1,3,1,1);

    //add our transform controls
    meshGridLayout->addWidget(new QLabel("Translate",gb), 2, 0, 1, 1);
    m_meshTranslateXDSpinBox = new QDoubleSpinBox(gb);
    m_meshTranslateXDSpinBox->setDecimals(3);
    m_meshTranslateXDSpinBox->setSingleStep(0.1);
    m_meshTranslateXDSpinBox->setMaximum(INFINITY);
    m_meshTranslateXDSpinBox->setMinimum(-INFINITY);
    meshGridLayout->addWidget(m_meshTranslateXDSpinBox, 2, 1, 1, 1);
    m_meshTranslateYDSpinBox = new QDoubleSpinBox(gb);
    m_meshTranslateYDSpinBox->setDecimals(3);
    m_meshTranslateYDSpinBox->setSingleStep(0.1);
    m_meshTranslateYDSpinBox->setMaximum(INFINITY);
    m_meshTranslateYDSpinBox->setMinimum(-INFINITY);
    meshGridLayout->addWidget(m_meshTranslateYDSpinBox, 2, 2, 1, 1);
    m_meshTranslateZDSpinBox = new QDoubleSpinBox(gb);
    m_meshTranslateZDSpinBox->setDecimals(3);
    m_meshTranslateZDSpinBox->setSingleStep(0.1);
    m_meshTranslateZDSpinBox->setMaximum(INFINITY);
    m_meshTranslateZDSpinBox->setMinimum(-INFINITY);
    meshGridLayout->addWidget(m_meshTranslateZDSpinBox, 2, 3, 1, 1);
    meshGridLayout->addWidget(new QLabel("Rotate",gb), 3, 0, 1, 1);
    m_meshRotateXDSpinBox = new QDoubleSpinBox(gb);
    m_meshRotateXDSpinBox->setDecimals(3);
    m_meshRotateXDSpinBox->setSingleStep(0.1);
    m_meshRotateXDSpinBox->setMaximum(INFINITY);
    m_meshRotateXDSpinBox->setMinimum(-INFINITY);
    meshGridLayout->addWidget(m_meshRotateXDSpinBox, 3, 1, 1, 1);
    m_meshRotateYDSpinBox = new QDoubleSpinBox(gb);
    m_meshRotateYDSpinBox->setDecimals(3);
    m_meshRotateYDSpinBox->setSingleStep(0.1);
    m_meshRotateYDSpinBox->setMaximum(INFINITY);
    m_meshRotateYDSpinBox->setMinimum(-INFINITY);
    meshGridLayout->addWidget(m_meshRotateYDSpinBox, 3, 2, 1, 1);
    m_meshRotateZDSpinBox = new QDoubleSpinBox(gb);
    m_meshRotateZDSpinBox->setDecimals(3);
    m_meshRotateZDSpinBox->setSingleStep(0.1);
    m_meshRotateZDSpinBox->setMaximum(INFINITY);
    m_meshRotateZDSpinBox->setMinimum(-INFINITY);
    meshGridLayout->addWidget(m_meshRotateZDSpinBox, 3, 3, 1, 1);
    meshGridLayout->addWidget(new QLabel("Scale",gb), 4, 0, 1, 1);
    m_meshScaleXDSpinBox = new QDoubleSpinBox(gb);
    m_meshScaleXDSpinBox->setDecimals(3);
    m_meshScaleXDSpinBox->setSingleStep(0.1);
    m_meshScaleXDSpinBox->setMaximum(INFINITY);
    m_meshScaleXDSpinBox->setMinimum(-INFINITY);
    m_meshScaleXDSpinBox->setValue(1.0);
    meshGridLayout->addWidget(m_meshScaleXDSpinBox, 4, 1, 1, 1);
    m_meshScaleYDSpinBox = new QDoubleSpinBox(gb);
    m_meshScaleYDSpinBox->setDecimals(3);
    m_meshScaleYDSpinBox->setSingleStep(0.1);
    m_meshScaleYDSpinBox->setMaximum(INFINITY);
    m_meshScaleYDSpinBox->setMinimum(-INFINITY);
    m_meshScaleYDSpinBox->setValue(1.0);
    meshGridLayout->addWidget(m_meshScaleYDSpinBox, 4, 2, 1, 1);
    m_meshScaleZDSpinBox = new QDoubleSpinBox(gb);
    m_meshScaleZDSpinBox->setDecimals(3);
    m_meshScaleZDSpinBox->setSingleStep(0.1);
    m_meshScaleZDSpinBox->setMaximum(INFINITY);
    m_meshScaleZDSpinBox->setMinimum(-INFINITY);
    m_meshScaleZDSpinBox->setValue(1.0);
    meshGridLayout->addWidget(m_meshScaleZDSpinBox, 4, 3, 1, 1);


    QPushButton *openMatLibBtn = new QPushButton("Select Material From Library",this);
    connect(openMatLibBtn,SIGNAL(clicked()),this,SLOT(applyMatFromLib()));
    meshGridLayout->addWidget(openMatLibBtn,5,0,1,1);


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
MeshWidget::~MeshWidget()
{
    std::map <QString,modelProp*>::const_iterator mp;
    for(mp = m_modelProperties.begin();mp!=m_modelProperties.end();mp++)
    {
        delete mp->second;
    }

}
//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::importModel()
{
    //Lets get the location of a mesh that we wish to import
    QStringList locations = QFileDialog::getOpenFileNames(this,tr("Import Mesh"), "models/", tr("Mesh Files (*.obj *.OBJ)"));
        for(int i=0;i<locations.size();i++)
        {
        //if nothing selected then we dont want to do anything
        if(locations[i].isEmpty())
        {
            std::cerr<<"Model Import: Nothing Selected"<<std::endl;
            return;
        }
        //Get some file information
        QFileInfo fileInfo(locations[i]);
        QString name = fileInfo.baseName();

        //check to see if our model name is taken
        bool nameOk = false;
        QString tempName = name;
        int nameIncrement = 1;
        while(!nameOk)
        {
            bool nameTake = false;
            for(int i=0;i<m_modelList->count();i++)
            {
                if (m_modelList->item(i)->text()==tempName)
                {
                    tempName = name+QString("%1").arg(nameIncrement);
                    nameIncrement++;
                    nameTake = true;
                }
            }
            if(nameTake == false) nameOk = true;
        }
        name = tempName;


        OptiXModel* tempHandle =  PathTracerScene::getInstance()->importMesh(name.toStdString(),locations[i].toStdString());

        if(tempHandle)
        {
            m_curModelProp = new modelProp;
            m_curModelProp->meshHandle = tempHandle;
            m_curModelProp->meshPath = locations[i];
            m_curModelProp->transX = m_curModelProp->transY = m_curModelProp->transZ = m_curModelProp->rotX = m_curModelProp->rotY = m_curModelProp->rotZ = 0;
            m_curModelProp->scaleX = m_curModelProp->scaleY = m_curModelProp->scaleZ = 1;
            m_curModelProp->materialName = "";
            m_modelList->addItem(name);
            m_modelList->item(m_modelList->count()-1)->setSelected(true);
            m_curMeshName = name;
            m_modelProperties[name] = m_curModelProp;
            updateScene();
        }
        else
        {
            QMessageBox::warning(this,"Import Model","Failed to import model" + locations[i]);
            continue;
        }
    }
}
//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::createInstance()
{
    QList<QListWidgetItem*> items = m_modelList->selectedItems();
    for(int i=0;i<items.size();i++){
        QString name = items[i]->text();
        if(!m_curModelProp->isIntance)
        {
            name+="_instance";
        }
        //check to see if our model name is taken
        bool nameOk = false;
        QString tempName = name;
        int nameIncrement = 1;
        while(!nameOk)
        {
            bool nameTake = false;
            for(int i=0;i<m_modelList->count();i++)
            {
                if (m_modelList->item(i)->text()==tempName)
                {
                    tempName = name+QString("%1").arg(nameIncrement);
                    nameIncrement++;
                    nameTake = true;
                }
            }
            if(nameTake == false) nameOk = true;
        }
        name = tempName;


        OptiXModel* tempHandle = PathTracerScene::getInstance()->createInstance(items[i]->text().toStdString(),name.toStdString());

        if(tempHandle)
        {
            QString path = m_curModelProp->meshPath;
            std::string matName = m_curModelProp->materialName;
            m_curModelProp = new modelProp;
            m_curModelProp->meshHandle = tempHandle;
            m_curModelProp->meshPath = path;
            m_curModelProp->materialName = matName;
            m_curModelProp->isIntance = true;
            m_curModelProp->transX = m_curModelProp->transY = m_curModelProp->transZ = m_curModelProp->rotX = m_curModelProp->rotY = m_curModelProp->rotZ = 0;
            m_curModelProp->scaleX = m_curModelProp->scaleY = m_curModelProp->scaleZ = 1;
            m_modelList->addItem(name);
            m_modelList->item(m_modelList->count()-1)->setSelected(true);
            m_curMeshName = name;
            m_modelProperties[name] = m_curModelProp;
            updateScene();
        }
        else
        {
            QMessageBox::warning(this,"Instance Model","Failed to instace model" + items[i]->text());
            continue;
        }

    }
}
//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::removeSelected()
{
    QList<QListWidgetItem*> items = m_modelList->selectedItems();
    for(int i=0;i<items.size();i++)
    {
        std::cout<<"Deleting "<<items[i]->text().toStdString()<<std::endl;
        PathTracerScene::getInstance()->removeGeomtry(items[i]->text().toStdString());
        std::map <QString, modelProp *>::iterator model=m_modelProperties.find(items[i]->text());
        delete model->second;
        m_modelProperties.erase(model);
        m_modelList->removeItemWidget(items[i]);
        delete items[i];
    }
    m_modelList->clearSelection();
    updateScene();
}
//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::modelSelected(QListWidgetItem *_item)
{
    QString modelName = _item->text();
    std::map <QString, modelProp *>::const_iterator model=m_modelProperties.find(modelName);
    if(model==m_modelProperties.end())
    {
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
void MeshWidget::signalTransformChange(double _val)
{

    QList<QListWidgetItem*> items = m_modelList->selectedItems();
    std::map <QString, modelProp *>::const_iterator model;
    for(int i=0;i<items.size();i++)
    {
        model=m_modelProperties.find(items[i]->text());
        if(model==m_modelProperties.end())
        {
            QString error = QString("Cannot find properties of model %1").arg(items[i]->text());
            QMessageBox::warning(this,"Model Library",error);
            continue;
        }
        // get our transform values
        model->second->transX = m_meshTranslateXDSpinBox->value();
        model->second->transY = m_meshTranslateYDSpinBox->value();
        model->second->transZ = m_meshTranslateZDSpinBox->value();
        model->second->rotX = m_meshRotateXDSpinBox->value();
        model->second->rotY = m_meshRotateYDSpinBox->value();
        model->second->rotZ = m_meshRotateZDSpinBox->value();
        model->second->scaleX = m_meshScaleXDSpinBox->value();
        model->second->scaleY = m_meshScaleYDSpinBox->value();
        model->second->scaleZ = m_meshScaleZDSpinBox->value();

        //create our transform matrix
        glm::mat4 rotXMat,rotYMat,rotZMat,finalRot;
        float DtoR = 3.14159265359/180.0;
        glm::mat4 finalTrans;
        finalTrans[0][0] = m_curModelProp->scaleX;
        finalTrans[1][1] = m_curModelProp->scaleY;
        finalTrans[2][2] = m_curModelProp->scaleZ;
        rotXMat = glm::rotate(rotXMat,m_curModelProp->rotX*DtoR,glm::vec3(1,0,0));
        rotYMat = glm::rotate(rotYMat,m_curModelProp->rotY*DtoR,glm::vec3(0,1,0));
        rotZMat = glm::rotate(rotZMat,m_curModelProp->rotZ*DtoR,glm::vec3(0,0,1));
        finalRot = rotXMat * rotYMat * rotZMat;
        finalTrans*=finalRot;
        finalTrans[3][0] = m_curModelProp->transX;
        finalTrans[3][1] = m_curModelProp->transY;
        finalTrans[3][2] = m_curModelProp->transZ;
        PathTracerScene::getInstance()->transformModel(m_curMeshName.toStdString(),finalTrans);
    }
    updateScene();

}
//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::applyOSLMaterial(Material _mat, std::string _matName)
{
    QList<QListWidgetItem*> items = m_modelList->selectedItems();
    if(items.size()==0)
    {
        QMessageBox::information(this,"Mesh Widget","Nothing selected to apply material to.");
        return;
    }
    std::map <QString, modelProp *>::const_iterator model;
    for(int i=0;i<items.size();i++)
    {
        std::cerr<<"Applying material "<<_matName<<" to mesh "<<items[i]->text().toStdString()<<std::endl;
        PathTracerScene::getInstance()->setModelMaterial(items[i]->text().toStdString(),_mat);
        model = m_modelProperties.find(items[i]->text());
        if(model!=m_modelProperties.end())
        {
            model->second->materialName = _matName;
        }
        else
        {
            std::cerr<<"Warning: MeshWidget, Could not find model properties for "<<items[i]->text().toStdString()<<std::endl;
        }
    }
    updateScene();
}
//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::applyMatFromLib()
{
    MaterialLibrary::getInstance()->applyLibMatToMesh(m_curMeshName.toStdString());
    updateScene();
}
//----------------------------------------------------------------------------------------------------------------------

