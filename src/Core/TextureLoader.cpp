#include "Core/TextureLoader.h"
//----------------------------------------------------------------------------------------------------------------------
#include <QImage>
#include <QRgb>
#include <QColor>
#include <iostream>
#include <QVector>
//----------------------------------------------------------------------------------------------------------------------
optix::TextureSampler loadTexture(optix::Context _context, const std::string &_filename){
    // Create tex sampler and populate with default values
    optix::TextureSampler sampler = _context->createTextureSampler();
    sampler->setWrapMode( 0, RT_WRAP_REPEAT );
    sampler->setWrapMode( 1, RT_WRAP_REPEAT );
    sampler->setWrapMode( 2, RT_WRAP_REPEAT );
    sampler->setIndexingMode( RT_TEXTURE_INDEX_NORMALIZED_COORDINATES );
    sampler->setReadMode( RT_TEXTURE_READ_NORMALIZED_FLOAT );
    sampler->setMaxAnisotropy( 1.0f );
    sampler->setMipLevelCount( 1u );
    sampler->setArraySize( 1u );

    QImage tex = QImage(_filename.c_str());
    const unsigned int nx = tex.width();
    const unsigned int ny = tex.height();

    // Create buffer and populate with image data
    optix::Buffer buffer = _context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT4, nx, ny );
    float* buffer_data = static_cast<float*>( buffer->map() );

    for ( int row = 1; row < tex.height() ; ++row ){
        for ( int col = 1; col < tex.width() ; ++col )
        {
            unsigned int buf_index = ( (row)*nx + (tex.width()-col-1) )*4;
            QColor clrCurrent( tex.pixel( nx - col,ny - row ) );
            float inv_255 = 1.0/255.0;
            buffer_data[buf_index +0 ] =  float(clrCurrent.red())*inv_255;
            buffer_data[buf_index +1 ] =  float(clrCurrent.green())*inv_255;
            buffer_data[buf_index +2 ] =  float(clrCurrent.blue())*inv_255;
            buffer_data[buf_index +3 ] =   float(clrCurrent.alpha())*inv_255;
      }
    }

    buffer->unmap();

    sampler->setBuffer( 0u, 0u, buffer );
    sampler->setFilteringModes( RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE );

    return sampler;
}
//----------------------------------------------------------------------------------------------------------------------
