// Copyright 2009-2014 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "subdivpatch1.h"
#include "common/ray16.h"
#include "geometry/filter.h"
#include "subdiv_intersector16.h"
#include "bicubic_bezier_patch.h"
#include "geometry/subdiv_cache.h"
#include "bvh4i/bvh4i.h"
#include "bvh4i/bvh4i_traversal.h"

namespace embree
{

  static SubdivCache subdivCache;

  static unsigned int BVH4I_LEAF_MASK = BVH4i::leaf_mask; // needed due to compiler efficiency bug

  static void recursiveFillSubdivCacheEntry(SubdivCache::Entry &entry,
					    const size_t bvh4i_parent_index,
					    const size_t bvh4i_parent_local_index,
					    size_t &bvh4i_node_index,
					    size_t &leaf_index,
					    const Vec2f &s,
					    const Vec2f &t,
					    const RegularCatmullClarkPatch &patch,
					    const unsigned int subdiv_level = 0)
  {
     if (subdiv_level == 0)
      {
	// DBG_PRINT(s);
	// DBG_PRINT(t);

	Vec3fa vtx[4];
	vtx[0] = patch.eval(s[0],t[0]);
	vtx[1] = patch.eval(s[1],t[0]);
	vtx[2] = patch.eval(s[1],t[1]);
	vtx[3] = patch.eval(s[0],t[1]);
	
	BBox3fa bounds( vtx[0] );
	bounds.extend( vtx[1] );
	bounds.extend( vtx[2] );
	bounds.extend( vtx[3] );

	//DBG_PRINT(bounds);

	entry.bvh4i_node[bvh4i_parent_index].setBounds(bvh4i_parent_local_index,bounds);

	// DBG_PRINT(leaf_index);
	// DBG_PRINT(bvh4i_parent_index);
	// DBG_PRINT(bvh4i_parent_local_index);

	BVH4i::NodeRef &parent_ref = entry.bvh4i_node[bvh4i_parent_index].child( bvh4i_parent_local_index );
	// DBG_PRINT( parent_ref );

	//createBVH4iLeaf( parent_ref, (unsigned int)leaf_index, 0 );
	parent_ref = (leaf_index << BVH4i::encodingBits) | BVH4i::leaf_mask;

	// DBG_PRINT( parent_ref );
	entry.uv_interval[leaf_index] = Vec4f(s.x,s.y,t.x,t.y);

	leaf_index++;
      }
    else
      {
	const float mid_s = 0.5f * (s[0]+s[1]);
	const float mid_t = 0.5f * (t[0]+t[1]);
	Vec2f s_left(s[0],mid_s);
	Vec2f s_right(mid_s,s[1]);
	Vec2f t_left(t[0],mid_t);
	Vec2f t_right(mid_t,t[1]);
	const size_t currentIndex = bvh4i_node_index++;
	createBVH4iNode<2>(*(BVH4i::NodeRef*)&entry.bvh4i_node[bvh4i_parent_index].child( bvh4i_parent_local_index ),currentIndex);

	recursiveFillSubdivCacheEntry(entry, currentIndex, 0, bvh4i_node_index, leaf_index, s_left ,t_left ,patch,subdiv_level-1);
	recursiveFillSubdivCacheEntry(entry, currentIndex, 1, bvh4i_node_index, leaf_index, s_right,t_left ,patch,subdiv_level-1);
	recursiveFillSubdivCacheEntry(entry, currentIndex, 2, bvh4i_node_index, leaf_index, s_right,t_right,patch,subdiv_level-1);
	recursiveFillSubdivCacheEntry(entry, currentIndex, 3, bvh4i_node_index, leaf_index, s_left ,t_right,patch,subdiv_level-1);

	BBox3fa bounds( empty );
	for (size_t i=0;i<4;i++)
	  bounds.extend( entry.bvh4i_node[currentIndex].bounds( i ) );

	entry.bvh4i_node[bvh4i_parent_index].setBounds(bvh4i_parent_local_index, bounds);	    
      }   
  }

