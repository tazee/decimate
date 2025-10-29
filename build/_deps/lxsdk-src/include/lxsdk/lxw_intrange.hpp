/*
 * C++ wrapper for lxintrange.h
 *
 *	Copyright 0000
 *
 */
#ifndef LXW_INTRANGE_HPP
#define LXW_INTRANGE_HPP

#include <lxsdk/lxintrange.h>
#include <lxsdk/lx_wrap.hpp>
#include <string>

namespace lx {
    static const LXtGUID guid_IntRange = {0xFEAF19EC,0xB819,0x4C46,0xAB,0xA3,0xEC,0x85,0x93,0xF8,0xBE,0x8C};
};

class CLxLoc_IntRange : public CLxLocalize<ILxIntRangeID>
{
public:
  void _init() {m_loc=0;}
  CLxLoc_IntRange() {_init();}
  CLxLoc_IntRange(ILxUnknownID obj) {_init();set(obj);}
  CLxLoc_IntRange(const CLxLoc_IntRange &other) {_init();set(other.m_loc);}
  const LXtGUID * guid() const {return &lx::guid_IntRange;}
    LxResult
  AllBefore (void) const
  {
    return m_loc[0]->AllBefore (m_loc);
  }
    LxResult
  AllAfter (void) const
  {
    return m_loc[0]->AllAfter (m_loc);
  }
    LxResult
  Next (int *i)
  {
    return m_loc[0]->Next (m_loc,i);
  }
    LxResult
  Prev (int *i)
  {
    return m_loc[0]->Prev (m_loc,i);
  }
    LxResult
  Min (int *min)
  {
    return m_loc[0]->Min (m_loc,min);
  }
    LxResult
  Max (int *max)
  {
    return m_loc[0]->Max (m_loc,max);
  }
    LxResult
  First (int *first)
  {
    return m_loc[0]->First (m_loc,first);
  }
    LxResult
  Last (int *last)
  {
    return m_loc[0]->Last (m_loc,last);
  }
    LxResult
  Current (int *current) const
  {
    return m_loc[0]->Current (m_loc,current);
  }
    LxResult
  Test (int i) const
  {
    return m_loc[0]->Test (m_loc,i);
  }
};

#endif
