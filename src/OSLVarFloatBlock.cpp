#include "OSLVarFloatBlock.h"

//------------------------------------------------------------------------------------------------------------------------------------
OSLVarFloatBlock::OSLVarFloatBlock(QGraphicsItem *parent) : QNEBlock(parent)
{
}

//------------------------------------------------------------------------------------------------------------------------------------
void OSLVarFloatBlock::setValue(float _value)
{
    //set our new value
    m_value = _value;
    std::vector<std::string> x;
    //if we haven't added our ports yet lets add them
    if(childItems().size()==0){
        addPort("float",0,x,QNEPort::TypeVoid, QNEPort::TypePort);
        addOutputPort(std::to_string(m_value).c_str(),x,QNEPort::TypeFloat);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------
