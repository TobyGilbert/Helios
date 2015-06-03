#include "Core/OptixModel.h"
#include "Core/pathtracerscene.h"

#include <optixu/optixu.h>

#include <iostream>
#include <sstream>

// Declare our static variable
std::map<std::string,int> OptiXModel::m_instanceCount;

//----------------------------------------------------------------------------------------------------------------------
OptiXModel::OptiXModel(std::string _path)
{
    std::cerr<<"Importing model "<<_path<<std::endl;
    std::stringstream id;
    m_instanceCount[id.str()] = 0;
    m_instanceId = id.str();
    // identity matrix to init our transformation
    float m[16];
    m[ 0] = 1.0f;  m[ 1] = 0.0f;  m[ 2] = 0.0f;  m[ 3] = 0.0f;
    m[ 4] = 0.0f;  m[ 5] = 1.0f;  m[ 6] = 0.0f;  m[ 7] = 0.0f;
    m[ 8] = 0.0f;  m[ 9] = 0.0f;  m[10] = 1.0f;  m[11] = 0.0f;
    m[12] = 0.0f;  m[13] = 0.0f;  m[14] = 0.0f;  m[15] = 1.0f;
    //init our trans
    m_trans = PathTracerScene::getInstance()->getContext()->createTransform();
    m_trans->setMatrix(false,m,0);
    createGeometry(_path);
}
//----------------------------------------------------------------------------------------------------------------------
OptiXModel::OptiXModel(OptiXModel *_instance){

    //get our model Id
    m_instanceId = _instance->m_instanceId;
    //increment the instance count
    std::map<std::string,int>::iterator inst = m_instanceCount.find(m_instanceId);
    inst->second++;
    // create our instance of our geometry
    m_geometry = _instance->m_geometry;
    m_geometryInstance = PathTracerScene::getInstance()->getContext()->createGeometryInstance();
    m_geometryInstance->setGeometry(m_geometry);
    Material mat = PathTracerScene::getInstance()->getContext()->createMaterial();
    mat->setClosestHitProgram(0,_instance->m_geometryInstance->getMaterial(0)->getClosestHitProgram(0));
    mat->setAnyHitProgram(1,_instance->m_geometryInstance->getMaterial(0)->getAnyHitProgram(1));
    setMaterial(mat);
    // create a new transform as we dont want this to be the same as our other model
    m_trans = PathTracerScene::getInstance()->getContext()->createTransform();
    // identity matrix to init our transformation
    float m[16];
    m[ 0] = 1.0f;  m[ 1] = 0.0f;  m[ 2] = 0.0f;  m[ 3] = 0.0f;
    m[ 4] = 0.0f;  m[ 5] = 1.0f;  m[ 6] = 0.0f;  m[ 7] = 0.0f;
    m[ 8] = 0.0f;  m[ 9] = 0.0f;  m[10] = 1.0f;  m[11] = 0.0f;
    m[12] = 0.0f;  m[13] = 0.0f;  m[14] = 0.0f;  m[15] = 1.0f;
    m_trans->setMatrix(false,m,0);
    // add our instance to a geomtry group so we can add it to our transform
    GeometryGroup geoGroup = PathTracerScene::getInstance()->getContext()->createGeometryGroup();
    geoGroup->setChildCount(1);
    //stick our instance in our group
    geoGroup->setChild(0,m_geometryInstance);
    //create our acceleration method, in this case none becuase we only have one peice of geomtry
    //set this acceleration in our geometry group
    geoGroup->setAcceleration(PathTracerScene::getInstance()->getContext()->createAcceleration("Sbvh","Bvh"));
    //make a acceleration dirty
    geoGroup->getAcceleration()->markDirty();
    m_trans->setChild(geoGroup);

    // now lets just copy everything else across like a normal copy contructor
    // all our buffers will be the same
    m_vertexBuffer = _instance->m_vertexBuffer;
    m_normalBuffer = _instance->m_normalBuffer;
    m_texCoordsBuffer = _instance->m_texCoordsBuffer;
    m_tangentsBuffer = _instance->m_tangentsBuffer;
    m_bitangentsBuffer = _instance->m_bitangentsBuffer;
    m_vertIdxBuffer = _instance->m_vertIdxBuffer;
    m_normIdxBuffer = _instance->m_normIdxBuffer;
    m_texCoordIdxBuffer = _instance->m_texCoordIdxBuffer;
    m_tangentsIdxBuffer = _instance->m_tangentsIdxBuffer;
    m_bitangentsIdxBuffer = _instance->m_bitangentsIdxBuffer;
    m_matIdxBuffer = _instance->m_matIdxBuffer;
    m_vertices = _instance->m_vertices;
    m_normals = _instance->m_normals;
    m_texCoords = _instance->m_texCoords;
    m_tangents = _instance->m_tangents;
    m_bitangents = _instance->m_bitangents;
//    m_indices = _instance->m_indices;
//    m_vertIndices = _instance->m_vertIndices;
//    m_normalIndices = _instance->m_normalIndices;
//    m_texCoordIndices = _instance->m_texCoordIndices;
//    m_tangentIndices = _instance->m_tangentIndices;
//    m_bitangentIndices = _instance->m_bitangentIndices;
}
//----------------------------------------------------------------------------------------------------------------------
OptiXModel::~OptiXModel(){
    std::map<std::string,int>::iterator inst = m_instanceCount.find(m_instanceId);
    if(inst->second==0){
        m_vertexBuffer->destroy();
        m_normalBuffer->destroy();
        m_texCoordsBuffer->destroy();
        m_tangentsBuffer->destroy();
        m_bitangentsBuffer->destroy();
        m_vertIdxBuffer->destroy();
        m_normIdxBuffer->destroy();
        m_texCoordIdxBuffer->destroy();
        m_tangentsIdxBuffer->destroy();
        m_bitangentsIdxBuffer->destroy();
        m_matIdxBuffer->destroy();
        m_trans->destroy();
        m_instanceCount.erase(inst);
    }
    else{
        inst->second--;
    }
}
//----------------------------------------------------------------------------------------------------------------------
void OptiXModel::createGeometry(std::string _loc){
    //import our mesh
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(_loc.c_str(), aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_Triangulate);
    if(scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cerr<<"The file was not successfully opened: "<<_loc.c_str()<<std::endl;
        return;
    }
    loadMesh(scene->mRootNode, scene);
}
//----------------------------------------------------------------------------------------------------------------------
void OptiXModel::loadMesh(const aiNode* _node, const aiScene *_scene){
    for (unsigned int i=0; i<_node->mNumMeshes; i++){
        aiMesh *mesh = _scene->mMeshes[_node->mMeshes[i]];
        processMesh(mesh);
    }

    for (unsigned int i=0; i<_node->mNumChildren; i++){
        loadMesh(_node->mChildren[i], _scene);
    }

    createBuffers();
}
//----------------------------------------------------------------------------------------------------------------------
void OptiXModel::processMesh(const aiMesh *_mesh){

    for(unsigned int i = 0; i <_mesh->mNumVertices; i++)
    {
        glm::vec3 tempVec;

        // position
        tempVec.x = _mesh->mVertices[i].x;
        tempVec.y = _mesh->mVertices[i].y;
        tempVec.z = _mesh->mVertices[i].z;

        m_vertices.push_back(tempVec);

        if(!_mesh->HasNormals()) std::cerr<<"WARNING:  Your mesh has no normals, this could end badly!"<<std::endl;
        // normals
        tempVec.x = _mesh->mNormals[i].x;
        tempVec.y = _mesh->mNormals[i].y;
        tempVec.z = _mesh->mNormals[i].z;

        m_normals.push_back(tempVec);

        // UV
        if(_mesh->HasTextureCoords(0)){
            tempVec.x = _mesh->mTextureCoords[0][i].x;
            tempVec.y = _mesh->mTextureCoords[0][i].y;
        }
        else{
            tempVec.x = tempVec.y = 0.0;
        }

        m_texCoords.push_back(glm::vec2(tempVec.x, tempVec.y));

        // tangent
        if(_mesh->HasTangentsAndBitangents()){
            tempVec.x = _mesh->mTangents[i].x;
            tempVec.y = _mesh->mTangents[i].y;
            tempVec.z = _mesh->mTangents[i].z;
            m_tangents.push_back(tempVec);

            // bitangent
            tempVec.x = _mesh->mBitangents[i].x;
            tempVec.y = _mesh->mBitangents[i].y;
            tempVec.z = _mesh->mBitangents[i].z;

            m_bitangents.push_back(tempVec);
        }
    }

    //lets fill up our idx buffers
//    typedef struct { int x; int y; int z;} ixyz;
//    unsigned int i;
//    for(i=0; i<_mesh->mNumFaces;i++){
//        m_vertIndices.push_back(glm::vec3(i*3, i*3+1, i*3+2));
//    }

//    for(i=0; i<_mesh->mNumFaces;i++){
//        m_normalIndices.push_back(glm::vec3(i*3, i*3+1, i*3+2));
//    }

//    for(i=0; i<_mesh->mNumFaces;i++){
//        m_texCoordIndices.push_back(glm::vec3(i*3, i*3+1, i*3+2));
//    }
//    for(i=0; i<_mesh->mNumFaces; i++){
//        m_tangentIndices.push_back(glm::vec3(i*3, i*3+1, i*3+2));
//    }
//    for(i=0; i<_mesh->mNumFaces; i++){
//        m_bitangentIndices.push_back(glm::vec3(i*3, i*3+1, i*3+2));
//    }
}
//----------------------------------------------------------------------------------------------------------------------
void OptiXModel::createBuffers(){

    std::cout<<"Num verts in mesh "<<m_vertices.size()/3<<std::endl;
    // Create vertex, normal, and texture_coordinate buffers
    m_vertexBuffer = PathTracerScene::getInstance()->getContext()->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, m_vertices.size() );
    m_normalBuffer = PathTracerScene::getInstance()->getContext()->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, m_vertices.size() );
    m_texCoordsBuffer = PathTracerScene::getInstance()->getContext()->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT2, m_vertices.size());
    m_tangentsBuffer = PathTracerScene::getInstance()->getContext()->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, m_tangents.size());
    m_bitangentsBuffer = PathTracerScene::getInstance()->getContext()->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, m_vertices.size());
    //now lets write our information to our buffers
    void *vertPtr = m_vertexBuffer->map();
    typedef struct { float x; float y; float z;} xyz;
    xyz* vertData = (xyz*)vertPtr;
    unsigned int i;
    for(i=0; i<m_vertices.size();i++){
        vertData[i].x = m_vertices[i].x;
        vertData[i].y = m_vertices[i].y;
        vertData[i].z = m_vertices[i].z;
    }
    m_vertexBuffer->unmap();

    vertPtr = m_normalBuffer->map();
    vertData = (xyz*)vertPtr;
    for(i=0; i<m_vertices.size();i++){
        vertData[i].x = m_normals[i].x;
        vertData[i].y = m_normals[i].y;
        vertData[i].z = m_normals[i].z;
    }
    m_normalBuffer->unmap();

    void *texPtr = m_texCoordsBuffer->map();
    typedef struct {float x; float y;} xy;
    xy* texData = (xy*)texPtr;

    for(i=0; i<m_vertices.size();i++){
        if (m_texCoords.size() > 0.0){
            texData[i].x = m_texCoords[i].x;
            texData[i].y = m_texCoords[i].y;
        }
        else{
            texData[i].x = texData[i].y = 0.0;
        }
    }
    m_texCoordsBuffer->unmap();

    void *tangPtr = m_tangentsBuffer->map();
    xyz* tangData = (xyz*)tangPtr;
    for(i=0; i<m_tangents.size(); i++){
        tangData[i].x = m_tangents[i].x;
        tangData[i].y = m_tangents[i].y;
        tangData[i].z = m_tangents[i].z;
    }
    m_tangentsBuffer->unmap();

    void *bitangPtr = m_bitangentsBuffer->map();
    xyz* bitangData = (xyz*)bitangPtr;
    for(i=0; i<m_bitangents.size(); i++){
        bitangData[i].x = m_bitangents[i].x;
        bitangData[i].y = m_bitangents[i].y;
        bitangData[i].z = m_bitangents[i].z;
    }
    m_bitangentsBuffer->unmap();

    //now lets set up our index buffers
    m_vertIdxBuffer = PathTracerScene::getInstance()->getContext()->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT3, m_vertices.size()/3 );
    m_normIdxBuffer = PathTracerScene::getInstance()->getContext()->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT3, m_vertices.size()/3 );
    m_texCoordIdxBuffer = PathTracerScene::getInstance()->getContext()->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT3, m_vertices.size()/3 );
    m_tangentsIdxBuffer = PathTracerScene::getInstance()->getContext()->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT3, m_vertices.size()/3 );
    m_bitangentsIdxBuffer = PathTracerScene::getInstance()->getContext()->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT3, m_vertices.size()/3 );
    m_matIdxBuffer = PathTracerScene::getInstance()->getContext()->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_INT, m_vertices.size()/3 );

    //lets fill up our idx buffers
    void *idxPtr = m_vertIdxBuffer->map();
    typedef struct { int x; int y; int z;} ixyz;
    ixyz* idxData = (ixyz*)idxPtr;
    for(i=0; i<m_vertices.size()/3;i++){
        idxData[i].x = i*3;
        idxData[i].y = i*3+1;
        idxData[i].z = i*3+2;
    }
    m_vertIdxBuffer->unmap();

    idxPtr = m_normIdxBuffer->map();
    idxData = (ixyz*)idxPtr;
    for(i=0; i<m_vertices.size()/3;i++){
        idxData[i].x = i*3;
        idxData[i].y = i*3+1;
        idxData[i].z = i*3+2;
    }
    m_normIdxBuffer->unmap();

    idxPtr = m_texCoordIdxBuffer->map();
    idxData = (ixyz*)idxPtr;
    for(i=0; i<m_vertices.size()/3;i++){
        idxData[i].x = i*3;
        idxData[i].y = i*3+1;
        idxData[i].z = i*3+2;
    }
    m_texCoordIdxBuffer->unmap();

    idxPtr = m_tangentsIdxBuffer->map();
    idxData = (ixyz*)idxPtr;
    for(i=0; i<m_vertices.size()/3; i++){
        idxData[i].x = i*3;
        idxData[i].y = i*3+1;
        idxData[i].z = i*3+2;
    }
    m_tangentsIdxBuffer->unmap();

    idxPtr = m_bitangentsIdxBuffer->map();
    idxData = (ixyz*)idxPtr;
    for(i=0; i<m_vertices.size()/3; i++){
        idxData[i].x = i*3;
        idxData[i].y = i*3+1;
        idxData[i].z = i*3+2;
    }
    m_bitangentsIdxBuffer->unmap();

    //Dont really know what to do with materials yet so lets just have them all default to 0
    unsigned int matIndices[m_vertices.size()/3];
    unsigned int * matPtr = static_cast<unsigned int*>(m_matIdxBuffer->map());
    for(i=0;i<m_vertices.size()/3;i++){
        matPtr[i]=0u;
        matIndices[i]=0u;
    }
    m_matIdxBuffer->unmap();

    //create our geometry in our engine
    m_geometry = PathTracerScene::getInstance()->getContext()->createGeometry();
    //create and set our intersection and bound programs
    Program interProg = PathTracerScene::getInstance()->getContext()->createProgramFromPTXFile("ptx/triangle_mesh.cu.ptx","mesh_intersect");
    Program boundProg = PathTracerScene::getInstance()->getContext()->createProgramFromPTXFile("ptx/triangle_mesh.cu.ptx","mesh_bounds");
    m_geometry->setIntersectionProgram(interProg);
    m_geometry->setBoundingBoxProgram(boundProg);

    //set our buffers for our geomtry
    m_geometry["vertex_buffer"]->setBuffer( m_vertexBuffer );
    m_geometry["vindex_buffer"]->setBuffer( m_vertIdxBuffer );

    m_geometry["normal_buffer"]->setBuffer( m_normalBuffer );
    m_geometry["nindex_buffer"]->setBuffer( m_normIdxBuffer );

    m_geometry["texcoord_buffer"]->setBuffer( m_texCoordsBuffer );
    m_geometry["tindex_buffer"]->setBuffer(m_texCoordIdxBuffer );

    m_geometry["tangent_buffer"]->setBuffer( m_tangentsBuffer );
    m_geometry["tanindex_buffer"]->setBuffer( m_tangentsIdxBuffer);

    m_geometry["bitangent_buffer"]->setBuffer( m_bitangentsBuffer );
    m_geometry["biindex_buffer"]->setBuffer( m_bitangentsIdxBuffer );

    m_geometry["material_buffer"]->setBuffer( m_matIdxBuffer );

    //create our cluster mesh thing
