#ifndef lnettlelib_h
#define lnettlelib_h

#ifdef __cplusplus
extern "C" {
#endif

#define LUA_NETTLELIBNAME	"nettle"
LUAMOD_API int (luaopen_nettle) (lua_State *L);

#ifdef __cplusplus
}
#endif

#endif /* lnettlelib_h */
