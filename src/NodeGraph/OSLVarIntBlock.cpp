#include "include/NodeGraph/OSLVarIntBlock.h"
#include "NodeGraph/IntNodeProxyWidget.h"
OSLVarIntBlock::OSLVarIntBlock(QGraphicsScene *_scene, Material _mat, QGraphicsItem *parent) : OSLAbstractVarBlock(_scene,_mat,parent)
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
OSLVarIntBlock::~OSLVarIntBlock()
{
}
//------------------------------------------------------------------------------------------------------------------------------------
void OSLVarIntBlock::save(QDataStream &ds)
{
    //write the type of varblock we are so one could know
    //what class to create from reading the file
    ds<<QNEPort::TypeInt;
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

    IntNodeProxyWidget * proxy = ((IntNodeProxyWidget*)m_widgetProxy);
    ds << proxy->m_spinBox->value();
}

//------------------------------------------------------------------------------------------------------------------------------------
void OSLVarIntBlock::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap)
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

    //get the values of our spinbox
    int x;
    ds >> x;
    IntNodeProxyWidget * proxy = ((IntNodeProxyWidget*)m_widgetProxy);
    proxy->m_spinBox->setValue(x);

}

//------------------------------------------------------------------------------------------------------------------------------------

