#ifndef OPTIXMODEL_H
#define OPTIXMODEL_H

/// @class OptiXModel
/// @author Declan Russell & Toby Gilbert
/// @date 28/01/2014
/// @brief This is a class to import models ready to be used with the OptiX ray tracing engine
/// @todo do something with the material buffer, atm it all just defaults to 0

#include <optixu/optixpp_namespace.h>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

using namespace optix;
class OptiXMesh;
class OptiXModel
{
public:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our default constructor, doesnt really do anything apart from init our members
    //----------------------------------------------------------------------------------------------------------------------
    OptiXModel(Context &_context);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our destructor
    //----------------------------------------------------------------------------------------------------------------------
    ~OptiXModel();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief creates our geomtry
    /// @param _loc - the location of the mesh we wish to import
    /// @param _context - a reference to the instance of our OptiX Engine
    //----------------------------------------------------------------------------------------------------------------------
    void createGeometry(std::string _loc, Context &_context);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief a mutator for our transformation matrix for our geomtry
    /// @brief this function converts glm matricies to OptiX compatible array of floats
    /// @param _trans our translation matrix
    /// @param _transpose - is our matrix transposed
    //----------------------------------------------------------------------------------------------------------------------
    void setTrans(glm::mat4 _trans, bool _transpose = false);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief a mutator for our transformation matrix for our geomtry
    /// @param _trans - our translation matrix
    /// @param _transpose - is our matrix transposed
    /// @param _invM - the inverse of our matrix, default not required.
    //----------------------------------------------------------------------------------------------------------------------
    void setTrans(float *_m, bool _transpose = false, float *_invM = 0);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief set our material list
    /// @param vector of our materials
    //----------------------------------------------------------------------------------------------------------------------
    inline void setMatrialList(std::vector<Material> _matList){m_materials = _matList;}
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief add to our material list
    /// @param matrial to add to list
    //----------------------------------------------------------------------------------------------------------------------
    inline void addMaterial(Material &_mat){m_materials.push_back(_mat);}
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief an accessor to our model with a transformation applied
    //----------------------------------------------------------------------------------------------------------------------
    inline Transform getGeomAndTrans(){return m_trans;}
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief an accessor to our geomtry instance
    //----------------------------------------------------------------------------------------------------------------------
    inline GeometryInstance getGeometryInstance(){return m_geometryInstance;}
    //----------------------------------------------------------------------------------------------------------------------




    void loadMesh(const aiNode *_node, const aiScene *_scene, Context &_context);
    void processMesh(const aiMesh *_mesh, Context &_context);
    void createBuffers(Context &_context);

protected:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief create a default defuse material
    //----------------------------------------------------------------------------------------------------------------------
    Material createDefaultMat(Context &_context);
    //----------------------------------------------------------------------------------------------------------------------
private:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our OptiX geometry instance
    //----------------------------------------------------------------------------------------------------------------------
    GeometryInstance m_geometryInstance;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our OptiX geomtry
    //----------------------------------------------------------------------------------------------------------------------
    Geometry m_geometry;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our list of materials applied to our geomtry
    //----------------------------------------------------------------------------------------------------------------------
    std::vector<Material> m_materials;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our model tranformation
    //----------------------------------------------------------------------------------------------------------------------
    Transform m_trans;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our vertex buffer
    //----------------------------------------------------------------------------------------------------------------------
    Buffer m_vertexBuffer;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our normals buffer
    //----------------------------------------------------------------------------------------------------------------------
    Buffer m_normalBuffer;
    //----------------------------------------------------------------------------------------------------------------------
    /// @briefour texture coordinates buffer
    //----------------------------------------------------------------------------------------------------------------------
    Buffer m_texCoordsBuffer;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our vertex index buffer
    //----------------------------------------------------------------------------------------------------------------------
    Buffer m_vertIdxBuffer;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our normals index buffer
    //----------------------------------------------------------------------------------------------------------------------
    Buffer m_normIdxBuffer;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our textures coordinates index buffer
    //----------------------------------------------------------------------------------------------------------------------
    Buffer m_texCoordIdxBuffer;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our material index buffer
    //----------------------------------------------------------------------------------------------------------------------
    Buffer m_matIdxBuffer;
    //----------------------------------------------------------------------------------------------------------------------
    std::vector<OptiXMesh*> m_meshes;

    std::vector<glm::vec3> m_vertices;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_texCoords;
    std::vector<glm::vec3> m_indices;
    std::vector <glm::vec3> m_vertIndices;
    std::vector <glm::vec3> m_normalIndices;
    std::vector <glm::vec3> m_texCoordIndices;
};


#endif // OPTIXMODEL_H
