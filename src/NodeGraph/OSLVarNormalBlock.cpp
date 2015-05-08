#include "NodeGraph/OSLVarNormalBlock.h"
#include "NodeGraph/FloatThreeNodeProxyWidget.h"

OSLVarNormalBlock::OSLVarNormalBlock(QGraphicsScene *_scene, optix::Material &_mat, QGraphicsItem *parent) : OSLAbstractVarBlock(_scene,_mat,parent)
{
    //draw our block a little bigger
    setWidth(50);
    setHeight(140);
    //set the name of our block
    setBlockName("Normal");
    //create our output port
    std::vector<std::string> zeroParams;
    QNEPort* varPort = addOutputPort("",zeroParams,QNEPort::TypeNormal);
    //Now lets add our float spin box which will change the value of our variable
    new FloatThreeNodeProxyWidget(varPort,m_material,this);
}
