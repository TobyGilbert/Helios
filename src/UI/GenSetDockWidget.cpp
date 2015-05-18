#include "UI/GenSetDockWidget.h"
#include <QSpinBox>
#include <QFile>


GenSetDockWidget::GenSetDockWidget(QWidget *parent) :
    QDockWidget(parent)
{
    //add the layout to our widget
    m_layout = new QGridLayout();
    //add our group box
    m_genSetGroupBox = new QGroupBox("General Settings",this);
    m_genSetGroupBox->setLayout(m_layout);
    this->setWidget(m_genSetGroupBox);
    // set the style sheet
    QFile file("styleSheet/darkOrange");
    file.open(QFile::ReadOnly);
    QString stylesheet = QLatin1String(file.readAll());
    this->setStyleSheet(stylesheet);
    //just a quick way to increment columns so we dont have to keep track

    //add a label for our movement render speed increase
    QLabel *mvRendSpeedLbl = new QLabel("Movement Render Reduction: ",this);
    mvRendSpeedLbl->setToolTip("This reduces the number of pixels rendered while moving for quicker interactibility");
    m_layout->addWidget(mvRendSpeedLbl,0,0,1,1);

    //add a slider for this paramiter
    QSlider *mvRenRudSldr = new QSlider(this);
    mvRenRudSldr->setOrientation(Qt::Horizontal);
    mvRenRudSldr->setValue(4);
    mvRenRudSldr->setMaximum(10);
    mvRenRudSldr->setMinimum(1);
    m_layout->addWidget(mvRenRudSldr,0,1,1,1);

    //add a Spinbox to show the value
    QSpinBox *mvRedRudSpnBx = new QSpinBox(this);
    mvRedRudSpnBx->setValue(4);
    mvRedRudSpnBx->setMaximum(10);
    mvRedRudSpnBx->setMinimum(1);
    m_layout->addWidget(mvRedRudSpnBx,0,2,1,1);

    // connect our signals and slots
    connect(mvRenRudSldr,SIGNAL(sliderMoved(int)),mvRedRudSpnBx,SLOT(setValue(int)));
    connect(mvRedRudSpnBx,SIGNAL(valueChanged(int)),mvRenRudSldr,SLOT(setValue(int)));
    connect(mvRenRudSldr,SIGNAL(valueChanged(int)),this,SLOT(moveRenderReductionSlot(int)));

    //setup a field for changing the timeout duration
    //a label
    m_layout->addWidget(new QLabel("Rendering timeout duration: ",this),1,0,1,1);
    //a spinbox
    QSpinBox *timeOutSpnBx = new QSpinBox(this);
    timeOutSpnBx->setValue(5);
    timeOutSpnBx->setMaximum(INFINITY);
    m_layout->addWidget(timeOutSpnBx,1,1,1,1);
    //connect our signals and slots
    connect(timeOutSpnBx,SIGNAL(valueChanged(int)),this,SLOT(setTimeOutDurSlot(int)));

    //add a field for our max depth
    m_layout->addWidget(new QLabel("Max Ray Depth",this),2,0,1,1);
    QSpinBox *maxDepthSpn = new QSpinBox(this);
    maxDepthSpn->setValue(5);
    maxDepthSpn->setMaximum(INFINITY);
    connect(maxDepthSpn,SIGNAL(valueChanged(int)),this,SLOT(setMaxRayDepth(int)));
    m_layout->addWidget(maxDepthSpn,2,1,1,1);



    QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_layout->addItem(spacer,5,0,1,1);
}
//----------------------------------------------------------------------------------------------------------------------
void GenSetDockWidget::installScene(OpenGLWidget *_scene){
    m_scene = _scene;
    //add field for our ray depth during camera movement
    m_layout->addWidget(new QLabel("Camera Movement Ray Depth",this),3,0,1,1);
    QSpinBox *camDepthSpn = new QSpinBox(this);
    camDepthSpn->setValue(2);
    camDepthSpn->setMaximum(INFINITY);
    connect(camDepthSpn,SIGNAL(valueChanged(int)),m_scene,SLOT(setCamMovRayDepth(int)));
    m_layout->addWidget(camDepthSpn,3,1,1,1);
}

//----------------------------------------------------------------------------------------------------------------------
GenSetDockWidget::~GenSetDockWidget(){
    delete m_layout;
    delete m_genSetGroupBox;
}
//----------------------------------------------------------------------------------------------------------------------
