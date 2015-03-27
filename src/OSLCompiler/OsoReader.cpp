/// @brief Class to interpret the oso code and turn it into a device function
/// @author Toby Gilbert
/// @date 04/03/15
//----------------------------------------------------------------------------------------------------------------------------------------
#include "OSLCompiler/OsoReader.h"
#include <iostream>
#include <fstream>
#include <FlexLexer.h>
#include <boost/bind.hpp>
#ifdef LINUX
    #include <algorithm>
#endif
//----------------------------------------------------------------------------------------------------------------------------------------
extern FILE *yyin;
//----------------------------------------------------------------------------------------------------------------------------------------
OsoReader::OsoReader(){
    m_lineNo = 1;
}
//----------------------------------------------------------------------------------------------------------------------------------------
OsoReader::~OsoReader(){
}
//----------------------------------------------------------------------------------------------------------------------------------------
void OsoReader::version(float _major, int _minor){
    m_osoMajor = _major;
    m_osoMinor = _minor;
}
//----------------------------------------------------------------------------------------------------------------------------------------
void OsoReader::shader(std::string _shaderType, std::string _shaderName){
    m_shaderType = _shaderType;
    m_shaderName = _shaderName;
}
//----------------------------------------------------------------------------------------------------------------------------------------
void OsoReader::parameter(std::pair<std::string, std::string> _params){
    m_iParams.push_back(_params);
}
//----------------------------------------------------------------------------------------------------------------------------------------
void OsoReader::addSymbols(int _symtype, Type _type, std::string _identifier){
    Symbol s;
    s.m_symType = _symtype;
    s.m_type = _type;
    s.m_name = _identifier;
    m_symbols.push_back(s);
}
//----------------------------------------------------------------------------------------------------------------------------------------
void OsoReader::addSymbolDefaults(std::string _default){
    m_symbols.back().m_initialParams.push_back(_default);
}
//----------------------------------------------------------------------------------------------------------------------------------------
void OsoReader::instruction(std::string _opcode, int _lineNo){
    Instruction i;
    i.m_opcode = _opcode;
    i.m_lineNo = _lineNo;
    m_instructions.push_back(i);
}
//----------------------------------------------------------------------------------------------------------------------------------------
void OsoReader::instructionArguments(std::string _argument){
    if (m_instructions.back().m_output == std::string("void")){
        m_instructions.back().m_output = _argument;
    }
    else{
        m_instructions.back().m_args.push_back(_argument);
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------
bool OsoReader::parseFile(const std::string &_filename){
    resetVectors();
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
std::vector<Symbol> OsoReader::getInputParams(){
    std::vector<Symbol> m_inputParams;
    // Go through all symbols looking for input parameters
    for (unsigned int i=0; i<m_symbols.size(); i++){
        if (m_symbols[i].m_symType == 0){ // If input parameter
            m_inputParams.push_back(m_symbols[i]);
        }
    }
    return m_inputParams;
}
//----------------------------------------------------------------------------------------------------------------------------------------
std::vector<Symbol> OsoReader::getOutputParams(){
    std::vector<Symbol> m_outputParams;
    // Go through all symbols looking for input parameters
    for (unsigned int i=0; i<m_symbols.size(); i++){
        if (m_symbols[i].m_symType == 1){ // If input parameter
            m_outputParams.push_back(m_symbols[i]);
        }
        if (m_symbols[i].m_name == std::string("Ci")){
            m_outputParams.push_back(m_symbols[i]);
        }
    }
    return m_outputParams;
}
//----------------------------------------------------------------------------------------------------------------------------------------
void OsoReader::printDeviceFunction(){
    std::cout<<generateDeviceFunction()<<std::endl;
}
//----------------------------------------------------------------------------------------------------------------------------------------
std::string OsoReader::generateDeviceFunction(){
    std::string s;
    s+="//  OptiX device function generated by Toby Gilbert's OSL Compilotmatic 3000\n";
    s+="__device__ void ";
    s+=m_shaderName.c_str();
    s+="(";
    // Veriable for use with commas between function parameters (maybe better way to do this)
    bool init = 0;
    // Go through all symbols looking for function parameters
    for (unsigned int i=0; i<m_symbols.size(); i++){
        if (m_symbols[i].m_symType == 0){
            if (init){
                s+=", ";
            }
            // Append the correct varible type, name and initial parameters
            if (m_symbols[i].m_type == 0 || m_symbols[i].m_type == 3 || m_symbols[i].m_type == 5 || m_symbols[i].m_type == 7){
                s+=" float3 ";
                s+=m_symbols[i].m_name.c_str();
                s+=" = make_float3( ";
                for (unsigned int j=0; j<m_symbols[i].m_initialParams.size(); j++){
                    s+=m_symbols[i].m_initialParams[j].c_str();
                    if (j != m_symbols[i].m_initialParams.size()-1){
                        s+=",";
                    }
                }
                s+=")";
            }
            else if (m_symbols[i].m_type == 1){
                s+=" float ";
                s+=m_symbols[i].m_name.c_str();
                s+=" = ";
                s+=m_symbols[i].m_initialParams[0].c_str();
            }
            else if (m_symbols[i].m_type == 2){
                s+=" int ";
                s+=m_symbols[i].m_name.c_str();
                s+=" = ";
                s+=m_symbols[i].m_initialParams[0].c_str();
            }
            else if (m_symbols[i].m_type == 4){
                s+=" matrix? ";
                s+=m_symbols[i].m_name.c_str();
                s+=" = ";
                s+=m_symbols[i].m_initialParams[0].c_str();
            }
            else if (m_symbols[i].m_type == 6){
                s+=" char* ";
                s+=m_symbols[i].m_name.c_str();
                s+=" = ";
                s+=m_symbols[i].m_initialParams[0].c_str();
            }
            else{
                s+="void ";
                s+=m_symbols[i].m_name.c_str();
                s+=" = ";
                s+=m_symbols[i].m_initialParams[0].c_str();
            }
            init = true;
        }
    }
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

    // Define all of the const and local variables defined in the shader
    for (unsigned int i=0; i<m_symbols.size(); i++){
        if (m_symbols[i].m_symType == 5 || m_symbols[i].m_symType == 2){
            s+="\t";
            if (m_symbols[i].m_type == 0){
                s+="int ";
                s+=m_symbols[i].m_name.c_str();
                if (m_symbols[i].m_symType == 5){
                    s+= " = ";
                    s+=m_symbols[i].m_initialParams[0].c_str();
                }
                s+=";\n";
            }
            else if (m_symbols[i].m_type == 1){
                s+="float ";
                s+=m_symbols[i].m_name.c_str();
                if (m_symbols[i].m_symType == 5){
                    s+=" = ";
                    s+=m_symbols[i].m_initialParams[0].c_str();
                }
                s+=";\n";
            }
            else if (m_symbols[i].m_type == 2){
                s+="char* ";
                s+=m_symbols[i].m_name.c_str();
                if (m_symbols[i].m_symType == 5){
                    s+=" = ";
                    s+=m_symbols[i].m_initialParams[0].c_str();
                }
                s+=";\n";
            }
            else if (m_symbols[i].m_type == 3 || m_symbols[i].m_type == 5 || m_symbols[i].m_type == 6 || m_symbols[i].m_type == 8){
                s+="float3 ";
                s+=m_symbols[i].m_name.c_str();
                if (m_symbols[i].m_symType == 5){
                    s+=" = make_float3( ";
                    for (unsigned int j=0; j<m_symbols[i].m_initialParams.size(); j++){
                        s+=m_symbols[i].m_initialParams[j].c_str();
                        if (j != m_symbols[i].m_initialParams.size()-1){
                            s+=",";
                        }
                    }
                    s+=")";
                }
                s+=";\n";
            }
            else if (m_symbols[i].m_type == 4){
                s+="matrix? ";
                s+=m_symbols[i].m_name.c_str();
                if (m_symbols[i].m_symType == 5){
                    s+=" = ";
                    s+=m_symbols[i].m_initialParams[0].c_str();
                }
                s+=";\n";
            }
            else if (m_symbols[i].m_type == 6){
                s+="int ";
                s+=m_symbols[i].m_name.c_str();
                if (m_symbols[i].m_symType == 5){
                    s+=" = ";
                    s+=m_symbols[i].m_initialParams[0].c_str();
                }
                s+=";\n";
            }
            else{
                s+="void ";
                s+=m_symbols[i].m_name.c_str();
                if (m_symbols[i].m_symType == 5){
                    s+=" = ";
                    s+=m_symbols[i].m_initialParams[0].c_str();
                }
                s+=";\n";
            }

        }
    }

    // Go through all the instructions and create the c++ equivalent
    for (unsigned int i=0; i<m_instructions.size(); i++){

        // Before we write a line check for any jumptargets
        for (unsigned int k=0; k<m_jumpTargets.size(); k++){
            if (m_jumpTargets[k].m_location == m_instructions[i].m_lineNo){
                s+="\t";
                s+="goto L";
                s+=std::to_string(m_jumpTargets[k].m_jump);
                s+=";\n";
            }
        }
        // Append a label before every line
        s+="\t";
        s+="L";
        s+=std::to_string(m_instructions[i].m_lineNo);
        s+=":\n";
        // If the instruction is a closure parameter find its function name and input its given arguments
        if (m_instructions[i].m_opcode == std::string("closure")){
            std::vector<Symbol>::iterator it = std::find_if(m_symbols.begin(), m_symbols.end(), boost::bind(&Symbol::m_name, _1) == m_instructions[i].m_output);
            std::string type;
            std::string output;
            if (m_instructions[i].m_output != std::string("Ci")){
                output = m_instructions[i].m_output;
                switch(it->m_type){
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
                        type = std::string("char*");
                        break;
                    default:
                        type = std::string("err");
                        break;
                }
            }
            else{
                type = std::string("");
                output = std::string("current_prd.attenuation");
            }
            std::vector<Symbol>::iterator itName = std::find_if(m_symbols.begin(), m_symbols.end(), boost::bind(&Symbol::m_name, _1) == m_instructions[i].m_args[0]);
            std::string functionName = itName->m_initialParams[0];
            functionName.erase(remove(functionName.begin(), functionName.end(), '\"'), functionName.end());
            s+="\t";
            s+=type.c_str();
            s+=" ";
            s+=output.c_str();
            s+=" = ";
            s+=functionName.c_str();
            s+="( ";
            for (unsigned int j=1; j<m_instructions[i].m_args.size(); j++){
                if (m_instructions[i].m_args[j] == std::string("N")){
                    s+="sg.N";
                }
                else{
                    s+=m_instructions[i].m_args[j].c_str();
                }
                if (j != (m_instructions[i].m_args.size()-1)){
                    s+=",";
                }
            }
            s+=" );\n";
        }
        // If the instruction is a multiply
        if (m_instructions[i].m_opcode == std::string("mul")){
            std::vector<Symbol>::iterator it = std::find_if(m_symbols.begin(), m_symbols.end(), boost::bind(&Symbol::m_name, _1) == m_instructions[i].m_output);
            std::string type;
            std::string output;
            if (m_instructions[i].m_output != std::string("Ci")){
                output = m_instructions[i].m_output;
                switch(it->m_type){
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
                        type = std::string("char*");
                        break;
                    default:
                        type = std::string("err");
                        break;
                }
            }
            else {
                type = std::string("");
                output = std::string("current_prd.attenuation");
            }
            s+="\t";
            s+=type.c_str();
            s+=" ";
            s+=output.c_str();
            s+=" = ";
            s+=m_instructions[i].m_args[0].c_str();
            s+=" * ";
            s+=m_instructions[i].m_args[1].c_str();
            s+=";\n";
        }
        // If the instruction is an addition
        if (m_instructions[i].m_opcode == std::string("add")){
            std::vector<Symbol>::iterator it = std::find_if(m_symbols.begin(), m_symbols.end(), boost::bind(&Symbol::m_name, _1) == m_instructions[i].m_output);
            std::string type;
            std::string output;
            if (m_instructions[i].m_output != std::string("Ci")){
                output = m_instructions[i].m_output;
                switch(it->m_type){
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
                        type = std::string("char*");
                        break;
                    default:
                        type = std::string("err");
                        break;
                }
            }
            else {
                type = std::string("");
                output = std::string("current_prd.attenuation");
            }
           s+="\t";
           s+=type.c_str();
           s+=" ";
           s+=output.c_str();
           s+=" = ";
           s+=m_instructions[i].m_args[0].c_str();
           s+=" + ";
           s+=m_instructions[i].m_args[1].c_str();
           s+=";\n";
        }
        if (m_instructions[i].m_opcode == std::string("div")){
            std::vector<Symbol>::iterator it = std::find_if(m_symbols.begin(), m_symbols.end(), boost::bind(&Symbol::m_name, _1) == m_instructions[i].m_output);
            std::string type;
            std::string output;
            if (m_instructions[i].m_output != std::string("Ci")){
                output = m_instructions[i].m_output;
                switch(it->m_type){
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
                        type = std::string("char*");
                        break;
                    default:
                        type = std::string("err");
                        break;
                }
            }
            else {
                type = std::string("");
                output = std::string("current_prd.attenuation");
            }
            s+="\t";
            s+=type.c_str();
            s+=" ";
            s+=output.c_str();
            s+=" = ";
            s+=m_instructions[i].m_args[0].c_str();
            s+=" / ";
            s+=m_instructions[i].m_args[1].c_str();
            s+=";\n";

        }
        if (m_instructions[i].m_opcode == std::string("neq")){
            s+="\tint ";
            s+=m_instructions[i].m_output.c_str();
            s+=";\n";
            s+="\tif (";
            s+=m_instructions[i].m_args[0].c_str();
            s+=" != ";
            s+=m_instructions[i].m_args[1].c_str();
            s+="){\n";
            s+="\t\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = 1;\n";
            s+="\t}\n";
            s+="\telse{\n";
            s+="\t\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = 0;\n";
            s+="\t}\n";
        }
        if (m_instructions[i].m_opcode == std::string("eq")){
            s+="\tint ";
            s+=m_instructions[i].m_output.c_str();
            s+=";\n";
            s+="\tif (";
            s+=m_instructions[i].m_args[0].c_str();
            s+=" == ";
            s+=m_instructions[i].m_args[1].c_str();
            s+="){\n";
            s+="\t\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = 1;\n";
            s+="\t}\n";
            s+="\telse{\n";
            s+="\t\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = 0;\n";
            s+="\t}\n";
        }
        if (m_instructions[i].m_opcode == std::string("assign")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = ";
            s+=m_instructions[i].m_args[0].c_str();
            s+=";\n";
        }
        if (m_instructions[i].m_opcode == std::string("raytype")){
            s+="\tint";
            s+=m_instructions[i].m_output.c_str();
            s+=" = raytype(";
            s+=m_instructions[i].m_args[0].c_str();
            s+=");\n";
        }
        if (m_instructions[i].m_opcode == std::string("if")){
            // In order to skip else code add a jump target to the jump targets vector
            JumpTarget jt;
            jt.m_location = atoi(m_instructions[i].m_args[0].c_str()) + 1;
            jt.m_jump = atoi(m_instructions[i].m_args[1].c_str()) + 1;
            m_jumpTargets.push_back(jt);
            // If the not true jump to the else condition
            s+="\t";
            s+="if (!";
            s+=m_instructions[i].m_output.c_str();
            s+="){\n";
            s+="\t\tgoto L";
            s+=std::to_string(atoi(m_instructions[i].m_args[0].c_str()) + 1); // Typecast string to int back to string
            s+=";\n\t}\n";
        }
    }
    s+="}\n";

    return s;
}
//----------------------------------------------------------------------------------------------------------------------------------------
void OsoReader::printParams(){
    for (unsigned int i=0; i<m_iParams.size(); i++){
        std::cout<<m_iParams[i].first<<" "<<m_iParams[i].second<<std::endl;
    }
}
//----------------------------------------------------------------------------------------------------------------------------------------
void OsoReader::printInstructions(){
    for (unsigned int i=0; i<m_instructions.size(); i++){
        std::cout<<m_instructions[i].m_opcode<<" ";
        std::cout<<m_instructions[i].m_output<<" ";
        for (unsigned int j=0; j<m_instructions[i].m_args.size(); j++){
            std::cout<<m_instructions[i].m_args[j]<<" ";
        }
        std::cout<<"\n";
    }
}
void OsoReader::resetVectors(){
    getOsoReader()->m_symbols.clear();
    m_instructionFunctions.clear();
    m_iParams.clear();
    m_instructions.clear();
}

//----------------------------------------------------------------------------------------------------------------------------------------
OsoReader* getOsoReader(){
    if (!osoReader)
           osoReader = new OsoReader();

    return osoReader;
}
//----------------------------------------------------------------------------------------------------------------------------------------