  static void fillSubdivCacheEntry(SubdivCache::Entry &entry,
				   const Vec2f &s,
				   const Vec2f &t,
				   const RegularCatmullClarkPatch &patch,
				   const unsigned int subdiv_level = 0)
  {
    entry.bvh4i_node[0].setInvalid();
    size_t bvh4i_parent_index = 0;
    size_t bvh4i_parent_local_index = 0;
    size_t bvh4i_node_index = 1;
    size_t leaf_index = 0;

    recursiveFillSubdivCacheEntry(entry, bvh4i_parent_index, bvh4i_parent_local_index, bvh4i_node_index, leaf_index, s , t, patch, subdiv_level);
    
  }


  static __forceinline bool intersect1_quad(const size_t rayIndex, 
					    const mic_f &dir_xyz,
					    const mic_f &org_xyz,
					    Ray16& ray16,
					    const Vec3fa &vtx0,
					    const Vec3fa &vtx1,
					    const Vec3fa &vtx2,
					    const Vec3fa &vtx3,
					    const unsigned int geomID,
					    const unsigned int primID)
  {
    const mic_f v0 = broadcast4to16f(&vtx0);
    const mic_f v1 = select(0x00ff,broadcast4to16f(&vtx1),broadcast4to16f(&vtx2));
    const mic_f v2 = select(0x00ff,broadcast4to16f(&vtx2),broadcast4to16f(&vtx3));
	      
    const mic_f e1 = v1 - v0;
    const mic_f e2 = v0 - v2;	     
    const mic_f normal = lcross_zxy(e1,e2);
    const mic_f org = v0 - org_xyz;
    const mic_f odzxy = msubr231(org * swizzle(dir_xyz,_MM_SWIZ_REG_DACB), dir_xyz, swizzle(org,_MM_SWIZ_REG_DACB));
    const mic_f den = ldot3_zxy(dir_xyz,normal);	      
    const mic_f rcp_den = rcp(den);
    const mic_f uu = ldot3_zxy(e2,odzxy); 
    const mic_f vv = ldot3_zxy(e1,odzxy); 
    const mic_f u = uu * rcp_den;
    const mic_f v = vv * rcp_den;

#if defined(__BACKFACE_CULLING__)
    const mic_m m_init = (mic_m)0x1111 & (den > zero);
#else
    const mic_m m_init = 0x1111;
#endif

    const mic_m valid_u = ge(m_init,u,zero);
    const mic_m valid_v = ge(valid_u,v,zero);
    const mic_m m_aperture = le(valid_v,u+v,mic_f::one()); 

    const mic_f nom = ldot3_zxy(org,normal);

    if (unlikely(none(m_aperture))) return false;
    const mic_f t = rcp_den*nom;
    mic_f max_dist_xyz = mic_f(ray16.tfar[rayIndex]);
    mic_m m_final  = lt(lt(m_aperture,mic_f(ray16.tnear[rayIndex]),t),t,max_dist_xyz);

    //////////////////////////////////////////////////////////////////////////////////////////////////

    /* did the ray hit one of the four triangles? */
    if (unlikely(any(m_final)))
      {
	STAT3(normal.trav_prim_hits,1,1,1);
	max_dist_xyz  = select(m_final,t,max_dist_xyz);
	const mic_f min_dist = vreduce_min(max_dist_xyz);
	const mic_m m_dist = eq(min_dist,max_dist_xyz);
	const size_t vecIndex = bitscan(toInt(m_dist));
	const size_t triIndex = vecIndex >> 2;

	const mic_m m_tri = m_dist^(m_dist & (mic_m)((unsigned int)m_dist - 1));

                
	prefetch<PFHINT_L1EX>(&ray16.tfar);  
	prefetch<PFHINT_L1EX>(&ray16.u);
	prefetch<PFHINT_L1EX>(&ray16.v);
	prefetch<PFHINT_L1EX>(&ray16.Ng.x); 
	prefetch<PFHINT_L1EX>(&ray16.Ng.y); 
	prefetch<PFHINT_L1EX>(&ray16.Ng.z); 
	prefetch<PFHINT_L1EX>(&ray16.geomID);
	prefetch<PFHINT_L1EX>(&ray16.primID);

	max_dist_xyz = min_dist;

	const mic_f gnormalx(normal[triIndex*4+1]);
	const mic_f gnormaly(normal[triIndex*4+2]);
	const mic_f gnormalz(normal[triIndex*4+0]);
		  
	compactustore16f_low(m_tri,&ray16.tfar[rayIndex],min_dist);
	compactustore16f_low(m_tri,&ray16.u[rayIndex],u); 
	compactustore16f_low(m_tri,&ray16.v[rayIndex],v); 
	compactustore16f_low(m_tri,&ray16.Ng.x[rayIndex],gnormalx); 
	compactustore16f_low(m_tri,&ray16.Ng.y[rayIndex],gnormaly); 
	compactustore16f_low(m_tri,&ray16.Ng.z[rayIndex],gnormalz); 

	ray16.geomID[rayIndex] = geomID;
	ray16.primID[rayIndex] = primID;
	return true;
      
      }
    return false;      
  };


