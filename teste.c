#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "osqp.h"
#include "assert.h"
#include "math.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"


/*typedef struct {
	OSQPFloat * vetor;
	OSQPFloat rotulo;
} PontoTreino;

OSQPFloat * criar_vetor_monovalor(OSQPInt n, OSQPFloat v){
	OSQPFloat * f = malloc(n*sizeof(*f));
	for (OSQPFloat * x = f;n >0; x++ ){
		n--;
		*x = v;
	}
	return f;
}


OSQPCscMatrix * custo_quadratico(){
	static const OSQPFloat xc[3] = {2.0, 2.0, 2.0};
	static const OSQPInt ic[3] = {0, 0, 1};
	static const OSQPInt pc[3] = {0, 1, 3};

	//printf("sizeof xc: %d\n", sizeof(xc) );

	OSQPFloat * x =  malloc(sizeof(xc));
	OSQPInt * i =  malloc(sizeof(ic));
	OSQPInt * p =  malloc(sizeof(pc));

	memcpy(x, xc, sizeof(xc));
	memcpy(i, ic, sizeof(ic));
	memcpy(p, pc, sizeof(pc));

	OSQPCscMatrix * P = OSQPCscMatrix_new(2,2, 3, x, i, p);
	return P;
}

OSQPCscMatrix * restricoes(){
	static const OSQPFloat xc[4] = {-1.0, 1.0, 1.0, 1.0};
	static const OSQPInt ic[4] = {0, 1, 0, 2};
	static const OSQPInt pc[3] = {0, 2, 4};

	//printf("sizeof xc: %d\n", sizeof(xc) );

	OSQPFloat * x =  malloc(sizeof(xc));
	OSQPInt * i =  malloc(sizeof(ic));
	OSQPInt * p =  malloc(sizeof(pc));

	memcpy(x, xc, sizeof(xc));
	memcpy(i, ic, sizeof(ic));
	memcpy(p, pc, sizeof(pc));

	OSQPCscMatrix * P = OSQPCscMatrix_new(3,2, 4, x, i, p);
	return P;
}
*/

void printar_vetor(OSQPFloat * q, int n){
	printf("[\n");
	for (; n > 0; q++){
		n--;
		printf("%f\n", *q);
	}
	printf("]\n");
}

void printar_csc(OSQPCscMatrix * M){
	
}


/* Retorna um userdata OSQPCscMatrix
 * Recebe uma lista de colunas
 * Cada coluna é um vetor, uma lista de numeros
 */
static int lua_matriz(lua_State * L){
			
	return 1;
}

static int luaosqp_vetor(lua_State *L){
	//o primeiro arg deveria ser uma lua table com os valores
	int argc = lua_gettop(L);
	lua_len(L, 1);
	int len = lua_tointeger(L, argc+1);
	lua_pop(L, 1);
	OSQPFloat * v = lua_newuserdatauv(L, sizeof(*v)*len, 0);
	for (int i = 0; i < len; i++){
		lua_pushinteger(L, i+1 );
		lua_gettable(L, 1);
		v[i] = lua_tonumber(L, argc+2);
		lua_pop(L, 1);
	}
	return 1;
}

/* osqp.printar_vetor
 * 1 - 
static int luaosqp_printar_vetor(lua_State *L){
	//o primeiro arg deveria ser uma lua table com os valores
	OSQPFloat * v = lua_touserdata(L,1);
	int len = lua_tointeger(L, 2);
	printar_vetor(v, len);
	return 0;
}


static int lua_matriz(lua_State *L){
	OSQPFloat * v = lua_newuserdatauv(L, sizeof(*v)*len, 0);

	return 1;
}

int luatable2vetor(lua_State * L, int index, OSQPFloat * q, int length){
	for (int i=0; i< length; i++){
		lua_pushinteger(L, i+1);
		lua_gettable(L, index );
		q[i] = lua_tonumber(L, -1);
		lua_pop(L,1);
	}
	return 0;
}

int luatable2csc(lua_State * L, int index, OSQPCscMatrix* P, int rows, int cols ){
	
}

/* Args:
 * 1 - csc P: nxn
 * 2 - vetor q: n
 * 3 - csc A: mxn
 * 4 - vetor l: m
 * 5 - vetor u: m
 * 6 - int m
 * 7 - int n
 */  
