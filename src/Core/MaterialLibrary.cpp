#include "Core/MaterialLibrary.h"

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
    //set our layout
    m_widgetLayout = new QGridLayout(this);
    this->setLayout(m_widgetLayout);
    m_widgetSpacer = new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    //if we already have materials in our class lets add
    //the relative buttons
    if(m_materials.size()>0){
        QString name;
        for(unsigned int i=0; i<m_materials.size(); i++){
            name = QString(m_materials[i]->getName().c_str());
            QPushButton *btn = new QPushButton(name,this);
            connect(btn,SIGNAL(clicked()),this,SLOT(materialSelected()));
            btn->setCheckable(true);
            m_widgetLayout->addWidget(btn,i,0,1,1);
            m_buttons.push_back(btn);
        }
        m_widgetLayout->addItem(m_widgetSpacer,m_materials.size(),0,1,1);
    }
    m_widgetLayout->addItem(m_widgetSpacer,0,0,1,1);
}
//----------------------------------------------------------------------------------------------------------------------
MaterialLibrary::~MaterialLibrary(){
    for(unsigned int i=0; i<m_buttons.size(); i++){
        delete m_buttons[i];
    }
}
//----------------------------------------------------------------------------------------------------------------------
void MaterialLibrary::addMaterialToLibrary(AbstractMaterialWidget *_material){
    //add our material to our library
    m_materials.push_back(_material);
    QString name(_material->getName().c_str());
    //create the button for our material
    QPushButton *btn = new QPushButton(name,this);
    connect(btn,SIGNAL(clicked()),this,SLOT(materialSelected()));
    btn->setCheckable(true);
    m_widgetLayout->addWidget(btn,m_materials.size()-1,0,1,1);
    m_buttons.push_back(btn);
    //move our spacer
    m_widgetLayout->removeItem(m_widgetSpacer);
    m_widgetLayout->addItem(m_widgetSpacer,m_materials.size(),0,1,1);
}
//----------------------------------------------------------------------------------------------------------------------
void MaterialLibrary::materialSelected(){
    int idx=0;
    //find which button has been pressed
    for(unsigned int i=0; i<m_buttons.size(); i++){
        if(m_buttons[i]->isChecked()){
            idx = i;
            break;
        }
    }
    //untoggle the button for next use
    m_buttons[idx]->toggle();
    //signal that we have selected a material
    signalMaterialSelected(m_materials[idx]);
    //hide this widget
    this->hide();
}
//----------------------------------------------------------------------------------------------------------------------