  static __forceinline bool occluded1_quad(const size_t rayIndex, 
					   const mic_f &dir_xyz,
					   const mic_f &org_xyz,
					   const Ray16& ray16,
					   mic_m &m_terminated,
					   const Vec3fa &vtx0,
					   const Vec3fa &vtx1,
					   const Vec3fa &vtx2,
					   const Vec3fa &vtx3)
  {
    const mic_f v0 = broadcast4to16f(&vtx0);
    const mic_f v1 = select(0x00ff,broadcast4to16f(&vtx1),broadcast4to16f(&vtx2));
    const mic_f v2 = select(0x00ff,broadcast4to16f(&vtx2),broadcast4to16f(&vtx3));

    const mic_f e1 = v1 - v0;
    const mic_f e2 = v0 - v2;	     
    const mic_f normal = lcross_zxy(e1,e2);

    const mic_f org = v0 - org_xyz;
    const mic_f odzxy = msubr231(org * swizzle(dir_xyz,_MM_SWIZ_REG_DACB), dir_xyz, swizzle(org,_MM_SWIZ_REG_DACB));
    const mic_f den = ldot3_zxy(dir_xyz,normal);	      
    const mic_f rcp_den = rcp(den);
    const mic_f uu = ldot3_zxy(e2,odzxy); 
    const mic_f vv = ldot3_zxy(e1,odzxy); 
    const mic_f u = uu * rcp_den;
    const mic_f v = vv * rcp_den;

#if defined(__BACKFACE_CULLING__)
    const mic_m m_init = (mic_m)0x1111 & (den > zero);
#else
    const mic_m m_init = 0x1111;
#endif

    const mic_m valid_u = ge((mic_m)m_init,u,zero);
    const mic_m valid_v = ge(valid_u,v,zero);
    const mic_m m_aperture = le(valid_v,u+v,mic_f::one()); 

    const mic_f nom = ldot3_zxy(org,normal);
    const mic_f t = rcp_den*nom;
    if (unlikely(none(m_aperture))) return false;

    mic_m m_final  = lt(lt(m_aperture,mic_f(ray16.tnear[rayIndex]),t),t,mic_f(ray16.tfar[rayIndex]));

    if (unlikely(any(m_final)))
      {
	STAT3(shadow.trav_prim_hits,1,1,1);
	m_terminated |= mic_m::shift1[rayIndex];
	return true;
      }
    return false;
  }



