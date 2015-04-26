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

AbstractMaterialWidget::AbstractMaterialWidget(QWidget *parent) :
    QWidget(parent)
{
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
    cudaDir = "/Developer/NVIDIA/CUDA-"+version
    optixDir = "/Developer/OptiX"
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
    std::string output = "/ptx/"+file.fileName().toStdString()+".ptx";
    std::cout<<"output "<<output<<std::endl;

    std::string nvccCallString = nvcc+nvccFlags+includePaths+libDirs+libs+" ./"+path+" -o "+output;
    std::cout<<"calling nvcc with: "<<nvccCallString<<std::endl;

    std::string test = "/usr/local/cuda-6.5/bin/nvcc -m64 -gencode arch=compute_50,code=sm_50 --compiler-options -fno-strict-aliasing -use_fast_math --ptxas-options=-v -ptx -I./include -I/opt/local/include -I/usr/local/include -I./include/OSL -I/usr/local/oiio/src/include -I/usr/local/OptiX/SDK/sutil -I/usr/local/OptiX/SDK -I/usr/local/cuda-6.5/include -I/usr/local/cuda-6.5/common/inc/ -I/usr/local/cuda-6.5/../shared/inc/ -I/usr/local/OptiX/include  -lfl -L/opt/local/lib -lIL -L/usr/local/lib -lOpenImageIO -lassimp -lboost_system -L./osl/lib -loslcomp -L/usr/local/lib -lGLEW -lcudart -loptix -loptixu optixSrc/tempMat.cu -o ptx/tempMat.cu.ptx";
    std::cout<<"test version     : "<<test<<std::endl;
    system(nvccCallString.c_str());
    //system(test.c_str());


//# Path to cuda SDK install
//macx:CUDA_DIR = /Developer/NVIDIA/CUDA-6.5
//linux:CUDA_DIR = /usr/local/cuda-6.5
//# Path to cuda toolkit install
//macx:CUDA_SDK = /Developer/NVIDIA/CUDA-6.5/samples
//linux:CUDA_SDK = /usr/local/cuda-6.5/samples

//# include paths
//macx:INCLUDEPATH += /Developer/OptiX/SDK/sutil
//macx:INCLUDEPATH += /Developer/OptiX/SDK
//linux:INCLUDEPATH += /usr/local/OptiX/SDK/sutil
//linux:INCLUDEPATH += /usr/local/OptiX/SDK
//INCLUDEPATH += $$CUDA_DIR/include
//INCLUDEPATH += $$CUDA_DIR/common/inc/
//INCLUDEPATH += $$CUDA_DIR/../shared/inc/
//macx:INCLUDEPATH += /Developer/OptiX/include
//linux:INCLUDEPATH += /usr/local/OptiX/include
//# lib dirs
//#QMAKE_LIBDIR += $$CUDA_DIR/lib64
//macx:QMAKE_LIBDIR += $$CUDA_DIR/lib
//linux:QMAKE_LIBDIR += $$CUDA_DIR/lib64
//QMAKE_LIBDIR += $$CUDA_SDK/common/lib
//macx:QMAKE_LIBDIR += /Developer/OptiX/lib64
//linux:QMAKE_LIBDIR += /usr/local/OptiX/lib64
//LIBS += -lcudart  -loptix -loptixu
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
