#include "include/NodeGraph/OSLVarIntBlock.h"
#include "NodeGraph/IntNodeProxyWidget.h"
OSLVarIntBlock::OSLVarIntBlock(QGraphicsScene *_scene, optix::Material &_mat, QGraphicsItem *parent) : OSLAbstractVarBlock(_scene,_mat,parent)
{
    //draw our block a little bigger
    setWidth(30);
    setHeight(20);
    //set the name of our block
    setBlockName("Int");
    //create our spin box
    std::vector<std::string> zeroParams;
    QNEPort* varPort = addOutputPort("",zeroParams,QNEPort::TypeInt);
    //Now lets add our float spin box which will change the value of our variable
    m_widgetProxy = new IntNodeProxyWidget(varPort,m_material,this);
}
//------------------------------------------------------------------------------------------------------------------------------------
OSLVarIntBlock::~OSLVarIntBlock(){
}
//------------------------------------------------------------------------------------------------------------------------------------
