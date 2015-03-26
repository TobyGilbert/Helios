#ifndef OSLBLOCK_H
#define OSLBLOCK_H

//------------------------------------------------------------------------------------------------------------------------------------
/// @class OLSBlock
/// @brief A QNEBlock that has been specialised for importing OSL shaders.
/// @brief This will be visualised as a node in our graphical user interface.
/// @author Declan Russell
/// @date 19/03/2015
//------------------------------------------------------------------------------------------------------------------------------------

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPathItem>
#include "NodeGraph/qneblock.h"
#include "OSLCompiler/OslReader.h"
#include "OSLCompiler/OsoReader.h"


class OSLBlock : public QNEBlock
{
public:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief our default constructor
    //------------------------------------------------------------------------------------------------------------------------------------
    OSLBlock(QGraphicsItem *parent=0);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief overiding of mousePressEvent to add functionality to our import shader button
    /// @brief written by Declan Russell
    /// @param _event - event data managed by Qt
    //------------------------------------------------------------------------------------------------------------------------------------
    void mousePressEvent(QGraphicsSceneMouseEvent *_event);
    //------------------------------------------------------------------------------------------------------------------------------------
private:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief A graphics path item to draw a button for importing
    //----------------------------------------------------------------------------------------------------------------------
    QGraphicsPathItem *m_importBtnGI;
    //----------------------------------------------------------------------------------------------------------------------
};

#endif // OSLBLOCK_H
