/*
 * C++ wrapper for lxlocator.h
 *
 *	Copyright 0000
 *
 */
#ifndef LXW_LOCATOR_HPP
#define LXW_LOCATOR_HPP

#include <lxsdk/lxlocator.h>
#include <lxsdk/lx_wrap.hpp>
#include <string>

namespace lx {
    static const LXtGUID guid_Locator = {0x50DCCB9D,0x9856,0x4A33,0x9B,0xDA,0xDA,0xF3,0xA7,0x1B,0xBD,0x2D};
};

class CLxLoc_Locator : public CLxLocalize<ILxLocatorID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_Locator() {_init();}
  CLxLoc_Locator(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_Locator(const CLxLoc_Locator &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_Locator;}
    LxResult
  Visible (ILxUnknownID chanRead) const
  {
    return m_loc[0]->Visible (m_loc,(ILxUnknownID)chanRead);
  }
    LxResult
  WorldTransform (ILxUnknownID chanRead, LXtMatrix xfrm, LXtVector pos) const
  {
    return m_loc[0]->WorldTransform (m_loc,(ILxUnknownID)chanRead,xfrm,pos);
  }
    LxResult
  GetTransformItem (LXtTransformType type, void **ppvObj) const
  {
    return m_loc[0]->GetTransformItem (m_loc,type,ppvObj);
  }
    CLxResult
  GetTransformItem (LXtTransformType type, CLxLocalizedObject &o_dest) const
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->GetTransformItem(m_loc,type,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  AddTransformItem (LXtTransformType type, void **ppvObj, unsigned *index)
  {
    return m_loc[0]->AddTransformItem (m_loc,type,ppvObj,index);
  }
    CLxResult
  AddTransformItem (LXtTransformType type, CLxLocalizedObject &o_dest, unsigned *index)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->AddTransformItem(m_loc,type,&o_obj,index);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  SetTransformVector (ILxUnknownID chanWrite, LXtTransformType type, const LXtVector value)
  {
    return m_loc[0]->SetTransformVector (m_loc,(ILxUnknownID)chanWrite,type,value);
  }
    LxResult
  AddPreTransformItem (ILxUnknownID chanWrite, LXtTransformType type, const LXtVector value, void **ppvObj, unsigned *index)
  {
    return m_loc[0]->AddPreTransformItem (m_loc,(ILxUnknownID)chanWrite,type,value,ppvObj,index);
  }
    CLxResult
  AddPreTransformItem (ILxUnknownID chanWrite, LXtTransformType type, const LXtVector value, CLxLocalizedObject &o_dest, unsigned *index)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->AddPreTransformItem(m_loc,(ILxUnknownID)chanWrite,type,value,&o_obj,index);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  AddPostTransformItem (ILxUnknownID chanWrite, LXtTransformType type, const LXtVector value, void **ppvObj, unsigned *index)
  {
    return m_loc[0]->AddPostTransformItem (m_loc,(ILxUnknownID)chanWrite,type,value,ppvObj,index);
  }
    CLxResult
  AddPostTransformItem (ILxUnknownID chanWrite, LXtTransformType type, const LXtVector value, CLxLocalizedObject &o_dest, unsigned *index)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->AddPostTransformItem(m_loc,(ILxUnknownID)chanWrite,type,value,&o_obj,index);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  SetTarget (ILxUnknownID target, void **ppvObj)
  {
    return m_loc[0]->SetTarget (m_loc,(ILxUnknownID)target,ppvObj);
  }
    CLxResult
  SetTarget (ILxUnknownID target, CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->SetTarget(m_loc,(ILxUnknownID)target,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  PrependTransformItem (ILxUnknownID chanWrite, LXtTransformType type, const LXtVector value, void **ppvObj, unsigned *index)
  {
    return m_loc[0]->PrependTransformItem (m_loc,(ILxUnknownID)chanWrite,type,value,ppvObj,index);
  }
    CLxResult
  PrependTransformItem (ILxUnknownID chanWrite, LXtTransformType type, const LXtVector value, CLxLocalizedObject &o_dest, unsigned *index)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->PrependTransformItem(m_loc,(ILxUnknownID)chanWrite,type,value,&o_obj,index);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  AppendTransformItem (ILxUnknownID chanWrite, LXtTransformType type, const LXtVector value, void **ppvObj, unsigned *index)
  {
    return m_loc[0]->AppendTransformItem (m_loc,(ILxUnknownID)chanWrite,type,value,ppvObj,index);
  }
    CLxResult
  AppendTransformItem (ILxUnknownID chanWrite, LXtTransformType type, const LXtVector value, CLxLocalizedObject &o_dest, unsigned *index)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->AppendTransformItem(m_loc,(ILxUnknownID)chanWrite,type,value,&o_obj,index);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  SetPosition (ILxUnknownID chanRead, ILxUnknownID chanWrite, const LXtVector pos, unsigned type, unsigned comp)
  {
    return m_loc[0]->SetPosition (m_loc,(ILxUnknownID)chanRead,(ILxUnknownID)chanWrite,pos,type,comp);
  }
    LxResult
  SetRotation (ILxUnknownID chanRead, ILxUnknownID chanWrite, const LXtMatrix m3, unsigned type, unsigned comp)
  {
    return m_loc[0]->SetRotation (m_loc,(ILxUnknownID)chanRead,(ILxUnknownID)chanWrite,m3,type,comp);
  }
    LxResult
  SetScale (ILxUnknownID chanRead, ILxUnknownID chanWrite, const LXtMatrix4 m4, unsigned type, unsigned comp)
  {
    return m_loc[0]->SetScale (m_loc,(ILxUnknownID)chanRead,(ILxUnknownID)chanWrite,m4,type,comp);
  }
    LxResult
  ExtractLocalPosition (ILxUnknownID chanRead, LXtVector pos)
  {
    return m_loc[0]->ExtractLocalPosition (m_loc,(ILxUnknownID)chanRead,pos);
  }
    LxResult
  ExtractLocalRotation (ILxUnknownID chanRead, LXtMatrix m3)
  {
    return m_loc[0]->ExtractLocalRotation (m_loc,(ILxUnknownID)chanRead,m3);
  }
    LxResult
  ZeroTransform (ILxUnknownID chanRead, ILxUnknownID chanWrite, LXtTransformType type)
  {
    return m_loc[0]->ZeroTransform (m_loc,(ILxUnknownID)chanRead,(ILxUnknownID)chanWrite,type);
  }
    LxResult
  WorldTransform4 (ILxUnknownID chanRead, LXtMatrix4 xfrm4) const
  {
    return m_loc[0]->WorldTransform4 (m_loc,(ILxUnknownID)chanRead,xfrm4);
  }
    LxResult
  WorldInvertTransform (ILxUnknownID chanRead, LXtMatrix xfrm, LXtVector pos)
  {
    return m_loc[0]->WorldInvertTransform (m_loc,(ILxUnknownID)chanRead,xfrm,pos);
  }
    LxResult
  LocalTransform (ILxUnknownID chanRead, LXtMatrix xfrm, LXtVector pos) const
  {
    return m_loc[0]->LocalTransform (m_loc,(ILxUnknownID)chanRead,xfrm,pos);
  }
    LxResult
  LocalTransform4 (ILxUnknownID chanRead, LXtMatrix4 xfrm4) const
  {
    return m_loc[0]->LocalTransform4 (m_loc,(ILxUnknownID)chanRead,xfrm4);
  }
};

#endif
