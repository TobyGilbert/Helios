#include "Core/MaterialLibrary.h"
#include <QMessageBox>
#include <iostream>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QPushButton>
#include "Core/pathtracerscene.h"
#include "UI/AbstractMaterialWidget.h"
#include "NodeGraph/OSLNodesEditor.h"

//Declare our static instance variable
MaterialLibrary* MaterialLibrary::m_instance;

MaterialLibrary* MaterialLibrary::getInstance(QWidget *parent){
    if(!m_instance){
        m_instance = new MaterialLibrary(parent);
    }
    else if(parent){
        m_instance->setParent(parent,Qt::Window);
    }
    return m_instance;
}
//----------------------------------------------------------------------------------------------------------------------
MaterialLibrary::MaterialLibrary(QWidget *parent) :
    QWidget(parent,Qt::Window)
{
    this->setWindowTitle("Material Library");
    this->setMinimumWidth(200);
    this->setMinimumHeight(100);
    //set our layout
    m_widgetLayout = new QGridLayout(this);
    this->setLayout(m_widgetLayout);
    m_matListWidget = new QListWidget(this);
    m_widgetLayout->addWidget(m_matListWidget,0,0,1,1);
    connect(m_matListWidget,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(matSelected(QListWidgetItem*)));
    m_widgetSpacer = new QSpacerItem(children().count(), 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_loadToHyperShader = false;
    m_applyMatToMesh = false;
    //button to delete materials in library
    QPushButton *dltBtn = new QPushButton("Delete Material",this);
    m_widgetLayout->addWidget(dltBtn,1,0,1,1);
    connect(dltBtn,SIGNAL(pressed()),this,SLOT(deleteSelectedMat()));

    //load in all our previously saved materials
    QDir ngDir("NodeGraphs/");
    ngDir.setNameFilters(QStringList("*.hel"));
    QStringList matLib = ngDir.entryList();
    for(int i=0;i<matLib.size();i++){
        QFileInfo path(matLib[i]);
        m_matListWidget->addItem(path.baseName());
        m_materials[path.baseName().toStdString()] = PathTracerScene::getInstance()->getContext()->createMaterial();
    }
}
//----------------------------------------------------------------------------------------------------------------------
MaterialLibrary::~MaterialLibrary(){
}
//----------------------------------------------------------------------------------------------------------------------
void MaterialLibrary::deleteSelectedMat(){
    QList<QListWidgetItem*> items = m_matListWidget->selectedItems();
    std::map <std::string, optix::Material >::const_iterator mat;
    for(int i=0; i<items.size();i++){
        QString name = items[i]->text();
        std::cerr<<"Deleting Material "<<name.toStdString()<<std::endl;
        m_matListWidget->removeItemWidget(items[i]);
        delete items[i];
        mat=m_materials.find(name.toStdString());
        if(mat!=m_materials.end()){
            m_materials.erase(mat);
        }
        QFile file("NodeGraphs/"+name+".hel");
        file.remove();
    }

}

//----------------------------------------------------------------------------------------------------------------------
bool MaterialLibrary::addMaterialToLibrary(std::string _name, optix::Material _material){
    //check to see if there is already a material of this name in our library
    std::map <std::string, optix::Material >::const_iterator mat=m_materials.find(_name);
    if(mat!=m_materials.end()){
        QMessageBox::warning(this,"Material Library Error","Material of this name already exists in library");
        return false;
    }
    std::cerr<<"Adding "<<_name<<" to material library"<<std::endl;
    m_materials[_name] = _material;
    m_matListWidget->addItem(_name.c_str());
    return true;
}
//----------------------------------------------------------------------------------------------------------------------
void MaterialLibrary::applyLibMatToMesh(std::string _meshId)
{
    this->show();
    m_applyMatToMesh = true;
    m_loadToHyperShader = false;
    m_selectedMeshId = _meshId;
}
//----------------------------------------------------------------------------------------------------------------------
void MaterialLibrary::loadMatToHyperShader(){
    this->show();
    m_loadToHyperShader = true;
    m_applyMatToMesh = false;
}

//----------------------------------------------------------------------------------------------------------------------
void MaterialLibrary::closeEvent(QCloseEvent *event)
{
    m_applyMatToMesh = false;
}
//----------------------------------------------------------------------------------------------------------------------
void MaterialLibrary::matSelected(QListWidgetItem *_item){
    std::string matName = _item->text().toStdString();
    std::map <std::string, optix::Material >::const_iterator mat=m_materials.find(matName);
    if(mat==m_materials.end()){
        QMessageBox::warning(this,"Material Library Error","Something went wrong with finding the material in the library");
        return;
    }
    if(m_applyMatToMesh){
        std::cerr<<"Applying material "<<mat->first<<" to mesh "<<m_selectedMeshId<<std::endl;
        PathTracerScene::getInstance()->setModelMaterial(m_selectedMeshId,mat->second);
        m_applyMatToMesh = false;
        this->hide();
        return;
    }
    if(m_loadToHyperShader){
        std::string path = "NodeGraphs/" + matName + ".hel";
        QFileInfo nodeGraph(path.c_str());
        if(nodeGraph.exists()){
            std::cerr<<"Loading material "<<matName<<" to hypershader"<<std::endl;
            AbstractMaterialWidget::getInstance()->setMaterial(mat->second);
            AbstractMaterialWidget::getInstance()->loadNodeGraph(path.c_str());
            m_loadToHyperShader = false;
            this->hide();
            return;

        }
        else{
            QMessageBox::warning(this,"Material Library Error","Cannot find node graph of material");
        }
    }
}
