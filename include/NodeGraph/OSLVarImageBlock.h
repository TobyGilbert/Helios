#ifndef OSLVARIMAGEBLOCK_H
#define OSLVARIMAGEBLOCK_H

#include "NodeGraph/OSLAbstractVarBlock.h"
#include <optixu/optixpp_namespace.h>
#include <QGraphicsScene>

class OSLVarImageBlock : public OSLAbstractVarBlock
{
public:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief default constructor
    //------------------------------------------------------------------------------------------------------------------------------------
    OSLVarImageBlock(QGraphicsScene *_scene,optix::Material &_mat,QGraphicsItem * parent = 0);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief default destuctor
    //------------------------------------------------------------------------------------------------------------------------------------
    ~OSLVarImageBlock();

};

#endif // OSLVARIMAGEBLOCK_H
