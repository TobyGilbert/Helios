#include "MeshDockWidget.h"
#include <iostream>

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
    m_meshDockGroupBox->setMinimumWidth(310);
    //add our add mesh button
    m_addMeshBtn = new QPushButton("Add Model");
    m_meshDockGridLayout->addWidget(m_addMeshBtn,0,0,1,1);
    //connect our add mesh button to our add mesh slot
    connect(m_addMeshBtn,SIGNAL(clicked()),this,SLOT(addMeshWidget()));
    m_meshDockSpacer = new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    m_meshDockGridLayout->addItem(m_meshDockSpacer, 4, 0, 1, 1);


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
        connect(m_meshDockComboBox,SIGNAL(currentIndexChanged(int)),this, SLOT(changeCurrentWidget(int)));
    }
    MeshWidget *wgt = new MeshWidget(m_meshCount);
    //---------connect our signals
    connect(wgt,SIGNAL(importMesh(int,std::string)),this,SLOT(importModel(int,std::string)));
    connect(wgt,SIGNAL(meshTransform(int,float,float,float,float,float,float,float,float,float)),this,SLOT(meshTransform(int,float,float,float,float,float,float,float,float,float)));
    //add to our map
    m_meshWidgets[m_meshCount] = wgt;
    /// @todo you can use QVarient with custom types, if we do this with our meshWidget is means we dont need our map
    m_meshDockComboBox->addItem(QString("Model %1").arg(m_meshCount),QVariant(m_meshCount));
    m_meshDockComboBox->setCurrentIndex(m_meshDockComboBox->count());
    m_meshCount++;
    if(m_currentShownWidget!=0){
        m_currentShownWidget->hide();
        m_meshDockGridLayout->removeWidget(m_currentShownWidget);
        m_meshDockGridLayout->addWidget(wgt,3,0,1,1);
        wgt->show();
        m_currentShownWidget = wgt;
    }
    else{
        m_meshDockGridLayout->addWidget(wgt,3,0,1,1);
        wgt->show();
        m_currentShownWidget = wgt;
    }
}

//----------------------------------------------------------------------------------------------------------------------
void MeshDockWidget::changeCurrentWidget(int _id){
    if(m_currentShownWidget!=0){
        QVariant widgetId = m_meshDockComboBox->itemData(_id);
        std::map<int,MeshWidget*>::iterator it = m_meshWidgets.find(widgetId.toInt());
        m_currentShownWidget->hide();
        m_meshDockGridLayout->removeWidget(m_currentShownWidget);
        m_currentShownWidget = it->second;
        m_meshDockGridLayout->addWidget(m_currentShownWidget,3,0,1,1);
        m_currentShownWidget->show();
    }
}
//----------------------------------------------------------------------------------------------------------------------
void MeshDockWidget::importModel(int _id, std::string _path){
    signalImportModel(_id,_path);
    std::cout<<"import: "<<_id<<std::endl;
}
//----------------------------------------------------------------------------------------------------------------------
void MeshDockWidget::meshTransform(int _id, float _transX, float _transY, float _transZ, float _rotX, float _rotY, float _rotZ, float _scaleX, float _scaleY, float _scaleZ){
    signalMeshTransform(_id,_transX,_transY,_transZ,_rotX,_rotY,_rotZ,_scaleX,_scaleY,_scaleZ);
}
//----------------------------------------------------------------------------------------------------------------------
