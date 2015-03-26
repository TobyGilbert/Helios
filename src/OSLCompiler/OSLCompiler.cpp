#include "OSLCompiler.h"
#include <iostream>

//----------------------------------------------------------------------------------------------------------------------------------------
extern FILE *yyin;
//----------------------------------------------------------------------------------------------------------------------------------------
OSLCompiler* OSLCompiler::m_OSLCompiler;
//----------------------------------------------------------------------------------------------------------------------------------------
OSLCompiler* OSLCompiler::getOSLCompiler(){
    if (!m_OSLCompiler){
        m_OSLCompiler = new OSLCompiler();
    }
    return m_OSLCompiler;
}
//----------------------------------------------------------------------------------------------------------------------------------------
OSLCompiler::OSLCompiler(){

}
//----------------------------------------------------------------------------------------------------------------------------------------
OSLCompiler::~OSLCompiler(){

}
//----------------------------------------------------------------------------------------------------------------------------------------
bool OSLCompiler::parseFile(const std::string &_filename){
    yyin = fopen(_filename.c_str(), "r");
    if (!yyin){
        std::cerr<<"Can't open "<<_filename<<std::endl;
    }
    bool ok = !yyparse();
    if (ok){
        std::cout<<"shader passed"<<std::endl;
    }
    else{
        std::cout<<"Failed parse of "<<_filename<<std::endl;
    }
    return ok;
}
//----------------------------------------------------------------------------------------------------------------------------------------
void OSLCompiler::inputParams(Type _t, std::string _name){
    Symbol s;
    s.m_type = _t;
    s.m_name = _name;
    m_inputParams.push_back(s);
}
//----------------------------------------------------------------------------------------------------------------------------------------
std::string OSLCompiler::writeDeviceFunction(){
    std::string s;
    s+="void ";
    s+=m_name.c_str();
    s+="( ";
    // Dictates whether the element is the first input parameter
    bool first = 1;
    // Go through all the input parameters and write their type and name into the device string
    for (int i=0; i<m_inputParams.size(); i++){
        std::string type = getType(m_inputParams[i].m_type);
        if (!first){
            s+=", ";
        }
        s+=type;
        s+=" ";
        s+=m_inputParams[i].m_name.c_str();
        first = 0;
    }
    // Write our shader globals info
    s+=" ){\n";
    s+="\tShaderGlobals sg;\n";
    s+="\t// Calcualte the shading and geometric normals for use with our OSL shaders\n";
    s+="\tsg.N = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );\n";
    s+="\tsg.Ng = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );\n";
    s+="\tsg.I = ray.direction;\n";
    s+="\t// The shading position\n";
    s+="\tsg.P = ray.origin + t_hit * ray.direction;\n";
    s+="\t// Texture coordinates\n";
    s+="\tsg.u = texcoord.x;\n";
    s+="\tsg.v = texcoord.y;\n";




    s+="}\n";
    return s;
}

void OSLCompiler::expression(std::string _expression){
    m_expression += _expression;
    std::cout<<m_expression<<std::endl;
}

std::string OSLCompiler::getType(int _type){
    std::string type;

    switch(_type){
        case 0:
        case 3:
        case 5:
        case 7:
            type = std::string("float3");
            break;
        case 1:
            type = std::string("float");
            break;
        case 2:
            type = std::string("int");
            break;
        case 4:
            type = std::string("matrix?");
            break;
        case 6:
            type = std::string("string");
            break;
        default:
            type = std::string("default");
            break;
    }
    return type;
}
