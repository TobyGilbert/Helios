#ifndef OSLVARFLOATBLOCK_H
#define OSLVARFLOATBLOCK_H

//------------------------------------------------------------------------------------------------------------------------------------
/// @class OSLVarFloatBlock
/// @brief This class is used for creating a Float variable node in our node graphics interface.

/// @author Declan Russell
/// @date 21/03/2015
//------------------------------------------------------------------------------------------------------------------------------------

#include "NodeGraph/OSLAbstractVarBlock.h"
#include <optixu/optixpp_namespace.h>
#include <QGraphicsScene>


class OSLVarFloatBlock : public OSLAbstractVarBlock
{
public:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief default constructor
    //------------------------------------------------------------------------------------------------------------------------------------
    OSLVarFloatBlock(QGraphicsScene *_scene,optix::Material &_mat,QGraphicsItem * parent = 0);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief default destuctor
    //------------------------------------------------------------------------------------------------------------------------------------
    ~OSLVarFloatBlock();
private:


};

#endif // OSLVARFLOATBLOCK_H
