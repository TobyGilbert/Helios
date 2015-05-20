/// @brief Class to interpret the oso code and turn it into a device function
/// @author Toby Gilbert
/// @date 04/03/15
//----------------------------------------------------------------------------------------------------------------------------------------
#include "OSLCompiler/OsoReader.h"
#include <iostream>
#include <fstream>
#include <FlexLexer.h>
#include <boost/bind.hpp>
#include <sstream>
#include <QStringList>
#include <QDir>
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
    if(m_array){
        s.m_array = true;
        s.m_arrayLength = m_arrayLength;
    }
    m_symbols.push_back(s);
    m_array = false;
}
//----------------------------------------------------------------------------------------------------------------------------------------
void OsoReader::addSymbolDefaults(std::string _default){
    m_symbols.back().m_initialParams.push_back(_default);
}
//----------------------------------------------------------------------------------------------------------------------------------------
void OsoReader::makeSymbolArrayType(std::string _arrayLength){
    m_array = true;
    m_arrayLength = _arrayLength;
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

//------------------------------------------------------------------------------------------------------------------------------------
bool OsoReader::parseBuffer(const std::string &_buffer, QString _filename){
    resetVectors();
    QString osofilename = _filename;
    QFileInfo info(osofilename);
    osofilename = info.baseName();

    if ( osofilename.endsWith(".osl")){
        osofilename = osofilename.split(".", QString::SkipEmptyParts).at(0);
    }
    osofilename += ".oso";
    std::cout<<("shaders/OSO/" + osofilename.toStdString()).c_str()<<std::endl;
    std::ofstream out(("shaders/OSO/" + osofilename.toStdString()).c_str());
    out << _buffer;
    out.close();
    yyin = fopen(("shaders/OSO/" + osofilename.toStdString()).c_str(), "r");
    if (!yyin){
        std::cerr<<"Can't load buffer"<<std::endl;
    }
    bool ok = !yyparse();
    if (ok){
        std::cout<<"shader passed"<<std::endl;
    }
    else{
        std::cout<<"Failed parse of buffer"<<std::endl;
    }
    return ok;
}

//----------------------------------------------------------------------------------------------------------------------------------------
std::vector<Symbol> OsoReader::getInputParams(){
    std::vector<Symbol> inputParams;
    // Go through all symbols looking for input parameters
    for (unsigned int i=0; i<m_symbols.size(); i++){
        if (m_symbols[i].m_symType == 0){ // If input parameter
            inputParams.push_back(m_symbols[i]);
        }
    }
    return inputParams;
}
//----------------------------------------------------------------------------------------------------------------------------------------
std::vector<Symbol> OsoReader::getOutputParams(){
    std::vector<Symbol> outputParams;
    // Go through all symbols looking for input parameters
    for (unsigned int i=0; i<m_symbols.size(); i++){
        if (m_symbols[i].m_symType == 1){ // If input parameter
            outputParams.push_back(m_symbols[i]);
        }
        if (m_symbols[i].m_name == std::string("Ci")){
            outputParams.push_back(m_symbols[i]);
        }
    }
    return outputParams;
}
//----------------------------------------------------------------------------------------------------------------------------------------
void OsoReader::printDeviceFunction(){
    std::cout<<generateDeviceFunction()<<std::endl;
}
//----------------------------------------------------------------------------------------------------------------------------------------
std::string OsoReader::generateDeviceFunction(){
    std::string s;
    bool midLoop = false; // A bool to tell if you are in the middle of a loop
    s+="//  OptiX device function generated by Toby Gilbert's OSL Compilotmatic 3000\n";
    s+="__device__ void ";
    s+=m_shaderName.c_str();
    s+="(";
    // Veriable for use with commas between function parameters (maybe better way to do this)
    // Go through all symbols looking for function parameters
    s+="ShaderGlobals &sg";
    for (unsigned int i=0; i<m_symbols.size(); i++){
        if (m_symbols[i].m_symType == 0){
            s+=", ";
            if(m_symbols[i].m_type == 2){
                s+=" rtTextureSampler<float4, 2> ";
                s+=m_symbols[i].m_name.c_str();
            }
            // Append the correct varible type, name and initial parameters
            if (m_symbols[i].m_type == 3 || m_symbols[i].m_type == 5 || m_symbols[i].m_type == 7){
                s+=" float3 ";
                s+=m_symbols[i].m_name.c_str();
            }
            else if (m_symbols[i].m_type == 1){
                s+=" float ";
                s+=m_symbols[i].m_name.c_str();
            }
            else if (m_symbols[i].m_type == 0){
                s+=" int ";
                s+=m_symbols[i].m_name.c_str();
            }
            else if (m_symbols[i].m_type == 4){
                s+=" Matrix4x4 ";
                s+=m_symbols[i].m_name.c_str();
            }
            else if (m_symbols[i].m_type == 6){
                s+=" char* ";
                s+=m_symbols[i].m_name.c_str();
            }
        }
        if (m_symbols[i].m_symType == 1){
            s+=", ";
            // Append the correct varible type, name and initial parameters
            if (m_symbols[i].m_type == 0 || m_symbols[i].m_type == 3 || m_symbols[i].m_type == 5 || m_symbols[i].m_type == 7){
                s+=" float3 &";
                s+=m_symbols[i].m_name.c_str();
            }
            else if (m_symbols[i].m_type == 1){
                s+=" float &";
                s+=m_symbols[i].m_name.c_str();
            }
            else if (m_symbols[i].m_type == 2){
                s+=" int &";
                s+=m_symbols[i].m_name.c_str();
                s+=" = ";
                s+=m_symbols[i].m_initialParams[0].c_str();
            }
            else if (m_symbols[i].m_type == 4){
                s+=" Matrix4x4 &";
                s+=m_symbols[i].m_name.c_str();
            }
            else if (m_symbols[i].m_type == 6){
                s+=" char* &";
                s+=m_symbols[i].m_name.c_str();
            }
            else{
                s+="void &";
                s+=m_symbols[i].m_name.c_str();
            }
        }
    }
    s+=" ){\n";


    // Define all of the const and local and temp variables defined in the shader
    for (unsigned int i=0; i<m_symbols.size(); i++){
        if (m_symbols[i].m_symType == 5 || m_symbols[i].m_symType == 2 || m_symbols[i].m_symType == 3){
            s+="\t";
            if (m_symbols[i].m_type == 0){
                s+="int ";
                s+=m_symbols[i].m_name.c_str();
                // Check if the variable is an array
                if(m_symbols[i].m_array == true){
                    s+="[";
                    s+=m_symbols[i].m_arrayLength;
                    s+="]";
                }
                if (m_symbols[i].m_symType == 5){
                    s+= " = ";
                    s+=m_symbols[i].m_initialParams[0].c_str();
                }
                s+=";\n";
            }
            else if (m_symbols[i].m_type == 1){
                s+="float ";
                s+=m_symbols[i].m_name.c_str();
                // Check if the variable is an array
                if(m_symbols[i].m_array == true){
                    s+="[";
                    s+=m_symbols[i].m_arrayLength;
                    s+="]";
                }
                if (m_symbols[i].m_symType == 5){
                    s+=" = ";
                    s+=m_symbols[i].m_initialParams[0].c_str();
                }
                s+=";\n";
            }
            else if (m_symbols[i].m_type == 2){
                s+="char* ";
                s+=m_symbols[i].m_name.c_str();
                // Check if the variable is an array
                if(m_symbols[i].m_array == true){
                    s+="[";
                    s+=m_symbols[i].m_arrayLength;
                    s+="]";
                }
                if (m_symbols[i].m_symType == 5){
                    s+=" = ";
                    s+=m_symbols[i].m_initialParams[0].c_str();
                }
                s+=";\n";
            }
            else if (m_symbols[i].m_type == 3 || m_symbols[i].m_type == 5 || m_symbols[i].m_type == 6 || m_symbols[i].m_type == 8){
                s+="float3 ";
                s+=m_symbols[i].m_name.c_str();
                // Check if the variable is an array
                if(m_symbols[i].m_array == true){
                    s+="[";
                    s+=m_symbols[i].m_arrayLength;
                    s+="]";
                }
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
                s+="Matrix4x4 ";
                s+=m_symbols[i].m_name.c_str();
                // Check if the variable is an array
                if(m_symbols[i].m_array == true){
                    s+="[";
                    s+=m_symbols[i].m_arrayLength;
                    s+="]";
                }
                if (m_symbols[i].m_symType == 5){
                    s+=" = ";
                    s+=m_symbols[i].m_initialParams[0].c_str();
                }
                s+=";\n";
            }
            else if (m_symbols[i].m_type == 6){
                s+="int ";
                s+=m_symbols[i].m_name.c_str();
                // Check if the variable is an array
                if(m_symbols[i].m_array == true){
                    s+="[";
                    s+=m_symbols[i].m_arrayLength;
                    s+="]";
                }
                if (m_symbols[i].m_symType == 5){
                    s+=" = ";
                    s+=m_symbols[i].m_initialParams[0].c_str();
                }
                s+=";\n";
            }
            else{
                s+="void ";
                s+=m_symbols[i].m_name.c_str();
                // Check if the variable is an array
                if(m_symbols[i].m_array == true){
                    s+="[";
                    s+=m_symbols[i].m_arrayLength;
                    s+="]";
                }
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
        // Check for loops
        for(unsigned int j=0; j<m_forLoopJumpTargets.size(); j++){
            if(m_forLoopJumpTargets[j].m_endCondition == m_instructions[i].m_lineNo){
                s+="\t){\n";
                midLoop = false;
            }
            if(m_forLoopJumpTargets[j].m_endLoop == m_instructions[i].m_lineNo){
                s+="\t}\n";
            }
        }
        // Append a label before every line
        if(!midLoop){
            s+="\t";
            s+="L";
            s+=std::to_string(m_instructions[i].m_lineNo);
            s+=":\n";
        }
        // If the instruction is a closure parameter find its function name and input its given arguments
        if (m_instructions[i].m_opcode == std::string("closure")){
            s+="\t";
            if (m_instructions[i].m_output != std::string("Ci")){
                s+=m_instructions[i].m_output;
            }
            else{
                s+=std::string("current_prd.result");
            }
            std::vector<Symbol>::iterator itName = std::find_if(m_symbols.begin(), m_symbols.end(), boost::bind(&Symbol::m_name, _1) == m_instructions[i].m_args[0]);
            std::string functionName = itName->m_initialParams[0];
            functionName.erase(remove(functionName.begin(), functionName.end(), '\"'), functionName.end());
            s+=" = ";
            s+=functionName.c_str();
            s+="( ";
            for (unsigned int j=1; j<m_instructions[i].m_args.size(); j++){
                if (checkShaderGlobals(m_instructions[i].m_args[j])){
                    s+="sg.";
                }
                s+=m_instructions[i].m_args[j].c_str();
                if (j != (m_instructions[i].m_args.size()-1)){
                    s+=",";
                }
            }
            s+=" );\n";
        }
        // negate the value
        if(m_instructions[i].m_opcode == std::string("neg")){
            s+="\t";
            if(m_instructions[i].m_output.c_str() == std::string("N") || m_instructions[i].m_output.c_str() == std::string("I") || m_instructions[i].m_output.c_str() == std::string("dPdu") || m_instructions[i].m_output.c_str() == std::string("dPdv") ){
                s+="sg.";
            }
            s+=m_instructions[i].m_output.c_str();
            s+=" = -";
            if(m_instructions[i].m_args[0].c_str() == std::string("N") || m_instructions[i].m_args[0].c_str() == std::string("I") || m_instructions[i].m_args[0].c_str() == std::string("dPdu") || m_instructions[i].m_args[0].c_str() == std::string("dPdv") ){
                s+="sg.";
            }
            s+=m_instructions[i].m_args[0].c_str();
            s+=";\n";
        }
        // If the instruction is a multiply
        if (m_instructions[i].m_opcode == std::string("mul")){
            s+="\t";
            if (m_instructions[i].m_output != std::string("Ci")){
                s+=m_instructions[i].m_output.c_str();
            }
            else {
                s+=std::string("current_prd.result");
            }
            if(checkShaderGlobals(m_instructions[i].m_output)){
                s+="sg.";
            }
            s+=" = ";
            if(m_instructions[i].m_args[0].c_str() != std::string("Ci")){
                if(checkShaderGlobals(m_instructions[i].m_args[0])){
                    s+="sg.";
                }
                s+=m_instructions[i].m_args[0].c_str();
            }
            else{
                s+="current_prd.result";
            }
            s+=" * ";
            if(checkShaderGlobals(m_instructions[i].m_args[1])){
                s+="sg.";
            }
            s+=m_instructions[i].m_args[1].c_str();
            s+=";\n";
        }
        // If the instruction is an addition
        if (m_instructions[i].m_opcode == std::string("add")){
            s+="\t";
            if (m_instructions[i].m_output != std::string("Ci")){
                s+= m_instructions[i].m_output;
            }
            else {
                s+=std::string("current_prd.result");
            }
           if(checkShaderGlobals(m_instructions[i].m_output) ){
               s+="sg.";
           }
           s+=" = ";
           if(m_instructions[i].m_args[0].c_str() != std::string("Ci")){
               if(checkShaderGlobals(m_instructions[i].m_args[0])){
                   s+="sg.";
               }
               s+=m_instructions[i].m_args[0].c_str();
           }
           else{
               s+="current_prd.result";
           }
           s+=" + ";
           if(checkShaderGlobals(m_instructions[i].m_args[1])){
               s+="sg.";
           }
           s+=m_instructions[i].m_args[1].c_str();
           s+=";\n";
        }
        // divide
        if (m_instructions[i].m_opcode == std::string("div")){
            std::string output;
            if (m_instructions[i].m_output != std::string("Ci")){
                output = m_instructions[i].m_output;
            }
            else {
                output = std::string("current_prd.result");
            }
            s+="\t";
            s+=output.c_str();
            s+=" = ";
            if(m_instructions[i].m_args[0].c_str() != std::string("Ci")){
                if(checkShaderGlobals(m_instructions[i].m_args[0])){
                    s+="sg.";
                }
                s+=m_instructions[i].m_args[0].c_str();
            }
            else{
                s+="current_prd.result";
            }
            s+=" / ";
            if(checkShaderGlobals(m_instructions[i].m_args[1])){
                s+="sg.";
            }
            s+=m_instructions[i].m_args[1].c_str();
            s+=";\n";

        }
        // subtract
        if(m_instructions[i].m_opcode == std::string("sub")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = ";
            if(checkShaderGlobals(m_instructions[i].m_args[0])){
                s+="sg.";
            }
            s+=m_instructions[i].m_args[0].c_str();
            s+=" - ";
            if(checkShaderGlobals(m_instructions[i].m_args[1])){
                s+="sg.";
            }
            s+=m_instructions[i].m_args[1].c_str();
            s+=";\n";
        }
        // dot product
        if(m_instructions[i].m_opcode == std::string("dot")){
            s+="\t";
            if(checkShaderGlobals(m_instructions[i].m_output)){
                s+="sg.";
            }
            s+=m_instructions[i].m_output.c_str();
            s+=" = optix::dot(";
            if(checkShaderGlobals(m_instructions[i].m_args[0])){
                s+="sg.";
            }
            s+=m_instructions[i].m_args[0].c_str();
            s+=", ";
            if(checkShaderGlobals(m_instructions[i].m_args[0])){
                s+="sg.";
            }
            s+=m_instructions[i].m_args[1].c_str();
            s+=");\n";
        }
        if(m_instructions[i].m_opcode == std::string("mix")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = (";
            s+=m_instructions[i].m_args[0].c_str();
            s+=" * ";
            s+=m_instructions[i].m_args[2].c_str();
            s+= ") + (";
            s+=m_instructions[i].m_args[1].c_str();
            s+=" * (1.0 -";
            s+=m_instructions[i].m_args[2].c_str();
            s+=") );\n";
        }
        if(m_instructions[i].m_opcode == std::string("pow")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = pow(";
            s+=m_instructions[i].m_args[0].c_str();
            s+=", ";
            s+=m_instructions[i].m_args[1].c_str();
            s+=");\n";
        }
        // random
        if(m_instructions[i].m_opcode == std::string("random")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+="= make_float3(rnd(current_prd.seed), rnd(current_prd.seed), rnd(current_prd.seed));\n";
        }
        // boolean operation for not equals to
        if (m_instructions[i].m_opcode == std::string("neq")){
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
        // boolean operation for equals to
        if (m_instructions[i].m_opcode == std::string("eq")){
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
        // Assignment
        if (m_instructions[i].m_opcode == std::string("assign")){
            s+="\t";
            if(m_instructions[i].m_output.c_str() == std::string("N") || m_instructions[i].m_output.c_str() == std::string("dPdu") || m_instructions[i].m_output.c_str() == std::string("dPdv")){
                s+="sg.";
            }
            s+=m_instructions[i].m_output.c_str();
            s+=" = ";
            std::vector<Symbol>::iterator it = std::find_if(m_symbols.begin(), m_symbols.end(), boost::bind(&Symbol::m_name, _1) == m_instructions[i].m_output);
            std::vector<Symbol>::iterator itr = std::find_if(m_symbols.begin(), m_symbols.end(), boost::bind(&Symbol::m_name, _1) == m_instructions[i].m_args[0]);

            if(it->m_type != 4){
//                if((it->m_type == 3 || it->m_type == 5 || it->m_type == 6 || it->m_type == 8) && m_instructions[i].m_args.size() < 3){
//                    s+="make_float3(";
//                }
                if(m_instructions[i].m_args.size())
                if(m_instructions[i].m_args[0].c_str() == std::string("N") || m_instructions[i].m_args[0].c_str() == std::string("dPdu") || m_instructions[i].m_args[0].c_str() == std::string("dPdv")){
                    s+="sg.";
                }
                s+=m_instructions[i].m_args[0].c_str();
//                if((it->m_type == 3 || it->m_type == 5 || it->m_type == 6 || it->m_type == 8) && m_instructions[i].m_args.size() < 3){
//                    s+=")";
//                }
            }
            else{
                std::vector<Symbol>::iterator itr = std::find_if(m_symbols.begin(), m_symbols.end(), boost::bind(&Symbol::m_name, _1) == m_instructions[i].m_args[0]);
                if(itr->m_initialParams[0] == std::string("1")){
                    s+="Matrix4x4::identity()";
                }
                else if(itr->m_initialParams[0] == std::string("0")){
                    // Do this
                }
            }
            s+=";\n";
        }
        // cellnoise
        if(m_instructions[i].m_opcode == std::string("cellnoise")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = OSLcellnoise(";
            if(m_instructions[i].m_args[0].c_str() == std::string("N") ||m_instructions[i].m_args[0].c_str() == std::string("dPdu") ||m_instructions[i].m_args[0].c_str() == std::string("dPdv") ){
                s+="sg.";
            }
            s+=m_instructions[i].m_output.c_str();
            s+=");\n";
        }
        // An array assignment
        if(m_instructions[i].m_opcode == std::string("aassign")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+="[";
            s+=m_instructions[i].m_args[0].c_str();
            s+="] = ";
            s+=m_instructions[i].m_args[1].c_str();
            s+=";\n";
        }
        // reference to an array
        if(m_instructions[i].m_opcode == std::string("aref")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = ";
            s+=m_instructions[i].m_args[0].c_str();
            s+="[";
            s+=m_instructions[i].m_args[1].c_str();
            s+="];\n";
        }
        // Assigning a vector
        if(m_instructions[i].m_opcode == std::string("vector")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = make_float3(";
            if(m_instructions[i].m_args[0] == std::string("u") || m_instructions[i].m_args[0] == std::string("v")){
                s+="sg.";
            }
            s+=m_instructions[i].m_args[0].c_str();
            s+=",";
            if(m_instructions[i].m_args[1] == std::string("u")|| m_instructions[i].m_args[1] == std::string("v")){
                s+="sg.";
            }
            s+=m_instructions[i].m_args[1].c_str();
            s+=",";
            if(m_instructions[i].m_args[2] == std::string("u")|| m_instructions[i].m_args[2] == std::string("v")){
                s+="sg.";
            }
            s+=m_instructions[i].m_args[2].c_str();
            s+=");\n";
        }
        // floor
        if(m_instructions[i].m_opcode == std::string("floor")){
            s+="\t";
            std::vector<Symbol>::iterator output = std::find_if(m_symbols.begin(), m_symbols.end(), boost::bind(&Symbol::m_name, _1) == m_instructions[i].m_output);
            s+=m_instructions[i].m_output.c_str();
            if(output->m_type == 3 ||output->m_type == 5 || output->m_type == 6 || output->m_type == 8){ // if you want to floor a vector
                s+=" = make_float3(floor(";
                s+=m_instructions[i].m_args[0].c_str();
                s+=".x), floor(";
                s+=m_instructions[i].m_args[0].c_str();
                s+=".y), floor(";
                s+=m_instructions[i].m_args[0].c_str();
                s+=".z))";
            }
            else{
                s+=" = floor(";
                s+=m_instructions[i].m_args[0].c_str();
                s+=")";
            }
            s+=";\n";
        }
        // normal
        /// @todo take into account the first parameter which specifies the coordinate space
        if (m_instructions[i].m_opcode == std::string("normal")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = make_float3(";
            // Get the symbol for the first symbol to check if its relating to the coordinate space
            std::vector<Symbol>::iterator sym = getSymbol(m_instructions[i].m_args[0]);
            if(sym->m_initialParams[0] != std::string("common")
                    && sym->m_initialParams[0] != std::string("\"object\"")
                    && sym->m_initialParams[0] != std::string("\"shader\"")
                    && sym->m_initialParams[0] != std::string("\"world\"")
                    && sym->m_initialParams[0] != std::string("\"camera\"")
                    && sym->m_initialParams[0] != std::string("\"screen\"")
                    && sym->m_initialParams[0] != std::string("\"raster\"")
                    && sym->m_initialParams[0] != std::string("\"NDC\"")){
                if(checkShaderGlobals(m_instructions[i].m_args[0])){
                    s+="sg.";
                }
                s+=m_instructions[i].m_args[0].c_str();
                s+=", ";
                if(checkShaderGlobals(m_instructions[i].m_args[1])){
                    s+="sg.";
                }
                s+=m_instructions[i].m_args[1].c_str();
                s+=", ";
                if(checkShaderGlobals(m_instructions[i].m_args[2])){
                    s+="sg.";
                }
                s+=m_instructions[i].m_args[2].c_str();
            }
            else{
                if(checkShaderGlobals(m_instructions[i].m_args[1])){
                    s+="sg.";
                }
                s+=m_instructions[i].m_args[1].c_str();
                s+=", ";
                if(checkShaderGlobals(m_instructions[i].m_args[2])){
                    s+="sg.";
                }
                s+=m_instructions[i].m_args[2].c_str();
                s+=", ";
                if(checkShaderGlobals(m_instructions[i].m_args[3])){
                    s+="sg.";
                }
                s+=m_instructions[i].m_args[3].c_str();
            }
            s+=");\n";
        }
        // Filling a matrix
        if (m_instructions[i].m_opcode == std::string("matrix")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=".setRow(0, make_float4(";
            s+=m_instructions[i].m_args[0].c_str();
            s+=", ";
            s+=m_instructions[i].m_args[1].c_str();
            s+=", ";
            s+=m_instructions[i].m_args[2].c_str();
            s+=", ";
            s+=m_instructions[i].m_args[3].c_str();
            s+="));\n";
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=".setRow(1, make_float4(";
            s+=m_instructions[i].m_args[4].c_str();
            s+=", ";
            s+=m_instructions[i].m_args[5].c_str();
            s+=", ";
            s+=m_instructions[i].m_args[6].c_str();
            s+=", ";
            s+=m_instructions[i].m_args[7].c_str();
            s+="));\n";
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=".setRow(2, make_float4(";
            s+=m_instructions[i].m_args[8].c_str();
            s+=", ";
            s+=m_instructions[i].m_args[9].c_str();
            s+=", ";
            s+=m_instructions[i].m_args[10].c_str();
            s+=", ";
            s+=m_instructions[i].m_args[11].c_str();
            s+="));\n";
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=".setRow(3, make_float4(";
            s+=m_instructions[i].m_args[12].c_str();
            s+=", ";
            s+=m_instructions[i].m_args[13].c_str();
            s+=", ";
            s+=m_instructions[i].m_args[14].c_str();
            s+=", ";
            s+=m_instructions[i].m_args[15].c_str();
            s+="));\n";
        }
        // min
        if(m_instructions[i].m_opcode == std::string("min")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = min(";
            s+=m_instructions[i].m_args[0].c_str();
            s+=",";
            s+=m_instructions[i].m_args[1].c_str();
            s+=");\n";
        }
        // max
        if(m_instructions[i].m_opcode == std::string("min")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = max(";
            s+=m_instructions[i].m_args[0].c_str();
            s+=",";
            s+=m_instructions[i].m_args[1].c_str();
            s+=");\n";
        }
        //
        if(m_instructions[i].m_opcode == std::string("compassign")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            std::vector<Symbol>::iterator it = std::find_if(m_symbols.begin(), m_symbols.end(), boost::bind(&Symbol::m_name, _1) == m_instructions[i].m_args[0]);
            if(it->m_initialParams[0].c_str() == std::string("0")){
                s+=".x = ";
            }
            else if(it->m_initialParams[0].c_str() == std::string("1")){
                s+=".y = ";
            }
            else if(it->m_initialParams[0].c_str() == std::string("2")){
                s+=".z = ";
            }
            else if(it->m_initialParams[0].c_str() == std::string("3")){
                s+=".w = ";
            }
            s+=m_instructions[i].m_args[1].c_str();
            s+=";\n";
        }
        // access element of 1d array
        if(m_instructions[i].m_opcode == std::string("compref")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = ";
            if(m_instructions[i].m_args[0].c_str() == std::string("dPdu") ||m_instructions[i].m_args[0].c_str() == std::string("dPdv") || m_instructions[i].m_args[0].c_str() == std::string("N")){
                s+="sg.";
            }
            s+=m_instructions[i].m_args[0].c_str();
            std::vector<Symbol>::iterator it = std::find_if(m_symbols.begin(), m_symbols.end(), boost::bind(&Symbol::m_name, _1) == m_instructions[i].m_args[1]);
            if(it->m_initialParams[0] == std::string("0")){
                s+=".x;\n";
            }
            if(it->m_initialParams[0] == std::string("1")){
                s+=".y;\n";
            }
            if(it->m_initialParams[0] == std::string("2")){
                s+=".z;\n";
            }
            if(it->m_initialParams[0] == std::string("3")){
                s+=".w;\n";
            }
        }
        //acces element of 2d array
        if(m_instructions[i].m_opcode == std::string("mxcompref")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = ";
            std::vector<Symbol>::iterator it = std::find_if(m_symbols.begin(), m_symbols.end(), boost::bind(&Symbol::m_name, _1) == m_instructions[i].m_args[0]);
            s+=m_instructions[i].m_args[0].c_str();
            if(it->m_type == 4){ // for matrices
                s+=".getRow(";
                s+=m_instructions[i].m_args[1].c_str();
                s+=")";
                std::vector<Symbol>::iterator itr = std::find_if(m_symbols.begin(), m_symbols.end(), boost::bind(&Symbol::m_name, _1) == m_instructions[i].m_args[2]);
                if(itr->m_initialParams[0].c_str() == std::string("0")){
                    s+=".x;\n";
                }
                else if (itr->m_initialParams[0].c_str() == std::string("1")){
                    s+=".y;\n";
                }
                else if (itr->m_initialParams[0].c_str() == std::string("2")){
                    s+=".z;\n";
                }
                else if (itr->m_initialParams[0].c_str() == std::string("3")){
                    s+=".w;\n";
                }
            }
            else{ // for two dimensional arrays
                s+="[";
                s+=m_instructions[i].m_args[1].c_str();
                s+="][";
                s+=m_instructions[i].m_args[2].c_str();
                s+="];\n";
            }
        }
        // function call
        if(m_instructions[i].m_opcode == std::string("functioncall")){
            // Do nothing for now
            s+="//FUNCTIONCALL!\n";

        }
        // normalize
        if(m_instructions[i].m_opcode == std::string("normalize")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = normalize(";
            if(m_instructions[i].m_args[0].c_str() == std::string("N") || m_instructions[i].m_args[0].c_str() == std::string("dPdu") || m_instructions[i].m_args[0].c_str() == std::string("dPdv")){
                s+="sg.";
            }
            s+=m_instructions[i].m_args[0].c_str();
            s+=");\n";
        }
        // the raytype osl function to check the incoming raytype
        if (m_instructions[i].m_opcode == std::string("raytype")){
            s+="\t ";
            s+=m_instructions[i].m_output.c_str();
            s+=" = OSLraytype(";
            s+=m_instructions[i].m_args[0].c_str();
            s+=");\n";
        }
        // check if the face is back facing
        if (m_instructions[i].m_opcode == std::string("backfacing")){
            s+="\t ";
            s+=m_instructions[i].m_output.c_str();
            s+=" = OSLbackfacing(sg);\n";
        }
        // texture lookup
        if(m_instructions[i].m_opcode == std::string("texture")){
            s+="\t";
            s+=m_instructions[i].m_output.c_str();
            s+=" = OSLTexture(";
            for (unsigned int j=0; j<m_instructions[i].m_args.size()-1; j++){
                if(m_instructions[i].m_args[j].c_str() == std::string("u")){
                    s+="sg.u";
                }
                else if(m_instructions[i].m_args[j].c_str() == std::string("v")){
                    s+="sg.v";
                }
                else{
                    s+=m_instructions[i].m_args[j].c_str();
                }
                s+=", ";
            }
            if(m_instructions[i].m_args[m_instructions[i].m_args.size()-1].c_str() == std::string("u")){
                s+="sg.u";
            }
            else if(m_instructions[i].m_args[m_instructions[i].m_args.size()-1].c_str() == std::string("v")){
                s+="sg.v";
            }
            else{
                s+=m_instructions[i].m_args[m_instructions[i].m_args.size()-1].c_str();
            }
            s+=");\n";
        }
        // for loop
        if (m_instructions[i].m_opcode == std::string("for")){
            midLoop = true;
            s+="\t for(";
            ForLoop fl;
            fl.m_endCondition = atoi(m_instructions[i].m_args[1].c_str())+1;
            fl.m_endLoop = atoi(m_instructions[i].m_args[3].c_str())+1;
            m_forLoopJumpTargets.push_back(fl);
        }

        // less than <
        if (m_instructions[i].m_opcode == std::string("lt")){
            s+="\t(";
            s+=m_instructions[i].m_output.c_str();
            s+=" = ";
            s+=m_instructions[i].m_args[0].c_str();
            s+=" < ";
            s+=m_instructions[i].m_args[1].c_str();
            s+=");\n";
        }
        // if statement
        if (m_instructions[i].m_opcode == std::string("if")){
            // In order to skip else code add a jump target to the jump targets vector
            JumpTarget jt;
            jt.m_location = atoi(m_instructions[i].m_args[0].c_str()) + 1;
            jt.m_jump = atoi(m_instructions[i].m_args[1].c_str()) + 1;
            m_jumpTargets.push_back(jt);
            // If not true jump to the else condition
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
    getOsoReader()->m_jumpTargets.clear();
    getOsoReader()->m_forLoopJumpTargets.clear();
    getOsoReader()->m_symbols.clear();
//    getOsoReader()->m_instructionFunctions.clear();
    getOsoReader()->m_iParams.clear();
    getOsoReader()->m_instructions.clear();
}
//----------------------------------------------------------------------------------------------------------------------------------------
std::vector<Symbol>::iterator OsoReader::getSymbol(std::string _symbolName){
    std::vector<Symbol>::iterator sym = std::find_if(m_symbols.begin(), m_symbols.end(), boost::bind(&Symbol::m_name, _1) == _symbolName);
    return sym;
}
//----------------------------------------------------------------------------------------------------------------------------------------
bool OsoReader::checkShaderGlobals(std::string _variable){
    if(_variable == std::string("N") || _variable == std::string("I") || _variable == std::string("dPdu") || _variable == std::string("dPdv") || _variable == std::string("u") || _variable == std::string("v")){
        return true;
    }
    return false;
}
//----------------------------------------------------------------------------------------------------------------------------------------
OsoReader* getOsoReader(){
    if (!osoReader)
           osoReader = new OsoReader();

    return osoReader;
}
//----------------------------------------------------------------------------------------------------------------------------------------
