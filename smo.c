#include "smo.h"
#include "stdlib.h"
#include <float.h>
#include <omp.h>
#include "math.h"

typedef struct {
	double A;
	double y;
	double G;
	unsigned int index;
	double Qdiag;
} SMO_Block;

struct SMO_State {
	double ** Q;
	unsigned int len;
	SMO_Block * s;
};


void SMO_close(SMO_State * s){
	free(s->s);

	for (unsigned int t = 0; t< s->len; t++){
		free(s->Q[t]);
	}
	free(s->Q);
	free(s);
}

SMO_State * SMO_newstate( unsigned int len){
	SMO_State * s = malloc(sizeof(*s));
	s->len = len;
	s->s = NULL;
	s->Q = NULL;
	if (!s) return NULL;
	s->Q = malloc(sizeof(*(s->Q))*len);
	s->s = malloc(sizeof(*(s->s))*len);

	for (unsigned int t = 0; t< s->len; t++){
		s->s[t].index = t;
		s->s[t].A = 0.0;
		s->s[t].G = -1.0;
		//Q[j][i] é o elemento da coluna j, linha i
		//so armazeno a triangular superior
		s->Q[t] = malloc(sizeof(double)*(t+1));
	}

	return s;
}

void SMO_sety(SMO_State * s, unsigned int index, int value){
	s->s[index].y = value;
}

void SMO_setk(SMO_State * s, unsigned int i, unsigned int j, double value){
	//qualquer j é valido, mas nem todo i é válido
	if (i > j){
		s->Q[i][j] = value*s->s[i].y*s->s[j].y;

	}else{
		s->Q[j][i] = value*s->s[i].y*s->s[j].y;
		if (i == j)
			s->s[i].Qdiag = s->s[j].Qdiag = s->Q[j][i];
	}
}


static inline double SMO_Q(SMO_State * s, unsigned int i, unsigned int j){
	//return s->s[i].y * s->s[i].y * ( (i > j) ? (  s->K[i][j] )  : s->K[j][i]  );
	return ( (i > j) ? (  s->Q[i][j] )  : s->Q[j][i]  );
}


static int selecti(SMO_State * s , const double C, double * G_max_ret){
	double G_max = -DBL_MAX;
	int i;
	double minus_yt_Gt;
	SMO_Block v;
	const unsigned int len = s->len;
	for (unsigned int t = 0; t < len; t++){
		v = s->s[t];
		if ((v.y == 1 && v.A < C) ||
			(v.y == -1 && v.A > 0)){
			minus_yt_Gt = -1.0*v.y*v.G;
			if ( minus_yt_Gt >= G_max){
				i = t;
				G_max = minus_yt_Gt;
			}
		}
	}

	*G_max_ret = G_max;
	return i;
}

static int selectj(SMO_State *s, const int i, const double C, const double G_max, double * G_min_ret){
	
	double a, b, yi;
	double obj_min = DBL_MAX;
	double G_min = DBL_MAX;
	double Qii = s->s[i].Qdiag;
	yi = s->s[i].y;
	int j = -1;
	SMO_Block vt;
	double minus_y_G;

	const unsigned int len = s->len;
	for (unsigned int t=0; t< len;t++){
		vt = s->s[t];
		if ((vt.y == 1 && vt.A > 0.0) ||
			(vt.y == -1 && vt.A < C)){
			b = G_max + vt.y*vt.G;
			minus_y_G = -1.0*vt.y*vt.G;
			if ( minus_y_G<= G_min)
				G_min = minus_y_G;
	
			if (b >0){
				//a=Qi[i]+Q[t][t]-2.0*y[i]*yt*Qi[t];
				a=Qii+vt.Qdiag -2.0*yi*vt.y*SMO_Q(s,i,t);
				if (a <= 0.0)
					a = 0.0;
				
				if (-1.0*(b*b)/a <= obj_min){
					j = t;
					obj_min = -1.0*(b*b)/a;
				}
			}
		}
	}
	*G_min_ret = G_min;
	return j;
}

static int SMO_selectB(SMO_State * s, double C, double eps, SMO_Block * Bi, SMO_Block * Bj){
	int i= -1;
	int j;
	double G_max = -DBL_MAX;
	double G_min = DBL_MAX;

	i = selecti(s,C,&G_max);

	j = selectj(s,i,C,G_max,&G_min);
	
	if ((G_max-G_min < eps) || (j == -1)){
		return 0;
	}else{
		*Bi = s->s[i];
		*Bj = s->s[j];
		return 1;
	}
}

void SMO_solve(SMO_State * s, double C){
	double a,b, sum, oldAi, oldAj, deltaAi, deltaAj;
	double eps = 10e-4;
	unsigned int t;

	SMO_Block Bi, Bj;
	while (SMO_selectB(s, C, eps, &Bi, &Bj)){
		oldAi = Bi.A;
		oldAj = Bj.A;
		sum = Bi.y*oldAi + Bj.y*oldAj;
		
		//a = Q[i][i] + Q[j][j] - 2*yi*yj*Q[i][j];
		a = Bi.Qdiag + Bj.Qdiag -2.0*Bi.y*Bj.y*SMO_Q(s,Bi.index,Bj.index);
		//a = SMO_Q(s,Bi.,s->i) + SMO_Q(s,s->j,s->j) -2*yi,yj*SMO_Q(s,s->i,s->j);
		//a = s->Qdiag[s->i] + s->Qdiag[j] -2*yi,yj*SMO_Q(s,i,j);
		
		if (a < 0.0) a = 0.0;
		b = -1.0*Bi.y*(Bi.G) + Bj.y*(Bj.G);

		//update alpha
		//
		Bi.A += Bi.y*b/a;
		Bj.A -= Bj.y*b/a;

		//project alpha back to the feasible region
	
		if (Bi.A > C)
			Bi.A = C;
		else if (Bi.A < 0.0)
			Bi.A = 0.0;

		Bj.A = Bj.y*(sum -Bi.y*Bi.A);
	
		if (Bj.A > C)
			Bj.A = C;
		else if (Bj.A < 0.0)
			Bj.A = 0.0;


		Bi.A = Bi.y*(sum -Bj.y*Bj.A);
		deltaAi = Bi.A - oldAi;
		deltaAj = Bj.A - oldAj;

		s->s[Bi.index].A = Bi.A;
		s->s[Bj.index].A = Bj.A;
		for (t = 0; t< s->len; t++)
			s->s[t].G += SMO_Q(s,t,Bi.index)*deltaAi + SMO_Q(s,t,Bj.index)*deltaAj;
	}
}


double SMO_getsolution(SMO_State * s, unsigned int index){
	return s->s[index].A;
}
