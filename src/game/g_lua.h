
#ifndef G_LUA_H
#define G_LUA_H

#include "qcommon/q_shared.h"

struct gentity_t;
struct gclient_t;

void Api_Init();
void Cmd_LuaLoad_f( gentity_t* );

#endif
