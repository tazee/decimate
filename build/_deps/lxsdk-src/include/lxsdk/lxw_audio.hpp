/*
 * C++ wrapper for lxaudio.h
 *
 *	Copyright 0000
 *
 */
#ifndef LXW_AUDIO_HPP
#define LXW_AUDIO_HPP

#include <lxsdk/lxaudio.h>
#include <lxsdk/lx_wrap.hpp>
#include <string>

namespace lx {
    static const LXtGUID guid_AudioAnimService = {0x42F4A65B,0xA5BE,0x4C77,0x8A,0x66,0x3C,0x1B,0x24,0x5C,0x6B,0xB0};
};

class CLxLoc_AudioAnimService : public CLxLocalizedService
{
public:
  ILxAudioAnimServiceID m_loc;
  void _init() {m_loc=0;}
  CLxLoc_AudioAnimService() {_init();set();}
 ~CLxLoc_AudioAnimService() {}
  void set() {if(!m_loc)m_loc=reinterpret_cast<ILxAudioAnimServiceID>(lx::GetGlobal(&lx::guid_AudioAnimService));}
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
  Sample (double time, unsigned int type, void *value)
  {
    return m_loc[0]->Sample (m_loc,time,type,value);
  }
    int
  Playing (void)
  {
    return m_loc[0]->Playing (m_loc);
  }
    int
  Loop (void)
  {
    return m_loc[0]->Loop (m_loc);
  }
    int
  Mute (void)
  {
    return m_loc[0]->Mute (m_loc);
  }
    int
  Scrub (void)
  {
    return m_loc[0]->Scrub (m_loc);
  }
    double
  Start (void)
  {
    return m_loc[0]->Start (m_loc);
  }
    double
  End (void)
  {
    return m_loc[0]->End (m_loc);
  }
    LxResult
  Audio (void **ppvObj)
  {
    return m_loc[0]->Audio (m_loc,ppvObj);
  }
    CLxResult
  Audio (CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->Audio(m_loc,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  Preview (double startTime, double endTime, void **ppvObj)
  {
    return m_loc[0]->Preview (m_loc,startTime,endTime,ppvObj);
  }
    CLxResult
  Preview (double startTime, double endTime, CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->Preview(m_loc,startTime,endTime,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  ItemSample (ILxUnknownID obj, int loop, double time, unsigned int type, void *value)
  {
    return m_loc[0]->ItemSample (m_loc,(ILxUnknownID)obj,loop,time,type,value);
  }
    LxResult
  ItemAudio (ILxUnknownID obj, void **ppvObj)
  {
    return m_loc[0]->ItemAudio (m_loc,(ILxUnknownID)obj,ppvObj);
  }
    CLxResult
  ItemAudio (ILxUnknownID obj, CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->ItemAudio(m_loc,(ILxUnknownID)obj,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
};

#endif
