#include "NodeGraph/OSLVarFloatThreeBlock.h"
#include "NodeGraph/FloatThreeNodeProxyWidget.h"

OSLVarFloatThreeBlock::OSLVarFloatThreeBlock(QGraphicsScene *_scene, optix::Material &_mat, QGraphicsItem *parent) : OSLAbstractVarBlock(_scene,_mat,parent)
{
    //draw our block a little bigger
    setWidth(50);
    setHeight(140);
    //set the name of our block
    setBlockName("float3");
    //create our output port
    QNEPort* varPort = addOutputPort("","",QNEPort::TypeColour);
    //Now lets add our float spin box which will change the value of our variable
    FloatThreeNodeProxyWidget *floatProxy = new FloatThreeNodeProxyWidget(varPort,m_material,this);

}
