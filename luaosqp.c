#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "osqp.h"
#include "assert.h"
#include "math.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

typedef struct {
	int size;
	OSQPFloat val[1];
} LuaOSQPVector;

void printar_vetor(OSQPFloat * q, int n){
	printf("{");
	for (; n > 0; q++){
		n--;
		printf("%f,", *q);
	}
	printf("}\n");
}

void printar_csc(OSQPCscMatrix * M){
	printf("{ m=%d, n=%d, nzmax=%d, nz=%d, owned=%d,\n p={", M->m, M->n, M->nzmax, M->nz, M->owned);
	for(int i = 0; i< M->n+1; i++){
		printf("%d,", M->p[i]);
	}	
	printf("},\n i={");
	for(int i = 0; i< M->nzmax; i++){
		printf("%d,", M->i[i]);

	}
	printf("},\n x={");
	for(int i = 0; i< M->nzmax; i++){
		printf("%f,", M->x[i]);

	}
	printf("}\n}\n");
}

static int lua_printar_vetor(lua_State * L){
	LuaOSQPVector * v  = lua_touserdata(L,1);
	OSQPFloat * q = v->val; 
	int n = lua_tointeger(L,2);
	lua_pushliteral(L, "{ ");
	for (int i=0; i<n; i++,q++){
		lua_pushfstring(L, "%f ", *q);
	}
	lua_pushliteral(L, "}\n");
	lua_concat(L,2+n);
	return 1;
}

static int lua_printar_csc(lua_State * L){
	OSQPCscMatrix * P = lua_touserdata(L,1);
	printar_csc(P);
	return 0;
}

/* solucionar(L,q,A,l,u)
 * 1 - matriz (custo quadratico)
 * 2 - vetor (custo linear)
 * 3 - matriz (restricoes)
 * 4 - vetor (limite inferior)
 * 5 - vetor (limite superior)
 */
static int lua_solucionar(lua_State * L){
	OSQPCscMatrix * P = lua_touserdata(L,1);
	LuaOSQPVector * q = lua_touserdata(L, 2);
	OSQPCscMatrix * A = lua_touserdata(L, 3);
	LuaOSQPVector * l = lua_touserdata(L, 4);
	LuaOSQPVector * u = lua_touserdata(L, 5);

	OSQPSolver * solver;
	OSQPSettings settings;
	osqp_set_default_settings(&settings);
	OSQPInt r = osqp_setup(&solver, P, q->val, A, l->val, u->val, A->m, A->n, &settings);
	osqp_solve(solver);

	size_t xsize = sizeof(OSQPFloat)*(A->n);
	size_t tam = sizeof(LuaOSQPVector)+ sizeof(OSQPFloat)*(A->n - 1);
	LuaOSQPVector * xv = lua_newuserdatauv(L, tam , 0);
	xv->size = A->n;
	memcpy(xv->val, solver->solution->x, xsize );
	osqp_cleanup(solver);
	return 1;
}


/* matriz(M)
 * 1 - M : lista de colunas, 
 */
static int lua_matriz(lua_State * L){
	int col = 0;
	int row = 0;
	//n de linhas
	
	OSQPCscMatrix * M = lua_newuserdatauv(L, sizeof(*M), 0);
	lua_len(L, 1);
	M->n = lua_tointeger(L, -1);
	lua_pop(L,1);
	M->nz = -1;
	M->owned = 0;

	lua_pushinteger(L, col+1);
	lua_gettable(L,1);
	//pega o tamanho da primeira coluna, e espero q seja o mesmo das demais
	lua_len(L,-1);
	M->m = lua_tointeger(L, -1);
	lua_pop(L,2);

	M->nzmax = 0;

	OSQPFloat val;

	M->p = malloc(sizeof(*(M->p))*(M->n+1));
	//calcula nzmax
	for(col = 0; col < M->n; col++ ){
		lua_pushinteger(L, col+1);
		lua_gettable(L,1);

		M->p[col] = M->nzmax;
		for(row = 0; row < M->m; row++ ){
			lua_pushinteger(L, row+1);
			lua_gettable(L,-2);
			val = lua_tonumber(L, -1 );
			lua_pop(L, 1);

			if (val != 0.0){
				M->nzmax += 1;	
			}
		}
		lua_pop(L,1);
	}

	M->p[M->n] = M->nzmax;
	//aloca coisas
	M->i = malloc(sizeof(*(M->i))*(M->nzmax));
	M->x = malloc(sizeof(*(M->x))*(M->nzmax));
	
	int nzcount = 0;
	for(col = 0; col < M->n; col++ ){
		lua_pushinteger(L, col+1);
		lua_gettable(L,1);
		for(row = 0; row < M->m; row++ ){
			lua_pushinteger(L, row+1);
			lua_gettable(L,-2);
			val = lua_tonumber(L, -1 );
			lua_pop(L, 1);

			if (val == 0.0){
				continue;
			}
			M->x[nzcount] = val;
			M->i[nzcount] = row;
			nzcount++;
			
		}
		lua_pop(L, 1);
	}
	
	return 1;
}


/** vetor(v)
 * 1 - lista de numeros
 */
static int lua_vetor(lua_State *L){

	lua_len(L, 1);
	int len = lua_tointeger(L, -1);
	lua_pop(L, 1);
	size_t tam = sizeof(LuaOSQPVector) + sizeof(OSQPFloat)*(len-1);
	LuaOSQPVector * lv = lua_newuserdatauv(L, tam, 0);
	lv->size = len; 
	for (int i = 0; i < len; i++){
		lua_pushinteger(L, i+1 );
		lua_gettable(L, 1);
		lv->val[i] = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
	return 1;
}

static int vetor_dot(lua_State * L){
	LuaOSQPVector * u = lua_touserdata(L, 1);
	LuaOSQPVector * v = lua_touserdata(L, 2);
	int len = u->size;

	OSQPFloat ret = 0.0;
	//TODO: BLASificar isso aqui
	for(int i =0; i<len; i++)
		ret += (u->val[i])*(v->val[i]);
	lua_pushnumber(L,ret);
	printf("dot size: %d\n", len);
	return 1;
}

static int luaopen_osqp(lua_State *L) {
    const luaL_Reg reg[] = {
	    {"matriz", lua_matriz}, 
	    {"solucionar", lua_solucionar}, 
	    {"vetor", lua_vetor}, 
	    {"printar_vetor", lua_printar_vetor}, 
	    {"printar_csc", lua_printar_csc}, 
	    {"dot", vetor_dot}, 
	    {NULL, NULL}    // Marca o fim da lista
	};
	// Cria uma nova tabela e registra as funções
    luaL_newlib(L, reg);
    return 1; // Retorna a tabela para Lua
}

static int panik(lua_State * L){
	fprintf(stderr, "PANIC: Um erro crítico ocorreu: %s\n", lua_tostring(L, -1));
	return 0; // Retorna 0 para indicar que o pânico foi tratado
}

int main(){
	lua_State * L = luaL_newstate();
	luaL_openlibs(L);
	lua_atpanic(L, panik);

	luaL_requiref(L, "osqp", luaopen_osqp, 1);
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
