#include "UI/AbstractMaterialWidget.h"
#include "Core/pathtracerscene.h"
#include "NodeGraph/OSLShaderBlock.h"
#include "NodeGraph/OSLVarFloatBlock.h"
#include <QMenu>
#include <QPoint>
#include <QFileDialog>
#include <QFileInfo>
#include <QPushButton>
#include <QMessageBox>
#include <cuda_runtime.h>
#include <iostream>

//declare our static class instance
AbstractMaterialWidget* AbstractMaterialWidget::m_instance;

AbstractMaterialWidget::AbstractMaterialWidget(QWidget *parent) :
    QWidget(parent,Qt::Window)
{
    this->setMinimumHeight(500);
    this->setMinimumWidth(700);
    m_matCreated = false;
    //set our widget layout
    m_widgetLayout = new QGridLayout(this);
    this->setLayout(m_widgetLayout);
    //add our groupbox
    m_widgetGroupBox = new QGroupBox(this);
    m_widgetLayout->addWidget(m_widgetGroupBox,0,0,1,1);
    m_groupBoxLayout = new QGridLayout(this);
    m_widgetGroupBox->setLayout(m_groupBoxLayout);
    //create our graphics view to hold our node interface scene
    m_graphicsView = new QGraphicsView(this);
    //add this widget to our group box
    m_groupBoxLayout->addWidget(m_graphicsView,0,0,1,1);
    //create our node interface scene
    m_nodeInterfaceScene = new QGraphicsScene(this);
    //set our graphics view scene to that of our node interface scene
    m_graphicsView->setScene(m_nodeInterfaceScene);
    //some stuff to make it look nicer
    m_graphicsView->setRenderHint(QPainter::Antialiasing, true);
    //create our node editor, could become useful in the future
    m_nodeEditor = new OSLNodesEditor(this);
    m_nodeEditor->install(m_nodeInterfaceScene);

    //add a button to to launch the creation of our shader
    QPushButton *createShaderBtn = new QPushButton("Create Shader",this);
    m_groupBoxLayout->addWidget(createShaderBtn,0,1,1,1);
    connect(createShaderBtn,SIGNAL(clicked()),this,SLOT(createOptixMaterial()));


    //Set up our menu for if you right click in our widget
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showContextMenu(QPoint)));

    //create a material in our context
    m_material = PathTracerScene::getInstance()->getContext()->createMaterial();
}
//------------------------------------------------------------------------------------------------------------------------------------
AbstractMaterialWidget::~AbstractMaterialWidget(){
    //remove our instance
    delete m_instance;
}

