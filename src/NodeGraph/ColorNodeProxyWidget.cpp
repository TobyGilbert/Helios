#include "NodeGraph/ColorNodeProxyWidget.h"
#include <QPushButton>
#include <QGridLayout>
#include <iostream>


ColorNodeProxyWidget::ColorNodeProxyWidget(QNEPort *_portConnected, Material &_mat, QGraphicsItem *parent): AbstractNodeProxyWidget(_portConnected,_mat,parent)
{
    //Create our buttons and color dialog
    m_colBtn  = new QPushButton("Select Color");
    setWidget(m_colBtn);
    m_colBtn->move(-60,-10);
    m_colorDialog = new QColorDialog();
    m_colorDialog->setWindowTitle("Select Color");
    m_colorDialog->hide();
    connect(m_colBtn,SIGNAL(pressed()),m_colorDialog,SLOT(show()));
    connect(m_colorDialog,SIGNAL(currentColorChanged(QColor)),this,SLOT(setMaterialVars(QColor)));
}
//------------------------------------------------------------------------------------------------------------------------------------
void ColorNodeProxyWidget::setMaterialVars(QColor _col){

    //Change the color of our button for some visual feedback for the user
    QString s = "background-color: ";
    m_colBtn->setStyleSheet(s + _col.name());

    //set our new color values
    m_red = static_cast<float>(_col.redF());
    m_green = static_cast<float>(_col.greenF());
    m_blue = static_cast<float>(_col.blueF());

    std::cout<<m_red<<" "<<m_green<<" "<<m_blue<<std::endl;

    //get all our linked variable names
    std::vector<std::string> _varNames;
    getLinkedVarName(_varNames);

    //set all our variables in our material
    for(unsigned int i=0;i<_varNames.size();i++){
//        m_material[_varNames[i].c_str()]->setFloat(_col.redF(),_col.greenF(),_col.blueF());
        m_material[_varNames[i]]->setFloat(m_red,m_green,m_blue);
    }

}
//------------------------------------------------------------------------------------------------------------------------------------
void ColorNodeProxyWidget::setLinkedVar(){
    //get all our linked variable names
    std::vector<std::string> _varNames;
    getLinkedVarName(_varNames);

    //set all our variables in our material
    for(unsigned int i=0;i<_varNames.size();i++){
        m_material[_varNames[i].c_str()]->setFloat(m_red,m_green,m_blue);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------
ColorNodeProxyWidget::~ColorNodeProxyWidget(){
    delete m_colorDialog;
}

//------------------------------------------------------------------------------------------------------------------------------------