  __forceinline void subdivide(const RegularCatmullClarkPatch &source,
			       RegularCatmullClarkPatch dest[4])
  {
    const mic_f row0  = load16f((const float *)&source.v[0][0]);
    const mic_f row1  = load16f((const float *)&source.v[1][0]);
    const mic_f row2  = load16f((const float *)&source.v[2][0]);
    const mic_f row3  = load16f((const float *)&source.v[3][0]);

    const mic_m m_edge0 = 0x0ff0;
    const mic_m m_edge1 = 0x0f0f;

    const mic_m m_left  = 0x0f0f;
    const mic_m m_right = 0xf0f0;

    /* ---- face 0 1 1 2 ---- */
    const mic_f row01 = row0 + row1;
    const mic_f row01shuffle = lshuf<2,1,2,1>(row01);
    const mic_f face0112 = (row01 + row01shuffle) * 0.25f;

    /* ---- edge edge0011 ---- */
    const mic_f face1021 = lshuf<2,3,0,1>(face0112);
    const mic_f mid0011 = select(m_edge0,row01,row01shuffle);
    const mic_f edge0011 = (mid0011 + face0112 + face1021) * 0.25f;

    const mic_f c0_v00 = select(m_left,face0112,edge0011);
    const mic_f c1_v00 = select(m_right,face0112,edge0011);

    /* ---- face 3 4 4 5 ---- */
    const mic_f row12 = row1 + row2;
    const mic_f row12shuffle = lshuf<2,1,2,1>(row12);
    const mic_f face3445 = (row12 + row12shuffle) * 0.25f;

    /* ---- edge edge2334 ---- */
    const mic_f row1shuffle = lshuf<2,1,2,1>(row1);
    const mic_f mid2334 = row1shuffle + row1;
    store16f(&dest[0].v[0][0],c0_v00);
    const mic_f edge2334 = (face0112 + face3445 + mid2334) * 0.25f;
    store16f(&dest[1].v[0][0],c1_v00);

    /* ---- edge edge5566 ---- */
    const mic_f face4354 = lshuf<2,3,0,1>(face3445);
    const mic_f mid5566 = select(m_edge0,row12,row12shuffle);
    const mic_f edge5566 = (mid5566 + face3445 + face4354) * 0.25f;
   
    const mic_f c0_v20 = select(m_left,face3445,edge5566);
    const mic_f c1_v20 = select(m_right,face3445,edge5566);
    const mic_f c2_v10 = c1_v20; // select(m_right,face3445,edge5566);
    const mic_f c3_v10 = c0_v20; // select(m_left,face3445,edge5566);

    /* ---- face 6 7 7 8 ---- */
    const mic_f row23 = row2 + row3;
    const mic_f row23shuffle = lshuf<2,1,2,1>(row23);
    const mic_f face6778 = (row23 + row23shuffle) * 0.25f;

    /* ---- edge edge7889 ---- */
    const mic_f row2shuffle = lshuf<2,1,2,1>(row2);
    store16f(&dest[0].v[2][0],c0_v20);
    const mic_f mid7889 = row2shuffle + row2;
    store16f(&dest[1].v[2][0],c1_v20);
    const mic_f edge7889 = (face3445 + face6778 + mid7889) * 0.25f;
    store16f(&dest[2].v[1][0],c2_v10);
    /* ---- edge edge10101111 ---- */
    const mic_f face7687 = lshuf<2,3,0,1>(face6778);
    store16f(&dest[3].v[1][0],c3_v10);
    const mic_f mid10101111 = select(m_edge0,row23,row23shuffle);
    const mic_f edge10101111 = (mid10101111 + face6778 + face7687) * 0.25;

    const mic_f c2_v30 = select(m_right,face6778,edge10101111);
    const mic_f c3_v30 = select(m_left,face6778,edge10101111);


    /* ---- new quad0011 ---- */
    const mic_f p5566 = lshuf<2,2,1,1>(row1);
    const mic_f row1_e = row1 + lshuf<1,0,0,2>(row1);

    const mic_f sum_edges01 = select(0xf00f,row1_e,row0+row2);
    store16f(&dest[2].v[3][0],c2_v30);
    const mic_f qr01_2lanes = face0112 + face3445 + sum_edges01;
    store16f(&dest[3].v[3][0],c3_v30);    

    const mic_f qr01 = qr01_2lanes + lshuf<2,3,0,1>(qr01_2lanes);
    const mic_f quad0011 = qr01 * 1.0f/16.0f + p5566 * 0.5f;


    const mic_f c0_v10 = select(m_left,edge2334,quad0011);
    const mic_f c1_v10 = select(m_right,edge2334,quad0011);
    const mic_f c2_v00 = c1_v10; // select(m_right,edge2334,quad0011);
    const mic_f c3_v00 = c0_v10; // select(m_left,edge2334,quad0011);

    /* ---- new quad2233 ---- */
    const mic_f p991010 = lshuf<2,2,1,1>(row2);
    const mic_f row2_e = row2 + lshuf<1,0,0,2>(row2);
    const mic_f sum_edges23 = select(0xf00f,row2_e,row1+row3);
    const mic_f qr23_2lanes = face3445 + face6778 + sum_edges23;
    const mic_f qr23 = qr23_2lanes + lshuf<2,3,0,1>(qr23_2lanes);
    const mic_f quad2233 = p991010 * 0.5f + qr23 * 1.0f/16.0f;

    store16f(&dest[0].v[1][0],c0_v10);

    const mic_f c0_v30 = select(m_left,edge7889,quad2233);
    const mic_f c1_v30 = select(m_right,edge7889,quad2233);

    const mic_f c2_v20 = c1_v30; // select(m_right,edge7889,quad2233);
    const mic_f c3_v20 = c0_v30; // select(m_left,edge7889,quad2233);
    store16f(&dest[1].v[1][0],c1_v10);
    store16f(&dest[2].v[0][0],c2_v00);
    store16f(&dest[3].v[0][0],c3_v00);

    store16f(&dest[0].v[3][0],c0_v30);
    store16f(&dest[3].v[2][0],c3_v20);
    store16f(&dest[1].v[3][0],c1_v30);
    store16f(&dest[2].v[2][0],c2_v20);


  }


