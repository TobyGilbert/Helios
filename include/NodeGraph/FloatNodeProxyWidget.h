#ifndef FLOATNODEPROXYWIDGET_H
#define FLOATNODEPROXYWIDGET_H

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
public slots:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief slot to set the varible in our material when our spin box value is changed
    /// @param _val the value to set the variables in our material
    //------------------------------------------------------------------------------------------------------------------------------------
    void setMaterialVariables(double _val);
    //------------------------------------------------------------------------------------------------------------------------------------
private:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief a member for our spin box
    //------------------------------------------------------------------------------------------------------------------------------------
    QDoubleSpinBox *m_spinBox;
    //------------------------------------------------------------------------------------------------------------------------------------
};

#endif // FLOATNODEPROXYWIDGET_H
