#include "UI/MeshDockWidget.h"
#include <QMessageBox>
#include <iostream>
#include "Core/pathtracerscene.h"
#include "UI/AbstractMaterialWidget.h"

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
    for(std::map<std::string,MeshWidget*>::iterator it = m_meshWidgets.begin(); it!=m_meshWidgets.end();it++){
        delete it->second;
    }
    delete m_meshDockGridLayout;
    delete m_meshDockGroupBox;
}
//----------------------------------------------------------------------------------------------------------------------
void MeshDockWidget::addMeshWidget(){

    //Lets get the location of a mesh that we wish to import
    QString location = QFileDialog::getOpenFileName(this,tr("Import Mesh"), "models/", tr("Mesh Files (*.obj)"));
    //if nothing selected then we dont want to do anything
    if(location.isEmpty()){
        QMessageBox::information(this,tr("Mesh Import"),tr("Nothing Selected"));
        return;
    }
    //Get some file information
    QFileInfo fileInfo(location);

    //if we dont have a combo box lets add one
    if(m_meshDockComboBox==0){
        m_meshSelectLabel = new QLabel("Select Model");
        m_meshDockGridLayout->addWidget(m_meshSelectLabel,1,0,1,1);
        m_meshDockComboBox = new QComboBox();
        m_meshDockGridLayout->addWidget(m_meshDockComboBox,2,0,1,1);
        connect(m_meshDockComboBox,SIGNAL(currentIndexChanged(QString)),this, SLOT(changeCurrentWidget(QString)));
    }

    //create a new mesh widget
    MeshWidget *wgt = new MeshWidget(fileInfo.fileName().toStdString());
    //---------connect our signals
    connect(wgt,SIGNAL(updateScene()),this,SLOT(signalUpdateScene()));
    //add to our map
    //if something of this name already exists then we want to name it something else
    std::map<std::string, MeshWidget*>::iterator it = m_meshWidgets.find(fileInfo.fileName().toStdString());
    int idx = 0;
    std::string finalName = fileInfo.fileName().toStdString();
    while(it!=m_meshWidgets.end()){
        idx++;
        finalName = fileInfo.fileName().toStdString() + std::to_string(idx);
        it = m_meshWidgets.find(finalName);
    }
    m_meshWidgets[finalName] = wgt;
    /// @todo you can use QVarient with custom types, if we do this with our meshWidget is means we dont need our map
    m_meshDockComboBox->addItem(QString(finalName.c_str()),QVariant(m_meshCount));
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
    //finally send a signal to our path tracer to import the model to our scene
    importModel(finalName,location.toStdString());
}

//----------------------------------------------------------------------------------------------------------------------
void MeshDockWidget::changeCurrentWidget(QString _id){
    if(m_currentShownWidget!=0){
        std::map<std::string,MeshWidget*>::iterator it = m_meshWidgets.find(_id.toStdString());
        m_currentShownWidget->hide();
        m_meshDockGridLayout->removeWidget(m_currentShownWidget);
        m_currentShownWidget = it->second;
        m_meshDockGridLayout->addWidget(m_currentShownWidget,3,0,1,1);
        m_currentShownWidget->show();
    }
}
//----------------------------------------------------------------------------------------------------------------------
void MeshDockWidget::importModel(std::string _id, std::string _path){
    PathTracerScene::getInstance()->importMesh(_id,_path);
    std::cout<<"import: "<<_id<<std::endl;
}