  __forceinline void init_bvh4node(BVH4i::Node &node,RegularCatmullClarkPatch dest[4])
  {
#pragma unroll(4)
    for (size_t i=0;i<4;i++)
      {
	const mic_f r0 = dest[i].getRow(0);
	const mic_f r1 = dest[i].getRow(1);
	const mic_f r2 = dest[i].getRow(2);
	const mic_f r3 = dest[i].getRow(3);       
	const mic_f r_min = min(min(r0,r1),min(r2,r3));
	const mic_f r_max = max(max(r0,r1),max(r2,r3));	
	const mic_f b_min = set_min_lanes(r_min);
	const mic_f b_max = set_max_lanes(r_max);
	store4f(&node.lower[i],b_min);
	store4f(&node.upper[i],b_max);
      }
  }


  __forceinline void init_bvh4node(BVH4i::Node &node,IrregularCatmullClarkPatch dest[4])
  {
    for (size_t i=0;i<4;i++)
      {
	Vec3fa_t r0 = dest[i].ring[0].vtx;
	Vec3fa_t r1 = dest[i].ring[1].vtx;
	Vec3fa_t r2 = dest[i].ring[2].vtx;
	Vec3fa_t r3 = dest[i].ring[3].vtx;

	Vec3fa_t b_min = min(min(r0,r1),min(r2,r3));
	Vec3fa_t b_max = max(max(r0,r1),max(r2,r3));

	for (size_t r=0;r<4;r++)
	  {
	    for (size_t n=0;n<dest[i].ring[r].num_vtx;n++)
	      {
		b_min = min(b_min,dest[i].ring[r].ring[n]);
		b_max = max(b_max,dest[i].ring[r].ring[n]);
	      }
	  }
	*(Vec3fa*)&node.lower[i] = b_min;
	*(Vec3fa*)&node.upper[i] = b_max;
      }
  }

