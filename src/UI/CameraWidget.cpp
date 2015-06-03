#include "UI/CameraWidget.h"
#include "Core/pathtracerscene.h"
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QDoubleSpinBox>

//decare our static instance of our widget
CameraWidget* CameraWidget::m_instance;
//----------------------------------------------------------------------------------------------------------------------
CameraWidget* CameraWidget::getInstance(QWidget *parent){
    if(!m_instance){
        m_instance = new CameraWidget(parent);
    }
    else if(parent){
        m_instance->setParent(parent);
    }
    return m_instance;
}
//----------------------------------------------------------------------------------------------------------------------
void CameraWidget::destroy(){
    delete m_instance;
}
//----------------------------------------------------------------------------------------------------------------------
CameraWidget::CameraWidget(QWidget *parent) : QDockWidget(parent){
    this->setWindowTitle("Camera");
    QGroupBox *groupbox = new QGroupBox(this);
    this->setWidget(groupbox);
    QGridLayout *gridLayout = new QGridLayout(groupbox);
    groupbox->setLayout(gridLayout);

    //Enable DOF checkbox
    QLabel *DOFlabel = new QLabel(QString("Enable Depth of Field"), this);
    gridLayout->addWidget(DOFlabel, 0, 0, 1, 1);
    QCheckBox* DOFcheckbox = new QCheckBox(this);
    DOFcheckbox->setChecked(false);
    gridLayout->addWidget(DOFcheckbox, 0, 1, 1, 1);

    // Aperture radius
    m_apertureLabel = new QLabel(QString("Aperture Radius"), this);
    m_apertureLabel->setEnabled(false);
    gridLayout->addWidget(m_apertureLabel, 1, 0, 1, 1);
    m_apertureRadiusSB = new QDoubleSpinBox(this);
    m_apertureRadiusSB->setEnabled(false);
    m_apertureRadiusSB->setValue(0.5);
    gridLayout->addWidget(m_apertureRadiusSB, 1, 1, 1, 1);

    // Focal point
    m_focalPointLabel = new QLabel(QString("Focal Point"), this);
    m_focalPointLabel->setEnabled(false);
    gridLayout->addWidget(m_focalPointLabel, 2, 0, 1, 1);
    m_focalPointXSB = new QDoubleSpinBox(this);
    m_focalPointYSB = new QDoubleSpinBox(this);
    m_focalPointZSB = new QDoubleSpinBox(this);
    m_focalPointXSB->setEnabled(false);
    m_focalPointYSB->setEnabled(false);
    m_focalPointZSB->setEnabled(false);
    m_focalPointXSB->setValue(0.0);
    m_focalPointYSB->setValue(0.0);
    m_focalPointZSB->setValue(0.0);
    m_focalPointXSB->setMaximum(10.0);
    m_focalPointYSB->setMaximum(10.0);
    m_focalPointZSB->setMaximum(10.0);
    m_focalPointXSB->setMinimum(-10.0);
    m_focalPointYSB->setMinimum(-10.0);
    m_focalPointZSB->setMinimum(-10.0);

    gridLayout->addWidget(m_focalPointXSB, 2, 1, 1, 1);
    gridLayout->addWidget(m_focalPointYSB, 2, 2, 1, 1);
    gridLayout->addWidget(m_focalPointZSB, 2, 3, 1, 1);

    QSpacerItem* spacer = new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    gridLayout->addItem(spacer, 3, 0, 1, 4);

    //----------------------------------------------------------------------------------
    //-----------------------------------Connections------------------------------------
    //----------------------------------------------------------------------------------
    connect(DOFcheckbox, SIGNAL(clicked(bool)), this, SLOT(enableDOF(bool)));
    connect(m_apertureRadiusSB, SIGNAL(valueChanged(double)), this, SLOT(updateDOF()));
    connect(m_focalPointXSB, SIGNAL(valueChanged(double)), this, SLOT(updateDOF()));
    connect(m_focalPointYSB, SIGNAL(valueChanged(double)), this, SLOT(updateDOF()));
    connect(m_focalPointZSB, SIGNAL(valueChanged(double)), this, SLOT(updateDOF()));

}
//----------------------------------------------------------------------------------------------------------------------
CameraWidget::~CameraWidget(){

}
//----------------------------------------------------------------------------------------------------------------------
void CameraWidget::enableDOF(bool _enabled){
    m_apertureLabel->setEnabled(_enabled);
    m_apertureRadiusSB->setEnabled(_enabled);
    m_focalPointLabel->setEnabled(_enabled);
    m_focalPointXSB->setEnabled(_enabled);
    m_focalPointYSB->setEnabled(_enabled);
    m_focalPointZSB->setEnabled(_enabled);

    if(!_enabled){
        PathTracerScene::getInstance()->getContext()["aperture_radius"]->setFloat(0.0);
    }
    else{
        PathTracerScene::getInstance()->getContext()["aperture_radius"]->setFloat(m_apertureRadiusSB->value());
        PathTracerScene::getInstance()->getContext()["focal_point"]->setFloat(m_focalPointXSB->value(), m_focalPointYSB->value(), m_focalPointZSB->value());
    }

    // Reset the frame
    PathTracerScene::getInstance()->signalSceneChanged();
}
//----------------------------------------------------------------------------------------------------------------------
void CameraWidget::updateDOF(){
    PathTracerScene::getInstance()->getContext()["aperture_radius"]->setFloat(m_apertureRadiusSB->value());
    PathTracerScene::getInstance()->getContext()["focal_point"]->setFloat(m_focalPointXSB->value(), m_focalPointYSB->value(), m_focalPointZSB->value());

    // Reset the frame
    PathTracerScene::getInstance()->signalSceneChanged();
}
//----------------------------------------------------------------------------------------------------------------------
