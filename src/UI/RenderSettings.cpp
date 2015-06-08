#include "UI/RenderSettings.h"
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QGroupBox>
#include "Core/pathtracerscene.h"

RenderSettings::RenderSettings(QWidget *parent) : QWidget(parent,Qt::Window),
                                                  m_imgWidth(512),
                                                  m_imgHeight(512),
                                                  m_numSamples(100),
                                                  m_imgFormat("png")

{
    QGroupBox *gb = new QGroupBox(this);
    QGridLayout *layout = new QGridLayout(gb);
    gb->setLayout(layout);

    //Image width field
    layout->addWidget(new QLabel("Image Width:",this),0,0,1,1);
    QSpinBox *widthSpn = new QSpinBox(this);
    widthSpn->setMaximum(INFINITY);
    widthSpn->setValue(m_imgWidth);
    connect(widthSpn,SIGNAL(valueChanged(int)),this,SLOT(setWidth(int)));
    layout->addWidget(widthSpn,0,1,1,1);

    //Image Height feild
    layout->addWidget(new QLabel("Image Height:",this),1,0,1,1);
    QSpinBox *heightSpn = new QSpinBox(this);
    heightSpn->setMaximum(INFINITY);
    heightSpn->setValue(m_imgHeight);
    connect(heightSpn,SIGNAL(valueChanged(int)),this,SLOT(setHeight(int)));
    layout->addWidget(heightSpn,1,1,1,1);

    //Number of samples field
    layout->addWidget(new QLabel("Number of Samples:",this),2,0,1,1);
    QSpinBox *sampleSpn = new QSpinBox(this);
    sampleSpn->setMaximum(INFINITY);
    sampleSpn->setValue(m_numSamples);
    connect(sampleSpn,SIGNAL(valueChanged(int)),this,SLOT(setNumSamples(int)));
    layout->addWidget(sampleSpn,2,1,1,1);

    QPushButton *renderBtn = new QPushButton("Render Image",this);
    connect(renderBtn,SIGNAL(pressed()),this,SLOT(renderImage()));
    layout->addWidget(renderBtn,3,0,1,2);

    m_progressBar = new QProgressBar(this);
    layout->addWidget(m_progressBar,4,0,1,2);
    m_progressBar->setValue(0);
    m_progressBar->show();

    //add a spacer to make things more organised
    QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    layout->addItem(spacer,children().size(),0,1,1);

}
//----------------------------------------------------------------------------------------------------------------------
void RenderSettings::renderImage()
{
    PathTracerScene *pt = PathTracerScene::getInstance();
    int currentWidth = pt->getWidth();
    int currentHeight = pt->getHeight();
    pt->resize(m_imgWidth,m_imgHeight);
    pt->signalSceneChanged();
    m_progressBar->setRange(0,m_numSamples);
    for(int i=0;i<m_numSamples;i++)
    {
        m_progressBar->setValue(i);
        pt->trace();
    }
    m_progressBar->setValue(0);
    QImage img = pt->saveImage();
    img.save("test",m_imgFormat.toUtf8());
    pt->resize(currentWidth,currentHeight);
    pt->signalSceneChanged();
}

//----------------------------------------------------------------------------------------------------------------------