  __forceinline mic_m intersect(const BVH4i::Node &node,
				const mic_f &rdir_xyz,
				const mic_f &org_rdir_xyz,
				const mic_f &min_dist,
				const mic_f &max_dist)
  {
    const float* __restrict const plower = (float*)&node.lower;
    const float* __restrict const pupper = (float*)&node.upper;
    const mic_f tLowerXYZ = rdir_xyz * load16f(plower) - org_rdir_xyz;
    const mic_f tUpperXYZ = rdir_xyz * load16f(pupper) - org_rdir_xyz;
    const mic_f tLower = select(0x7777,min(tLowerXYZ,tUpperXYZ),min_dist);
    const mic_f tUpper = select(0x7777,max(tLowerXYZ,tUpperXYZ),max_dist);
    const mic_f tNear = vreduce_max4(tLower);
    const mic_f tFar  = vreduce_min4(tUpper);  
    return le(0x1111,tNear,tFar);    
  }


  
  void subdivide_intersect1_eval(const size_t rayIndex, 
				 const mic_f &dir_xyz,
				 const mic_f &org_xyz,
				 Ray16& ray16,
				 const RegularCatmullClarkPatch &patch,
				 const unsigned int geomID,
				 const unsigned int primID,
				 const Vec2f &s,
				 const Vec2f &t,
				 const unsigned int subdiv_level)
  {

#if 1
    __aligned(64) BVH4i::NodeRef stack_node[32];
    __aligned(64) float stack_dist[32];

    SubdivCache::Tag &tag     = subdivCache.lookupEntry(geomID,primID,subdiv_level);
    SubdivCache::Entry &entry = subdivCache.getEntry(tag);

    //DBG_PRINT(tag);

    if (unlikely(!tag.inCache(geomID,primID,subdiv_level)))
      {
	//DBG_PRINT("FILL");

	fillSubdivCacheEntry(entry,s,t,patch,subdiv_level);	
	tag = SubdivCache::Tag(geomID,primID,subdiv_level);

	//DBG_PRINT(tag);

	//DBG_PRINT(subdivCache);

      }

    const mic_f rdir_xyz      = rcp(dir_xyz);
    const mic_f org_rdir_xyz  = rdir_xyz * org_xyz;
    const mic_f min_dist_xyz  = ray16.tnear[rayIndex];
    const mic_f max_dist_xyz  = ray16.tfar[rayIndex];

    stack_node[0] = BVH4i::invalidNode;
    stack_node[1] = entry.getRoot();

    size_t sindex = 2;
    const BVH4i::Node * __restrict__ const nodes = entry.bvh4i_node;

    const unsigned int leaf_mask = BVH4I_LEAF_MASK;

    while(1)
      {
	BVH4i::NodeRef curNode = stack_node[sindex-1];
	sindex--;

	traverse_single_intersect<false>(curNode,
					 sindex,
					 rdir_xyz,
					 org_rdir_xyz,
					 min_dist_xyz,
					 max_dist_xyz,
					 stack_node,
					 stack_dist,
					 nodes,
					 leaf_mask);            		    

	/* return if stack is empty */
	if (unlikely(curNode == BVH4i::invalidNode)) break;

	unsigned int leafIndex = curNode.offsetIndex();

	const Vec4f &uv = entry.uv_interval[leafIndex];

	//DBG_PRINT(uv);

	const float u_min = uv[0];
	const float u_max = uv[1];
	const float v_min = uv[2];
	const float v_max = uv[3];

	Vec3fa vtx[4];
	vtx[0] = patch.eval(u_min,v_min);
	vtx[1] = patch.eval(u_max,v_min);
	vtx[2] = patch.eval(u_max,v_max);
	vtx[3] = patch.eval(u_min,v_max);

	intersect1_quad(rayIndex,
			dir_xyz,
			org_xyz,
			ray16,
			vtx[0],
			vtx[1],
			vtx[2],
			vtx[3],
			geomID,
			primID);      	
      }


#else

    if (subdiv_level == 0)
      {
	Vec3fa vtx[4];
	vtx[0] = patch.eval(s[0],t[0]);
	vtx[1] = patch.eval(s[1],t[0]);
	vtx[2] = patch.eval(s[1],t[1]);
	vtx[3] = patch.eval(s[0],t[1]);

       intersect1_quad(rayIndex,
		       dir_xyz,
		       org_xyz,
		       ray16,
		       vtx[0],
		       vtx[1],
		       vtx[2],
		       vtx[3],
		       geomID,
		       primID);      
      }
    else
      {
	const float mid_s = 0.5f * (s[0]+s[1]);
	const float mid_t = 0.5f * (t[0]+t[1]);
	Vec2f s_left(s[0],mid_s);
	Vec2f s_right(mid_s,s[1]);
	Vec2f t_left(t[0],mid_t);
	Vec2f t_right(mid_t,t[1]);
	subdivide_intersect1_eval(rayIndex,dir_xyz,org_xyz,ray16,patch,geomID,primID,s_left ,t_left,subdiv_level - 1);
	subdivide_intersect1_eval(rayIndex,dir_xyz,org_xyz,ray16,patch,geomID,primID,s_right,t_left,subdiv_level - 1);
	subdivide_intersect1_eval(rayIndex,dir_xyz,org_xyz,ray16,patch,geomID,primID,s_right,t_right,subdiv_level - 1);
	subdivide_intersect1_eval(rayIndex,dir_xyz,org_xyz,ray16,patch,geomID,primID,s_left ,t_right,subdiv_level - 1);
      }
#endif
  }


