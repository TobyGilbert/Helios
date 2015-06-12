#include "UI/CameraWidget.h"
#include "Core/pathtracerscene.h"
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QPushButton>

/// @author Toby Gilbert
/// @brief A camera widget used to update camera setting e.g. depth of field

//decare our static instance of our widget
CameraWidget* CameraWidget::m_instance;
//----------------------------------------------------------------------------------------------------------------------
CameraWidget* CameraWidget::getInstance(QWidget *parent)
{
    if(!m_instance)
    {
        m_instance = new CameraWidget(parent);
    }
    else if(parent)
    {
        m_instance->setParent(parent);
    }
    return m_instance;
}
//----------------------------------------------------------------------------------------------------------------------
void CameraWidget::destroy()
{
    delete m_instance;
}
//----------------------------------------------------------------------------------------------------------------------
CameraWidget::CameraWidget(QWidget *parent) : QDockWidget(parent)
{
    this->setWindowTitle("Camera");
    QGroupBox *groupbox = new QGroupBox(this);
    this->setWidget(groupbox);
    QGridLayout *gridLayout = new QGridLayout(groupbox);
    groupbox->setLayout(gridLayout);

    // Position
    QLabel *posLabel = new QLabel(QString("Position"), this);
    m_positionX = new QDoubleSpinBox(this);
    m_positionY = new QDoubleSpinBox(this);
    m_positionZ = new QDoubleSpinBox(this);
    m_positionX->setMinimum(-INFINITY);
    m_positionY->setMinimum(-INFINITY);
    m_positionZ->setMinimum(-INFINITY);
    m_positionX->setMaximum(INFINITY);
    m_positionY->setMaximum(INFINITY);
    m_positionZ->setMaximum(INFINITY);
    m_positionX->setValue(0.0);
    m_positionY->setValue(0.0);
    m_positionZ->setValue(-25.0);
    gridLayout->addWidget(posLabel, 0, 0, 1, 1);
    gridLayout->addWidget(m_positionX, 0, 1, 1, 1);
    gridLayout->addWidget(m_positionY, 0, 2, 1, 1);
    gridLayout->addWidget(m_positionZ, 0, 3, 1, 1);

    // Lookat
    QLabel *lookAtLabel = new QLabel(QString("Look At"), this);
    m_lookAtX = new QDoubleSpinBox(this);
    m_lookAtY = new QDoubleSpinBox(this);
    m_lookAtZ = new QDoubleSpinBox(this);
    m_lookAtX->setValue(0.0);
    m_lookAtY->setValue(0.0);
    m_lookAtZ->setValue(0.0);
    m_lookAtX->setMinimum(-INFINITY);
    m_lookAtY->setMinimum(-INFINITY);
    m_lookAtZ->setMinimum(-INFINITY);
    m_lookAtX->setMaximum(INFINITY);
    m_lookAtY->setMaximum(INFINITY);
    m_lookAtZ->setMaximum(INFINITY);
    gridLayout->addWidget(lookAtLabel, 1, 0, 1, 1);
    gridLayout->addWidget(m_lookAtX, 1, 1, 1, 1);
    gridLayout->addWidget(m_lookAtY, 1, 2, 1, 1);
    gridLayout->addWidget(m_lookAtZ, 1, 3, 1, 1);

    // Up
    QLabel *upLabel = new QLabel(QString("Up"), this);
    m_upX = new QDoubleSpinBox(this);
    m_upY = new QDoubleSpinBox(this);
    m_upZ = new QDoubleSpinBox(this);
    m_upX->setValue(0.0);
    m_upY->setValue(1.0);
    m_upZ->setValue(0.0);
    m_upX->setMinimum(-INFINITY);
    m_upY->setMinimum(-INFINITY);
    m_upZ->setMinimum(-INFINITY);
    m_upX->setMaximum(INFINITY);
    m_upY->setMaximum(INFINITY);
    m_upZ->setMaximum(INFINITY);
    gridLayout->addWidget(upLabel, 2, 0, 1, 1);
    gridLayout->addWidget(m_upX, 2, 1, 1, 1);
    gridLayout->addWidget(m_upY, 2, 2, 1, 1);
    gridLayout->addWidget(m_upZ, 2, 3, 1, 1);

    // FOV
    QLabel *hFOVlabel = new QLabel(QString("Horizontal Field of View"), this);
    m_hFOV = new QDoubleSpinBox(this);
    m_hFOV->setValue(35.0);
    gridLayout->addWidget(hFOVlabel, 3, 0, 1, 1);
    gridLayout->addWidget(m_hFOV, 3, 1, 1, 1);
    QLabel *vFOVlabel = new QLabel(QString("Vertical Field of View"), this);
    m_vFOV = new QDoubleSpinBox(this);
    m_vFOV->setValue(35.0);
    gridLayout->addWidget(vFOVlabel, 4, 0, 1, 1);
    gridLayout->addWidget(m_vFOV, 4, 1, 1, 1);

    QGroupBox* DOFgroupbox = new QGroupBox(this);
    gridLayout->addWidget(DOFgroupbox, 5, 0, 1, 4);
    QGridLayout* DOFgridLayout = new QGridLayout(groupbox);
    DOFgroupbox->setLayout(DOFgridLayout);

    //Enable DOF checkbox
    QLabel *DOFlabel = new QLabel(QString("Enable Depth of Field"), this);
    DOFgridLayout->addWidget(DOFlabel, 0, 0, 1, 1);
    QCheckBox* DOFcheckbox = new QCheckBox(this);
    DOFcheckbox->setChecked(false);
    DOFgridLayout->addWidget(DOFcheckbox, 0, 1, 1, 1);

    // Aperture radius
    m_apertureLabel = new QLabel(QString("Aperture Radius"), this);
    m_apertureLabel->setEnabled(false);
    DOFgridLayout->addWidget(m_apertureLabel, 1, 0, 1, 1);
    m_apertureRadiusSB = new QDoubleSpinBox(this);
    m_apertureRadiusSB->setEnabled(false);
    m_apertureRadiusSB->setValue(0.5);
    m_apertureRadiusSB->setDecimals(3);
    m_apertureRadiusSB->setSingleStep(0.001);
    m_apertureRadiusSB->setMaximum(INFINITY);
    DOFgridLayout->addWidget(m_apertureRadiusSB, 1, 1, 1, 1);

    // Focal point
    m_focalPointLabel = new QLabel(QString("Focal Point"), this);
    m_focalPointLabel->setEnabled(false);
    DOFgridLayout->addWidget(m_focalPointLabel, 2, 0, 1, 1);
    m_focalPointXSB = new QDoubleSpinBox(this);
    m_focalPointYSB = new QDoubleSpinBox(this);
    m_focalPointZSB = new QDoubleSpinBox(this);
    m_focalPointXSB->setEnabled(false);
    m_focalPointYSB->setEnabled(false);
    m_focalPointZSB->setEnabled(false);
    m_focalPointXSB->setValue(0.0);
    m_focalPointYSB->setValue(0.0);
    m_focalPointZSB->setValue(0.0);
    m_focalPointXSB->setMaximum(INFINITY);
    m_focalPointYSB->setMaximum(INFINITY);
    m_focalPointZSB->setMaximum(INFINITY);
    m_focalPointXSB->setMinimum(-INFINITY);
    m_focalPointYSB->setMinimum(-INFINITY);
    m_focalPointZSB->setMinimum(-INFINITY);
    m_focalPointXSB->setSingleStep(0.01);
    m_focalPointYSB->setSingleStep(0.01);
    m_focalPointZSB->setSingleStep(0.01);

    DOFgridLayout->addWidget(m_focalPointXSB, 2, 1, 1, 1);
    DOFgridLayout->addWidget(m_focalPointYSB, 2, 2, 1, 1);
    DOFgridLayout->addWidget(m_focalPointZSB, 2, 3, 1, 1);

    QPushButton *resetCamera = new QPushButton(QString("Reset Camera"), this);
    gridLayout->addWidget(resetCamera, 6, 0, 1, 4);

    QSpacerItem* spacer = new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    gridLayout->addItem(spacer, 7, 0, 1, 4);

    //----------------------------------------------------------------------------------
    //-----------------------------------Connections------------------------------------
    //----------------------------------------------------------------------------------
    connect(DOFcheckbox, SIGNAL(clicked(bool)), this, SLOT(enableDOF(bool)));
    connect(m_apertureRadiusSB, SIGNAL(valueChanged(double)), this, SLOT(updateDOF()));
    connect(m_focalPointXSB, SIGNAL(valueChanged(double)), this, SLOT(updateDOF()));
    connect(m_focalPointYSB, SIGNAL(valueChanged(double)), this, SLOT(updateDOF()));
    connect(m_focalPointZSB, SIGNAL(valueChanged(double)), this, SLOT(updateDOF()));
    connect(m_hFOV, SIGNAL(valueChanged(double)), this, SLOT(updateCamera()));
    connect(m_vFOV, SIGNAL(valueChanged(double)), this, SLOT(updateCamera()));
    connect(m_positionX, SIGNAL(valueChanged(double)), this, SLOT(updateCamera()));
    connect(m_positionY, SIGNAL(valueChanged(double)), this, SLOT(updateCamera()));
    connect(m_positionZ, SIGNAL(valueChanged(double)), this, SLOT(updateCamera()));
    connect(m_lookAtX, SIGNAL(valueChanged(double)), this, SLOT(updateCamera()));
    connect(m_lookAtY, SIGNAL(valueChanged(double)), this, SLOT(updateCamera()));
    connect(m_lookAtZ, SIGNAL(valueChanged(double)), this, SLOT(updateCamera()));
    connect(m_upX, SIGNAL(valueChanged(double)), this, SLOT(updateCamera()));
    connect(m_upY, SIGNAL(valueChanged(double)), this, SLOT(updateCamera()));
    connect(m_upZ, SIGNAL(valueChanged(double)), this, SLOT(updateCamera()));
    connect(resetCamera, SIGNAL(clicked()), this, SLOT(resetCamera()));

}
//----------------------------------------------------------------------------------------------------------------------
CameraWidget::~CameraWidget()
{

}
//----------------------------------------------------------------------------------------------------------------------
void CameraWidget::enableDOF(bool _enabled)
{
    m_apertureLabel->setEnabled(_enabled);
    m_apertureRadiusSB->setEnabled(_enabled);
    m_focalPointLabel->setEnabled(_enabled);
    m_focalPointXSB->setEnabled(_enabled);
    m_focalPointYSB->setEnabled(_enabled);
    m_focalPointZSB->setEnabled(_enabled);

    if(!_enabled)
    {
        PathTracerScene::getInstance()->getContext()["aperture_radius"]->setFloat(0.0);
    }
    else
    {
        PathTracerScene::getInstance()->getContext()["aperture_radius"]->setFloat(m_apertureRadiusSB->value());
        PathTracerScene::getInstance()->getContext()["focal_point"]->setFloat(m_focalPointXSB->value(), m_focalPointYSB->value(), m_focalPointZSB->value());
    }

    // Reset the frame
    updateScene();
}
//----------------------------------------------------------------------------------------------------------------------
void CameraWidget::updateDOF()
{
    PathTracerScene::getInstance()->getContext()["aperture_radius"]->setFloat(m_apertureRadiusSB->value());
    PathTracerScene::getInstance()->getContext()["focal_point"]->setFloat(m_focalPointXSB->value(), m_focalPointYSB->value(), m_focalPointZSB->value());

    // Reset the frame
    updateScene();
}
//----------------------------------------------------------------------------------------------------------------------
void CameraWidget::updateCamera()
{
    PathTracerScene::getInstance()->getCamera()->setParameters(optix::make_float3( m_positionX->value(), m_positionY->value(), m_positionZ->value() ),      //eye
                                                               optix::make_float3( m_lookAtX->value(), m_lookAtY->value(), m_lookAtZ->value() ),        //lookat
                                                               optix::make_float3( m_upX->value(), m_upY->value(),  m_upZ->value() ),       // up
                                                               m_vFOV->value(),
                                                               m_hFOV->value());

    PathTracerScene::getInstance()->updateCamera();

    updateScene();
}

void CameraWidget::resetCamera()
{
    m_positionX->setValue(0.0);
    m_positionY->setValue(0.0);
    m_positionZ->setValue(-25.0);
    m_lookAtX->setValue(0.0);
    m_lookAtY->setValue(0.0);
    m_lookAtZ->setValue(0.0);
    m_upX->setValue(0.0);
    m_upY->setValue(1.0);
    m_upZ->setValue(0.0);

    PathTracerScene::getInstance()->getCamera()->setParameters(optix::make_float3(0.0, 0.0, -25.0 ),      //eye
                                                               optix::make_float3( 0.0, 0.0, 0.0 ),        //lookat
                                                               optix::make_float3( 0.0, 1.0,  0.0 ),       // up
                                                               35.0,
                                                               35.0);

    PathTracerScene::getInstance()->updateCamera();
    resetGlabalTrans();
    updateScene();
}
