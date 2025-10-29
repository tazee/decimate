/*
 * C++ wrapper for lxshdr.h
 *
 *	Copyright 0000
 *
 */
#ifndef LXW_SHDR_HPP
#define LXW_SHDR_HPP

#include <lxsdk/lxshdr.h>
#include <lxsdk/lx_wrap.hpp>
#include <string>

namespace lx {
    static const LXtGUID guid_Shader = {0x051ba6d8,0x46ba,0x4722,0x8b,0x07,0x94,0x39,0x61,0xc3,0xae,0xda};
    static const LXtGUID guid_ShaderService = {0x0b4e73ac,0x9325,0x4920,0xaf,0xcc,0x78,0x14,0x95,0x3a,0x26,0xb6};
};

class CLxLoc_Shader : public CLxLocalize<ILxShaderID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_Shader() {_init();}
  CLxLoc_Shader(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_Shader(const CLxLoc_Shader &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_Shader;}
    LxResult
  Enumerate (ILxUnknownID visitor)
  {
    return m_loc[0]->Enumerate (m_loc,(ILxUnknownID)visitor);
  }
    LxResult
  Spawn (void **ppvObj)
  {
    return m_loc[0]->Spawn (m_loc,ppvObj);
  }
    CLxResult
  Spawn (CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->Spawn(m_loc,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  ShaderItemGet (void **ppvObj)
  {
    return m_loc[0]->ShaderItemGet (m_loc,ppvObj);
  }
    CLxResult
  ShaderItemGet (CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->ShaderItemGet(m_loc,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
};

class CLxLoc_ShaderService : public CLxLocalizedService
{
public:
  ILxShaderServiceID m_loc;
  void _init() {m_loc=0;}
  CLxLoc_ShaderService() {_init();set();}
 ~CLxLoc_ShaderService() {}
  void set() {if(!m_loc)m_loc=reinterpret_cast<ILxShaderServiceID>(lx::GetGlobal(&lx::guid_ShaderService));}
    float
  ComputeFresnel (const LXtFVector inRay, const LXtFVector normalRay, float normReflAmt) const
  {
    return m_loc[0]->ComputeFresnel (m_loc,inRay,normalRay,normReflAmt);
  }
    float
  ScalarBlendValue (float v1, float v2, float opa, int mode)
  {
    return m_loc[0]->ScalarBlendValue (m_loc,v1,v2,opa,mode);
  }
    void
  ColorBlendValue (LXtFVector c, const LXtFVector c1, const LXtFVector c2, float opa, int mode)
  {
    m_loc[0]->ColorBlendValue (m_loc,c,c1,c2,opa,mode);
  }
    void
  SquareToCircle (float *x, float *y) const
  {
    m_loc[0]->SquareToCircle (m_loc,x,y);
  }
    LxResult
  SampleCloud (ILxUnknownID sample, void **ppvObj) const
  {
    return m_loc[0]->SampleCloud (m_loc,(ILxUnknownID)sample,ppvObj);
  }
    CLxResult
  SampleCloud (ILxUnknownID sample, CLxLocalizedObject &o_dest) const
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->SampleCloud(m_loc,(ILxUnknownID)sample,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  MeshShaderAccessor (ILxUnknownID meshItem, void **ppvObj)
  {
    return m_loc[0]->MeshShaderAccessor (m_loc,(ILxUnknownID)meshItem,ppvObj);
  }
    CLxResult
  MeshShaderAccessor (ILxUnknownID meshItem, CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->MeshShaderAccessor(m_loc,(ILxUnknownID)meshItem,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  PolyShaderAccessor (ILxUnknownID meshItem, LXtPolygonID polyID, void **ppvObj)
  {
    return m_loc[0]->PolyShaderAccessor (m_loc,(ILxUnknownID)meshItem,polyID,ppvObj);
  }
    CLxResult
  PolyShaderAccessor (ILxUnknownID meshItem, LXtPolygonID polyID, CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->PolyShaderAccessor(m_loc,(ILxUnknownID)meshItem,polyID,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    float
  RussianRoulette (ILxUnknownID vector, float importance) const
  {
    return m_loc[0]->RussianRoulette (m_loc,(ILxUnknownID)vector,importance);
  }
    float
  NextRandom (ILxUnknownID vector) const
  {
    return m_loc[0]->NextRandom (m_loc,(ILxUnknownID)vector);
  }
    LxResult
  PoissonOffset (ILxUnknownID vector, float *u, float *v) const
  {
    return m_loc[0]->PoissonOffset (m_loc,(ILxUnknownID)vector,u,v);
  }
    LxResult
  CollectMaterials (ILxUnknownID collection)
  {
    return m_loc[0]->CollectMaterials (m_loc,(ILxUnknownID)collection);
  }
    LxResult
  MeshMaskAccessor (ILxUnknownID meshItem, void **ppvObj)
  {
    return m_loc[0]->MeshMaskAccessor (m_loc,(ILxUnknownID)meshItem,ppvObj);
  }
    CLxResult
  MeshMaskAccessor (ILxUnknownID meshItem, CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->MeshMaskAccessor(m_loc,(ILxUnknownID)meshItem,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  CollectMaskedItems (ILxUnknownID collection)
  {
    return m_loc[0]->CollectMaskedItems (m_loc,(ILxUnknownID)collection);
  }
};

#endif