AbstractMaterialWidget* AbstractMaterialWidget::getInstance(QWidget *parent)
{
    if(m_instance){
        if(parent) std::cerr<<"AbstractMaterialWidget already has a parent"<<std::endl;
        return m_instance;
    }
    else{
        m_instance = new AbstractMaterialWidget(parent);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------
void AbstractMaterialWidget::createOptixMaterial(){
    //turn our graph editor into an optix material program
    m_nodeEditor->compileMaterial();
    //get the path to the file
    std::string path = m_nodeEditor->getMatDestination();

    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, 0);
    std::string gencodeFlag = " -gencode arch=compute_"+std::to_string(prop.major)+"0,code=sm_"+std::to_string(prop.major)+"0";
    int v;
    cudaRuntimeGetVersion(&v);
    int vMajor = floor(v/1000);
    int vMinor = (v - (floor(v/1000)*1000))/10;
    std::string version = std::to_string(vMajor)+"."+std::to_string(vMinor);
    std::cout<<"version "<<version<<std::endl;
    std::string cudaDir,optixDir;
#ifdef DARWIN
    cudaDir = "/Developer/NVIDIA/CUDA-"+version;
    optixDir = "/Developer/OptiX";
#else
    cudaDir = "/usr/local/cuda-"+version;
    optixDir = "/usr/local/OptiX";
#endif
    std::string cudaSDKDir;
    cudaSDKDir = cudaDir + "/samples";
    std::string includePaths = " -I"+optixDir+"/SDK"+" -I"+optixDir+"/SDK/sutil"+
                               " -I"+optixDir+"/include"+" -I"+cudaDir+"/include"+
                               " -I"+cudaDir+"/common/inc"+" -I"+cudaDir+"/../shared/inc"+
                               " -I./include";
    std::cout<<"includePaths "<<includePaths<<std::endl;

    std::string libDirs = " -L"+cudaDir+"/lib64"+" -L"+cudaDir+"/lib"+" -L"+cudaSDKDir+"/common/lib"+
                          " -L"+optixDir+"/lib64";
    std::string libs = " -lcudart -loptix -loptixu";
    std::string nvcc = "nvcc ";
    std::string nvccFlags =" -m64"+gencodeFlag+" --compiler-options -fno-strict-aliasing -use_fast_math --ptxas-options=-v -ptx";

    QFileInfo file = QString(path.c_str());
    std::string output = "./ptx/"+file.fileName().toStdString()+".ptx";
    std::cout<<"output "<<output<<std::endl;
    std::string nvccCallString = nvcc+nvccFlags+includePaths+libDirs+libs+" ./"+path+" -o "+output;
    std::cout<<"calling nvcc with: "<<nvccCallString<<std::endl;

    if(system(nvccCallString.c_str())==NULL){
        optix::Context optiXEngine = PathTracerScene::getInstance()->getContext();
        optix::Program closestHitProgram = optiXEngine->createProgramFromPTXFile(output,m_nodeEditor->getMaterialName());
        Program anyHitProgram = optiXEngine->createProgramFromPTXFile( "ptx/path_tracer.cu.ptx", "shadow" );
        m_material->setClosestHitProgram(0,closestHitProgram);
        m_material->setAnyHitProgram(1,anyHitProgram);
        m_material->validate();
        m_matCreated = true;
    }
    else{
        QMessageBox::warning(this,tr("Shader Compilation"),tr("Compilation Failed"));
        m_matCreated = false;
    }
}
//------------------------------------------------------------------------------------------------------------------------------------
void AbstractMaterialWidget::applyMaterialToMesh(std::string _mesh)
{
    if(m_matCreated){
        std::cerr<<"Adding material "<<m_materialName<<" to mesh "<<_mesh<<std::endl;
        PathTracerScene::getInstance()->setModelMaterial(_mesh,m_material);
    }
    else{
        QMessageBox::warning(this,"Add Material","No OSL shader created");
    }
}
//------------------------------------------------------------------------------------------------------------------------------------
void AbstractMaterialWidget::showContextMenu(const QPoint &pos){

    QPoint globalPos = this->mapToGlobal(pos);

    //our drop down menu
    QMenu myMenu;
    QAction *addShaderNodeBtn = new QAction(&myMenu);
    addShaderNodeBtn->setText("Add Shader Node");
    addShaderNodeBtn->setData(QVariant(0));
    myMenu.addAction(addShaderNodeBtn);

    QAction *addFloatNodeBtn = new QAction(&myMenu);
    addFloatNodeBtn->setText("Add Float Node");
    addFloatNodeBtn->setData(QVariant(1));
    myMenu.addAction(addFloatNodeBtn);

    //find out if something has been clicked
    QAction* selectedItem = myMenu.exec(globalPos);
    if(selectedItem){
        switch(selectedItem->data().toInt())
        {
            case(0): addShaderNode(); break;
            case(1): addFloatNode(); break;
            //if nothing do nothing
            default: break;
        }
    }


}

//------------------------------------------------------------------------------------------------------------------------------------
void AbstractMaterialWidget::addShaderNode()
{
    //let the user select a shader to load in
    QString location = QFileDialog::getOpenFileName(0,QString("Import Shader"), QString("shaders/"), QString("OSL files (*.osl)"));
    //if nothing selected then we dont want to do anything
    if(location.isEmpty()) return;

    //create a new shader node in our ui
    OSLShaderBlock *b = new OSLShaderBlock();
    //add it to out interface. This needs to be don before we add any
    //ports or it will not work, should probably do something about this
    m_nodeInterfaceScene->addItem(b);
    if(!b->loadShader(location)){
        QMessageBox::warning(this,"Compile Error","OSL Shader could not be compiled!");
    }
    //add it to our list of nodes
    m_nodes.push_back(b);
}
//------------------------------------------------------------------------------------------------------------------------------------
void AbstractMaterialWidget::addFloatNode()
{
    //create a float block
    OSLVarFloatBlock *b = new OSLVarFloatBlock(m_nodeInterfaceScene, m_material);
    m_nodes.push_back(b);
}
//------------------------------------------------------------------------------------------------------------------------------------
