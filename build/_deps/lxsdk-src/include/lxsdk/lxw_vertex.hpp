/*
 * C++ wrapper for lxvertex.h
 *
 *	Copyright 0000
 *
 */
#ifndef LXW_VERTEX_HPP
#define LXW_VERTEX_HPP

#include <lxsdk/lxvertex.h>
#include <lxsdk/lx_wrap.hpp>
#include <string>

namespace lx {
    static const LXtGUID guid_VertexFeatureService = {0xB40D51A9,0x6C04,0x46D1,0xBA,0x8A,0x0F,0xE9,0xB3,0x2E,0xAC,0x3C};
    static const LXtGUID guid_TableauVertex = {0xF90A0A39,0xEE2A,0x4D11,0x91,0x2B,0x93,0x38,0xEF,0x27,0x1D,0xFF};
};

class CLxLoc_VertexFeatureService : public CLxLocalizedService
{
public:
  ILxVertexFeatureServiceID m_loc;
  void _init() {m_loc=0;}
  CLxLoc_VertexFeatureService() {_init();set();}
 ~CLxLoc_VertexFeatureService() {}
  void set() {if(!m_loc)m_loc=reinterpret_cast<ILxVertexFeatureServiceID>(lx::GetGlobal(&lx::guid_VertexFeatureService));}
    LxResult
  ScriptQuery (void **ppvObj)
  {
    return m_loc[0]->ScriptQuery (m_loc,ppvObj);
  }
    CLxResult
  ScriptQuery (CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->ScriptQuery(m_loc,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  AllocVertex (void **ppvObj)
  {
    return m_loc[0]->AllocVertex (m_loc,ppvObj);
  }
    CLxResult
  AllocVertex (CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->AllocVertex(m_loc,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  Lookup (LXtID4 type, const char *name, const char **ident)
  {
    return m_loc[0]->Lookup (m_loc,type,name,ident);
  }
    LxResult
  Type (const char *ident, LXtID4 *type)
  {
    return m_loc[0]->Type (m_loc,ident,type);
  }
    LxResult
  Name (const char *ident, const char **name)
  {
    return m_loc[0]->Name (m_loc,ident,name);
  }
    LxResult
  Dimension (const char *ident, unsigned *dimension)
  {
    return m_loc[0]->Dimension (m_loc,ident,dimension);
  }
    LxResult
  VectorType (const char *ident, const char **vecType)
  {
    return m_loc[0]->VectorType (m_loc,ident,vecType);
  }
    LxResult
  DataType (const char *ident, const char **typeName)
  {
    return m_loc[0]->DataType (m_loc,ident,typeName);
  }
    LxResult
  TestCategory (const char *ident, const char *category)
  {
    return m_loc[0]->TestCategory (m_loc,ident,category);
  }
};

class CLxLoc_TableauVertex : public CLxLocalize<ILxTableauVertexID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_TableauVertex() {_init();}
  CLxLoc_TableauVertex(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_TableauVertex(const CLxLoc_TableauVertex &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_TableauVertex;}
    LxResult
  AddFeature (LXtID4 type, const char *name, unsigned int *index)
  {
    return m_loc[0]->AddFeature (m_loc,type,name,index);
  }
    LxResult
  Lookup (LXtID4 type, const char *name, unsigned int *offset)
  {
    return m_loc[0]->Lookup (m_loc,type,name,offset);
  }
    unsigned int
  Size (void)
  {
    return m_loc[0]->Size (m_loc);
  }
    unsigned int
  Count (void)
  {
    return m_loc[0]->Count (m_loc);
  }
    LxResult
  ByIndex (unsigned int index, LXtID4 *type, const char **name, unsigned int *offset)
  {
    return m_loc[0]->ByIndex (m_loc,index,type,name,offset);
  }
};

#endif
