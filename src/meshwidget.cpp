#include "meshwidget.h"
#include <QMessageBox>
#include <iostream>

MeshWidget::MeshWidget(int _id) :
    QWidget()
{
    m_meshId = _id;
    //add our grid layout to our widget
    m_meshGridLayout = new QGridLayout();
    this->setLayout(m_meshGridLayout);


    m_importMeshBtn = new QPushButton("Import Mesh");
    m_meshGridLayout->addWidget(m_importMeshBtn,0,0,1,1);

    m_meshTranslateLabel = new QLabel("Translate");
    m_meshTranslateLabel->hide();
    m_meshGridLayout->addWidget(m_meshTranslateLabel, 1, 0, 1, 1);
    m_meshTranslateXDSpinBox = new QDoubleSpinBox();
    m_meshTranslateXDSpinBox->setMaximum(1000.0);
    m_meshTranslateXDSpinBox->setMinimum(-1000.0);
    m_meshTranslateXDSpinBox->hide();
    m_meshGridLayout->addWidget(m_meshTranslateXDSpinBox, 1, 1, 1, 1);
    m_meshTranslateYDSpinBox = new QDoubleSpinBox();
    m_meshTranslateYDSpinBox->setMaximum(1000.0);
    m_meshTranslateYDSpinBox->setMinimum(-1000.0);
    m_meshTranslateYDSpinBox->hide();
    m_meshGridLayout->addWidget(m_meshTranslateYDSpinBox, 1, 2, 1, 1);
    m_meshTranslateZDSpinBox = new QDoubleSpinBox();
    m_meshTranslateZDSpinBox->setMaximum(1000.0);
    m_meshTranslateZDSpinBox->setMinimum(-1000.0);
    m_meshTranslateZDSpinBox->hide();
    m_meshGridLayout->addWidget(m_meshTranslateZDSpinBox, 1, 3, 1, 1);
    m_meshRotateLabel = new QLabel("Rotate");
    m_meshRotateLabel->hide();
    m_meshGridLayout->addWidget(m_meshRotateLabel, 2, 0, 1, 1);
    m_meshRotateXDSpinBox = new QDoubleSpinBox();
    m_meshRotateXDSpinBox->setMaximum(1000.0);
    m_meshRotateXDSpinBox->setMinimum(-1000.0);
    m_meshRotateXDSpinBox->hide();
    m_meshGridLayout->addWidget(m_meshRotateXDSpinBox, 2, 1, 1, 1);
    m_meshRotateYDSpinBox = new QDoubleSpinBox();
    m_meshRotateYDSpinBox->setMaximum(1000.0);
    m_meshRotateYDSpinBox->setMinimum(-1000.0);
    m_meshRotateYDSpinBox->hide();
    m_meshGridLayout->addWidget(m_meshRotateYDSpinBox, 2, 2, 1, 1);
    m_meshRotateZDSpinBox = new QDoubleSpinBox();
    m_meshRotateZDSpinBox->setMaximum(1000.0);
    m_meshRotateZDSpinBox->setMinimum(-1000.0);
    m_meshRotateZDSpinBox->hide();
    m_meshGridLayout->addWidget(m_meshRotateZDSpinBox, 2, 3, 1, 1);
    m_meshScaleLabel = new QLabel("Scale");
    m_meshScaleLabel->hide();
    m_meshGridLayout->addWidget(m_meshScaleLabel, 3, 0, 1, 1);
    m_meshScaleXDSpinBox = new QDoubleSpinBox();
    m_meshScaleXDSpinBox->setMaximum(1000.0);
    m_meshScaleXDSpinBox->setMinimum(-1000.0);
    m_meshScaleXDSpinBox->setValue(1.0);
    m_meshScaleXDSpinBox->hide();
    m_meshGridLayout->addWidget(m_meshScaleXDSpinBox, 3, 1, 1, 1);
    m_meshScaleYDSpinBox = new QDoubleSpinBox();
    m_meshScaleYDSpinBox->setMaximum(1000.0);
    m_meshScaleYDSpinBox->setMinimum(-1000.0);
    m_meshScaleYDSpinBox->setValue(1.0);
    m_meshScaleYDSpinBox->hide();
    m_meshGridLayout->addWidget(m_meshScaleYDSpinBox, 3, 2, 1, 1);
    m_meshScaleZDSpinBox = new QDoubleSpinBox();
    m_meshScaleZDSpinBox->setMaximum(1000.0);
    m_meshScaleZDSpinBox->setMinimum(-1000.0);
    m_meshScaleZDSpinBox->setValue(1.0);
    m_meshScaleZDSpinBox->hide();
    m_meshGridLayout->addWidget(m_meshScaleZDSpinBox, 3, 3, 1, 1);
    m_meshSpacer = new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_meshGridLayout->addItem(m_meshSpacer, 5, 0, 2, 1);

    //-------------------------------------------------------------------------------------------------
    //-------------------------Connect up our signals and slots----------------------------------------
    //-------------------------------------------------------------------------------------------------

    connect(m_meshTranslateXDSpinBox,SIGNAL(editingFinished()),this,SLOT(signalTransformChange()));
    connect(m_meshTranslateYDSpinBox,SIGNAL(editingFinished()),this,SLOT(signalTransformChange()));
    connect(m_meshTranslateZDSpinBox,SIGNAL(editingFinished()),this,SLOT(signalTransformChange()));
    connect(m_meshRotateXDSpinBox,SIGNAL(editingFinished()),this,SLOT(signalTransformChange()));
    connect(m_meshRotateYDSpinBox,SIGNAL(editingFinished()),this,SLOT(signalTransformChange()));
    connect(m_meshRotateZDSpinBox,SIGNAL(editingFinished()),this,SLOT(signalTransformChange()));
    connect(m_meshScaleXDSpinBox,SIGNAL(editingFinished()),this,SLOT(signalTransformChange()));
    connect(m_meshScaleYDSpinBox,SIGNAL(editingFinished()),this,SLOT(signalTransformChange()));
    connect(m_meshScaleZDSpinBox,SIGNAL(editingFinished()),this,SLOT(signalTransformChange()));
    connect(m_importMeshBtn,SIGNAL(clicked()),this,SLOT(signalImportMesh()));

}
//----------------------------------------------------------------------------------------------------------------------
MeshWidget::~MeshWidget(){
    delete m_meshTranslateXDSpinBox;
    delete m_meshTranslateYDSpinBox;
    delete m_meshTranslateZDSpinBox;
    delete m_meshRotateXDSpinBox;
    delete m_meshRotateYDSpinBox;
    delete m_meshRotateZDSpinBox;
    delete m_meshScaleXDSpinBox;
    delete m_meshScaleYDSpinBox;
    delete m_meshScaleZDSpinBox;
    delete m_meshRotateLabel;
    delete m_meshTranslateLabel;
    delete m_meshScaleLabel;
    delete m_meshGridLayout;
}
//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::signalTransformChange(){

    // get our transform values
    float transX = m_meshTranslateXDSpinBox->value();
    float transY = m_meshTranslateYDSpinBox->value();
    float transZ = m_meshTranslateZDSpinBox->value();
    float rotX = m_meshRotateXDSpinBox->value();
    float rotY = m_meshRotateYDSpinBox->value();
    float rotZ = m_meshRotateZDSpinBox->value();
    float scaleX = m_meshScaleXDSpinBox->value();
    float scaleY = m_meshScaleYDSpinBox->value();
    float scaleZ = m_meshScaleZDSpinBox->value();

    // signal the change to whatever is hooked up to use them
    meshTransform(m_meshId,transX,transY,transZ,rotX,rotY,rotZ,scaleX,scaleY,scaleZ);
}

//----------------------------------------------------------------------------------------------------------------------
void MeshWidget::signalImportMesh(){
    QString location = QFileDialog::getOpenFileName(this,tr("Import Mesh"), "models/", tr("Mesh Files (*.obj)"));
    if (!location.isEmpty()){
            std::string path = location.toUtf8().constData();
            importMesh(m_meshId, path);

            //hide our import mesh button as we done need that anymore
            m_importMeshBtn->hide();
            //Show all our mesh properties properties
            m_meshTranslateLabel->show();
            m_meshTranslateXDSpinBox->show();
            m_meshTranslateYDSpinBox->show();
            m_meshTranslateZDSpinBox->show();
            m_meshRotateLabel->show();
            m_meshRotateXDSpinBox->show();
            m_meshRotateYDSpinBox->show();
            m_meshRotateZDSpinBox->show();
            m_meshScaleLabel->show();
            m_meshScaleXDSpinBox->show();
            m_meshScaleYDSpinBox->show();
            m_meshScaleZDSpinBox->show();
    }
    else{
            QMessageBox::information(this,tr("MeshImport"),tr("Cannot load mesh %1.").arg(location));
    }


}
