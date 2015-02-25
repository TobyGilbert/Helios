#include "Shading.h"
#include <QDir>

//------------------------------------------------------------------------------------------------------------------------------------
Shading::Shading(){
//    initialise();
}
//------------------------------------------------------------------------------------------------------------------------------------
Shading::~Shading(){
//    delete m_shadingSystem;
}
//------------------------------------------------------------------------------------------------------------------------------------
void Shading::initialise(){
//    m_shadingSystem = new OSL::ShadingSystem();
}
//------------------------------------------------------------------------------------------------------------------------------------
void Shading::compileOSL(QString _shaderName){
    QString oslfilename = _shaderName;
    if (! oslfilename.endsWith(".osl")){
        oslfilename += ".osl";
    }

    OSL::OSLCompiler compiler;
    std::vector<std::string> options;

    bool ok = compiler.compile(oslfilename.toStdString(), options, QDir::currentPath().toStdString() + "/include/stdosl.h");

    if (ok) {
        std::cout << "Compiled " << oslfilename.toStdString() << " -> " << compiler.output_filename() << "\n";
    }
    else {
        std::cout << "FAILED " << oslfilename.toStdString() << "\n";
    }
}
//------------------------------------------------------------------------------------------------------------------------------------
void Shading::compileOSLtoBuffer(QString _shaderName){
    QString oslfilename = _shaderName;
    if (oslfilename.endsWith(".osl")){
        oslfilename = oslfilename.split(".", QString::SkipEmptyParts).at(0);
    }
    std::string sourcecode;
    if (! read_text_file (oslfilename, sourcecode)) {
        std::cerr << "Could not open \"" << oslfilename.toStdString() << "\"\n";
        exit (EXIT_FAILURE);
    }
    std::string osobuffer;
    OSL::OSLCompiler compiler;
    std::vector<std::string> options;

    if (! compiler.compile_buffer(sourcecode, osobuffer, options, QDir::currentPath().toStdString() + "/include/stdosl.h")){
        std::cerr<<"Could not compile \"" << oslfilename.toStdString() << "\"\n";
    }

//    if (! m_shadingSystem->LoadMemoryCompiledShader(oslfilename.toStdString(), osobuffer)){
//        std::cerr << "Could not load compiled buffer from \"" << oslfilename.toStdString() << "\"\n";
//    }
}
//------------------------------------------------------------------------------------------------------------------------------------
