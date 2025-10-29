/*
 * C++ wrapper for lxproxy.h
 *
 *	Copyright 0000
 *
 */
#ifndef LXW_PROXY_HPP
#define LXW_PROXY_HPP

#include <lxsdk/lxproxy.h>
#include <lxsdk/lx_wrap.hpp>
#include <string>

namespace lx {
    static const LXtGUID guid_SceneContents = {0x89448742,0xE996,0x11E1,0x89,0x7D,0x24,0xD8,0x61,0x88,0x70,0x9B};
    static const LXtGUID guid_ProxyOptions = {0x68ACA4BE,0xA274,0x4D04,0xA5,0xC9,0x85,0xAB,0xFA,0x27,0xD2,0xE6};
};

class CLxLoc_SceneContents : public CLxLocalize<ILxSceneContentsID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_SceneContents() {_init();}
  CLxLoc_SceneContents(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_SceneContents(const CLxLoc_SceneContents &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_SceneContents;}
    LxResult
  Item (unsigned int type, const char *internal_name, const char *user_name)
  {
    return m_loc[0]->Item (m_loc,type,internal_name,user_name);
  }
    LxResult
  SetBBox (double min_X, double min_Y, double min_Z, double max_X, double max_Y, double max_Z)
  {
    return m_loc[0]->SetBBox (m_loc,min_X,min_Y,min_Z,max_X,max_Y,max_Z);
  }
    LxResult
  AddLink (unsigned int type, const char *internal_name, const char *user_name)
  {
    return m_loc[0]->AddLink (m_loc,type,internal_name,user_name);
  }
    unsigned int
  Count (void)
  {
    return m_loc[0]->Count (m_loc);
  }
    LxResult
  InternalName (unsigned int index, char *buf, unsigned len)
  {
    return m_loc[0]->InternalName (m_loc,index,buf,len);
  }
    LxResult
  InternalName (unsigned int index, std::string &result)
  {
    LXWx_SBUF1
    rc = m_loc[0]->InternalName (m_loc,index,buf,len);
    LXWx_SBUF2
  }
    LxResult
  UserName (unsigned int index, char *buf, unsigned len)
  {
    return m_loc[0]->UserName (m_loc,index,buf,len);
  }
    LxResult
  UserName (unsigned int index, std::string &result)
  {
    LXWx_SBUF1
    rc = m_loc[0]->UserName (m_loc,index,buf,len);
    LXWx_SBUF2
  }
    LxResult
  BBox (unsigned int index, LXtBBox *box)
  {
    return m_loc[0]->BBox (m_loc,index,box);
  }
    LxResult
  Type (unsigned int index, unsigned int *type)
  {
    return m_loc[0]->Type (m_loc,index,type);
  }
    LxResult
  ByInternalName (const char *internal_name, unsigned int *index)
  {
    return m_loc[0]->ByInternalName (m_loc,internal_name,index);
  }
    LxResult
  ByUserName (const char *user_name, unsigned int *index)
  {
    return m_loc[0]->ByUserName (m_loc,user_name,index);
  }
    LxResult
  Reset (void)
  {
    return m_loc[0]->Reset (m_loc);
  }
    unsigned int
  LinkCount (unsigned int index)
  {
    return m_loc[0]->LinkCount (m_loc,index);
  }
};

class CLxLoc_ProxyOptions : public CLxLocalize<ILxProxyOptionsID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_ProxyOptions() {_init();}
  CLxLoc_ProxyOptions(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_ProxyOptions(const CLxLoc_ProxyOptions &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_ProxyOptions;}
    LxResult
  LoadList (void)
  {
    return m_loc[0]->LoadList (m_loc);
  }
    LxResult
  LoadNone (void)
  {
    return m_loc[0]->LoadNone (m_loc);
  }
    LxResult
  AddItem (unsigned int type, const char *internal_name, const char *user_name)
  {
    return m_loc[0]->AddItem (m_loc,type,internal_name,user_name);
  }
    LxResult
  SetFlags (unsigned int flags)
  {
    return m_loc[0]->SetFlags (m_loc,flags);
  }
};

#endif
