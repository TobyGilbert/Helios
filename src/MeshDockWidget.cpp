#include "MeshDockWidget.h"

MeshDockWidget::MeshDockWidget(QWidget *parent) :
    QDockWidget(parent)
{
    m_meshCount=0;
    m_currentShownWidget = 0;
    m_meshDockComboBox = 0;
    //add our group box to put stuff in
    m_meshDockGroupBox = new QGroupBox();
    this->setWidget(m_meshDockGroupBox);
    //add our grid layout to our groupbox
    m_meshDockGridLayout = new QGridLayout();
    m_meshDockGroupBox->setLayout(m_meshDockGridLayout);
    //add our add mesh button
    m_addMeshBtn = new QPushButton("Add Model");
    m_meshDockGridLayout->addWidget(m_addMeshBtn,0,0,1,1);
    //connect our add mesh button to our add mesh slot
    connect(m_addMeshBtn,SIGNAL(clicked()),this,SLOT(addMeshWidget()));


}
//----------------------------------------------------------------------------------------------------------------------
MeshDockWidget::~MeshDockWidget(){
    delete(m_addMeshBtn);
    delete(m_meshDockComboBox);
    for(std::map<int,MeshWidget*>::iterator it = m_meshWidgets.begin(); it!=m_meshWidgets.end();it++){
        delete it->second;
    }
    delete m_meshDockGridLayout;
    delete m_meshDockGroupBox;
}
//----------------------------------------------------------------------------------------------------------------------
void MeshDockWidget::addMeshWidget(){
    //if we dont have a combo box lets add one
    if(m_meshDockComboBox==0){
        m_meshSelectLabel = new QLabel("Select Model");
        m_meshDockGridLayout->addWidget(m_meshSelectLabel,1,0,1,1);
        m_meshDockComboBox = new QComboBox();
        m_meshDockGridLayout->addWidget(m_meshDockComboBox,2,0,1,1);
    }
    MeshWidget *wgt = new MeshWidget(m_meshCount);
    m_meshWidgets[m_meshCount] = wgt;
    /// @todo you can use QVarient with custom types, if we do this with our meshWidget is means we dont need our map
    m_meshDockComboBox->addItem(QString("Model %1").arg(m_meshCount),QVariant(m_meshCount));
    m_meshDockGridLayout->addWidget(wgt,m_meshCount+3,0,1,1);
    m_meshCount++;
    if(m_currentShownWidget){
        m_currentShownWidget->hide();
        wgt->show();
        m_currentShownWidget = wgt;
    }
    else{
        wgt->show();
        m_currentShownWidget = wgt;
    }
}

//----------------------------------------------------------------------------------------------------------------------
