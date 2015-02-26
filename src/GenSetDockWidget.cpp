#include "GenSetDockWidget.h"
#include <QSpinBox>


GenSetDockWidget::GenSetDockWidget(QWidget *parent) :
    QDockWidget(parent)
{
    //add the layout to our widget
    m_layout = new QGridLayout();
    //add our group box
    m_genSetGroupBox = new QGroupBox("General Settings",this);
    m_genSetGroupBox->setLayout(m_layout);
    this->setWidget(m_genSetGroupBox);
    //just a quick way to increment columns so we dont have to keep track
    int rowCount=0;

    //add a label for our movement render speed increase
    QLabel *mvRendSpeedLbl = new QLabel("Movement Render Reduction",this);
    mvRendSpeedLbl->setToolTip("This reduces the number of pixels rendered while moving for quicker interactibility");
    m_layout->addWidget(mvRendSpeedLbl,rowCount,0,1,1);
    m_widgetObjects.push_back(mvRendSpeedLbl);

    //add a slider for this paramiter
    QSlider *mvRenRudSldr = new QSlider(this);
    mvRenRudSldr->setOrientation(Qt::Horizontal);
    m_widgetObjects.push_back(mvRenRudSldr);
    mvRenRudSldr->setValue(4);
    mvRenRudSldr->setMaximum(10);
    mvRenRudSldr->setMinimum(1);
    m_layout->addWidget(mvRenRudSldr,rowCount,1,1,1);

    //add a Spinbox to show the value
    QSpinBox *mvRedRudSpnBx = new QSpinBox(this);
    m_widgetObjects.push_back(mvRedRudSpnBx);
    mvRedRudSpnBx->setValue(4);
    mvRedRudSpnBx->setMaximum(10);
    mvRedRudSpnBx->setMinimum(1);
    m_layout->addWidget(mvRedRudSpnBx,rowCount,2,1,1);
    rowCount++;

    connect(mvRenRudSldr,SIGNAL(sliderMoved(int)),mvRedRudSpnBx,SLOT(setValue(int)));
    connect(mvRedRudSpnBx,SIGNAL(valueChanged(int)),mvRenRudSldr,SLOT(setValue(int)));
    connect(mvRenRudSldr,SIGNAL(valueChanged(int)),this,SLOT(moveRenderReductionSlot(int)));



}
//----------------------------------------------------------------------------------------------------------------------
GenSetDockWidget::~GenSetDockWidget(){
    delete m_layout;
    delete m_genSetGroupBox;

    for(unsigned int i=0; i<m_widgetObjects.size(); i++){
        delete m_widgetObjects[i];
    }
}
//----------------------------------------------------------------------------------------------------------------------
