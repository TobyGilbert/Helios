#ifndef OSLVARFLOATBLOCK_H
#define OSLVARFLOATBLOCK_H

//------------------------------------------------------------------------------------------------------------------------------------
/// @class OSLVarFloatBlock
/// @brief This class is used for creating a Float variable node in our node graphics interface.

/// @author Declan Russell
/// @date 21/03/2015
//------------------------------------------------------------------------------------------------------------------------------------

#include "qneblock.h"
#include <QDoubleSpinBox>
#include <QGraphicsScene>

/// @todo ask Jon/Richard about multiple inheritance

class OSLVarFloatBlock : public QNEBlock
{
public:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief default constructor
    //------------------------------------------------------------------------------------------------------------------------------------
    OSLVarFloatBlock(QGraphicsScene *_scene,QGraphicsItem * parent = 0);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief QDoubleSpinBox to store and mutate the value of our block
    /// @brief This is public so it can be easily connected to other QObjects with signals and slots
    //------------------------------------------------------------------------------------------------------------------------------------
    QDoubleSpinBox *m_value;
    //------------------------------------------------------------------------------------------------------------------------------------
};

#endif // OSLVARFLOATBLOCK_H
