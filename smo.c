#include "smo.h"
#include "stdlib.h"
#include <float.h>

struct SMO_State {
	double * G;
	double * A;
	double ** K;
	int * y;
	unsigned int len;
	int i;
	int j;
};

void SMO_close(SMO_State * s){
	free(s->G);
	free(s->A);
	free(s->y);

	for (unsigned int t = 0; t< s->len; t++){
		free(s->K[t]);
	}
	free(s->K);
	free(s);
}

SMO_State * SMO_newstate( unsigned int len){
	SMO_State * s = malloc(sizeof(*s));
	s->len = len;
	s->i = s->j = -1;
	s->G = s->A = NULL;
	s->K = NULL;
	s->y = NULL;
	if (!s) return NULL;
	s->K = malloc(sizeof(*(s->K))*len);
	s->y = malloc(sizeof(*(s->y))*len);
	s->A = malloc(sizeof(*(s->A))*len);
	s->G = malloc(sizeof(*(s->G))*len);

	for (unsigned int t = 0; t< s->len; t++){
		s->A[t] = 0.0;
		s->G[t] = -1.0;
		//Q[j][i] é o elemento da coluna j, linha i
		//so armazeno a triangular superior
		s->K[t] = malloc(sizeof(double)*(t+1));
	}

	return s;
}

void SMO_sety(SMO_State * s, unsigned int index, int value){
	s->y[index] = value;
}

void SMO_setk(SMO_State * s, unsigned int i, unsigned int j, double value){
	//qualquer j é valido, mas nem todo i é válido
	if (i > j){
		s->K[i][j] = value;
	}else{
		s->K[j][i] = value;
	}
}


static inline double SMO_Q(SMO_State * s, unsigned int i, unsigned int j){
	return s->y[i] * s->y[j] * ( (i > j) ? (  s->K[i][j] )  : s->K[j][i]  );
}


static int SMO_selectB(SMO_State * s, double C, double eps){
	int i= -1;
	int j;
	unsigned int t;
	double G_max = -DBL_MAX;
	double G_min = DBL_MAX;
	double obj_min = DBL_MAX;
	double a, b, yt;
	int * y = s->y;
	double * A = s->A;
	double * G = s->G;

	for (t = 0; t < s->len; t++){
		yt = y[t];
		if ((yt == 1 && A[t] < C) ||
			(yt == -1 && A[t] > 0)){
			if (-yt*G[t] >= G_max){
				i = t;
				G_max = -1.0*yt*G[t];
			}
		}
	}
	
	j = -1;
	for (t=0; t<s->len;t++){
		yt = y[t];
		if ((yt == 1 && A[t] > 0.0) ||
			(yt == -1 && A[t] < C)){
			b = G_max + yt*G[t];
			if (-1.0*yt*G[t] <= G_min)
				G_min = -1.0*yt*G[t];
	
			if (b >0){
				//a=Qi[i]+Q[t][t]-2.0*y[i]*yt*Qi[t];
				a=SMO_Q(s,i,i)+SMO_Q(s,t,t)-2.0*y[i]*yt*SMO_Q(s,i,t);
				if (a <= 0.0)
					a = 0.0;
				
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
		s->i = i;
		s->j = j;
		return 1;
	}
}

void SMO_solve(SMO_State * s, double C){
	int yi, yj;
	double a,b,Ai, Aj, sum, oldAi, oldAj, deltaAi, deltaAj;
	double eps = 10e-4;
	unsigned int t;

	while (SMO_selectB(s, C, eps)){
		yi = s->y[s->i];
		yj = s->y[s->j];
		Ai = s->A[s->i];
		Aj = s->A[s->j];
		oldAi = Ai;
		oldAj = Aj;
		sum = yi*oldAi + yj*oldAj;
		
		//a = Q[i][i] + Q[j][j] - 2*yi*yj*Q[i][j];
		a = SMO_Q(s,s->i,s->i) + SMO_Q(s,s->j,s->j) -2*yi,yj*SMO_Q(s,s->i,s->j);
		//a = s->Qdiag[s->i] + s->Qdiag[j] -2*yi,yj*SMO_Q(s,i,j);
		if (a <= 0.0)
			a = 0.0;
		b = -1.0*yi*(s->G[s->i]) + yj*(s->G[s->j]);

		//update alpha
		//
		Ai = Ai + yi*b/a;
		Aj = Aj - yj*b/a;

		//project alpha back to the feasible region
		if (Ai > C )
			Ai = C;
		else if (Ai < 0.0)
			Ai = 0.0;
	
		Aj = yj*(sum -yi*Ai);
		if (Aj > C)
			Aj = C;
		else if (Aj < 0.0)
			Aj = 0.0;
		Ai = yi*(sum -yj*Aj);
		deltaAi = Ai - oldAi;
		deltaAj = Aj - oldAj;

		s->A[s->i] = Ai;
		s->A[s->j] = Aj;
		for (t = 0; t< s->len; t++)
			s->G[t] += SMO_Q(s,t,s->i)*deltaAi + SMO_Q(s,t,s->j)*deltaAj;
	}
}


double SMO_getsolution(SMO_State * s, unsigned int index){
	return s->A[index];
}
