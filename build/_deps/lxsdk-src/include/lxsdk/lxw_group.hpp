/*
 * C++ wrapper for lxgroup.h
 *
 *	Copyright 0000
 *
 */
#ifndef LXW_GROUP_HPP
#define LXW_GROUP_HPP

#include <lxsdk/lxgroup.h>
#include <lxsdk/lx_wrap.hpp>
#include <string>

namespace lx {
    static const LXtGUID guid_PresetPackageService = {0xb37c7c4d,0xe91f,0x406b,0xa8,0x0b,0x8d,0x76,0x1a,0x62,0x30,0x88};
    static const LXtGUID guid_GroupItem = {0x47FDFD87,0x3FBA,0x41A4,0x80,0x44,0x74,0xEC,0x9D,0x9A,0xA9,0x86};
    static const LXtGUID guid_GroupEnumerator = {0xCA4E1FE0,0xB655,0x429F,0x96,0x74,0x3E,0x1D,0x7D,0xEA,0x5D,0x04};
};

class CLxLoc_PresetPackageService : public CLxLocalizedService
{
public:
  ILxPresetPackageServiceID m_loc;
  void _init() {m_loc=0;}
  CLxLoc_PresetPackageService() {_init();set();}
 ~CLxLoc_PresetPackageService() {}
  void set() {if(!m_loc)m_loc=reinterpret_cast<ILxPresetPackageServiceID>(lx::GetGlobal(&lx::guid_PresetPackageService));}
    const char*
  Version (const char *pkg)
  {
    return m_loc[0]->Version (m_loc,pkg);
  }
    const char*
  UserVersion (const char *pkg)
  {
    return m_loc[0]->UserVersion (m_loc,pkg);
  }
    const char*
  Type (const char *pkg)
  {
    return m_loc[0]->Type (m_loc,pkg);
  }
    unsigned
  AssemblyType (const char *pkg)
  {
    return m_loc[0]->AssemblyType (m_loc,pkg);
  }
    unsigned
  CountFiles (const char *pkg)
  {
    return m_loc[0]->CountFiles (m_loc,pkg);
  }
    LxResult
  Filename (const char *pkg, unsigned id, char *buf, int len)
  {
    return m_loc[0]->Filename (m_loc,pkg,id,buf,len);
  }
    LxResult
  HasFile (const char *pkg, const char *file)
  {
    return m_loc[0]->HasFile (m_loc,pkg,file);
  }
    LxResult
  Extract (const char *pkg, const char *file, const char *dest)
  {
    return m_loc[0]->Extract (m_loc,pkg,file,dest);
  }
    LxResult
  ExtractAll (const char *pkg, const char *dest, const char *subDir)
  {
    return m_loc[0]->ExtractAll (m_loc,pkg,dest,subDir);
  }
    LxResult
  AddFile (const char *pkg, const char *filename, const char *filepath)
  {
    return m_loc[0]->AddFile (m_loc,pkg,filename,filepath);
  }
    LxResult
  RemoveFile (const char *pkg, const char *filename)
  {
    return m_loc[0]->RemoveFile (m_loc,pkg,filename);
  }
    LxResult
  ReplaceFile (const char *pkg, const char *filename, const char *filepath)
  {
    return m_loc[0]->ReplaceFile (m_loc,pkg,filename,filepath);
  }
};

class CLxLoc_GroupItem : public CLxLocalize<ILxGroupItemID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_GroupItem() {_init();}
  CLxLoc_GroupItem(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_GroupItem(const CLxLoc_GroupItem &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_GroupItem;}
    LxResult
  Enumerator (void **ppvObj)
  {
    return m_loc[0]->Enumerator (m_loc,ppvObj);
  }
    CLxResult
  Enumerator (CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->Enumerator(m_loc,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  Type (int *type)
  {
    return m_loc[0]->Type (m_loc,type);
  }
    LxResult
  SetType (int type)
  {
    return m_loc[0]->SetType (m_loc,type);
  }
};

class CLxLoc_GroupEnumerator : public CLxLocalize<ILxGroupEnumeratorID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_GroupEnumerator() {_init();}
  CLxLoc_GroupEnumerator(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_GroupEnumerator(const CLxLoc_GroupEnumerator &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_GroupEnumerator;}
    LxResult
  Enumerate (ILxUnknownID visitor, unsigned mask)
  {
    return m_loc[0]->Enumerate (m_loc,(ILxUnknownID)visitor,mask);
  }
    unsigned
  Type (void)
  {
    return m_loc[0]->Type (m_loc);
  }
    LxResult
  Item (void **ppvObj)
  {
    return m_loc[0]->Item (m_loc,ppvObj);
  }
    CLxResult
  Item (CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->Item(m_loc,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  Channel (void **ppvObj, unsigned *index)
  {
    return m_loc[0]->Channel (m_loc,ppvObj,index);
  }
    CLxResult
  Channel (CLxLocalizedObject &o_dest, unsigned *index)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->Channel(m_loc,&o_obj,index);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
};

#endif
