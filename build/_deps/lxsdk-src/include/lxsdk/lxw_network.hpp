/*
 * C++ wrapper for lxnetwork.h
 *
 *	Copyright 0000
 *
 */
#ifndef LXW_NETWORK_HPP
#define LXW_NETWORK_HPP

#include <lxsdk/lxnetwork.h>
#include <lxsdk/lx_wrap.hpp>
#include <string>

namespace lx {
    static const LXtGUID guid_NetworkService = {0x333d439d,0xcfc8,0x43e6,0xad,0x3c,0x1e,0x2b,0x6f,0xda,0x27,0xfb};
};

class CLxLoc_NetworkService : public CLxLocalizedService
{
public:
  ILxNetworkServiceID m_loc;
  void _init() {m_loc=0;}
  CLxLoc_NetworkService() {_init();set();}
 ~CLxLoc_NetworkService() {}
  void set() {if(!m_loc)m_loc=reinterpret_cast<ILxNetworkServiceID>(lx::GetGlobal(&lx::guid_NetworkService));}
    LxResult
  ScriptQuery (void **ppvObj)
  {
    return m_loc[0]->ScriptQuery (m_loc,ppvObj);
  }
    CLxResult
  ScriptQuery (CLxLocalizedObject &o_dest)
  {
    LXtObjectID o_obj;
    LxResult r_c = m_loc[0]->ScriptQuery(m_loc,&o_obj);
    return lx::TakeResult(o_dest,r_c,o_obj);
  }
    LxResult
  OneOffHostListAdd (const char *hostname, LXtIP4 ip, int port)
  {
    return m_loc[0]->OneOffHostListAdd (m_loc,hostname,ip,port);
  }
    LxResult
  OneOffHostListRemove (const char *hostname, LXtIP4 ip, int port)
  {
    return m_loc[0]->OneOffHostListRemove (m_loc,hostname,ip,port);
  }
};

#endif
