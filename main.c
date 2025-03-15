#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "osqp.h"
#include "assert.h"
#include "math.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "svm.h"



static int panik(lua_State * L){
	fprintf(stderr, "PANIC: Um erro crítico ocorreu: %s\n", lua_tostring(L, -1));
	return 0; // Retorna 0 para indicar que o pânico foi tratado
}

int main(){
	lua_State * L = luaL_newstate();
	luaL_openlibs(L);
	lua_atpanic(L, panik);

	luaL_requiref(L, "svm", luaopen_svm, 1);
	lua_pop(L, 1); // Remove a biblioteca da pilha

	if (luaL_dofile(L, "svm.lua") != LUA_OK){
		fprintf(stderr, "Erro ao executar svm.lua: %s\n", lua_tostring(L, -1));
	        lua_pop(L, 1); // Remove a mensagem de erro da pilha
	}
	if (luaL_dofile(L, "main.lua") != LUA_OK){
		fprintf(stderr, "Erro ao executar main.lua: %s\n", lua_tostring(L, -1));
	        lua_pop(L, 1); // Remove a mensagem de erro da pilha
	}
	return 0;
}
