#include "optixmodel.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <optixu/optixu.h>

#include <iostream>

//----------------------------------------------------------------------------------------------------------------------
OptiXModel::OptiXModel(Context &_context)
{
    // identity matrix to init our transformation
    float m[16];
    m[ 0] = 1.0f;  m[ 1] = 0.0f;  m[ 2] = 0.0f;  m[ 3] = 0.0f;
    m[ 4] = 0.0f;  m[ 5] = 1.0f;  m[ 6] = 0.0f;  m[ 7] = 0.0f;
    m[ 8] = 0.0f;  m[ 9] = 0.0f;  m[10] = 1.0f;  m[11] = 0.0f;
    m[12] = 0.0f;  m[13] = 0.0f;  m[14] = 0.0f;  m[15] = 1.0f;
    //init our trans
    m_trans = _context->createTransform();
    m_trans->setMatrix(false,m,0);
}
//----------------------------------------------------------------------------------------------------------------------
void OptiXModel::createGeomtry(std::string _loc, Context &_context){
    //import our mesh
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(_loc.c_str(), aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
    aiMesh* mesh = scene->mMeshes[0];

    // Create vertex, normal, and texture_coordinate buffers
    m_vertexBuffer = _context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, mesh->mNumVertices );
    m_normalBuffer = _context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, mesh->mNumVertices );
    m_texCoordsBuffer = _context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT2, mesh->mNumVertices);
    //now lets write our information to our buffers
    void *vertPtr = m_vertexBuffer->map();
    typedef struct { float x; float y; float z;} xyz;
    xyz* vertData = (xyz*)vertPtr;
    float vertArray[mesh->mNumVertices*3];
    unsigned int i;
    for(i=0; i<mesh->mNumVertices;i++){
        vertData[i].x = mesh->mVertices[i].x;
        vertData[i].y = mesh->mVertices[i].y;
        vertData[i].z = mesh->mVertices[i].z;
        vertArray[i*3] = mesh->mVertices[i].x;
        vertArray[i*3+1] = mesh->mVertices[i].x;
        vertArray[i*3+2] = mesh->mVertices[i].x;
    }
    m_vertexBuffer->unmap();
    vertPtr = m_normalBuffer->map();
    vertData = (xyz*)vertPtr;
    for(i=0; i<mesh->mNumVertices;i++){
        vertData[i].x = mesh->mNormals[i].x;
        vertData[i].y = mesh->mNormals[i].y;
        vertData[i].z = mesh->mNormals[i].z;
    }
    m_normalBuffer->unmap();
    void *texPtr = m_texCoordsBuffer->map();
    typedef struct {float x; float y;} xy;
    xy* texData = (xy*)texPtr;
    for(i=0; i<mesh->mNumVertices;i++){
        texData[i].x = mesh->mTextureCoords[0][i].x;
        texData[i].y = mesh->mTextureCoords[0][i].y;
    }
    m_texCoordsBuffer->unmap();

    //now lets set up our index buffers
    m_vertIdxBuffer = _context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT3, mesh->mNumFaces );
    m_normIdxBuffer = _context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT3, mesh->mNumFaces );
    m_texCoordIdxBuffer = _context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT3, mesh->mNumFaces );
    m_matIdxBuffer = _context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_INT, mesh->mNumFaces );

    //lets fill up our idx buffers
    unsigned int vertIdices[mesh->mNumVertices];
    void *idxPtr = m_vertIdxBuffer->map();
    typedef struct { int x; int y; int z;} ixyz;
    ixyz* idxData = (ixyz*)idxPtr;
    for(i=0; i<mesh->mNumFaces;i++){
        idxData[i].x = i*3;
        idxData[i].y = i*3+1;
        idxData[i].z = i*3+2;
        vertIdices[i*3] = i*3;
        vertIdices[i*3+1] = i*3+1;
        vertIdices[i*3+2] = i*3+2;
    }
    m_vertIdxBuffer->unmap();

    idxPtr = m_normIdxBuffer->map();
    idxData = (ixyz*)idxPtr;
    for(i=0; i<mesh->mNumFaces;i++){
        idxData[i].x = i*3;
        idxData[i].y = i*3+1;
        idxData[i].z = i*3+2;
    }
    m_normIdxBuffer->unmap();

    idxPtr = m_texCoordIdxBuffer->map();
    idxData = (ixyz*)idxPtr;
    for(i=0; i<mesh->mNumFaces;i++){
        idxData[i].x = i*3;
        idxData[i].y = i*3+1;
        idxData[i].z = i*3+2;
    }
    m_texCoordIdxBuffer->unmap();

    //Dont really know what to do with materials yet so lets just have them all default to 0
    unsigned int matIndices[mesh->mNumFaces];
    unsigned int * matPtr = static_cast<unsigned int*>(m_matIdxBuffer->map());
    for(i=0;i<mesh->mNumFaces;i++){
        matPtr[i]=0u;
        matIndices[i]=0u;
    }
    m_matIdxBuffer->unmap();

    //create our geometry in our engine
    m_geometry = _context->createGeometry();
    //create and set our intersection and bound programs
    Program interProg = _context->createProgramFromPTXFile("ptx/triangle_mesh.cu.ptx","mesh_intersect");
    Program boundProg = _context->createProgramFromPTXFile("ptx/triangle_mesh.cu.ptx","mesh_bounds");
    m_geometry->setIntersectionProgram(interProg);
    m_geometry->setBoundingBoxProgram(boundProg);

    //set our buffers for our geomtry
    m_geometry["vertex_buffer"]->setBuffer( m_vertexBuffer );
    m_geometry["vindex_buffer"]->setBuffer( m_vertIdxBuffer );

    m_geometry["normal_buffer"]->setBuffer( m_normalBuffer );
    m_geometry["nindex_buffer"]->setBuffer( m_normIdxBuffer );

    m_geometry["texcoord_buffer"]->setBuffer( m_texCoordsBuffer );
    m_geometry["tindex_buffer"]->setBuffer(m_texCoordIdxBuffer );

    m_geometry["material_buffer"]->setBuffer( m_matIdxBuffer );

    //create our cluster mesh thing
    unsigned int usePTX32InHost64 = 0;
    RTgeometry geo = m_geometry->get();

