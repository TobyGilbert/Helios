#ifndef ABSTRACTNODEPROXYWIDGET_H
#define ABSTRACTNODEPROXYWIDGET_H

//------------------------------------------------------------------------------------------------------------------------------------
/// @class AbstractNodeProxyWidget
/// @brief Abstract base class for all variable proxy widgets in our scene.
/// @brief This extends from QGraphicsProxyWidget which allows us to attach QWidgets onto QGraphicsItems.
/// @brief We need this got the ability to user signals and slots to change attributes in our optix material
/// @author Declan Russell
/// @date 05/05/2015
//------------------------------------------------------------------------------------------------------------------------------------

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
    /// @brief virtual method to set whatever linked variables we have
    //------------------------------------------------------------------------------------------------------------------------------------
    virtual void setLinkedVar(){}
    //------------------------------------------------------------------------------------------------------------------------------------
signals:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief signal to notify if an attribute has changed
    //------------------------------------------------------------------------------------------------------------------------------------
    void attributeChanged();
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
