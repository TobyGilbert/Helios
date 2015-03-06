#include "AbstractMaterialWidget.h"
#include "pathtracerscene.h"

AbstractMaterialWidget::AbstractMaterialWidget(QWidget *parent) :
    QWidget(parent)
{
    //set our widget layout
    m_widgetLayout = new QGridLayout(this);
    this->setLayout(m_widgetLayout);
    //add our groupbox
    m_widgetGroupBox = new QGroupBox(this);
    m_widgetLayout->addWidget(m_widgetGroupBox,0,0,1,1);
    m_groupBoxLayout = new QGridLayout(this);
    m_widgetGroupBox->setLayout(m_groupBoxLayout);

    //create a material in our context
    m_material = PathTracerScene::getInstance()->getContext()->createMaterial();
}
//------------------------------------------------------------------------------------------------------------------------------------
AbstractMaterialWidget::~AbstractMaterialWidget(){
    for(unsigned int i=0; i<m_widgetObjects.size();i++){
        delete m_widgetObjects[i];
    }
    delete m_widgetLayout;
    delete m_widgetGroupBox;
}
//------------------------------------------------------------------------------------------------------------------------------------
