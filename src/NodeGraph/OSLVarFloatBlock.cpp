#include "NodeGraph/OSLVarFloatBlock.h"
#include "NodeGraph/FloatNodeProxyWidget.h"
//------------------------------------------------------------------------------------------------------------------------------------
OSLVarFloatBlock::OSLVarFloatBlock(QGraphicsScene *_scene, optix::Material &_mat, QGraphicsItem *parent) : OSLAbstractVarBlock(_scene,_mat,parent)
{
    //draw our block a little bigger
    setWidth(30);
    setHeight(20);
    //set the name of our block
    setBlockName("float");
    //create our spin box
    QNEPort* varPort = addOutputPort("","",QNEPort::TypeFloat);
    //Now lets add our float spin box which will change the value of our variable
    FloatNodeProxyWidget *floatProxy = new FloatNodeProxyWidget(varPort,m_material,this);
}
//------------------------------------------------------------------------------------------------------------------------------------
OSLVarFloatBlock::~OSLVarFloatBlock(){
}
//------------------------------------------------------------------------------------------------------------------------------------
