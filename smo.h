#ifndef SMO_H
#define SMO_H

struct SMO_State;
typedef struct SMO_State SMO_State;

SMO_State * SMO_newstate(unsigned int len);
void SMO_close(SMO_State * s);

void SMO_sety(SMO_State * s, unsigned int index, int value );
void SMO_setk(SMO_State * s, unsigned int i, unsigned int j, double value);

void SMO_solve(SMO_State * s, double C);

double SMO_getsolution(SMO_State * s, unsigned int index);

#endif
