#include "NodeGraph/OSLVarColorBlock.h"
#include "NodeGraph/FloatThreeNodeProxyWidget.h"

OSLVarColorBlock::OSLVarColorBlock(QGraphicsScene *_scene, optix::Material &_mat, QGraphicsItem *parent) : OSLAbstractVarBlock(_scene,_mat,parent)
{
    //draw our block a little bigger
    setWidth(50);
    setHeight(140);
    //set the name of our block
    setBlockName("Color");
    //create our output port
    std::vector<std::string> zeroParams;
    QNEPort* varPort = addOutputPort("",zeroParams,QNEPort::TypeColour);
    //Now lets add our float spin box which will change the value of our variable
    FloatThreeNodeProxyWidget *floatProxy = new FloatThreeNodeProxyWidget(varPort,m_material,this);

}
