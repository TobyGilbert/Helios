#ifndef OSLNODESEDITOR_H
#define OSLNODESEDITOR_H

//------------------------------------------------------------------------------------------------------------------------------------
/// @class OSLNodesEditor
/// @brief an extention to QNodesEditor which adds OSL to Optix customisations
/// @author Declan Russell
/// @date 20/03/2015
//------------------------------------------------------------------------------------------------------------------------------------

#include <QObject>
#include <cuda_runtime.h>
#include <optixu/optixpp_namespace.h>
#include "NodeGraph/OSLShaderBlock.h"
#include "NodeGraph/qnodeseditor.h"
#include "NodeGraph/qneport.h"

class OSLNodesEditor : public QNodesEditor
{
    Q_OBJECT
public:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief default constructor
    //------------------------------------------------------------------------------------------------------------------------------------
    explicit OSLNodesEditor(QObject *parent = 0);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief mutator to the path where our optix material is created
    //------------------------------------------------------------------------------------------------------------------------------------
    inline void setMatDestination(std::string _dest){m_optixMatDest = _dest;}
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief accessor to the path where our optix material is created
    //------------------------------------------------------------------------------------------------------------------------------------
    inline std::string getMatDestination(){return m_optixMatDest;}
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief mutator for our material name
    //------------------------------------------------------------------------------------------------------------------------------------
    inline void setMatName(std::string _name){m_materialName = _name;}
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief accessor to our material name
    /// @return material name
    //------------------------------------------------------------------------------------------------------------------------------------
    inline std::string &getMaterialName(){return m_materialName;}
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief creates an optix material program from our OSL node graph.
    /// @brief The file will default be created in OptixMaterials/tempMat.cu unless specified otherwise
    /// @param _mat - material we wish to compile OSL shader to
    /// @return Material Compiled if compilation succesful otherwise oppropriate error message
    //------------------------------------------------------------------------------------------------------------------------------------
    std::string compileMaterial(optix::Material &_mat);
    //------------------------------------------------------------------------------------------------------------------------------------
signals:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief signal to give the progress of compilation
    /// @param _percent - percent from 0 to 100 of progress of compilation
    //------------------------------------------------------------------------------------------------------------------------------------
    void percentCompiled(int _percent);
    //------------------------------------------------------------------------------------------------------------------------------------
private:
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief a recursive function to to evaluate our blocks to find there order
    /// @param _block - the block we want to evaluate
    /// @param _blockVector - the vector we wish to store our ordered blocks
    //------------------------------------------------------------------------------------------------------------------------------------
    void evaluateBlock(QNEBlock *_block, std::vector<QNEBlock*> &_blockVector);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief iterates through our scenes children to the last block to be written in our material.
    /// @brief this will be the first node found with Ci as an output.
    //------------------------------------------------------------------------------------------------------------------------------------
    QNEBlock *getLastBlock();
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief takes in an QNEPort::type and returns the type in string form
    //------------------------------------------------------------------------------------------------------------------------------------
    QString portTypeToString(QNEPort::variableType _type);
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief the destination that we wish to create our optix material. Defualt is OptixMaterials/tempMat.cu
    //------------------------------------------------------------------------------------------------------------------------------------
    std::string m_optixMatDest;
    //------------------------------------------------------------------------------------------------------------------------------------
    /// @brief the name of our material, this will default to tempMat
    //------------------------------------------------------------------------------------------------------------------------------------
    std::string m_materialName;
    //------------------------------------------------------------------------------------------------------------------------------------


};

#endif // OSLNODESEDITOR_H
