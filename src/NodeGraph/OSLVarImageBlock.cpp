#include "NodeGraph/OSLVarImageBlock.h"
#include "NodeGraph/ImageNodeProxyWidget.h"

//------------------------------------------------------------------------------------------------------------------------------------
OSLVarImageBlock::OSLVarImageBlock(QGraphicsScene *_scene, optix::Material &_mat, QGraphicsItem *parent) : OSLAbstractVarBlock(_scene,_mat,parent)
{
    //draw our block a little bigger
    setWidth(255);
    setHeight(255);
    //set the name of our block
    setBlockName("Image");
    //create our spin box
    std::vector<std::string> zeroInitParams;
    QNEPort* varPort = addOutputPort("",zeroInitParams,QNEPort::TypeString);
    //Now lets add our float spin box which will change the value of our variable
    m_widgetProxy = new ImageNodeProxyWidget(varPort,m_material,this);
}
//------------------------------------------------------------------------------------------------------------------------------------
OSLVarImageBlock::~OSLVarImageBlock(){
}
//------------------------------------------------------------------------------------------------------------------------------------