//    unsigned int usePTX32InHost64 = 0;
    RTgeometry geo = m_geometry->get();

//    rtuCreateClusteredMeshExt( PathTracerScene::getInstance()->getContext()->get(),usePTX32InHost64,&geo,
//                                     (unsigned int)_mesh->mNumVertices,
//                                     vertArray,
//                                     (unsigned int)_mesh->mNumFaces,
//                                     vertIdices,
//                                     matIndices,
//                                     m_normalBuffer->get(),
//                                     vertIdices,
//                                     m_texCoordsBuffer->get(),
//                                     vertIdices );

    m_geometry.take(geo);
    m_geometry->setPrimitiveCount(m_vertices.size()/3);

    m_geometryInstance = PathTracerScene::getInstance()->getContext()->createGeometryInstance();
    m_geometryInstance->setGeometry(m_geometry);

    m_geometryGroup = PathTracerScene::getInstance()->getContext()->createGeometryGroup();
    m_geometryGroup->setChildCount(1);
    //stick our instance in our group
    m_geometryGroup->setChild(0,m_geometryInstance);

    //create our acceleration method, in this case none becuase we only have one peice of geomtry
    Acceleration acceleration = PathTracerScene::getInstance()->getContext()->createAcceleration("Sbvh","Bvh");
    //set this acceleration in our geometry group
    m_geometryGroup->setAcceleration(acceleration);

    //make a acceleration dirty
    acceleration->markDirty();

    m_trans->setChild(m_geometryGroup);
}
//----------------------------------------------------------------------------------------------------------------------
void OptiXModel::setMaterial(Material _mat){
    if(m_geometryInstance->getMaterialCount()==0){
        m_geometryInstance->addMaterial(_mat);
    }
    else{
        m_geometryInstance->setMaterial(0,_mat);
    }
}
//----------------------------------------------------------------------------------------------------------------------
Material OptiXModel::createDefaultMat(){
    //create our closest hit and any hit programs
    // Set up diffuse material
    Material diffuse = PathTracerScene::getInstance()->getContext()->createMaterial();
//    std::string ptx_path = "ptx/tempMat.cu.ptx";
    std::string ptx_path = "ptx/path_tracer.cu.ptx";
    Program diffuse_ch = PathTracerScene::getInstance()->getContext()->createProgramFromPTXFile( ptx_path, "constructShaderGlobals" );
    Program diffuse_ah = PathTracerScene::getInstance()->getContext()->createProgramFromPTXFile( ptx_path, "shadow" );
    diffuse->setClosestHitProgram( 0, diffuse_ch );
    diffuse->setAnyHitProgram( 1, diffuse_ah );

    //now we're all done lets return our material
    return diffuse;
}

