#ifndef OSLVARNORMALBLOCK_H
#define OSLVARNORMALBLOCK_H

//------------------------------------------------------------------------------------------------------------------------------------
/// @class OSLVarFloatBlock
/// @brief This class is used for creating a normals variable node in our node graphics interface.
/// @author Declan Russell
/// @date 21/03/2015
//------------------------------------------------------------------------------------------------------------------------------------

#include "NodeGraph/OSLAbstractVarBlock.h"
#include <optixu/optixpp_namespace.h>
#include <QGraphicsScene>

class OSLVarNormalBlock : public OSLAbstractVarBlock
{
public:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief default constructor
    //------------------------------------------------------------------------------------------------------------------------------------
    OSLVarNormalBlock(QGraphicsScene *_scene, optix::Material _mat, QGraphicsItem *parent = 0);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief default destructor
    //------------------------------------------------------------------------------------------------------------------------------------
    ~OSLVarNormalBlock(){}
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief overload our save function for normal node implimentation
    //------------------------------------------------------------------------------------------------------------------------------------
    void save(QDataStream &ds);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief overload our load function for normal node implimentation
    //------------------------------------------------------------------------------------------------------------------------------------
    void load(QDataStream &, QMap<quint64, QNEPort *> &portMap);
    //------------------------------------------------------------------------------------------------------------------------------------

};

#endif // OSLVARNORMALBLOCK_H
