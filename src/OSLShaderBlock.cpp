#include "OSLShaderBlock.h"
#include "OslReader.h"
#include "OsoReader.h"

//------------------------------------------------------------------------------------------------------------------------------------
OSLShaderBlock::OSLShaderBlock(QGraphicsItem *parent) : QNEBlock(parent)
{
}
//------------------------------------------------------------------------------------------------------------------------------------
void OSLShaderBlock::loadShader(QString _path){
    //lets run our OSL shader through Toby Gilbert's OSL Compilotmatic 3000
    OslReader shade;
    shade.compileOSL(_path);

    OsoReader* reader = getOsoReader();
    reader->printVersion();
    reader->printShader();
    reader->printParams();
    reader->generateDeviceFunction();
    std::vector<std::string>x;
    addPort(reader->getShaderName().c_str(),0,x,QNEPort::TypeVoid, QNEPort::NamePort);
    addPort(reader->getShaderType().c_str(),0,x,QNEPort::TypeVoid, QNEPort::TypePort);

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

//------------------------------------------------------------------------------------------------------------------------------------
