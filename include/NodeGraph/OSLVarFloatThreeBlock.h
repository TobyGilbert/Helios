#ifndef OSLVARFLOATTHREEBLOCK_H
#define OSLVARFLOATTHREEBLOCK_H

#include "NodeGraph/OSLAbstractVarBlock.h"
#include <optixu/optixpp_namespace.h>
#include <QGraphicsScene>

/// @todo comment this class

class OSLVarFloatThreeBlock : public OSLAbstractVarBlock
{
public:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief default constructor
    //------------------------------------------------------------------------------------------------------------------------------------
    OSLVarFloatThreeBlock(QGraphicsScene *_scene,optix::Material &_mat,QGraphicsItem * parent = 0);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief default destructor
    //------------------------------------------------------------------------------------------------------------------------------------
    ~OSLVarFloatThreeBlock(){}
    //------------------------------------------------------------------------------------------------------------------------------------

};

#endif // OSLVARFLOATTHREEBLOCK_H
