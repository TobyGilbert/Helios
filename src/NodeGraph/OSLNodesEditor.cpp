#include "NodeGraph/OSLNodesEditor.h"
#include <iostream>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QDate>
#include <QGraphicsScene>
#include <algorithm>
#include "NodeGraph/qneconnection.h"


//------------------------------------------------------------------------------------------------------------------------------------
OSLNodesEditor::OSLNodesEditor(QObject *parent) :
    QNodesEditor(parent)
{
    //set our material destination
    m_optixMatDest = "OptixMaterials/tempMat.cu";
    m_materialName = "tempMat";

}
//------------------------------------------------------------------------------------------------------------------------------------
QString OSLNodesEditor::portTypeToString(QNEPort::variableType _type){
    switch(_type){
        case(QNEPort::TypeFloat): return QString("float"); break;
        case(QNEPort::TypeInt): return QString("int"); break;
        case(QNEPort::TypeNormal): return QString("float3"); break;
        case(QNEPort::TypeColour): return QString("float3"); break;
        case(QNEPort::TypePoint): return QString("float3"); break;
        case(QNEPort::TypeMatrix): return QString("float*"); break;
        default: std::cerr<<"Unknown Port Type in shader Compilation"<<std::endl; break;
    }
    return "";
}
//------------------------------------------------------------------------------------------------------------------------------------
QNEBlock *OSLNodesEditor::getLastBlock(){
    //iterate through our graphics items in our scene
    foreach(QGraphicsItem *item, getScene()->items())
    {
        //if its a shader block then lets see if its got Ci in it
        if (item->type() == OSLShaderBlock::Type)
        {
            //iterate through all our ports of the block
            QVector<QNEPort*> blockPorts = ((OSLShaderBlock*)item)->ports();
            foreach(QNEPort* port,blockPorts){
                //if the port is an output and its name is Ci then we have a last block
                if(port->isOutput() && (port->getName() == "Ci")){
                    return (QNEBlock*)item;
                }
            }
        }
    }
    //if we get to here then we have not found the last block
    return 0;

}
//------------------------------------------------------------------------------------------------------------------------------------
void OSLNodesEditor::evaluateBlock(QNEBlock *_block, std::vector<QNEBlock *> &_blockVector){
    //check if our block has already been evaluated
    std::vector<QNEBlock*>::iterator it = std::find(_blockVector.begin(),_blockVector.end(),_block);
    if(it != _blockVector.end()){
        return;
    }
    else{
        //if its not been evaluated check all of its imputs to see if they have been evaluated
        QVector<QNEPort*> blockPorts = ((OSLShaderBlock*)_block)->ports();
        foreach(QNEPort* port,blockPorts){
            //if the port is an input check to see if its connected to another shader
            if(!port->isOutput()){
                //get all the connections of our port
                QVector<QNEConnection*> connections = port->connections();
                foreach(QNEConnection* con, connections){
                    //check which port of the connection is the output of the previous node
                    if(con->port1()->isOutput()){
                        evaluateBlock(con->port1()->block(),_blockVector);
                    }
                    else{
                        evaluateBlock(con->port2()->block(),_blockVector);
                    }
                }
            }
        }
        _blockVector.push_back(_block);
        return;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------
void OSLNodesEditor::compileMaterial()
{
    QFileInfo fileInfo(m_optixMatDest.c_str());
    if(!QDir(fileInfo.absoluteDir()).exists()){
        QDir().mkdir(fileInfo.absolutePath());
    }
    QFile myfile(m_optixMatDest.c_str());

    if(myfile.open(QIODevice::WriteOnly)){

        QTextStream stream(&myfile);
        QDate date;
        stream<<"//This file has been automatically generated by Declan Russell's xX_OSL Node Editor Hyper_Xx #NoScopez on the "<<date.currentDate().toString("dd.MM.yyyy")<<endl;

        //add our includes and namespaces
        stream<<"#include <optix.h>"<<endl;
        stream<<"#include <optixu/optixu_math_namespace.h>"<<endl;
        stream<<"#include \"Core/path_tracer.h\""<<endl;
        stream<<"#include \"Core/random.h\""<<endl;
        stream<<"#include \"BRDFUtils.h\""<<endl;
        stream<<"#include \"helpers.h\""<<endl;
        stream<<"using namespace optix;"<<endl;
        stream<<"\n\n";

        //Declare our path tracer variables
        stream<<"rtDeclareVariable(float,         scene_epsilon, , );\n"<<endl;
        stream<<"rtDeclareVariable(rtObject,      top_object, , );\n"<<endl;
        stream<<"rtBuffer<ParallelogramLight>     lights;\n"<<endl;
        stream<<"rtDeclareVariable(unsigned int,  pathtrace_shadow_ray_type, , );\n"<<endl;
        stream<<"// Camera Variables\n"<<endl;
        stream<<"rtDeclareVariable(float3,        eye, , );"<<endl;
        stream<<"// Geometry Variables "<<endl;
        stream<<"rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); "<<endl;
        stream<<"rtDeclareVariable(float3, shading_normal,   attribute shading_normal, );"<<endl;
        stream<<"rtDeclareVariable(float3, texcoord, attribute teQTextStream in(&inputFile);xcoord, );"<<endl;
        stream<<"// Our current ray and payload variables"<<endl;
        stream<<"rtDeclareVariable(optix::Ray, ray,          rtCurrentRay, );"<<endl;
        stream<<"rtDeclareVariable(float,      t_hit,        rtIntersectionDistance, );"<<endl;
        stream<<"rtDeclareVariable(PerRayData_pathtrace, current_prd, rtPayload, );"<<endl;
        stream<<"\n\n";

        stream<<"//Dynamic Input Variables"<<endl;
        //run through all our connections to find if we have any input variables to declare
        foreach(QGraphicsItem *item, getScene()->items()){
            if (item->type() == QNEConnection::Type)
            {

                //check to see which way round our connection is
                if(((QNEConnection*) item)->port1()->isOutput()){
                    //Convert our input type into a string
                    QString type = portTypeToString(((QNEConnection*) item)->port1()->getVaribleType());
                    //if we have a type and our port belongs to a node that is not a shader node
                    //lets add it to our material variables
                    if((type.size()>0)&&(((QNEConnection*) item)->port1()->block()->type() != OSLShaderBlock::Type)){
                        //node: if multiple shaders have the same variable name then they will clash in the program
                        //to solve this we put the shader name on the start of the variable
                        OSLShaderBlock *block = (OSLShaderBlock*) (((QNEConnection*) item)->port2()->block());
                        stream<<"rtDeclareVariable("<<type<<","<<block->getShaderName().c_str()<<((QNEConnection*) item)->port2()->getName()<<",,);"<<endl;
                    }
                }
                else{
                    //Convert our input type into a string
                    QString type = portTypeToString(((QNEConnection*) item)->port2()->getVaribleType());
                    //if we have a type and our port belongs to a node that is not a shader node
                    //lets add it to our material variables
                    if((type.size()>0)&&(((QNEConnection*) item)->port2()->block()->type() != OSLShaderBlock::Type)){
                        //node: if multiple shaders have the same variable name then they will clash in the program
                        //to solve this we put the shader name on the start of the variable
                        OSLShaderBlock *block = (OSLShaderBlock*) (((QNEConnection*) item)->port1()->block());
                        stream<<"rtDeclareVariable("<<type<<","<<block->getShaderName().c_str()<<((QNEConnection*) item)->port1()->getName()<<",,);"<<endl;
                    }
                }
            }
        }
        stream<<"\n\n";

        //our BRDF functions
        QFile brdfsLib("brdfs/brdfs.txt");
        if(brdfsLib.open(QIODevice::ReadOnly)){
            QTextStream in(&brdfsLib);
            while(!in.atEnd()){
                stream<<in.readLine()<<endl;
            }
            brdfsLib.close();
        }


        //add all the device functions that we need in our material program
        stream<<"//Our OSL device functions"<<endl;
        foreach(QGraphicsItem *item, getScene()->items()){
            if (item->type() == OSLShaderBlock::Type)
            {
                stream<<((OSLShaderBlock*)item)->getDevicefunction().c_str()<<endl;
            }
        }
        stream<<"\n\n";


        //add our main material program funcion
        stream<<"//-------Main Material Program-----------"<<endl;
        stream<<"RT_PROGRAM void "<<m_materialName.c_str()<<"(){"<<endl;


        //set up our shader globals, These are the equivilent to globals about our surface in OSL
        stream<<"ShaderGlobals sg;\n"<<endl;
        stream<<"// Calcualte the shading and geometric normals for use with our OSL shaders\n"<<endl;
        stream<<"sg.N = normalize( rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));\n"<<endl;
        stream<<"sg.Ng = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );\n"<<endl;
        stream<<"sg.I = ray.direction;\n"<<endl;
        stream<<"// The shading position\n"<<endl;
        stream<<"sg.P = ray.origin + t_hit * ray.direction;\n"<<endl;
        stream<<"// Texture coordinates\n"<<endl;
        stream<<"sg.u = texcoord.x;\n"<<endl;
        stream<<"sg.v = texcoord.y;\n"<<endl;


        //Retrieve and print out any variables that we need for our kernal functions
        foreach(QGraphicsItem *item, getScene()->items()){
            if (item->type() == OSLShaderBlock::Type)
            {
                //get the ports of our block
                QVector<QNEPort*> ports= ((OSLShaderBlock*)item)->ports();
                foreach(QNEPort* p, ports){
                    if(p->isOutput()){
                        //if the output ports of our hsader block is connected to
                        //another shader block then we need it as a variable we can
                        //pass data around in our kernals
                        if(p->connections().size()>0){
                            stream<<portTypeToString(p->getVaribleType())<<" ";
                            //note: if 2 shaders have the same variable name we need to
                            //distinguish between them so to solve this we will have the
                            //name of the shader concatinated with the varibale name
                            stream<<((OSLShaderBlock*)item)->getShaderName().c_str()<<p->getName();
                            //lets also set its default value
                            stream<<" = "<<p->getInitParams()<<";"<<endl;
                        }
                    }
                }
            }
        }

        stream<<"\n\n";

        //get our last shader block;
        QNEBlock *lastBlock = getLastBlock();
        //check that we have found a last block
        if(!lastBlock){
            std::cerr<<"Cannot find end block"<<std::endl;
            return;
        }
        //evaluate our blocks so we know the order to call our
        //device functions
        std::vector<QNEBlock*> orderedBlocks;
        evaluateBlock(lastBlock,orderedBlocks);

        //write our code
        for(unsigned int i=0;i<orderedBlocks.size();i++){
            if(orderedBlocks[i]->type() == OSLShaderBlock::Type){
                //note the shader name is always the same as the device function we need to call
                stream<<((OSLShaderBlock*)orderedBlocks[i])->getShaderName().c_str()<<"(";
                //put in our shader globals
                stream<<"sg,";
                //now lets print out our the paramiters we need in the function
                QVector<QNEPort*> ports = ((OSLShaderBlock*)orderedBlocks[i])->ports();
                for(int i=0; i<ports.size(); i++){
                    QNEPort* p = ports[i];

                    //if our port is not a variable then lets skip
                    if(p->getVaribleType() == QNEPort::TypeVoid){
                        continue;
                    }

                    if(p->connections().size()==0){
                        //if there is nothing connected we just stick in our default paramiter
                        stream<<p->getInitParams();
                        if(i<(ports.size()-2)){
                            stream<<",";
                        }
                    }
                    else if(!p->isOutput()){
                        QVector<QNEConnection*> con = p->connections();
                        if(con.size()>1){
                            std::cerr<<"Error: Input to shader has multiple input connections"<<std::endl;
                            return;
                        }
                        else{
                            //find which port of the connection is the input and print the variable name
                            if(!con[0]->port1()->isOutput()){
                                OSLShaderBlock* b = (OSLShaderBlock*)con[0]->port1()->block();
                                stream<<b->getShaderName().c_str()<<con[0]->port1()->getName();
                            }
                            else{
                                OSLShaderBlock* b = (OSLShaderBlock*)con[0]->port2()->block();
                                stream<<b->getShaderName().c_str()<<con[0]->port2()->getName();
                            }
                            if(i<(ports.size()-2)){
                                stream<<",";
                            }
                        }
                    }

                }

                //finish our kernal call
                stream<<");"<<endl;
            }
        }






        //calculate our shadows
        stream<<"// Compute our shadows\n"<<endl;
        stream<<"unsigned int num_lights = lights.size();\n"<<endl;
        stream<<"float3 result = make_float3(0.0f);\n"<<endl;

        stream<<"for(int i = 0; i < num_lights; ++i) {\n"<<endl;
        stream<<"   ParallelogramLight light = lights[i];\n"<<endl;
        stream<<"   float z1 = rnd(current_prd.seed);\n"<<endl;
        stream<<"   float z2 = rnd(current_prd.seed);\n"<<endl;
        stream<<"   float3 light_pos = light.corner + light.v1 * z1 + light.v2 * z2; \n"<<endl;
        stream<<"    float Ldist = length(light_pos - sg.P);\n"<<endl;
        stream<<"    float3 L = normalize(light_pos - sg.P);\n"<<endl;
        stream<<"    float nDl = dot( sg.N, L );\n"<<endl;
        stream<<"    float LnDl = dot( light.normal, L );\n"<<endl;
        stream<<"    float A = length(cross(light.v1, light.v2));\n"<<endl;
        stream<<"    // cast shadow ray\n"<<endl;
        stream<<"    if ( nDl > 0.0f && LnDl > 0.0f ) {\n"<<endl;
        stream<<"       PerRayData_pathtrace_shadow shadow_prd;\n"<<endl;
        stream<<"        shadow_prd.inShadow = false;\n"<<endl;
        stream<<"        shadow_prd.type = shadowRay;\n"<<endl;
        stream<<"        Ray shadow_ray = make_Ray( sg.P, L, pathtrace_shadow_ray_type, scene_epsilon, Ldist );\n"<<endl;
        stream<<"        rtTrace(top_object, shadow_ray, shadow_prd);\n"<<endl;

        stream<<"        if(!shadow_prd.inShadow){\n"<<endl;
        stream<<"            float weight=nDl * LnDl * A / (M_PIf*Ldist*Ldist);\n"<<endl;
        stream<<"            result += light.emission * weight;\n"<<endl;
        stream<<"        }\n"<<endl;
        stream<<"    }\n"<<endl;
        stream<<"}\n"<<endl;
        stream<<"current_prd.radiance = result;\n"<<endl;


        //end of our material program
        stream<<"}"<<endl;





    }
    else{
        std::cerr<<"Cannot create material file"<<std::endl;
    }

}
//------------------------------------------------------------------------------------------------------------------------------------