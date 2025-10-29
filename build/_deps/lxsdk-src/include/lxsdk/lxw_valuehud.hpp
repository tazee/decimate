/*
 * C++ wrapper for lxvaluehud.h
 *
 *	Copyright 0000
 *
 */
#ifndef LXW_VALUEHUD_HPP
#define LXW_VALUEHUD_HPP

#include <lxsdk/lxvaluehud.h>
#include <lxsdk/lx_wrap.hpp>
#include <string>

namespace lx {
    static const LXtGUID guid_ValueHUDService = {0x7ed5ebf7,0x578c,0x4cf6,0x98,0xe7,0x2e,0x0a,0xf5,0x3e,0x56,0xfa};
};

class CLxLoc_ValueHUDService : public CLxLocalizedService
{
public:
  ILxValueHUDServiceID m_loc;
  void _init() {m_loc=0;}
  CLxLoc_ValueHUDService() {_init();set();}
 ~CLxLoc_ValueHUDService() {}
  void set() {if(!m_loc)m_loc=reinterpret_cast<ILxValueHUDServiceID>(lx::GetGlobal(&lx::guid_ValueHUDService));}
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
  SetHUDCommands (const char *cookie, const char *command1, const char *command2, const char *command3, int alphaSort)
  {
    return m_loc[0]->SetHUDCommands (m_loc,cookie,command1,command2,command3,alphaSort);
  }
    LxResult
  ClearHUD (const char *cookie)
  {
    return m_loc[0]->ClearHUD (m_loc,cookie);
  }
};

#endif
