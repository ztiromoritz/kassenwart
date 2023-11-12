#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

int luaTest() {

  char buff[256];
  int error;
  lua_State *L = luaL_newstate();
  luaL_openlibs(L);

  error = luaL_loadfile(L, "../functions.lua") || lua_pcall(L, 0, 0, 0);
  if (error) {
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
  }
  while (fgets(buff, sizeof(buff), stdin) != NULL) {
    error = luaL_loadstring(L, buff) || lua_pcall(L, 0, 0, 0);
    if (error) {
      fprintf(stderr, "%s\n", lua_tostring(L, -1));
      lua_pop(L, 1);
    }
  }
  lua_close(L);
  return 0;
}
