/*
 * C++ wrapper for lxformsys.h
 *
 *	Copyright 0000
 *
 */
#ifndef LXW_FORMSYS_HPP
#define LXW_FORMSYS_HPP

#include <lxsdk/lxformsys.h>
#include <lxsdk/lx_wrap.hpp>
#include <string>

namespace lx {
    static const LXtGUID guid_FormDeleteEntryDest = {0xa70bedf3,0xbd43,0x40c9,0x9e,0x77,0xa7,0x63,0x2e,0x0f,0x35,0xed};
    static const LXtGUID guid_FormEntryDropPreview = {0x43908515,0xdd63,0x4b36,0xa8,0xca,0x56,0x99,0xd0,0x18,0x82,0x5d};
    static const LXtGUID guid_FormEntryDest = {0xd8c44d68,0xba77,0x4193,0x94,0x2a,0xd5,0x9e,0xe1,0x15,0xd9,0x9f};
};

class CLxLoc_FormDeleteEntryDest : public CLxLocalize<ILxFormDeleteEntryDestID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_FormDeleteEntryDest() {_init();}
  CLxLoc_FormDeleteEntryDest(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_FormDeleteEntryDest(const CLxLoc_FormDeleteEntryDest &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_FormDeleteEntryDest;}
    LxResult
  Delete (const char *hash)
  {
    return m_loc[0]->Delete (m_loc,hash);
  }
};

class CLxLoc_FormEntryDropPreview : public CLxLocalize<ILxFormEntryDropPreviewID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_FormEntryDropPreview() {_init();}
  CLxLoc_FormEntryDropPreview(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_FormEntryDropPreview(const CLxLoc_FormEntryDropPreview &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_FormEntryDropPreview;}
    LxResult
  MarkControl (const char *hash, int syntheticIndex, int pos)
  {
    return m_loc[0]->MarkControl (m_loc,hash,syntheticIndex,pos);
  }
};

class CLxLoc_FormEntryDest : public CLxLocalize<ILxFormEntryDestID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_FormEntryDest() {_init();}
  CLxLoc_FormEntryDest(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_FormEntryDest(const CLxLoc_FormEntryDest &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_FormEntryDest;}
    LxResult
  FormHash (const char **hash)
  {
    return m_loc[0]->FormHash (m_loc,hash);
  }
    LxResult
  ControlHash (const char **hash)
  {
    return m_loc[0]->ControlHash (m_loc,hash);
  }
    LxResult
  SyntheticIndex (int *index)
  {
    return m_loc[0]->SyntheticIndex (m_loc,index);
  }
    LxResult
  Position (int *pos)
  {
    return m_loc[0]->Position (m_loc,pos);
  }
};

#endif
