#include "NodeGraph/AbstractNodeProxyWidget.h"
#include "NodeGraph/OSLShaderBlock.h"
#include "NodeGraph/qneconnection.h"

//------------------------------------------------------------------------------------------------------------------------------------
AbstractNodeProxyWidget::AbstractNodeProxyWidget(Material &_mat, QGraphicsItem *_parent) : QGraphicsProxyWidget(_parent)
{
    m_material = _mat;
}
//------------------------------------------------------------------------------------------------------------------------------------
void AbstractNodeProxyWidget::getLinkedVarName(QNEPort *_port, std::vector<std::string> &_linkedVarNames){
    foreach(QNEConnection *c,_port->connections()){
        std::string name;
        //find the input variable we are setting
        if(!c->port1()->isOutput()){
            OSLShaderBlock* b = (OSLShaderBlock*)c->port1()->block();
            name = b->getShaderName() + c->port1()->getName().toStdString();
            _linkedVarNames.push_back(name);
        }
        else{
            OSLShaderBlock* b = (OSLShaderBlock*)c->port2()->block();
            name = b->getShaderName() + c->port2()->getName().toStdString();
            _linkedVarNames.push_back(name);
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------
