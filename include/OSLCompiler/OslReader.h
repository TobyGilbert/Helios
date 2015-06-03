#ifndef OSLREADER_H_
#define OSLREADER_H_
#include <OSL/oslexec.h>
#include <OSL/oslcomp.h>
#include <QString>
#include <fstream>
#include <iostream>
//------------------------------------------------------------------------------------------------------------------------------------
class OslReader{
public:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief Ctor
    //------------------------------------------------------------------------------------------------------------------------------------
    OslReader();
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief Dtor
    //------------------------------------------------------------------------------------------------------------------------------------
    ~OslReader();
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief Initialise variables in the shading system
    //------------------------------------------------------------------------------------------------------------------------------------
    void initialise();
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief Register closures implemented in the Helios renderer
    //------------------------------------------------------------------------------------------------------------------------------------
    void registerClosures(OSL::ShadingSystem *_shadingSytem);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief Compiles osl code to oso and dumps to file
    //------------------------------------------------------------------------------------------------------------------------------------
    bool compileOSL(QString _shaderName);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief Compiles osl code to oso and dumps to buffer
    //------------------------------------------------------------------------------------------------------------------------------------
    bool compileOSLtoBuffer(QString _shaderName);
    //------------------------------------------------------------------------------------------------------------------------------------
private:
    //------------------------------------------------------------------------------------------------------------------------------------
    OSL::ShadingSystem *m_shadingSystem;
    //------------------------------------------------------------------------------------------------------------------------------------
};
//------------------------------------------------------------------------------------------------------------------------------------
// Subclass ErrorHandler because we want our messages to appear somewhat
// differant than the default ErrorHandler base class, in order to match
// typical compiler command line messages.
class OSLC_ErrorHandler : public OSL::ErrorHandler {
public:
    virtual void operator () (int errcode, const std::string &msg) {
        static OIIO::mutex err_mutex;
        OIIO::lock_guard guard (err_mutex);
        switch (errcode & 0xffff0000) {
        case EH_INFO :
            if (verbosity() >= VERBOSE)
                std::cout << msg << std::endl;
            break;
        case EH_WARNING :
            if (verbosity() >= NORMAL)
                std::cerr << msg << std::endl;
            break;
        case EH_ERROR :
            std::cerr << msg << std::endl;
            break;
        case EH_SEVERE :
            std::cerr << msg << std::endl;
            break;
        case EH_DEBUG :
#ifdef NDEBUG
            break;
#endif
        default :
            if (verbosity() > QUIET)
                std::cout << msg;
            break;
        }
    }
};

static OSLC_ErrorHandler default_oslc_error_handler;
#endif

/// Read the entire contents of the named file and place it in str,
/// returning true on success, false on failure.
inline bool read_text_file (QString filename, std::string &str)
{
    std::ifstream in (filename.toStdString().c_str(), std::ios::in | std::ios::binary);
    if (in) {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close ();
        str = contents.str();
        return true;
    }
    return false;
}
