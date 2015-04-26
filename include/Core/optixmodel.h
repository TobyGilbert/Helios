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
    /// @param _path - the path to the geometry we want to import
    //----------------------------------------------------------------------------------------------------------------------
    OptiXModel(std::string _path, Context &_context);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our copy contructor that will create an instance of geomtry in our scene
    /// @brief instancing is great becuase it means we need less data on the gpu!
    /// @param _instance - the model that we wish to create an instance of
    //----------------------------------------------------------------------------------------------------------------------
    OptiXModel(OptiXModel *_instance, Context &_context);
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
    /// @brief set our models material
    /// @param matrial we wish to apply to our model
    //----------------------------------------------------------------------------------------------------------------------
    void setMaterial(Material &_mat);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief an accessor to our model geomtry
    //----------------------------------------------------------------------------------------------------------------------
    inline Geometry getGeometry(){return m_geometry;}
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief an accessor to our model with a transformation applied
    //----------------------------------------------------------------------------------------------------------------------
    inline Transform getGeomAndTrans(){return m_trans;}
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief an accessor to our geomtry instance
    //----------------------------------------------------------------------------------------------------------------------
    inline GeometryInstance getGeometryInstance(){return m_geometryInstance;}
    //----------------------------------------------------------------------------------------------------------------------
protected:
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief loads a mesh from our current assimp scene
    //----------------------------------------------------------------------------------------------------------------------
    void loadMesh(const aiNode *_node, const aiScene *_scene, Context &_context);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief processes an imported mesh
    //----------------------------------------------------------------------------------------------------------------------
    void processMesh(const aiMesh *_mesh, Context &_context);
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief creates our buffers in our OptiX context and sets our data
    //----------------------------------------------------------------------------------------------------------------------
    void createBuffers(Context &_context);
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
    /// @brief our model tranformation
    //----------------------------------------------------------------------------------------------------------------------
    Transform m_trans;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our geomtry group
    //----------------------------------------------------------------------------------------------------------------------
    GeometryGroup m_geometryGroup;
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
    /// @todo something with this. At the moment every primative is set to the first matrial assigned
    //----------------------------------------------------------------------------------------------------------------------
    Buffer m_matIdxBuffer;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our client side vertices
    //----------------------------------------------------------------------------------------------------------------------
    std::vector<glm::vec3> m_vertices;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our client side normals
    //----------------------------------------------------------------------------------------------------------------------
    std::vector<glm::vec3> m_normals;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our client side texture coordinates
    //----------------------------------------------------------------------------------------------------------------------
    std::vector<glm::vec2> m_texCoords;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our client side indicies of some discription. Material I think
    /// @todo not let toby touch my classes, toby what is this member for??
    //----------------------------------------------------------------------------------------------------------------------
    std::vector<glm::vec3> m_indices;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our client side vertex indicies
    //----------------------------------------------------------------------------------------------------------------------
    std::vector <glm::vec3> m_vertIndices;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our client side normal indicies
    //----------------------------------------------------------------------------------------------------------------------
    std::vector <glm::vec3> m_normalIndices;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief our client side texture coordinate indicies
    //----------------------------------------------------------------------------------------------------------------------
    std::vector <glm::vec3> m_texCoordIndices;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief a static member to keep track of how many instances we have so if we remove one we dont delete the buffers
    /// @brief that the other instances are using
    //----------------------------------------------------------------------------------------------------------------------
    static int m_numInstances;
    //----------------------------------------------------------------------------------------------------------------------

};


#endif // OPTIXMODEL_H