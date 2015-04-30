#include "Lights/LightManager.h"
#include <iostream>

// Declare our static instance of our class
LightManager* LightManager::m_instance;

//----------------------------------------------------------------------------------------------------------------------
LightManager* LightManager::getInstance(QWidget *parent){
    if(!m_instance){
        m_instance = new LightManager(parent);
    }
    else{
        if(parent!=0) std::cerr<<"LightManager already has a parent class"<<std::endl;
    }
    return m_instance;
}
LightManager::LightManager(QWidget *parent) : QDockWidget(parent){

}

LightManager::~LightManager(){
    delete m_instance;
}
