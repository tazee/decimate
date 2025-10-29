/*
 * C++ wrapper for lxactionclip.h
 *
 *	Copyright 0000
 *
 */
#ifndef LXW_ACTIONCLIP_HPP
#define LXW_ACTIONCLIP_HPP

#include <lxsdk/lxactionclip.h>
#include <lxsdk/lx_wrap.hpp>
#include <string>

namespace lx {
    static const LXtGUID guid_ActionClip = {0xA312921B,0x41D2,0x4D2A,0x86,0x78,0xC9,0x0E,0xEA,0x38,0x1F,0xAE};
};

class CLxLoc_ActionClip : public CLxLocalize<ILxActionClipID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_ActionClip() {_init();}
  CLxLoc_ActionClip(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_ActionClip(const CLxLoc_ActionClip &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_ActionClip;}
    LxResult
  SetParenting (ILxUnknownID group)
  {
    return m_loc[0]->SetParenting (m_loc,(ILxUnknownID)group);
  }
    LxResult
  SetActive (int state)
  {
    return m_loc[0]->SetActive (m_loc,state);
  }
    unsigned
  Active (void)
  {
    return m_loc[0]->Active (m_loc);
  }
    LxResult
  SetEnabled (int state)
  {
    return m_loc[0]->SetEnabled (m_loc,state);
  }
    unsigned
  Enabled (void)
  {
    return m_loc[0]->Enabled (m_loc);
  }
    LxResult
  Action (double time, void **ppvObj)
  {
    return m_loc[0]->Action (m_loc,time,ppvObj);
  }
    CLxResult
  Action (double time, CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->Action(m_loc,time,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  Extents (double *timeS, double *timeE, int layers)
  {
    return m_loc[0]->Extents (m_loc,timeS,timeE,layers);
  }
    LxResult
  Create (void)
  {
    return m_loc[0]->Create (m_loc);
  }
};

#endif
