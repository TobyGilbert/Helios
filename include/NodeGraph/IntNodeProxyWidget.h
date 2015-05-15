#ifndef INTNODEPROXYWIDGET_H
#define INTNODEPROXYWIDGET_H

#include <QGraphicsItem>
#include <optixu/optixpp_namespace.h>
#include "AbstractNodeProxyWidget.h"
#include <QSpinBox>
#include <NodeGraph/qneport.h>

class IntNodeProxyWidget : public AbstractNodeProxyWidget
{
    Q_OBJECT
public:
    IntNodeProxyWidget(QNEPort *_portConnected,optix::Material &_mat,QGraphicsItem *parent = 0);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief default destructor
    //------------------------------------------------------------------------------------------------------------------------------------
    ~IntNodeProxyWidget();
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief overite our setLinkedVar function to put our own functionality in
    //------------------------------------------------------------------------------------------------------------------------------------
    void setLinkedVar();
    //------------------------------------------------------------------------------------------------------------------------------------    
    /// @brief a member for our spin box
    //------------------------------------------------------------------------------------------------------------------------------------
    QSpinBox *m_spinBox;
    //------------------------------------------------------------------------------------------------------------------------------------
public slots:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief slot to set the varible in our material when our spin box value is changed
    /// @param _val the value to set the variables in our material
    //------------------------------------------------------------------------------------------------------------------------------------
    void setMaterialVariables(int _val);
    //------------------------------------------------------------------------------------------------------------------------------------

};

#endif // INTNODEPROXYWIDGET_H
