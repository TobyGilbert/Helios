#include "OsoReader.h"
#include <iostream>
#include <fstream>
#include <FlexLexer.h>
extern FILE *yyin;
//----------------------------------------------------------------------------------------------------------------------------------------
OsoReader::OsoReader(){
}
//----------------------------------------------------------------------------------------------------------------------------------------
OsoReader::~OsoReader(){

}
//----------------------------------------------------------------------------------------------------------------------------------------
void OsoReader::version(float _major, int _minor){
    m_osoMajor = _major;
    m_osoMajor = _minor;
}
//----------------------------------------------------------------------------------------------------------------------------------------
void OsoReader::shader(std::string _shaderType, std::string _shaderName){
    m_shaderType = _shaderType;
    m_shaderName = _shaderName;
}
//----------------------------------------------------------------------------------------------------------------------------------------
bool OsoReader::parseFile(const std::string &_filename){
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
