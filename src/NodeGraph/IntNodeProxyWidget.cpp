#include "include/NodeGraph/IntNodeProxyWidget.h"

IntNodeProxyWidget::IntNodeProxyWidget(QNEPort *_portConnected, Material &_mat, QGraphicsItem *parent) :
    AbstractNodeProxyWidget(_portConnected,_mat,parent)
{
    m_spinBox = new QSpinBox();
    m_spinBox->setMinimum(-(int)INFINITY);
    m_spinBox->setMaximum(INFINITY);
    //move into position
    m_spinBox->setMaximumWidth(70);
    m_spinBox->move(-35,-5);
    setWidget(m_spinBox);
    setGraphicsItem(parent);
    connect(m_spinBox,SIGNAL(valueChanged(int)),this,SLOT(setMaterialVariables(int)));
}
//------------------------------------------------------------------------------------------------------------------------------------
void IntNodeProxyWidget::setMaterialVariables(int _val){
    //get all our linked variable names
    std::vector<std::string> _varNames;
    getLinkedVarName(_varNames);

    //set all our variables in our material
    for(unsigned int i=0;i<_varNames.size();i++){
        m_material[_varNames[i].c_str()]->setInt(_val);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------
void IntNodeProxyWidget::setLinkedVar(){
    //get all our linked variable names
    std::vector<std::string> _varNames;
    getLinkedVarName(_varNames);

    //set all our variables in our material
    for(unsigned int i=0;i<_varNames.size();i++){
        m_material[_varNames[i].c_str()]->setInt(m_spinBox->value());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------
IntNodeProxyWidget::~IntNodeProxyWidget(){
}
//------------------------------------------------------------------------------------------------------------------------------------
