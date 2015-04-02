#include "NodeGraph/OSLVarFloatBlock.h"
#include <QGraphicsProxyWidget>
#include "NodeGraph/qneconnection.h"
#include "NodeGraph/OSLShaderBlock.h"
#include <math.h>
//------------------------------------------------------------------------------------------------------------------------------------
OSLVarFloatBlock::OSLVarFloatBlock(QGraphicsScene *_scene, optix::Material &_mat, QGraphicsItem *parent) : QNEBlock(parent)
{
    // add this block to our scene
    _scene->addItem(this);
    setWidth(30);
    setHeight(20);
    //if we haven't added our ports yet lets add them
    if(childItems().size()==0){
        addPort("float",0,"",QNEPort::TypeVoid, QNEPort::TypePort);
        addOutputPort("","",QNEPort::TypeFloat);
    }

    //set our material
    m_material = _mat;

    QGraphicsProxyWidget *widgetProxy = new QGraphicsProxyWidget(this);
    m_value = new QDoubleSpinBox();
    m_value->setMinimum((float)-INFINITY);
    m_value->setMaximum((float)INFINITY);
    m_value->setMinimumWidth(70);
    m_value->move(-35,-5);
    widgetProxy->setWidget(m_value);
    //set up our signals and slots
//    connect(m_value,SIGNAL(valueChanged(double)),this,SLOT(setValue(double)));
}
//------------------------------------------------------------------------------------------------------------------------------------
//void OSLVarFloatBlock::setValue(double _val)
//{
//    QVector<QNEPort*> p = ports();
//    foreach(QNEConnection *c,p[0]->connections()){
//        std::string name;
//        //find the input variable we are setting
//        if(!c->port1()->isOutput()){
//            OSLShaderBlock* b = (OSLShaderBlock*)c->port1()->block();
//            name = b->getShaderName() + c->port1()->getName().toStdString();
//            m_material[name.c_str()]->setFloat((float)_val);
//        }
//        else{
//            OSLShaderBlock* b = (OSLShaderBlock*)c->port2()->block();
//            name = b->getShaderName() + c->port2()->getName().toStdString();
//            m_material[name.c_str()]->setFloat((float)_val);
//        }
//    }
//}
