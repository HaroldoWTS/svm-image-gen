#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "assert.h"
#include "math.h"
#include "float.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "smo.h"

static int lua_solve_smo_wss3(lua_State * L){
	const int K_INDEX = 1;
	const int y_INDEX = 2;
	const int C_INDEX = 3;
	int t,tt;
	double a;

	lua_len(L, y_INDEX);
	int len = lua_tointeger(L, -1);
	SMO_State * s = SMO_newstate(len);
	printf("SMO inicializado.\n");

	lua_pop(L, 1);

	//y tem q ser informado primeiro
	for (t =0; t< len; t++){
		lua_pushinteger(L, t+1);
		lua_gettable(L, y_INDEX);
		SMO_sety(s,t, lua_tointeger(L, -1));
		lua_pop(L, 1);
	}

	for (t =0; t< len; t++){
		
		for (tt =0; tt< len; tt++){
			lua_pushvalue(L, K_INDEX);
			lua_pushinteger(L, t+1);
			lua_pushinteger(L, tt+1);
			lua_call(L, 2,1);
				
			SMO_setk(s, tt, t, lua_tonumber(L, -1));
			lua_pop(L, 1);
		}
	}
	
	SMO_solve(s, lua_tonumber(L, C_INDEX));

	lua_createtable(L,len,0);
	for (t =0; t< len;t++){
		a = SMO_getsolution(s, t);
		if (a*a > 10e-4){
			lua_pushinteger(L, t+1);
			lua_pushnumber(L, a);
			lua_settable(L, -3);
		}
	}
	return 1;
}



int luaopen_svm(lua_State *L) {
    const luaL_Reg reg[] = {
	    {"solve_smo_wss3", lua_solve_smo_wss3}, 
	    {NULL, NULL}    // Marca o fim da lista
	};
	// Cria uma nova tabela e registra as funções
    luaL_newlib(L, reg);
    return 1; // Retorna a tabela para Lua
}
