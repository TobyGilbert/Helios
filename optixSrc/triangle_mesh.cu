
/*
 * Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu_aabb_namespace.h>

using namespace optix;

// This is to be plugged into an RTgeometry object to represent
// a triangle mesh with a vertex buffer of triangle soup (triangle list)
// with an interleaved position, normal, texturecoordinate layout.

rtBuffer<float3> vertex_buffer;     
rtBuffer<float3> normal_buffer;
rtBuffer<float2> texcoord_buffer;
rtBuffer<float3> tangent_buffer;
rtBuffer<float3> bitangent_buffer;


rtDeclareVariable(float3, texcoord, attribute texcoord, ); 
rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(float3, tangent, attribute tangent, );
rtDeclareVariable(float3, bitangent, attribute bitangent, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

RT_PROGRAM void mesh_intersect( int primIdx )
{
  int3 v_idx;
  v_idx.x = primIdx*3;
  v_idx.y = v_idx.x+1;
  v_idx.z = v_idx.y+1;

  float3 p0 = vertex_buffer[ v_idx.x];
  float3 p1 = vertex_buffer[ v_idx.y ];
  float3 p2 = vertex_buffer[ v_idx.z ];

  // Intersect ray with triangle
  float3 n;
  float  t, beta, gamma;
  if( intersect_triangle( ray, p0, p1, p2, n, t, beta, gamma ) ) {

    if(  rtPotentialIntersection( t ) ) {

      if ( normal_buffer.size() == 0 || v_idx.x < 0 || v_idx.y < 0 || v_idx.z < 0 ) {
        shading_normal = normalize( n );
      } else {
        float3 n0 = normal_buffer[ v_idx.x ];
        float3 n1 = normal_buffer[ v_idx.y ];
        float3 n2 = normal_buffer[ v_idx.z ];
        shading_normal = normalize( n1*beta + n2*gamma + n0*(1.0f-beta-gamma) );
      }
      geometric_normal = normalize( n );

      if ( texcoord_buffer.size() == 0 || v_idx.x < 0 || v_idx.y < 0 || v_idx.z < 0 ) {
        texcoord = make_float3( 1.0f, 0.0f, 0.0f );
      } else {
        float2 t0 = texcoord_buffer[ v_idx.x ];
        float2 t1 = texcoord_buffer[ v_idx.y ];
        float2 t2 = texcoord_buffer[ v_idx.z ];
        texcoord = make_float3( t1*beta + t2*gamma + t0*(1.0f-beta-gamma) );
      }

      if( tangent_buffer.size() == 0 || v_idx.x < 0 || v_idx.y < 0 || v_idx.z < 0) {
          tangent = make_float3(0.0f, 0.0f, 0.0f);
      }
      else{
          float3 tan0 = tangent_buffer[ v_idx.x ];
          float3 tan1 = tangent_buffer[ v_idx.y ];
          float3 tan2 = tangent_buffer[ v_idx.z ];
          tangent = normalize( tan1*beta + tan2*gamma + tan0*(1.0f-beta-gamma) );
      }

      if( bitangent_buffer.size() == 0 || v_idx.x < 0 || v_idx.y < 0 || v_idx.z < 0) {
          bitangent = make_float3(0.0f, 0.0f, 0.0f);
      }
      else{
          float3 bitan0 = bitangent_buffer[ v_idx.x ];
          float3 bitan1 = bitangent_buffer[ v_idx.y ];
          float3 bitan2 = bitangent_buffer[ v_idx.z ];
          bitangent = normalize( bitan1*beta + bitan2*gamma + bitan0*(1.0f-beta-gamma) );
      }

      rtReportIntersection(0u);
    }
  }
}

RT_PROGRAM void mesh_bounds (int primIdx, float result[6])
{
  int3 v_idx;
  v_idx.x = primIdx*3;
  v_idx.y = v_idx.x+1;
  v_idx.z = v_idx.y+1;

  const float3 v0   = vertex_buffer[ v_idx.x ];
  const float3 v1   = vertex_buffer[ v_idx.y ];
  const float3 v2   = vertex_buffer[ v_idx.z ];
  const float  area = length(cross(v1-v0, v2-v0));

  optix::Aabb* aabb = (optix::Aabb*)result;
  
  if(area > 0.0f && !isinf(area)) {
    aabb->m_min = fminf( fminf( v0, v1), v2 );
    aabb->m_max = fmaxf( fmaxf( v0, v1), v2 );
  } else {
    aabb->invalidate();
  }
}