  void subdivide_intersect1_eval(const size_t rayIndex, 
				 const mic_f &dir_xyz,
				 const mic_f &org_xyz,
				 Ray16& ray16,
				 const GregoryPatch &patch,
				 const unsigned int geomID,
				 const unsigned int primID,
				 const Vec2f &s,
				 const Vec2f &t,
				 const unsigned int subdiv_level)
  {
    if (subdiv_level == 0)
      {
	Vec3fa vtx[4];

	vtx[0] = patch.eval(s[0],t[0]);
	vtx[1] = patch.eval(s[1],t[0]);
	vtx[2] = patch.eval(s[1],t[1]);
	vtx[3] = patch.eval(s[0],t[1]);
	
	
	intersect1_quad(rayIndex,
			dir_xyz,
			org_xyz,
			ray16,
			vtx[0],
			vtx[1],
			vtx[2],
			vtx[3],
			geomID,
			primID);      
      }
    else
      {
	const float mid_s = 0.5f * (s[0]+s[1]);
	const float mid_t = 0.5f * (t[0]+t[1]);
	Vec2f s_left(s[0],mid_s);
	Vec2f s_right(mid_s,s[1]);
	Vec2f t_left(t[0],mid_t);
	Vec2f t_right(mid_t,t[1]);
	subdivide_intersect1_eval(rayIndex,dir_xyz,org_xyz,ray16,patch,geomID,primID,s_left ,t_left,subdiv_level - 1);
	subdivide_intersect1_eval(rayIndex,dir_xyz,org_xyz,ray16,patch,geomID,primID,s_right,t_left,subdiv_level - 1);
	subdivide_intersect1_eval(rayIndex,dir_xyz,org_xyz,ray16,patch,geomID,primID,s_right,t_right,subdiv_level - 1);
	subdivide_intersect1_eval(rayIndex,dir_xyz,org_xyz,ray16,patch,geomID,primID,s_left ,t_right,subdiv_level - 1);
      }
  }

