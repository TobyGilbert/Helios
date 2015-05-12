#include "NodeGraph/AbstractNodeProxyWidget.h"
#include "NodeGraph/OSLShaderBlock.h"
#include "NodeGraph/qneconnection.h"


//------------------------------------------------------------------------------------------------------------------------------------
AbstractNodeProxyWidget::AbstractNodeProxyWidget(QNEPort *_connectedPort, Material &_mat, QGraphicsItem *_parent) : QGraphicsProxyWidget(_parent)
{
    m_material = _mat;
    m_connectedPort = _connectedPort;
}
//------------------------------------------------------------------------------------------------------------------------------------
void AbstractNodeProxyWidget::getLinkedVarName(std::vector<std::string> &_linkedVarNames){
    foreach(QNEConnection *c,m_connectedPort->connections()){
        std::string name;
        //find the input variable we are setting
        if(!c->port1()->isOutput()){
            OSLShaderBlock* b = (OSLShaderBlock*)c->port1()->block();
            name = b->getBlockName() + c->port1()->getName().toStdString();
            _linkedVarNames.push_back(name);
        }
        else{
            OSLShaderBlock* b = (OSLShaderBlock*)c->port2()->block();
            name = b->getBlockName() + c->port2()->getName().toStdString();
            _linkedVarNames.push_back(name);
        }
    }
}
//------------------------------------------------------------------------------------------------------------------------------------
