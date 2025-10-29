/*
 * C++ wrapper for lxprojdir.h
 *
 *	Copyright 0000
 *
 */
#ifndef LXW_PROJDIR_HPP
#define LXW_PROJDIR_HPP

#include <lxsdk/lxprojdir.h>
#include <lxsdk/lx_wrap.hpp>
#include <string>

namespace lx {
    static const LXtGUID guid_ProjDirOverride = {0x257bac48,0x5e70,0x42a3,0xb5,0xa4,0x7e,0xb3,0x3f,0xdb,0xa1,0x14};
};

class CLxImpl_ProjDirOverride {
  public:
    virtual ~CLxImpl_ProjDirOverride() {}
    virtual LxResult
      pdo_OverrideWith (const char *originalPath, char *buf, unsigned len)
        { return LXe_NOTIMPL; }
};
#define LXxD_ProjDirOverride_OverrideWith LxResult pdo_OverrideWith (const char *originalPath, char *buf, unsigned len)
#define LXxO_ProjDirOverride_OverrideWith LXxD_ProjDirOverride_OverrideWith LXx_OVERRIDE
#define LXxC_ProjDirOverride_OverrideWith(c) LxResult c::pdo_OverrideWith (const char *originalPath, char *buf, unsigned len)
template <class T>
class CLxIfc_ProjDirOverride : public CLxInterface
{
    static LxResult
  OverrideWith (LXtObjectID wcom, const char *originalPath, char *buf, unsigned len)
  {
    LXCWxINST (CLxImpl_ProjDirOverride, loc);
    try {
      return loc->pdo_OverrideWith (originalPath,buf,len);
    } catch (LxResult rc) { return rc; }
  }
  ILxProjDirOverride vt;
public:
  CLxIfc_ProjDirOverride ()
  {
    vt.OverrideWith = OverrideWith;
    vTable = &vt.iunk;
    iid = &lx::guid_ProjDirOverride;
  }
};
class CLxLoc_ProjDirOverride : public CLxLocalize<ILxProjDirOverrideID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_ProjDirOverride() {_init();}
  CLxLoc_ProjDirOverride(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_ProjDirOverride(const CLxLoc_ProjDirOverride &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_ProjDirOverride;}
    LxResult
  OverrideWith (const char *originalPath, char *buf, unsigned len)
  {
    return m_loc[0]->OverrideWith (m_loc,originalPath,buf,len);
  }
    LxResult
  OverrideWith (const char *originalPath, std::string &result)
  {
    LXWx_SBUF1
    rc = m_loc[0]->OverrideWith (m_loc,originalPath,buf,len);
    LXWx_SBUF2
  }
};

#endif