//    rtuCreateClusteredMeshExt( _context->get(),usePTX32InHost64,&geo,
//                                     (unsigned int)mesh->mNumVertices,
//                                     vertArray,
//                                     (unsigned int)mesh->mNumFaces,
//                                     vertIdices,
//                                     matIndices,
//                                     m_normalBuffer->get(),
//                                     vertIdices,
//                                     m_texCoordsBuffer->get(),
//                                     vertIdices );

    m_geometry.take(geo);
    m_geometry->setPrimitiveCount(mesh->mNumFaces);

    m_geometryInstance = _context->createGeometryInstance();
    m_geometryInstance->setGeometry(m_geometry);
    if(m_materials.size()==0){
        m_geometryInstance->addMaterial(createDefaultMat(_context));
        m_geometryInstance["diffuse_color"]->setFloat(1.0f,1.0f,1.0f);
    }
    else{
        for(i=0;i<m_materials.size();i++){
            m_geometryInstance->addMaterial(m_materials[i]);
        }
    }

    GeometryGroup geoGroup = _context->createGeometryGroup();
    geoGroup->setChildCount(1);
    //stick our instance in our group
    geoGroup->setChild(0,m_geometryInstance);

    //create our acceleration method, in this case none becuase we only have one peice of geomtry
    Acceleration acceleration = _context->createAcceleration("Bvh","Bvh");
    //set this acceleration in our geometry group
    geoGroup->setAcceleration(acceleration);

    //make a acceleration dirty
    acceleration->markDirty();

    m_trans->setChild(geoGroup);
}
//----------------------------------------------------------------------------------------------------------------------
Material OptiXModel::createDefaultMat(Context &_context){
    //create our closest hit and any hit programs
    // Set up diffuse material
    Material diffuse = _context->createMaterial();
    std::string ptx_path = "ptx/path_tracer.cu.ptx";
    Program diffuse_ch = _context->createProgramFromPTXFile( ptx_path, "diffuse" );
    Program diffuse_ah = _context->createProgramFromPTXFile( ptx_path, "shadow" );
    diffuse->setClosestHitProgram( 0, diffuse_ch );
    diffuse->setAnyHitProgram( 1, diffuse_ah );

    //now we're all done lets return our material
    return diffuse;
}

//----------------------------------------------------------------------------------------------------------------------
void OptiXModel::setTrans(float *_m, bool _transpose, float *_invM){
    m_trans->setMatrix(_transpose,_m,_invM);
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
