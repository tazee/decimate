/*
 * C++ wrapper for lxviewobject.h
 *
 *	Copyright 0000
 *
 */
#ifndef LXW_VIEWOBJECT_HPP
#define LXW_VIEWOBJECT_HPP

#include <lxsdk/lxviewobject.h>
#include <lxsdk/lx_wrap.hpp>
#include <string>

namespace lx {
    static const LXtGUID guid_ViewObject = {0x81C986D1,0xA7BA,0x4278,0xB0,0x60,0xE7,0x84,0x77,0x9F,0x5C,0x36};
};

class CLxImpl_ViewObject {
  public:
    virtual ~CLxImpl_ViewObject() {}
    virtual LxResult
      viewobj_TestMode (unsigned int type)
        { return LXe_NOTAVAILABLE; }
    virtual unsigned int
      viewobj_Flags (void)
        { return 0; }
    virtual void
      viewobj_Generate (unsigned int type)
        { }
    virtual unsigned int
      viewobj_Count (unsigned int type)
        { return 0; }
    virtual LxResult
      viewobj_ByIndex (unsigned int type, unsigned int index, void **ppvObj)
        { return LXe_NOTIMPL; }
    virtual LxResult
      viewobj_ByView (ILxUnknownID view, void **ppvObj)
        { return LXe_NOTIMPL; }
};
#define LXxD_ViewObject_TestMode LxResult viewobj_TestMode (unsigned int type)
#define LXxO_ViewObject_TestMode LXxD_ViewObject_TestMode LXx_OVERRIDE
#define LXxC_ViewObject_TestMode(c) LxResult c::viewobj_TestMode (unsigned int type)
#define LXxD_ViewObject_Flags unsigned int viewobj_Flags (void)
#define LXxO_ViewObject_Flags LXxD_ViewObject_Flags LXx_OVERRIDE
#define LXxC_ViewObject_Flags(c) unsigned int c::viewobj_Flags (void)
#define LXxD_ViewObject_Generate void viewobj_Generate (unsigned int type)
#define LXxO_ViewObject_Generate LXxD_ViewObject_Generate LXx_OVERRIDE
#define LXxC_ViewObject_Generate(c) void c::viewobj_Generate (unsigned int type)
#define LXxD_ViewObject_Count unsigned int viewobj_Count (unsigned int type)
#define LXxO_ViewObject_Count LXxD_ViewObject_Count LXx_OVERRIDE
#define LXxC_ViewObject_Count(c) unsigned int c::viewobj_Count (unsigned int type)
#define LXxD_ViewObject_ByIndex LxResult viewobj_ByIndex (unsigned int type, unsigned int index, void **ppvObj)
#define LXxO_ViewObject_ByIndex LXxD_ViewObject_ByIndex LXx_OVERRIDE
#define LXxC_ViewObject_ByIndex(c) LxResult c::viewobj_ByIndex (unsigned int type, unsigned int index, void **ppvObj)
#define LXxD_ViewObject_ByView LxResult viewobj_ByView (ILxUnknownID view, void **ppvObj)
#define LXxO_ViewObject_ByView LXxD_ViewObject_ByView LXx_OVERRIDE
#define LXxC_ViewObject_ByView(c) LxResult c::viewobj_ByView (ILxUnknownID view, void **ppvObj)
template <class T>
class CLxIfc_ViewObject : public CLxInterface
{
    static LxResult
  TestMode (LXtObjectID wcom, unsigned int type)
  {
    LXCWxINST (CLxImpl_ViewObject, loc);
    try {
      return loc->viewobj_TestMode (type);
    } catch (LxResult rc) { return rc; }
  }
    static unsigned int
  Flags (LXtObjectID wcom)
  {
    LXCWxINST (CLxImpl_ViewObject, loc);
    return loc->viewobj_Flags ();
  }
    static void
  Generate (LXtObjectID wcom, unsigned int type)
  {
    LXCWxINST (CLxImpl_ViewObject, loc);
    loc->viewobj_Generate (type);
  }
    static unsigned int
  Count (LXtObjectID wcom, unsigned int type)
  {
    LXCWxINST (CLxImpl_ViewObject, loc);
    return loc->viewobj_Count (type);
  }
    static LxResult
  ByIndex (LXtObjectID wcom, unsigned int type, unsigned int index, void **ppvObj)
  {
    LXCWxINST (CLxImpl_ViewObject, loc);
    try {
      return loc->viewobj_ByIndex (type,index,ppvObj);
    } catch (LxResult rc) { return rc; }
  }
    static LxResult
  ByView (LXtObjectID wcom, LXtObjectID view, void **ppvObj)
  {
    LXCWxINST (CLxImpl_ViewObject, loc);
    try {
      return loc->viewobj_ByView ((ILxUnknownID)view,ppvObj);
    } catch (LxResult rc) { return rc; }
  }
  ILxViewObject vt;
public:
  CLxIfc_ViewObject ()
  {
    vt.TestMode = TestMode;
    vt.Flags = Flags;
    vt.Generate = Generate;
    vt.Count = Count;
    vt.ByIndex = ByIndex;
    vt.ByView = ByView;
    vTable = &vt.iunk;
    iid = &lx::guid_ViewObject;
  }
};
class CLxLoc_ViewObject : public CLxLocalize<ILxViewObjectID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_ViewObject() {_init();}
  CLxLoc_ViewObject(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_ViewObject(const CLxLoc_ViewObject &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_ViewObject;}
    LxResult
  TestMode (unsigned int type)
  {
    return m_loc[0]->TestMode (m_loc,type);
  }
    unsigned int
  Flags (void)
  {
    return m_loc[0]->Flags (m_loc);
  }
    void
  Generate (unsigned int type)
  {
    m_loc[0]->Generate (m_loc,type);
  }
    unsigned int
  Count (unsigned int type)
  {
    return m_loc[0]->Count (m_loc,type);
  }
    LxResult
  ByIndex (unsigned int type, unsigned int index, void **ppvObj)
  {
    return m_loc[0]->ByIndex (m_loc,type,index,ppvObj);
  }
    CLxResult
  ByIndex (unsigned int type, unsigned int index, CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->ByIndex(m_loc,type,index,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  ByView (ILxUnknownID view, void **ppvObj)
  {
    return m_loc[0]->ByView (m_loc,(ILxUnknownID)view,ppvObj);
  }
    CLxResult
  ByView (ILxUnknownID view, CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->ByView(m_loc,(ILxUnknownID)view,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
};

#endif
