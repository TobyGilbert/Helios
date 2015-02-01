#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

/// @brief A function for converting image files into optix::TextureSamples
/// @author Toby Gilbert
/// @date 31/01/2015
//----------------------------------------------------------------------------------------------------------------------
#include <optixu/optixpp_namespace.h>
//----------------------------------------------------------------------------------------------------------------------
/// @brief A function for converting image files into optix::TextureSamples
//----------------------------------------------------------------------------------------------------------------------
optix::TextureSampler loadTexture(optix::Context _context, const std::string& _filename);
//----------------------------------------------------------------------------------------------------------------------

#endif
