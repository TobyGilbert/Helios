#include "NodeGraph/OSLAbstractVarBlock.h"

OSLAbstractVarBlock::OSLAbstractVarBlock(QGraphicsScene *_scene,optix::Material &_mat,QGraphicsItem * parent) : QNEBlock(parent)
{
    // add this block to our scene
    _scene->addItem(this);
    //set our material
    m_material = _mat;
}
//------------------------------------------------------------------------------------------------------------------------------------
void OSLAbstractVarBlock::setBlockName(std::string _name){
    m_blockName = _name;
    std::vector<std::string> zeroParams;
    addPort(_name.c_str(),0,zeroParams,QNEPort::TypeVoid, QNEPort::TypePort);
}

//------------------------------------------------------------------------------------------------------------------------------------
