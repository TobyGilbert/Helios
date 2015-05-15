#include "NodeGraph/OSLShaderBlock.h"
#include "OSLCompiler/OslReader.h"
#include "OSLCompiler/OsoReader.h"
#include <QString>

//------------------------------------------------------------------------------------------------------------------------------------
OSLShaderBlock::OSLShaderBlock(QGraphicsItem *parent) : QNEBlock(parent)
{
}
//------------------------------------------------------------------------------------------------------------------------------------
void OSLShaderBlock::save(QDataStream &ds){
    //give the position of our block in the scene
    ds<<pos();
    int count(0);
    //calculate the number of ports we have
    foreach(QGraphicsItem *port_, childItems())
    {
        if (port_->type() != QNEPort::Type)
            continue;

        count++;
    }
    //write in the number of ports we have
    ds << count;

    //write in our port information
    foreach(QGraphicsItem *port_, childItems())
    {
        if (port_->type() != QNEPort::Type)
            continue;

        QNEPort *port = (QNEPort*) port_;
        ds << (quint64) port;
        ds << port->portName();
        ds << port->isOutput();
        //write our port variable type
        ds << port->getVaribleType();
        //write how many init params it has
        std::vector<std::string> initParams = port->getInitParams();
        std::cout<<"initParamsSize"<<(int)initParams.size()<<std::endl;
        ds << (int)initParams.size();
        //write in our init params
        for(unsigned int i=0;i<initParams.size();i++){
            ds << QString(initParams[i].c_str());
        }
    }

    //write our shader name and our cuda kernal
    ds << QString(m_shaderName.c_str());
    ds << QString(m_cudaKernal.c_str());


}
//------------------------------------------------------------------------------------------------------------------------------------
void OSLShaderBlock::load(QDataStream &ds, QMap<quint64, QNEPort *> &portMap){
    //first load in our postion
    QPointF p;
    ds >> p;
    setPos(p);

    //get the number of ports we have in our block
    int count;
    ds >> count;

    for (int i = 0; i < count; i++)
    {
        QString name;
        bool output;
        quint64 ptr;
        //our variable type
        int varType;

        ds >> ptr;
        ds >> name;
        ds >> output;
        ds >> varType;

        //now lets get our input params
        int numInitParams;
        ds >> numInitParams;
        std::vector<std::string> initParams;
        QString tempString;
        for(int j=0;j<numInitParams;j++){
            ds >> tempString;
            initParams.push_back(tempString.toStdString());
        }

        switch(i){
        case(0):portMap[ptr] = addPort(name, output,initParams, (QNEPort::variableType)varType, QNEPort::NamePort, ptr); break;
        case(1):portMap[ptr] = addPort(name, output,initParams, (QNEPort::variableType)varType, QNEPort::TypePort, ptr); break;
        default: portMap[ptr] = addPort(name, output,initParams, (QNEPort::variableType)varType, 0, ptr); break;
        }
    }

    //set our shader name and kernal
    QString sn, ck;
    ds >> sn;
    m_shaderName = sn.toStdString();
    ds >> ck;
    m_cudaKernal = ck.toStdString();
}

//------------------------------------------------------------------------------------------------------------------------------------
bool OSLShaderBlock::loadShader(QString _path){
    //lets run our OSL shader through Toby Gilbert's OSL Compilotmatic 3000
    OslReader shade;
    if(!shade.compileOSLtoBuffer(_path)){
        return false;
    }

    OsoReader* reader = getOsoReader();
    reader->printVersion();
    reader->printShader();
    reader->printParams();
    m_cudaKernal = reader->generateDeviceFunction();
    m_shaderName = reader->getShaderName();
    std::vector<std::string> zeroInitParams;
    addPort(reader->getShaderName().c_str(),false,zeroInitParams,QNEPort::TypeVoid, QNEPort::NamePort);
    addPort(reader->getShaderType().c_str(),false,zeroInitParams,QNEPort::TypeVoid, QNEPort::TypePort);

    //add our input ports required by our shader
    std::vector<Symbol> symbols = reader->getInputParams();
    std::cout<<symbols.size()<<std::endl;
    for (unsigned int i=0; i<symbols.size(); i++){
        std::cout<<symbols[i].m_name<<std::endl;
        std::string name = symbols[i].m_name;
        addInputPort(name.c_str(),symbols[i].m_initialParams,(QNEPort::variableType)symbols[i].m_type);
    }

    //add our input ports required by our shader
    std::vector<Symbol> outputSymbols = reader->getOutputParams();
    std::cout<<"out params size "<<outputSymbols.size()<<std::endl;
    std::cout<<outputSymbols.size()<<std::endl;
    for (unsigned int i=0; i<outputSymbols.size(); i++){
        std::cout<<outputSymbols[i].m_name<<std::endl;
        addOutputPort(QString(outputSymbols[i].m_name.c_str()),outputSymbols[i].m_initialParams,(QNEPort::variableType)outputSymbols[i].m_type);
    }
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------
