#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "osqp.h"
#include "assert.h"
#include "math.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"


/* svm.solve(D, y, C)
 * D - matriz nxn de kernel(xi,xj)*yj*yi
 * y - vetor de n rotulos
 * C - fator de generalizacao
 */
static int lua_solve(lua_State * L){
	const int D_INDEX = 1;
	const int y_INDEX = 2;
	const int C_INDEX = 3;
	lua_len(L, 2);
	int n = lua_tointeger(L, -1);
	lua_pop(L,1);
	int m = n+1;
	int i,j;

	OSQPFloat C = lua_tonumber(L, 3);
	OSQPFloat * q = malloc(sizeof(*q)*n);
	OSQPFloat * y = malloc(sizeof(*y)*n);
	OSQPFloat * l = malloc(sizeof(*l)*(m));
	OSQPFloat * u = malloc(sizeof(*u)*(m));
	OSQPCscMatrix P;
	OSQPCscMatrix A;
	OSQPFloat val;
	int nzcount;
	//OSQPFloat * A = malloc(sizeof(*A)*(m*n);
	
	for (i =0; i< n; i++){
		q[i] = -1.0;
		u[i] = C;
		l[i] = 0.0;

		lua_pushinteger(L, i+1);
		lua_gettable(L, y_INDEX);
		y[i] = lua_tonumber(L, -1);
		lua_pop(L, 1); 
	}
	l[m-1] = 0.0;
	u[m-1] = 0.0;
	
	//preenchendo matriz P
	P.m = n;
	P.n = n;
	P.nz = -1;
	P.owned = 0;
	P.p = malloc(sizeof(OSQPInt)*(n+1));
	//sera alocado mais tamanho que o necessario pra essas duas
	P.i = malloc(sizeof(OSQPInt)*(n*n));
	P.x = malloc(sizeof(OSQPFloat)*(n*n));

	nzcount = 0;
	for(j =0; j<n; j++){
		lua_pushinteger(L, j+1);
		lua_gettable(L, D_INDEX);
		P.p[j] = nzcount;
		//matriz triangular superior
		for(i=0;i<j+1;i++){
			lua_pushinteger(L, i+1);
			lua_gettable(L, -2);
			val = lua_tonumber(L, -1);
			if (val == 0.0)
				continue;
			P.x[nzcount] = val;
			P.i[nzcount] = i;
			nzcount++;
			lua_pop(L, 1);
		}
		lua_pop(L, 1);
	}
	P.p[n] = nzcount;
	P.nzmax = nzcount;
	P.i = realloc(P.i, sizeof(OSQPInt)*nzcount);
	P.x = realloc(P.x, sizeof(OSQPFloat)*nzcount);

	//preenchendo matriz A
	//A = [ eye(n,n) ; y']
	A.m = m;
	A.n = n;
	A.nz = -1;
	A.owned = 0;
	A.nzmax = 2*n;
	A.p = malloc(sizeof(OSQPInt)*(n+1));
	A.i = malloc(sizeof(OSQPInt)*(A.nzmax));
	A.x = malloc(sizeof(OSQPFloat)*(A.nzmax));

	for(j =0; j<n; j++){
		A.p[j] = j*2;
		A.x[j*2] = 1.0;
		A.i[j*2] = j;
		A.x[1 + j*2] = y[j];
		A.i[1 + j*2] = A.m - 1;
	}
	A.p[n] = 2*n;

	//resolvendo
	OSQPSolver * solver;
	OSQPSettings settings;
	osqp_set_default_settings(&settings);
	OSQPInt r = osqp_setup(&solver, &P, q, &A, l, u, m, n, &settings);
	osqp_solve(solver);
	
	lua_createtable(L,n,0);
	for(i = 0; i<n;i++){
		lua_pushinteger(L, i+1);
		lua_pushnumber(L, solver->solution->x[i]);
		lua_settable(L, -3);
	}

	return 1;
}

int luaopen_svm(lua_State *L) {
    const luaL_Reg reg[] = {
	    {"solve", lua_solve}, 
	    {NULL, NULL}    // Marca o fim da lista
	};
	// Cria uma nova tabela e registra as funções
    luaL_newlib(L, reg);
    return 1; // Retorna a tabela para Lua
}
