#include "OSLCompiler/OslReader.h"
#include <QDir>
#include "OSLCompiler/OsoReader.h"

//------------------------------------------------------------------------------------------------------------------------------------
OslReader::OslReader(){
}
//------------------------------------------------------------------------------------------------------------------------------------
OslReader::~OslReader(){
}
//------------------------------------------------------------------------------------------------------------------------------------
void OslReader::initialise(){
}
//------------------------------------------------------------------------------------------------------------------------------------
bool OslReader::compileOSL(QString _shaderName){
    QString oslfilename = _shaderName;
    if (! oslfilename.endsWith(".osl")){
        oslfilename += ".osl";
    }
    OSL::OSLCompiler compiler(&default_oslc_error_handler);
    std::vector<std::string> options;
    bool ok;
    try {
        ok = compiler.compile(oslfilename.toStdString(), options, QDir::currentPath().toStdString() + "/include/stdosl.h");
    }
    catch (int e){
        std::cout<<"OSL compile failed!"<<e<<std::endl;
        return 0;
    }

    if (ok) {
        std::cout << "Compiled " << oslfilename.toStdString() << " -> " << compiler.output_filename() << "\n";
    }
    else {
        std::cout << "FAILED " << oslfilename.toStdString() << "\n";
        return false;
    }
    OsoReader *osoread = OsoReader::getOsoReader();
    QFileInfo info(_shaderName);

    osoread->parseFile((info.baseName()+QString(".oso")).toStdString());
    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
bool OslReader::compileOSLtoBuffer(QString _shaderName){
    QString oslfilename = _shaderName;
    std::cout<<"Compiling \""<<oslfilename.toStdString()<<"\""<<std::endl;
    if (!oslfilename.endsWith(".osl")){
        oslfilename = oslfilename.split(".", QString::SkipEmptyParts).at(0);
    }
    std::string sourcecode;
    if (! read_text_file (oslfilename, sourcecode)) {
        std::cerr << "Could not open \"" << oslfilename.toStdString() << "\"\n";
        exit (EXIT_FAILURE);
    }
    std::string osobuffer;
    OSL::OSLCompiler compiler(&default_oslc_error_handler);
    std::vector<std::string> options;

    if (! compiler.compile_buffer(sourcecode, osobuffer, options, QDir::currentPath().toStdString() + "/include/stdosl.h")){
        std::cerr<<"Could not compile \"" << oslfilename.toStdString() << "\"\n";
        return false;
    }

    OsoReader *osoread = OsoReader::getOsoReader();
    if(!osoread->parseBuffer(osobuffer.c_str(), oslfilename)){
        std::cerr<<"Could not load buffer \""<<oslfilename.toStdString() << "\"\n";
        return false;
    }

    return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
