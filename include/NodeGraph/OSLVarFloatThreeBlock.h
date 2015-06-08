#ifndef OSLVARFLOATTHREEBLOCK_H
#define OSLVARFLOATTHREEBLOCK_H

//------------------------------------------------------------------------------------------------------------------------------------
/// @class OSLVarFloatBlock
/// @brief This class is used for creating a vectors variable node in our node graphics interface.
/// @brief This will be used for such tripplets as vectors
/// @author Declan Russell
/// @date 21/03/2015
//------------------------------------------------------------------------------------------------------------------------------------

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
    OSLVarFloatThreeBlock(QGraphicsScene *_scene, Material _mat, QGraphicsItem * parent = 0);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief default destructor
    //------------------------------------------------------------------------------------------------------------------------------------
    ~OSLVarFloatThreeBlock(){}
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief overload our save function for float 3 node implimentation
    //------------------------------------------------------------------------------------------------------------------------------------
    void save(QDataStream &ds);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief overload our load function for float 3 node implimentation
    //------------------------------------------------------------------------------------------------------------------------------------
    void load(QDataStream &, QMap<quint64, QNEPort *> &portMap);
    //------------------------------------------------------------------------------------------------------------------------------------

};

#endif // OSLVARFLOATTHREEBLOCK_H
