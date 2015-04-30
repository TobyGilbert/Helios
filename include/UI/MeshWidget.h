#ifndef MeshWidget_H
#define MeshWidget_H

/// @class MeshWidget
/// @date 29/01/14
/// @author Declan Russell
/// @brief This class is an extention of QWidget that adds all our mesh properties controls as default

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpacerItem>
#include <QPushButton>
#include <QFileDialog>
#include <QString>

#include "AbstractMaterialWidget.h"

class MeshWidget : public QWidget
{
    Q_OBJECT
public:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our default constructor
    //----------------------------------------------------------------------------------------------------------------------
    explicit MeshWidget(std::string _id = 0);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our destructor
    //----------------------------------------------------------------------------------------------------------------------
    ~MeshWidget();
    //----------------------------------------------------------------------------------------------------------------------
signals:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief a signal called when something has changed to promt the update of our scene
    //----------------------------------------------------------------------------------------------------------------------
    void updateScene();
    //----------------------------------------------------------------------------------------------------------------------
public slots:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our signal to norify if any tranform spinbox have been changed
    //----------------------------------------------------------------------------------------------------------------------
    void signalTransformChange();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief slot called when a material has been selected for our model
    //----------------------------------------------------------------------------------------------------------------------
    void setMaterial(AbstractMaterialWidget * _mat);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief applys the material from our node graph to our mesh
    //----------------------------------------------------------------------------------------------------------------------
    void applyOSLMaterial();
    //----------------------------------------------------------------------------------------------------------------------
private:
    AbstractMaterialWidget *m_testMat;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief a place holder for our material widget
    //----------------------------------------------------------------------------------------------------------------------
    AbstractMaterialWidget *m_currentMatWidget;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief set material button
    //----------------------------------------------------------------------------------------------------------------------
    QPushButton *m_setMatButton;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our spacer for the widget
    //----------------------------------------------------------------------------------------------------------------------
    QSpacerItem* m_meshSpacer;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our spinbox for x rotation
    //----------------------------------------------------------------------------------------------------------------------
    QDoubleSpinBox* m_meshRotateXDSpinBox;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our spinbox for y rotation
    //----------------------------------------------------------------------------------------------------------------------
    QDoubleSpinBox* m_meshRotateYDSpinBox;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our spinbox for z rotation
    //----------------------------------------------------------------------------------------------------------------------
    QDoubleSpinBox* m_meshRotateZDSpinBox;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our rotate label
    //----------------------------------------------------------------------------------------------------------------------
    QLabel* m_meshRotateLabel;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our spinbox for x translation
    //----------------------------------------------------------------------------------------------------------------------
    QDoubleSpinBox* m_meshTranslateXDSpinBox;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our spinbox for y translation
    //----------------------------------------------------------------------------------------------------------------------
    QDoubleSpinBox* m_meshTranslateYDSpinBox;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our spinbox for z translation
    //----------------------------------------------------------------------------------------------------------------------
    QDoubleSpinBox* m_meshTranslateZDSpinBox;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our translate label
    //----------------------------------------------------------------------------------------------------------------------
    QLabel* m_meshTranslateLabel;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our spinbox for x scale
    //----------------------------------------------------------------------------------------------------------------------
    QDoubleSpinBox* m_meshScaleXDSpinBox;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our spinbox for Y scale
    //----------------------------------------------------------------------------------------------------------------------
    QDoubleSpinBox* m_meshScaleYDSpinBox;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our spinbox for z scale
    //----------------------------------------------------------------------------------------------------------------------
    QDoubleSpinBox* m_meshScaleZDSpinBox;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our scale label
    //----------------------------------------------------------------------------------------------------------------------
    QLabel* m_meshScaleLabel;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our grid layour for our widget
    //----------------------------------------------------------------------------------------------------------------------
    QGridLayout* m_meshGridLayout;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the mesh id
    //----------------------------------------------------------------------------------------------------------------------
    std::string m_meshId;
    //----------------------------------------------------------------------------------------------------------------------

};

#endif // MeshWidget_H