//----------------------------------------------------------------------------------------------------------------------
void OptiXModel::setTrans(float *_m, bool _transpose, float *_invM){
    m_trans->setMatrix(_transpose,_m,_invM);
    m_geometryGroup->getAcceleration()->markDirty();
}
//----------------------------------------------------------------------------------------------------------------------
void OptiXModel::setTrans(glm::mat4 _trans, bool _transpose){
    // identity matrix to init our transformation
    float m[16];
    m[ 0] = _trans[0][0];  m[ 1] = _trans[1][0];  m[ 2] = _trans[2][0];  m[ 3] = _trans[3][0];
    m[ 4] = _trans[0][1];  m[ 5] = _trans[1][1];  m[ 6] = _trans[2][1];  m[ 7] = _trans[3][1];
    m[ 8] = _trans[0][2];  m[ 9] = _trans[1][2];  m[ 10] = _trans[2][2];  m[ 11] = _trans[3][2];
    m[ 12] = _trans[0][3];  m[ 13] = _trans[1][3];  m[ 14] = _trans[2][3];  m[ 15] = _trans[3][3];
    // create our inverse transform
    float invM[16];
    glm::mat4 inv = glm::inverse(_trans);
    invM[ 0] = inv[0][0];  invM[ 1] = inv[1][0];  invM[ 2] = inv[2][0];  invM[ 3] = inv[3][0];
    invM[ 4] = inv[0][1];  invM[ 5] = inv[1][1];  invM[ 6] = inv[2][1];  invM[ 7] = inv[3][1];
    invM[ 8] = inv[0][2];  invM[ 9] = inv[1][2];  invM[ 10] = inv[2][2];  invM[ 11] = inv[3][2];
    invM[ 12] = inv[0][3];  invM[ 13] = inv[1][3];  invM[ 14] = inv[2][3];  invM[ 15] = inv[3][3];
    // set our transform
    m_trans->setMatrix(_transpose,m,invM);
}
//----------------------------------------------------------------------------------------------------------------------

