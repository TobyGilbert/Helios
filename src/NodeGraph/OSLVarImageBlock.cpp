#include "NodeGraph/OSLVarImageBlock.h"
#include "NodeGraph/ImageNodeProxyWidget.h"

//------------------------------------------------------------------------------------------------------------------------------------
OSLVarImageBlock::OSLVarImageBlock(QGraphicsScene *_scene, Material _mat, QGraphicsItem *parent) : OSLAbstractVarBlock(_scene,_mat,parent)
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
OSLVarImageBlock::~OSLVarImageBlock()
{
}
//------------------------------------------------------------------------------------------------------------------------------------
void OSLVarImageBlock::save(QDataStream &ds)
{
    //write the type of varblock we are so one could know
    //what class to create from reading the file
    ds<<QNEPort::TypeString;
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

    ImageNodeProxyWidget * proxy = ((ImageNodeProxyWidget*)m_widgetProxy);
    ds << proxy->getImagePath();

}

//------------------------------------------------------------------------------------------------------------------------------------
void OSLVarImageBlock::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap)
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

    //get the path to our image
    QString path;
    ds >> path;

    ImageNodeProxyWidget * proxy = ((ImageNodeProxyWidget*)m_widgetProxy);
    proxy->loadImage(path);


}

//------------------------------------------------------------------------------------------------------------------------------------

