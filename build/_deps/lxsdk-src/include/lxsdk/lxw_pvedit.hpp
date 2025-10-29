/*
 * C++ wrapper for lxpvedit.h
 *
 *	Copyright 0000
 *
 */
#ifndef LXW_PVEDIT_HPP
#define LXW_PVEDIT_HPP

#include <lxsdk/lxpvedit.h>
#include <lxsdk/lx_wrap.hpp>
#include <string>

namespace lx {
    static const LXtGUID guid_SolidDrill = {0x0D6D30C6,0x0DEB,0x4848,0xA9,0xF9,0x73,0x14,0x72,0x25,0x5B,0xA9};
    static const LXtGUID guid_PolygonEdit = {0x3EA0362E,0x61B6,0x4BFE,0xA9,0x8B,0xCE,0x72,0x22,0x1F,0xC9,0x6B};
    static const LXtGUID guid_PolygonSlice = {0x99C623EE,0x0873,0x4DB5,0x90,0xDF,0xCC,0x94,0x60,0xDA,0x8F,0xA8};
};

class CLxLoc_SolidDrill : public CLxLocalize<ILxSolidDrillID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_SolidDrill() {_init();}
  CLxLoc_SolidDrill(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_SolidDrill(const CLxLoc_SolidDrill &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_SolidDrill;}
    LxResult
  Clear (void)
  {
    return m_loc[0]->Clear (m_loc);
  }
    LxResult
  AddMesh (ILxUnknownID meshObj, const LXtMatrix4 xfrm)
  {
    return m_loc[0]->AddMesh (m_loc,(ILxUnknownID)meshObj,xfrm);
  }
    LxResult
  Execute (ILxUnknownID meshObj, const LXtMatrix4 xfrm, LXtMarkMode pick, unsigned int mode, const char *sten, ILxUnknownID monitor)
  {
    return m_loc[0]->Execute (m_loc,(ILxUnknownID)meshObj,xfrm,pick,mode,sten,(ILxUnknownID)monitor);
  }
    LxResult
  UnifyDrivers (bool solid)
  {
    return m_loc[0]->UnifyDrivers (m_loc,solid);
  }
    LxResult
  UnionAll (const LXtMatrix4 xfrm, LXtMarkMode pick, const char *sset, void **ppvObj)
  {
    return m_loc[0]->UnionAll (m_loc,xfrm,pick,sset,ppvObj);
  }
    CLxResult
  UnionAll (const LXtMatrix4 xfrm, LXtMarkMode pick, const char *sset, CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->UnionAll(m_loc,xfrm,pick,sset,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
};

class CLxLoc_PolygonEdit : public CLxLocalize<ILxPolygonEditID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_PolygonEdit() {_init();}
  CLxLoc_PolygonEdit(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_PolygonEdit(const CLxLoc_PolygonEdit &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_PolygonEdit;}
    LxResult
  SetMesh (ILxUnknownID meshObj, ILxUnknownID baseObj)
  {
    return m_loc[0]->SetMesh (m_loc,(ILxUnknownID)meshObj,(ILxUnknownID)baseObj);
  }
    LxResult
  SetPolygon (ILxUnknownID meshObj, LXtPolygonID polygonID)
  {
    return m_loc[0]->SetPolygon (m_loc,(ILxUnknownID)meshObj,polygonID);
  }
    LxResult
  SetSearchBox (const LXtBBox *box, double tol)
  {
    return m_loc[0]->SetSearchBox (m_loc,box,tol);
  }
    LxResult
  SetSearch (int search, double tol)
  {
    return m_loc[0]->SetSearch (m_loc,search,tol);
  }
    LxResult
  SetSearchPolygon (LXtPolygonID polygonID, bool search)
  {
    return m_loc[0]->SetSearchPolygon (m_loc,polygonID,search);
  }
    LxResult
  ClearSearch (void)
  {
    return m_loc[0]->ClearSearch (m_loc);
  }
    LxResult
  NewVertex (const LXtVector pos, LXtPointID *pointID)
  {
    return m_loc[0]->NewVertex (m_loc,pos,pointID);
  }
    LxResult
  FindVertex (const LXtVector pos, LXtPointID *pointID)
  {
    return m_loc[0]->FindVertex (m_loc,pos,pointID);
  }
    LxResult
  NewLoneVertex (const LXtVector pos, LXtPointID *pointID)
  {
    return m_loc[0]->NewLoneVertex (m_loc,pos,pointID);
  }
    LxResult
  CopyVertex (LXtPointID sourceID, const LXtVector pos, LXtPointID *pointID)
  {
    return m_loc[0]->CopyVertex (m_loc,sourceID,pos,pointID);
  }
    LxResult
  CopyVrtOfPol (LXtPointID sourceID, LXtPolygonID polygonID, const LXtVector pos, LXtPointID *pointID)
  {
    return m_loc[0]->CopyVrtOfPol (m_loc,sourceID,polygonID,pos,pointID);
  }
    LxResult
  AddMiddleVertex (const LXtVector pos, LXtPointID v1, LXtPointID v2, double f, LXtPointID *pointID)
  {
    return m_loc[0]->AddMiddleVertex (m_loc,pos,v1,v2,f,pointID);
  }
    LxResult
  AddFaceVertex (const LXtVector pos, LXtPolygonID polygonID, const double *vwt, LXtPointID *pointID)
  {
    return m_loc[0]->AddFaceVertex (m_loc,pos,polygonID,vwt,pointID);
  }
    LxResult
  CopyPolygon (ILxUnknownID meshObj, LXtPolygonID sourceID, unsigned int rev, LXtPolygonID *polygonID)
  {
    return m_loc[0]->CopyPolygon (m_loc,(ILxUnknownID)meshObj,sourceID,rev,polygonID);
  }
    LxResult
  RenewVList (LXtPolygonID sourceID, unsigned int nvert, const LXtPointID *verts, unsigned int rev, LXtPolygonID *polygonID)
  {
    return m_loc[0]->RenewVList (m_loc,sourceID,nvert,verts,rev,polygonID);
  }
    LxResult
  NewPolygon (LXtID4 type, unsigned int nvert, const LXtPointID *verts, unsigned int rev, LXtPolygonID *polygonID)
  {
    return m_loc[0]->NewPolygon (m_loc,type,nvert,verts,rev,polygonID);
  }
    LxResult
  DeletePolygon (LXtPolygonID polygonID)
  {
    return m_loc[0]->DeletePolygon (m_loc,polygonID);
  }
    LxResult
  RemoveEdges (LXtMarkMode pick, bool keepVertices)
  {
    return m_loc[0]->RemoveEdges (m_loc,pick,keepVertices);
  }
    LxResult
  UnwrapStart (LXtPolygonID polygonID)
  {
    return m_loc[0]->UnwrapStart (m_loc,polygonID);
  }
    LxResult
  UnwrapAddEdge (LXtPointID v0, LXtPointID v1, bool both)
  {
    return m_loc[0]->UnwrapAddEdge (m_loc,v0,v1,both);
  }
    LxResult
  UnwrapDone (const LXtVector axis, unsigned int *npol)
  {
    return m_loc[0]->UnwrapDone (m_loc,axis,npol);
  }
    LxResult
  UnwrapPolygonByIndex (unsigned int index, LXtPolygonID *polygonID)
  {
    return m_loc[0]->UnwrapPolygonByIndex (m_loc,index,polygonID);
  }
};

class CLxLoc_PolygonSlice : public CLxLocalize<ILxPolygonSliceID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_PolygonSlice() {_init();}
  CLxLoc_PolygonSlice(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_PolygonSlice(const CLxLoc_PolygonSlice &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_PolygonSlice;}
    LxResult
  SetAxis (unsigned axis)
  {
    return m_loc[0]->SetAxis (m_loc,axis);
  }
    LxResult
  SetAxisByVector (const LXtVector axis)
  {
    return m_loc[0]->SetAxisByVector (m_loc,axis);
  }
    LxResult
  Start (ILxUnknownID meshObj, LXtPolygonID pol, unsigned setAxis)
  {
    return m_loc[0]->Start (m_loc,(ILxUnknownID)meshObj,pol,setAxis);
  }
    LxResult
  ByLine (const LXtVector pos0, const LXtVector pos1)
  {
    return m_loc[0]->ByLine (m_loc,pos0,pos1);
  }
    int
  Done (void)
  {
    return m_loc[0]->Done (m_loc);
  }
};

#endif
