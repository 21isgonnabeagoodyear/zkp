#ifndef SAT_H
#define SAT_H


//64 bit key
#define VARIABLEMASK 0x7f
#define NUMVARIABLES (VARIABLEMASK+1)
#define CLAUSEMASK ((VARIABLEMASK|0x80)<<24 | (VARIABLEMASK|0x80)<<16 | (VARIABLEMASK|0x80)<<8)
//#define NUMCLAUSES 256
//#define NUMCLAUSES (NUMVARIABLES*NUMVARIABLES)
//#define NUMCLAUSES 128
//#define NUMCLAUSES 8
#define NUMCLAUSES NUMVARIABLES



typedef struct{
	char variables[NUMVARIABLES];
	char replacements[NUMVARIABLES];
	int clauses[NUMCLAUSES];//negative byte = not variable
} sat3;

void sortsat3(sat3 *s);
void newsat3(sat3 *s);
void permutesat3(sat3 *s);
void printsat3(sat3 *s);
void applytransformsat3(sat3 *s);
//1 = good, 0 = bad
int checksat3(sat3 *s);
void loadsat3(sat3 *s, char fn[]);
void savesat3(sat3 *s, char fn[]);
void clearanssat3(sat3 *s);
void printstatsat3(sat3 *s);


#endif
