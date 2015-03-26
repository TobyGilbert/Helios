#include "NodeGraph/OSLVarFloatBlock.h"
#include <QGraphicsProxyWidget>

//------------------------------------------------------------------------------------------------------------------------------------
OSLVarFloatBlock::OSLVarFloatBlock(QGraphicsScene *_scene, QGraphicsItem *parent) : QNEBlock(parent)
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

    QGraphicsProxyWidget *widgetProxy = new QGraphicsProxyWidget(this);
    m_value = new QDoubleSpinBox();
    m_value->setMinimum((float)-INFINITY);
    m_value->setMaximum((float)INFINITY);
    m_value->setMinimumWidth(70);
    m_value->move(-35,-5);
    widgetProxy->setWidget(m_value);
}
