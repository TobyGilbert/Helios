#ifndef ABSTRACTNODEPROXYWIDGET_H
#define ABSTRACTNODEPROXYWIDGET_H

#include <QGraphicsProxyWidget>
#include <optixu/optixpp_namespace.h>
#include "Core/pathtracerscene.h"
#include "NodeGraph/qneport.h"
class AbstractNodeProxyWidget : public QGraphicsProxyWidget
{
    Q_OBJECT
public:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief our default constructor
    //------------------------------------------------------------------------------------------------------------------------------------
    AbstractNodeProxyWidget(QNEPort *_connectedPort,optix::Material &_mat, QGraphicsItem *_parent = 0);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief returns the linked varible name connected to desired port
    /// @param _linkedVarNames - vector of strings to store linked variable names
    //------------------------------------------------------------------------------------------------------------------------------------
    void getLinkedVarName(std::vector<std::string> &_linkedVarNames);
    //------------------------------------------------------------------------------------------------------------------------------------
protected:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief a member to hold the material to be edited
    //------------------------------------------------------------------------------------------------------------------------------------
    optix::Material m_material;
    //------------------------------------------------------------------------------------------------------------------------------------
private:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief a pointer to the port linked to our spin box
    //------------------------------------------------------------------------------------------------------------------------------------
    QNEPort *m_connectedPort;
    //------------------------------------------------------------------------------------------------------------------------------------
};

#endif // ABSTRACTNODEPROXYWIDGET_H
