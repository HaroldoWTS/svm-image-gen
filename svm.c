#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "osqp.h"
#include "assert.h"
#include "math.h"
#include "float.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"


int selectB(int len, double ** Q, double * A, double * G, const int * y, double C, double tau, double eps, int * ret_i, int * ret_j){
	int i= -1;
	int j, t;
	double G_max = -DBL_MAX;
	double G_min = DBL_MAX;
	double obj_min = DBL_MAX;
	double a, b, yt;
	double * Qi;

	for (t = 0; t < len; t++){
		yt = y[t];
		if ((yt == 1 && A[t] < C) ||
			(yt == -1 && A[t] > 0)){
			if (-yt*G[t] >= G_max){
				i = t;
				G_max = -1.0*yt*G[t];
			}
		}
	}
	
	Qi = Q[i];
	
	j = -1;
	for (t=0; t<len;t++){
		yt = y[t];
		if ((yt == 1 && A[t] > 0.0) ||
			(yt == -1 && A[t] < C)){
			b = G_max + yt*G[t];
			if (-1.0*yt*G[t] <= G_min)
				G_min = -1.0*yt*G[t];
	
			if (b >0){
				a=Qi[i]+Q[t][t]-2.0*y[i]*yt*Qi[t];
				if (a <= 0.0)
					a = tau;
				
				if (-1.0*(b*b)/a <= obj_min){
					j = t;
					obj_min = -1.0*(b*b)/a;
				}
			}
		}
	}
	if ((G_max-G_min < eps) || (j == -1)){
		return 0;
	}else{
		*ret_i = i;
		*ret_j = j;
		return 1;
	}
	
}

static int lua_solve_smo_wss3(lua_State * L){
	const int Q_INDEX = 1;
	const int y_INDEX = 2;
	const int C_INDEX = 3;

	//TODO: calcular isso aqui
	const double eps = 1e-3;
	const double tau = 1e-12;

	int * y;
	lua_Number ** Q;
	lua_Number * A;
	lua_Number * G;
	lua_Number C, yi, yj, oldAi, oldAj, sum, deltaAi, deltaAj;
	lua_len(L, 2);
	int len = lua_tointeger(L, -1);
	int n = len; 
	lua_pop(L, 1);
	int t, tt;
	int i, j;

	lua_Number a,b;

	y = malloc(sizeof(*y)*n);
	A = malloc(sizeof(*A)*n);
	G = malloc(sizeof(*G)*n);
	Q = malloc(sizeof(*Q)*n);

	//pegando os valores
	for (t =0; t< len; t++){
		lua_pushinteger(L, t+1);
		lua_gettable(L, y_INDEX);
		y[t] = lua_tointeger(L, -1);
		lua_pop(L, 1); 

		A[t] = 0.0;
		G[t] = -1.0;
	}

	
	for (t =0; t< n; t++){
		Q[t] = malloc(sizeof(lua_Number)*n);
		lua_pushinteger(L, t+1);
		lua_gettable(L, Q_INDEX);
		for (tt =0; tt< n; tt++){
			lua_pushinteger(L, tt+1);
			lua_gettable(L, -2);
			Q[t][tt] = lua_tonumber(L, -1);
			lua_pop(L, 1);
		}
		lua_pop(L, 1);
	}

	C = lua_tonumber(L, C_INDEX);

	while (selectB(len, Q, A, G, y, C, tau, eps, &i, &j)){
		yi = y[i];
		yj = y[j];
		a = Q[i][i] + Q[j][j] - 2*yi*yj*Q[i][j];
		if (a <= 0.0)
			a = tau;
		b = -1.0*yi*G[i] + yj*G[j];

		//update alpha
		oldAi = A[i];
		oldAj = A[j];
		A[i] = A[i] + yi*b/a;
		A[j] = A[j] - yj*b/a;

		//project alpha back to the feasible region
		sum = yi*oldAi + yj*oldAj;
		if (A[i] > C )
			A[i] = C;
		else if (A[i] < 0.0)
			A[i] = 0.0;
	
		A[j] = yj*(sum -yi*A[i]);
		if (A[j] > C)
			A[j] = C;
		else if (A[j] < 0.0)
			A[j] = 0.0;
		A[i] = yi*(sum -yj*A[j]);
		deltaAi = A[i] - oldAi;
		deltaAj = A[j] - oldAj;
		for (t = 0; t< len; t++)
			G[t] = G[t] + Q[t][i]*deltaAi + Q[t][j]*deltaAj;
	}

	lua_createtable(L,n,0);
	for (t =0; t< len;t++){
		if (A[t]*A[t] > tau){
			lua_pushinteger(L, t+1);
			lua_pushnumber(L, A[t]);
			lua_settable(L, -3);
		}
	}
	return 1;

}

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
	settings.max_iter = 100000;
	osqp_set_default_settings(&settings);
	OSQPInt r = osqp_setup(&solver, &P, q, &A, l, u, m, n, &settings);
	osqp_solve(solver);
	
	lua_createtable(L,n,0);
	OSQPFloat ci;
	for(i = 0; i<n;i++){
		ci =solver->solution->x[i] ;
		//TODO: Precisao arbitrária!
		if (ci*ci < 0.00001 )
			continue;
		lua_pushinteger(L, i+1);
		lua_pushnumber(L, ci);
		lua_settable(L, -3);
	}

	return 1;
}

int luaopen_svm(lua_State *L) {
    const luaL_Reg reg[] = {
	    {"solve", lua_solve}, 
	    {"solve_smo_wss3", lua_solve_smo_wss3}, 
	    {NULL, NULL}    // Marca o fim da lista
	};
	// Cria uma nova tabela e registra as funções
    luaL_newlib(L, reg);
    return 1; // Retorna a tabela para Lua
}
