#include "Core/MaterialLibrary.h"
#include <QMessageBox>
#include <iostream>
#include "Core/pathtracerscene.h"

//Declare our static instance variable
MaterialLibrary* MaterialLibrary::m_instance;

MaterialLibrary* MaterialLibrary::getInstance(){
    if(!m_instance){
        m_instance = new MaterialLibrary();
    }
    return m_instance;
}
//----------------------------------------------------------------------------------------------------------------------
MaterialLibrary::MaterialLibrary(QWidget *parent) :
    QWidget(parent)
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
    m_widgetSpacer = new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);


}
//----------------------------------------------------------------------------------------------------------------------
MaterialLibrary::~MaterialLibrary(){
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
    m_selectedMeshId = _meshId;
}
//----------------------------------------------------------------------------------------------------------------------
void MaterialLibrary::closeEvent(QCloseEvent *event)
{
    m_applyMatToMesh = false;
}
//----------------------------------------------------------------------------------------------------------------------
void MaterialLibrary::matSelected(QListWidgetItem *_item){
    std::string matName = _item->text().toStdString();
    if(m_applyMatToMesh){
        std::map <std::string, optix::Material >::const_iterator mat=m_materials.find(matName);
        if(mat==m_materials.end()){
            QMessageBox::warning(this,"Material Library Error","Unknown error");
            return;
        }
        std::cerr<<"Applying material "<<mat->first<<" to mesh "<<m_selectedMeshId<<std::endl;
        PathTracerScene::getInstance()->setModelMaterial(m_selectedMeshId,mat->second);
        m_applyMatToMesh = false;
        this->hide();
        return;
    }
}
