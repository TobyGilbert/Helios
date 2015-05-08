#ifndef OSLVARCOLORBLOCK_H
#define OSLVARCOLORBLOCK_H

#include "NodeGraph/OSLAbstractVarBlock.h"
#include <optixu/optixpp_namespace.h>
#include <QGraphicsScene>

class OSLVarColorBlock : public OSLAbstractVarBlock
{
public:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief default constructor
    //------------------------------------------------------------------------------------------------------------------------------------
    OSLVarColorBlock(QGraphicsScene *_scene,optix::Material &_mat,QGraphicsItem * parent = 0);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief default destructor
    //------------------------------------------------------------------------------------------------------------------------------------
    ~OSLVarColorBlock(){}
    //------------------------------------------------------------------------------------------------------------------------------------

};

#endif // OSLVARCOLORBLOCK_H
