#include "AbstractMaterialWidget.h"
#include "pathtracerscene.h"

#include "qneblock.h"
#include "qneport.h"

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
    m_nodeInterfaceScene = new QGraphicsScene();
    //set our graphics view scene to that of our node interface scene
    m_graphicsView->setScene(m_nodeInterfaceScene);
    //some stuff to make it look nicer
    m_graphicsView->setRenderHint(QPainter::Antialiasing, true);
    //create our node editor, could become useful in the future
    m_nodeEditor = new QNodesEditor(this);
    m_nodeEditor->install(m_nodeInterfaceScene);

    //lets do some testing
    QNEBlock *b = new QNEBlock(0);
    m_nodeInterfaceScene->addItem(b);
    b->addPort("test", 0, QNEPort::NamePort);
    b->addPort("TestBlock", 0, QNEPort::TypePort);


    //create a material in our context
    m_material = PathTracerScene::getInstance()->getContext()->createMaterial();
}
//------------------------------------------------------------------------------------------------------------------------------------
AbstractMaterialWidget::~AbstractMaterialWidget(){
    for(unsigned int i=0; i<m_widgetObjects.size();i++){
        delete m_widgetObjects[i];
    }
    delete m_graphicsView;
    delete m_nodeInterfaceScene;

    delete m_widgetLayout;
    delete m_widgetGroupBox;
}
//------------------------------------------------------------------------------------------------------------------------------------