 void subdivide_intersect1(const size_t rayIndex, 
			   const mic_f &dir_xyz,
			   const mic_f &org_xyz,
			   Ray16& ray16,
			   const IrregularCatmullClarkPatch &patch,
			   const unsigned int geomID,
			   const unsigned int primID,
			   const unsigned int subdiv_level)
 {
   if (subdiv_level == 0)
     {
       intersect1_quad(rayIndex,
		       dir_xyz,
		       org_xyz,
		       ray16,
		       patch.ring[0].vtx,
		       patch.ring[1].vtx,
		       patch.ring[2].vtx,
		       patch.ring[3].vtx,
		       geomID,
		       primID);      

     }
   else
     {
       IrregularCatmullClarkPatch subpatches[4];
       patch.subdivide(subpatches);

       const mic_f rdir_xyz     = rcp(dir_xyz);
       const mic_f org_rdir_xyz = rdir_xyz * org_xyz;
       const mic_f min_dist     = ray16.tnear[rayIndex];
       const mic_f max_dist     = ray16.tfar[rayIndex];

#if 1
       BVH4i::Node node;
       init_bvh4node(node,subpatches);
       const mic_m m_hit = intersect(node,
				     rdir_xyz,
				     org_rdir_xyz,
				     min_dist,
				     max_dist);
       long index = -1;
       while((index = bitscan64(index,m_hit)) != BITSCAN_NO_BIT_SET_64) 
	 {
	   const unsigned int i = (unsigned int)index >> 2;
	   subdivide_intersect1(rayIndex, 
				dir_xyz,
				org_xyz,
				ray16,
				subpatches[i],
				geomID,
				primID,
				subdiv_level - 1);	    
	 }
#else				     
       for (size_t i=0;i<4;i++)
	 subdivide_intersect1(rayIndex, 
			      dir_xyz,
			      org_xyz,
			      ray16,
			      subpatches[i],
			      geomID,
			      primID,
			      subdiv_level - 1);	    
#endif
     }
   
 }


 void subdivide_intersect1(const size_t rayIndex, 
			   const mic_f &dir_xyz,
			   const mic_f &org_xyz,
			   Ray16& ray16,
			   const RegularCatmullClarkPatch &patch,
			   const unsigned int geomID,
			   const unsigned int primID,
			   const unsigned int subdiv_level)
 {
   if (subdiv_level == 0)
     {
       intersect1_quad(rayIndex,
		       dir_xyz,
		       org_xyz,
		       ray16,
		       patch.v[1][1],
		       patch.v[1][2],
		       patch.v[2][2],
		       patch.v[2][1],
		       geomID,
		       primID);      
     }
   else
     {
       RegularCatmullClarkPatch subpatches[4];
       subdivide(patch,subpatches);
       
       const mic_f rdir_xyz     = rcp(dir_xyz);
       const mic_f org_rdir_xyz = rdir_xyz * org_xyz;
       const mic_f min_dist     = ray16.tnear[rayIndex];
       const mic_f max_dist     = ray16.tfar[rayIndex];

#if 1
       BVH4i::Node node;
       init_bvh4node(node,subpatches);
       const mic_m m_hit = intersect(node,
				     rdir_xyz,
				     org_rdir_xyz,
				     min_dist,
				     max_dist);
       long index = -1;
       while((index = bitscan64(index,m_hit)) != BITSCAN_NO_BIT_SET_64) 
	 {
	   const unsigned int i = (unsigned int)index >> 2;
	   subdivide_intersect1(rayIndex, 
				dir_xyz,
				org_xyz,
				ray16,
				subpatches[i],
				geomID,
				primID,
				subdiv_level - 1);	    
	 }
#else				     

       for (size_t i=0;i<4;i++)
	 subdivide_intersect1(rayIndex, 
			      dir_xyz,
			      org_xyz,
			      ray16,
			      subpatches[i],
			      geomID,
			      primID,
			      subdiv_level - 1);	    
#endif
     }
   
 }

};
