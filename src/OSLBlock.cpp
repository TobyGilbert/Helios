#include "OSLBlock.h"
#include <QPen>
#include <QFileDialog>
#include <QColor>

OSLBlock::OSLBlock(QGraphicsItem *parent) : QNEBlock(parent)
{

    QPainterPath p2;
    p2.addRoundedRect(-25,-10,30,10,5,5);
    m_importBtnGI = new QGraphicsPathItem(this);
    m_importBtnGI->setPath(p2);
    m_importBtnGI->setPen(QPen(QColor(177,177,177)));
    m_importBtnGI->setBrush(QBrush(QColor(177,177,177)));

}

//----------------------------------------------------------------------------------------------------------------------
void OSLBlock::mousePressEvent(QGraphicsSceneMouseEvent *_event)
{
    //get the possition of our button and the positions of our mouse
    QRectF btnBB = m_importBtnGI->path().boundingRect();
    QPointF pos = _event->pos();
    //test for intersection of the two
    if(pos.x()>btnBB.x()&&pos.x()<(btnBB.x()+btnBB.width())&&(pos.y()>btnBB.y()&&pos.y()<(btnBB.y()+btnBB.height()))){
        //if we have an intersection lets allow the user to select a file in import
        QString location = QFileDialog::getOpenFileName(0,QString("Import Shader"), QString("shaders/"), QString("OSL files (*.osl)"));
        //if nothing selected then we dont want to do anything
        if(location.isEmpty()) return;
        //if we do have something selected lets run it through Toby Gilbert's OSL Compilotmatic 3000
        OslReader shade;
        shade.compileOSL(location);

        OsoReader* reader = getOsoReader();
        reader->printVersion();
        reader->printShader();
        reader->printParams();
        reader->generateDeviceFunction();

        //add our input ports required by our shader
        std::vector<Symbol> symbols = reader->getInputParams();
        std::cout<<symbols.size()<<std::endl;
        for (unsigned int i=0; i<symbols.size(); i++){
            std::cout<<symbols[i].m_name<<std::endl;
            addInputPort(QString(symbols[i].m_name.c_str()),symbols[i].m_initialParams,(QNEPort::variableType)symbols[i].m_type);
        }

        //add our input ports required by our shader
        std::vector<Symbol> outputSymbols = reader->getOutputParams();
        std::cout<<outputSymbols.size()<<std::endl;
        for (unsigned int i=0; i<outputSymbols.size(); i++){
            std::cout<<outputSymbols[i].m_name<<std::endl;
            addOutputPort(QString(outputSymbols[i].m_name.c_str()),outputSymbols[i].m_initialParams,(QNEPort::variableType)outputSymbols[i].m_type);
        }



    }
}
//----------------------------------------------------------------------------------------------------------------------
