/*
 * C++ wrapper for lxpattern.h
 *
 *	Copyright 0000
 *
 */
#ifndef LXW_PATTERN_HPP
#define LXW_PATTERN_HPP

#include <lxsdk/lxpattern.h>
#include <lxsdk/lx_wrap.hpp>
#include <string>

namespace lx {
    static const LXtGUID guid_Pattern = {0x16EE409B,0x6B63,0x4CB0,0x8D,0xE6,0xC3,0xD9,0xAD,0x12,0x4D,0xBE};
};

class CLxLoc_Pattern : public CLxLocalize<ILxPatternID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_Pattern() {_init();}
  CLxLoc_Pattern(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_Pattern(const CLxLoc_Pattern &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_Pattern;}
    LxResult
  Test (int index, int offset) const
  {
    return m_loc[0]->Test (m_loc,index,offset);
  }
    LxResult
  GenerateSequenceString (int start, int end, char *buf, unsigned int len) const
  {
    return m_loc[0]->GenerateSequenceString (m_loc,start,end,buf,len);
  }
    LxResult
  GenerateSequenceString (int start, int end, std::string &result) const
  {
    LXWx_SBUF1
    rc = m_loc[0]->GenerateSequenceString (m_loc,start,end,buf,len);
    LXWx_SBUF2
  }
};

#endif