/*static int osqplua_solve(lua_State * L){
	int m = lua_tointeger(L, 6);
	int n = lua_tointeger(L, 7);
	OSQPCscMatrix P,A;
	OSQPFloat * q, *l, *m;
	q = malloc(sizeof(q)*n);
	l = malloc(sizeof(l)*m);
	m = malloc(sizeof(m)*m);

	luatable2csc(L, 1, &P, n, n);
	luatable2csc(L, 3, &A, m, n);
	//TODO: checar dimensoes
	luatable2vetor(L, 2, q, n);
	luatable2vetor(L, 4, l, m);
	luatable2vetor(L, 5, u, m);
	OSQPSolver * solver;
	OSQPSettings *settings = malloc(sizeof(*settings));

	OSQPInt r = osqp_setup(&solver, P, q, A, l, u, m, n, settings);

	printar_vetor(solver->solution->x, n);

}
*/

int luaopen_osqp(lua_State *L) {
    const luaL_Reg osqp[] = {
	    {"vetor", lua_vetor}, 
	    {"printar_vetor", lua_printar_vetor}, 
	    {NULL, NULL}    // Marca o fim da lista
	};
	// Cria uma nova tabela e registra as funções
    luaL_newlib(L, osqp);
    return 1; // Retorna a tabela para Lua
}


/* svm.train(set, kernel)
 * 1 - set: {{vetor, bool}, ...}
 * 2 - kernel: func(vetor, vetor) -> number
 * 3 - C: number
 */  
int luasvm_train(lua_State * L){
	//vendo tamanho do conjunto de treinamento
	lua_len(L, 1);
	int n = lua_tointeger(L, -1); 
	int m = n + 1;
	lua_pop(L,1);
	
	//alocando espaços
	OSQPFloat * q, *l, *u, *y;
	OSQPCscMatrix *P, *A;
	PontoTreino * pontos;
	q = malloc(sizeof(q)*n);
	y = malloc(sizeof(y)*n);
	l = malloc(sizeof(l)*m);
	u = malloc(sizeof(u)*m);
	pontos = malloc(sizeof(pontos)*n);

	//setando valores gerais de todo svm
	for (int i =0; i < n; i++){
		q[i] = -1.0;
	}

	for (i =0; i < m; i++){
		l[i] = 0.0;
		u[i] = C;
	}
	u[0] = 0.0;

	//prenche os pontos de treino
	i = 0;
	lua_pushnil(L);
	//iterando sobre o conjunto de treino
	for (i = 0; i < n; i++){
		lua_pushinteger(L, i+1);
		lua_gettable(L, 1); 

		//pega o vetor
		lua_pushcfunction(L, luaosqp_vetor);
		lua_pushinteger(L, 1);
		lua_gettable(L, -2);
		lua_call(L, 1, 1);
		pontos[i]->vetor = lua_touserdata(L, -1);
		lua_pop(L, 1);

		//pega o rotulo
		lua_pushinteger(L, 2);
		lua_gettable(L, -2);
		pontos[i]->rotulo = lua_toboolean(L, -1) ? 1.0 : -1.0;
		lua_pop(L, 1);
	}	
	

	//gera a matriz P
	for (i = 0; i< n; i++){
		for (int j =0;


	OSQPSolver * solver;
	OSQPSettings *settings = malloc(sizeof(*settings));

	OSQPInt r = osqp_setup(&solver, P, q, A, l, u, m, n, settings);

	printar_vetor(solver->solution->x, n);

	return 0	
}

int panik(lua_State * L){
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
		fprintf(stderr, "Erro ao executar o arquivo: %s\n", lua_tostring(L, -1));
	        lua_pop(L, 1); // Remove a mensagem de erro da pilha
	}
	/*OSQPSolver * solver;
	OSQPSettings *settings = malloc(sizeof(*settings));
	OSQPInt n = 2;
	OSQPCscMatrix * P = custo_quadratico();
	OSQPFloat * q = criar_vetor_monovalor(n, -1.0);
	OSQPCscMatrix * A = restricoes();
	OSQPFloat * l = criar_vetor_monovalor(3, 0.0);
	OSQPFloat * u = criar_vetor_monovalor(3, INFINITY);
	u[0] = 0.0;
	OSQPInt m = 3;

	printf("q:\n");
	printar_vetor(q, n);

	osqp_set_default_settings(settings);
	OSQPInt r = osqp_setup(&solver, P, q, A, l, u, m, n, settings);
	assert(r == 0);
	r = osqp_solve(solver);
	assert(r == 0);
	printar_vetor(solver->solution->x, n);
	r = osqp_cleanup(solver);
	assert(r == 0);*/

	return 0;
}
