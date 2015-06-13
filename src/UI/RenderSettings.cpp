#include "UI/RenderSettings.h"
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QGroupBox>
#include <QFileDialog>
#include "Core/pathtracerscene.h"

RenderSettings::RenderSettings(QWidget *parent) : QWidget(parent,Qt::Window),
                                                  m_imgWidth(512),
                                                  m_imgHeight(512),
                                                  m_numPasses(100),
                                                  m_imgFormat("png")

{
    QGridLayout *layout = new QGridLayout(this);
    this->setLayout(layout);

    //Image width field
    layout->addWidget(new QLabel("Image Width:",this),0,0,1,1);
    QSpinBox *widthSpn = new QSpinBox(this);
    widthSpn->setMaximum(INFINITY);
    widthSpn->setValue(m_imgWidth);
    connect(widthSpn,SIGNAL(valueChanged(int)),this,SLOT(setWidth(int)));
    layout->addWidget(widthSpn,0,1,1,2);

    //Image Height feild
    layout->addWidget(new QLabel("Image Height:",this),1,0,1,1);
    QSpinBox *heightSpn = new QSpinBox(this);
    heightSpn->setMaximum(INFINITY);
    heightSpn->setValue(m_imgHeight);
    connect(heightSpn,SIGNAL(valueChanged(int)),this,SLOT(setHeight(int)));
    layout->addWidget(heightSpn,1,1,1,2);

    //Number of samples field
    layout->addWidget(new QLabel("Number of Passes:",this),2,0,1,1);
    QSpinBox *sampleSpn = new QSpinBox(this);
    sampleSpn->setMaximum(INFINITY);
    sampleSpn->setValue(m_numPasses);
    connect(sampleSpn,SIGNAL(valueChanged(int)),this,SLOT(setNumPasses(int)));
    layout->addWidget(sampleSpn,2,1,1,2);

    //save location
    layout->addWidget(new QLabel("Save Location:",this),3,0,1,1);
    m_saveLocation = new QLineEdit(this);
    m_saveLocation ->setText("untitled");
    layout->addWidget(m_saveLocation,3,1,1,1);
    QPushButton *selSaveLoc = new QPushButton("Select Save location",this);
    connect(selSaveLoc,SIGNAL(pressed()),this,SLOT(getSaveLocation()));
    layout->addWidget(selSaveLoc,3,2,1,1);

    //image format
    layout->addWidget(new QLabel("Image format:",this),4,0,1,1);
    QComboBox *imgFmt = new QComboBox(this);
    imgFmt->addItem("png");
    imgFmt->addItem("jpg");
    imgFmt->addItem("jpeg");
    imgFmt->addItem("bmp");
    imgFmt->addItem("ppm");
    imgFmt->addItem("tiff");
    imgFmt->setCurrentIndex(0);
    connect(imgFmt,SIGNAL(activated(QString)),this,SLOT(setImageFormat(QString)));
    layout->addWidget(imgFmt,4,1,1,2);


    QPushButton *renderBtn = new QPushButton("Render Image",this);
    connect(renderBtn,SIGNAL(pressed()),this,SLOT(renderImage()));
    layout->addWidget(renderBtn,5,0,1,3);

    m_progressBar = new QProgressBar(this);
    layout->addWidget(m_progressBar,6,0,1,3);
    m_progressBar->setValue(0);
    m_progressBar->show();

    //add a spacer to make things more organised
    QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    layout->addItem(spacer,children().size(),0,1,1);

}
//----------------------------------------------------------------------------------------------------------------------
void RenderSettings::setImageFormat(QString _fmt){
    m_imgFormat = _fmt;
}

//----------------------------------------------------------------------------------------------------------------------
void RenderSettings::getSaveLocation()
{
    QString path = QFileDialog::getSaveFileName(this,"Save Location","","*."+m_imgFormat);
    if(path.isEmpty())
    {
        //if nothing selected
        return;
    }
    m_saveLocation->setText(path);
}

//----------------------------------------------------------------------------------------------------------------------
void RenderSettings::renderImage()
{
    PathTracerScene *pt = PathTracerScene::getInstance();
    int currentWidth = pt->getWidth();
    int currentHeight = pt->getHeight();
    pt->resize(m_imgWidth,m_imgHeight);
    pt->signalSceneChanged();
    m_progressBar->setRange(0,m_numPasses);
    for(int i=0;i<m_numPasses;i++)
    {
        m_progressBar->setValue(i);
        pt->trace();
    }
    m_progressBar->setValue(0);
    QImage img = pt->saveImage();
    img.save(m_saveLocation->text()+"."+m_imgFormat,m_imgFormat.toUtf8());
    pt->resize(currentWidth,currentHeight);
    pt->signalSceneChanged();
}

//----------------------------------------------------------------------------------------------------------------------
