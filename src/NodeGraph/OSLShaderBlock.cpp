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
    if(!shade.compileOSL(_path)){
        return false;
    }

    OsoReader* reader = getOsoReader();
    reader->printVersion();
    reader->printShader();
    reader->printParams();
    m_cudaKernal = reader->generateDeviceFunction();
    m_shaderName = reader->getShaderName();
    addPort(reader->getShaderName().c_str(),0,"",QNEPort::TypeVoid, QNEPort::NamePort);
    addPort(reader->getShaderType().c_str(),0,"",QNEPort::TypeVoid, QNEPort::TypePort);

    //add our input ports required by our shader
    std::vector<Symbol> symbols = reader->getInputParams();
    std::cout<<symbols.size()<<std::endl;
    for (unsigned int i=0; i<symbols.size(); i++){
        std::cout<<symbols[i].m_name<<std::endl;
        std::string initParam;
        std::string name = symbols[i].m_name;

        if(symbols[i].m_initialParams.size()==1){
            initParam = symbols[i].m_initialParams[0];
        }
        else if(symbols[i].m_initialParams.size()==3){
            initParam = "make_float3(" + symbols[i].m_initialParams[0] + "," + symbols[i].m_initialParams[1] + "," + symbols[i].m_initialParams[2] + ")";
        }
        addInputPort(name.c_str(),initParam.c_str(),(QNEPort::variableType)symbols[i].m_type);
    }

    //add our input ports required by our shader
    std::vector<Symbol> outputSymbols = reader->getOutputParams();
    std::cout<<outputSymbols.size()<<std::endl;
    for (unsigned int i=0; i<outputSymbols.size(); i++){
        std::cout<<outputSymbols[i].m_name<<std::endl;
        std::string initParam;

        if(outputSymbols[i].m_initialParams.size()==1){
            initParam = outputSymbols[i].m_initialParams[0];
        }
        else if(outputSymbols[i].m_initialParams.size()==3){
            initParam = "make_float3(" + outputSymbols[i].m_initialParams[0] + "," + outputSymbols[i].m_initialParams[1] + "," + outputSymbols[i].m_initialParams[2] + ")";
        }

        addOutputPort(QString(outputSymbols[i].m_name.c_str()),initParam.c_str(),(QNEPort::variableType)outputSymbols[i].m_type);
    }
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------
