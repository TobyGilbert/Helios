#include "NodeGraph/FloatNodeProxyWidget.h"

//------------------------------------------------------------------------------------------------------------------------------------
FloatNodeProxyWidget::FloatNodeProxyWidget(QNEPort *_portConnected, Material &_mat, QGraphicsItem *parent) :
    AbstractNodeProxyWidget(_portConnected,_mat,parent)
{
    m_spinBox = new QDoubleSpinBox();
    m_spinBox->setMinimum((float)-INFINITY);
    m_spinBox->setMaximum((float)INFINITY);
    //move into position
    m_spinBox->setMinimumWidth(70);
    m_spinBox->move(-35,-5);
    setWidget(m_spinBox);
    setGraphicsItem(parent);
    connect(m_spinBox,SIGNAL(valueChanged(double)),this,SLOT(setMaterialVariables(double)));
}
//------------------------------------------------------------------------------------------------------------------------------------
void FloatNodeProxyWidget::setMaterialVariables(double _val){
    //get all our linked variable names
    std::vector<std::string> _varNames;
    getLinkedVarName(_varNames);

    //set all our variables in our material
    for(unsigned int i=0;i<_varNames.size();i++){
        m_material[_varNames[i].c_str()]->setFloat(_val);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------
void FloatNodeProxyWidget::setLinkedVar(){
    //get all our linked variable names
    std::vector<std::string> _varNames;
    getLinkedVarName(_varNames);

    //set all our variables in our material
    for(unsigned int i=0;i<_varNames.size();i++){
        m_material[_varNames[i].c_str()]->setFloat(m_spinBox->value());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------
FloatNodeProxyWidget::~FloatNodeProxyWidget(){
}

//------------------------------------------------------------------------------------------------------------------------------------
