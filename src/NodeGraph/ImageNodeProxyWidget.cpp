#include "NodeGraph/ImageNodeProxyWidget.h"
#include "Core/TextureLoader.h"
#include "Core/pathtracerscene.h"
#include <QFileDialog>
#include <QPushButton>
#include <QGridLayout>
#include <iostream>

//------------------------------------------------------------------------------------------------------------------------------------
ImageNodeProxyWidget::ImageNodeProxyWidget(QNEPort *_portConnected, Material &_mat, QGraphicsItem *parent) :
    AbstractNodeProxyWidget(_portConnected,_mat,parent)
{
    m_groupBox = new QGroupBox();
    m_groupBox->move(-120,-120);
    m_groupBox->setMaximumHeight(255);
    m_groupBox->setMaximumWidth(230);
    this->setWidget(m_groupBox);
    QGridLayout *layout = new QGridLayout(m_groupBox);
    m_groupBox->setLayout(layout);
    QPushButton *imprtImgbtn = new QPushButton("Import Image",m_groupBox);
    connect(imprtImgbtn,SIGNAL(pressed()),this,SLOT(getImage()));
    layout->addWidget(imprtImgbtn,0,0,1,1);
    m_imgLabel = new QLabel(m_groupBox);
    layout->addWidget(m_imgLabel,1,0,1,1);
    m_imgLabel->setMaximumHeight(255);
    m_imgLabel->setMaximumWidth(230);
    m_imgCreated = false;

}
//------------------------------------------------------------------------------------------------------------------------------------
void ImageNodeProxyWidget::getImage()
{
    QString location = QFileDialog::getOpenFileName(0,QString("Import Texture"), QString("textures/"));
    //if nothing selected then we dont want to do anything
    if(location.isEmpty()) return;
    QPixmap img(location);
    m_imgLabel->setPixmap(img.scaled(m_imgLabel->width(),m_imgLabel->height(),Qt::KeepAspectRatio));
    m_imagePath = location;
    if(m_imgCreated)m_texure->destroy();
    m_texure = loadTexture(PathTracerScene::getInstance()->getContext(),location.toStdString());
    m_imgCreated = true;

    //set our linked variables
    //get all our linked variable names
    std::vector<std::string> _varNames;
    getLinkedVarName(_varNames);

    //set all our variables in our material
    for(unsigned int i=0;i<_varNames.size();i++){
        m_material[_varNames[i].c_str()]->setTextureSampler(m_texure);
    }
    //attributeChanged();
}
//------------------------------------------------------------------------------------------------------------------------------------
void ImageNodeProxyWidget::setLinkedVar(){
    if(!m_imgCreated) return;
    //set our linked variables
    //get all our linked variable names
    std::vector<std::string> _varNames;
    getLinkedVarName(_varNames);

    //set all our variables in our material
    for(unsigned int i=0;i<_varNames.size();i++){
        m_material[_varNames[i].c_str()]->setTextureSampler(m_texure);
    }
}
//------------------------------------------------------------------------------------------------------------------------------------
void ImageNodeProxyWidget::loadImage(QString _path)
{
    QPixmap img(_path);
    if(!img.isNull())
    {
        m_imgLabel->setPixmap(img.scaled(200,255,Qt::KeepAspectRatio));

        if(m_imgCreated)m_texure->destroy();
        m_texure = loadTexture(PathTracerScene::getInstance()->getContext(),_path.toStdString());
        m_imgCreated = true;
        m_imagePath = _path;
        //set our linked variables
        //get all our linked variable names
        std::vector<std::string> _varNames;
        getLinkedVarName(_varNames);

        //set all our variables in our material
        for(unsigned int i=0;i<_varNames.size();i++)
        {
            m_material[_varNames[i].c_str()]->setTextureSampler(m_texure);
        }
    }
    else
    {
        std::cerr<<"Could not load image "<<_path.toStdString()<<std::endl;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------
ImageNodeProxyWidget::~ImageNodeProxyWidget(){
}


