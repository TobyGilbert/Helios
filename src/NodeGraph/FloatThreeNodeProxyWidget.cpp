#include "NodeGraph/FloatThreeNodeProxyWidget.h"
#include <QGridLayout>


FloatThreeNodeProxyWidget::FloatThreeNodeProxyWidget(QNEPort *_portConnected,optix::Material &_mat,QGraphicsItem *parent) : AbstractNodeProxyWidget(_portConnected,_mat,parent)
{
    m_groupBox = new QGroupBox();
    setWidget(m_groupBox);
    m_groupBox->move(-40,-60);
    m_groupBox->setMaximumHeight(130);
    m_groupBox->setMaximumWidth(80);
    QGridLayout *layout = new QGridLayout(m_groupBox);
    m_groupBox->setLayout(layout);
    m_spinBoxX = new QDoubleSpinBox(m_groupBox);
    m_spinBoxX->setMinimum((float)-INFINITY);
    m_spinBoxX->setMaximum((float)INFINITY);
    layout->addWidget(m_spinBoxX,0,0,1,1);
    connect(m_spinBoxX,SIGNAL(editingFinished()),this,SLOT(setMaterialVars()));
    m_spinBoxY = new QDoubleSpinBox(m_groupBox);
    m_spinBoxY->setMinimum((float)-INFINITY);
    m_spinBoxY->setMaximum((float)INFINITY);
    layout->addWidget(m_spinBoxY,1,0,1,1);
    connect(m_spinBoxY,SIGNAL(editingFinished()),this,SLOT(setMaterialVars()));
    m_spinBoxZ = new QDoubleSpinBox(m_groupBox);
    m_spinBoxZ->setMinimum((float)-INFINITY);
    m_spinBoxZ->setMaximum((float)INFINITY);
    layout->addWidget(m_spinBoxZ,2,0,1,1);
    connect(m_spinBoxZ,SIGNAL(editingFinished()),this,SLOT(setMaterialVars()));
}
//------------------------------------------------------------------------------------------------------------------------------------
void FloatThreeNodeProxyWidget::setMaterialVars(){
    //get all our linked variable names
    std::vector<std::string> _varNames;
    getLinkedVarName(_varNames);

    //set all our variables in our material
    for(unsigned int i=0;i<_varNames.size();i++){
        m_material[_varNames[i].c_str()]->setFloat(m_spinBoxX->value(),m_spinBoxY->value(),m_spinBoxZ->value());
    }
}
//------------------------------------------------------------------------------------------------------------------------------------
void FloatThreeNodeProxyWidget::setLinkedVar(){
    //get all our linked variable names
    std::vector<std::string> _varNames;
    getLinkedVarName(_varNames);

    //set all our variables in our material
    for(unsigned int i=0;i<_varNames.size();i++){
        m_material[_varNames[i].c_str()]->setFloat(m_spinBoxX->value(),m_spinBoxY->value(),m_spinBoxZ->value());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------
FloatThreeNodeProxyWidget::~FloatThreeNodeProxyWidget(){
}

//------------------------------------------------------------------------------------------------------------------------------------
