#include "NodeGraph/OSLVarFloatThreeBlock.h"
#include "NodeGraph/FloatThreeNodeProxyWidget.h"

OSLVarFloatThreeBlock::OSLVarFloatThreeBlock(QGraphicsScene *_scene, optix::Material _mat, QGraphicsItem *parent) : OSLAbstractVarBlock(_scene,_mat,parent)
{
    //draw our block a little bigger
    setWidth(50);
    setHeight(140);
    //set the name of our block
    setBlockName("Vector");
    //create our output port
    std::vector<std::string> zeroParams;
    QNEPort* varPort = addOutputPort("",zeroParams,QNEPort::TypeVector);
    //Now lets add our float spin box which will change the value of our variable
    m_widgetProxy = new FloatThreeNodeProxyWidget(varPort,m_material,this);

}
//------------------------------------------------------------------------------------------------------------------------------------
void OSLVarFloatThreeBlock::save(QDataStream &ds)
{
    //write the type of varblock we are so one could know
    //what class to create from reading the file
    ds<<QNEPort::TypeVector;
    //give the postion of our block
    ds<<pos();
    //write in our port information
    foreach(QGraphicsItem *port_, childItems())
    {
        if (port_->type() != QNEPort::Type)
            continue;
        //we need this for linking our ports back up when we load
        //the data back in
        QNEPort *port = (QNEPort*) port_;
        ds << (quint64) port;
    }

    FloatThreeNodeProxyWidget * proxy = ((FloatThreeNodeProxyWidget*)m_widgetProxy);
    ds << proxy->m_spinBoxX->value();
    ds << proxy->m_spinBoxY->value();
    ds << proxy->m_spinBoxZ->value();
}

//------------------------------------------------------------------------------------------------------------------------------------
void OSLVarFloatThreeBlock::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap)
{
    //first load in our postion
    QPointF p;
    ds >> p;
    setPos(p);


    //write in our port information
    foreach(QGraphicsItem *port_, childItems())
    {
        if (port_->type() != QNEPort::Type)
            continue;
        quint64 ptr;
        ds >> ptr;
        portMap[ptr] = (QNEPort*) port_;
    }

    //get the values of our spinboxes
    double x,y,z;
    ds >> x;
    ds >> y;
    ds >> z;
    FloatThreeNodeProxyWidget * proxy = ((FloatThreeNodeProxyWidget*)m_widgetProxy);
    proxy->m_spinBoxX->setValue(x);
    proxy->m_spinBoxY->setValue(y);
    proxy->m_spinBoxZ->setValue(z);


}

//------------------------------------------------------------------------------------------------------------------------------------
