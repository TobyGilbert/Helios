#include "NodeGraph/OSLShaderBlock.h"
#include "OSLCompiler/OslReader.h"
#include "OSLCompiler/OsoReader.h"

//------------------------------------------------------------------------------------------------------------------------------------
OSLShaderBlock::OSLShaderBlock(QGraphicsItem *parent) : QNEBlock(parent)
{
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
