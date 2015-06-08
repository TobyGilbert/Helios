#ifndef FLOATNODEPROXYWIDGET_H
#define FLOATNODEPROXYWIDGET_H

//------------------------------------------------------------------------------------------------------------------------------------
/// @class FloatNodeProxyWidget
/// @brief Extention of AbstractNodeProxyWidget that allows us to select a float and apply it to a attribute of a material.
/// @brief This widget consists of a single Double spinbox to input a float value.
/// @author Declan Russell
/// @date 05/05/2015
//------------------------------------------------------------------------------------------------------------------------------------

#include <QGraphicsItem>
#include <optixu/optixpp_namespace.h>
#include "AbstractNodeProxyWidget.h"
#include <QDoubleSpinBox>
#include <NodeGraph/qneport.h>

class FloatNodeProxyWidget : public AbstractNodeProxyWidget
{
    Q_OBJECT
public:
    FloatNodeProxyWidget(QNEPort *_portConnected,optix::Material &_mat,QGraphicsItem *parent = 0);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief default destructor
    //------------------------------------------------------------------------------------------------------------------------------------
    ~FloatNodeProxyWidget();
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief overite our setLinkedVar function to put our own functionality in
    //------------------------------------------------------------------------------------------------------------------------------------
    void setLinkedVar();
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief a member for our spin box
    //------------------------------------------------------------------------------------------------------------------------------------
    QDoubleSpinBox *m_spinBox;
    //------------------------------------------------------------------------------------------------------------------------------------
public slots:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief slot to set the varible in our material when our spin box value is changed
    /// @param _val the value to set the variables in our material
    //------------------------------------------------------------------------------------------------------------------------------------
    void setMaterialVariables(double _val);
    //------------------------------------------------------------------------------------------------------------------------------------
};

#endif // FLOATNODEPROXYWIDGET_H
