#include "sat.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rng.h"

static int compareints(const void* p1, const void* p2){
	return (*((unsigned int*)(p2))>*((unsigned int*)(p1)));//hope this doesn't fuck up when p2==p1
}

static int evalclause(int clause, char *vars){
	int a = (clause >> 24)&0xff;//could probably be prettier
	int b = (clause >> 16)&0xff;
	int c = (clause >>  8)&0xff;
	int av, bv, cv;
	if(a&0x80)
		av = !(vars[a&VARIABLEMASK]);
	else
		av = (vars[a&VARIABLEMASK]);
	if(b&0x80)
		bv = !(vars[b&VARIABLEMASK]);
	else
		bv = (vars[b&VARIABLEMASK]);
	if(c&0x80)
		cv = !(vars[c&VARIABLEMASK]);
	else
		cv = (vars[c&VARIABLEMASK]);
//one in 3 makes it (slightly) less likely to have multiple solutions
	if(av && !bv && !cv)
		return 1;
	if(!av && bv && !cv)
		return 1;
	if(!av && !bv && cv)
		return 1;
	return 0;
	//return av || bv || cv;
}

void sortsat3(sat3 *s){
	int i;
	for(i=0;i<NUMCLAUSES;i++)
	{
		s->clauses[i] &= CLAUSEMASK;
		if(((s->clauses[i] >> 16)&0xff) > ((s->clauses[i] >> 24)&0xff))//sort first two
			s->clauses[i] = ((s->clauses[i] >> 16)&0xff)<<24 | ((s->clauses[i] >> 24)&0xff) << 16            | ((s->clauses[i] >> 8)&0xff)<<8;
		if(((s->clauses[i] >> 8)&0xff) > ((s->clauses[i] >> 16)&0xff))//sort second two
			s->clauses[i] = ((s->clauses[i] >> 8)&0xff)<<16 | ((s->clauses[i] >> 16)&0xff) << 8              | ((s->clauses[i] >> 24)&0xff)<<24;
	}
	qsort(&(s->clauses[0]), NUMCLAUSES, sizeof(int), compareints);
	//TODO:sort individual clauses
}

//apply replacements to original to see if the conversion is valid
void applytransformsat3(sat3 *s){
//static void applyreplacements(sat3 *s){
	int i, j;
	char buf[NUMVARIABLES];
	memcpy(buf, &s->variables[0], NUMVARIABLES);
	for(i=0;i<NUMVARIABLES;i++){
		//printf("replace %d %d (and %d)\n", s->replacements[i]&VARIABLEMASK, i, s->replacements[i]&0x80);
		//	s->variables[s->replacements[i]&VARIABLEMASK] = (buf[i]&VARIABLEMASK);
		if(s->replacements[i]&0x80)
			s->variables[s->replacements[i]&VARIABLEMASK] = !buf[i]&VARIABLEMASK;
		else
			s->variables[s->replacements[i]&VARIABLEMASK] = buf[i]&VARIABLEMASK;
	}

	//swap clause variables
	for(i=0;i<NUMCLAUSES;i++){
		char *a = (char*)&(s->clauses[i]);
		*a = s->replacements[*a&VARIABLEMASK]&VARIABLEMASK | ((*a&0x80)^(s->replacements[*a&VARIABLEMASK]&0x80));
		a ++;
		*a = s->replacements[*a&VARIABLEMASK]&VARIABLEMASK | ((*a&0x80)^(s->replacements[*a&VARIABLEMASK]&0x80));
		a ++;
		*a = s->replacements[*a&VARIABLEMASK]&VARIABLEMASK | ((*a&0x80)^(s->replacements[*a&VARIABLEMASK]&0x80));
		a ++;
		*a = s->replacements[*a&VARIABLEMASK]&VARIABLEMASK | ((*a&0x80)^(s->replacements[*a&VARIABLEMASK]&0x80));
		s->clauses[i] &= ~0xff;//kill em all, let god sort em out
	}
/*
	//apply negations
	for(i=0;i<NUMVARIABLES;i++){
		if(s->replacements[i]&0x80){
			s->variables[i] = !s->variables[i];//toggle variable value
			for(j=0;j<NUMCLAUSES;j++){//toggle every instance of it
				//FUCK C's OPERATOR PRECEDENCE
				if(i == ((s->clauses[j] >> 24 )&VARIABLEMASK))
					s->clauses[j] ^= (1<<31);
				if(i == ((s->clauses[j] >> 16 )&VARIABLEMASK))
					s->clauses[j] ^= (1<<23);
				if(i == ((s->clauses[j] >>  8 )&VARIABLEMASK))
					s->clauses[j] ^= (1<< 15);
			}

		}
	}
*/
	sortsat3(s);
}

void newsat3(sat3 *s){
	//generate random variable values
	int i;
	for(i=0;i<NUMVARIABLES;i++)
		s->variables[i] = (char)(((unsigned int)(goodrand()))%2);
	//generate random clauses
	for(i=0;i<NUMCLAUSES;i++){
		while(1){
			s->clauses[i] = goodrand();
			s->clauses[i] &= ~0xff;//last byte does nothing
			//printf("%d %d %d: %d %d %d\n", (clause >> 24)&0xff, (clause >> 16)&0xff, (clause >>  8)&0xff, s->variables[(clause >> 24)&0xff], s->variables[(clause >> 16)&0xff], s->variables[(clause >> 8)&0xff]);
			if(evalclause(s->clauses[i], s->variables))//brute force method, 7/8 chance of getting it each time?
				break;                             //probably not secure at all.
		}
	}
	sortsat3(s);

}
void permutesat3(sat3 *s){
	int i, j;

	//make new variable names
	for(i=0;i<NUMVARIABLES;i++){s->replacements[i]=i;}
	for(i=0;i<NUMVARIABLES;i++){//shuffle
		int swapwith = ((unsigned int)(goodrand()) %(NUMVARIABLES-i))+i;//not quite randomly distributed
//		if(swapwith == i){printf("asdffffffffffffffffff %d\n", i);}
		char temp = s->replacements[swapwith];
		s->replacements[swapwith] = s->replacements[i];
		s->replacements[i] = temp;
	}

	//randomly toggle variables
	//FIXME:need to store this in transform
	for(i=0;i<NUMVARIABLES;i++){
		if(goodrand()%2 ){
			//FIXME:XXX XXX XXX XXX XXX 
			s->replacements[i] |= 0x80;//mark as being swapped (swap in applytransform)
//			s->variables[i] = !s->variables[i];//toggle variable value
//			for(j=0;j<NUMCLAUSES;j++){//toggle every instance of it
//				//FUCK C's OPERATOR PRECEDENCE
//				if(i == ((s->clauses[j] >> 24 )&VARIABLEMASK))
//					s->clauses[j] ^= (1<<31);
//				if(i == ((s->clauses[j] >> 16 )&VARIABLEMASK))
//					s->clauses[j] ^= (1<<23);
//				if(i == ((s->clauses[j] >>  8 )&VARIABLEMASK))
//					s->clauses[j] ^= (1<< 15);
//			}
		}
	}
	applytransformsat3(s);
}
int checksat3(sat3 *s){
	int i;
	for(i=0;i<NUMCLAUSES;i++){
		if(evalclause(s->clauses[i], &s->variables[0]) == 0)
			return 0;
	}
	return 1;
}
void printsat3(sat3 *s){
	char isvalid = 1;
	int i;
	char *not = " \0N\0";
	printf("variables:\n");
	for(i=0;i<NUMVARIABLES;i++){
		printf("%4d:%d\n", i, s->variables[i]);
	}
	printf("clauses:\n");
	for(i=0;i<NUMCLAUSES;i++){
		printf("0x%10x\t", s->clauses[i]);

		int whichvar = ((s->clauses[i] >>24)&VARIABLEMASK);
		char *negstat = not+2*((s->clauses[i] >>(24+7))&1);
		printf("%s%4d (%d)     ", negstat, whichvar, s->variables[whichvar]);

		whichvar = ((s->clauses[i] >>16)&VARIABLEMASK);
		negstat = not+2*((s->clauses[i] >>(16+7))&1);
		printf("%s%4d (%d)     ", negstat, whichvar, s->variables[whichvar]);

		whichvar = ((s->clauses[i] >>8)&VARIABLEMASK);
		negstat = not+2*((s->clauses[i] >>(8+7))&1);
		printf("%s%4d (%d)     ", negstat, whichvar, s->variables[whichvar]);
		printf(" = %4d \n", evalclause( s->clauses[i], &s->variables[0]));
		isvalid = isvalid && evalclause( s->clauses[i], &s->variables[0]);
	}
	printf("%s\n", isvalid?"GOOD":"INVALID");

}
//so basically this function is a load of crap and you shouldn't believe anything it says
void printstatsat3(sat3 *s){
	//int counts[NUMVARIABLES] = {0};
	int counts[0xff] = {0};
	int i, j;
	for(i=0;i<NUMCLAUSES;i++){
		for(j=0;j<3;j++){
			//do we count a and not a as two, one or a half bit?
			counts[(s->clauses[i] >> (8+8*j))&VARIABLEMASK] ++;
			//counts[(s->clauses[i] >> (8+8*j))&(VARIABLEMASK|0x80)] ++;
		}
	}
	for(i=0;i<NUMVARIABLES;i++)
		printf("%d occurs %d times and inverted %d times\n", i, counts[i], counts[i|0x80]);
	int occurrencecounts[NUMCLAUSES*3] = {0};
	for(i=0;i</*NUMVARIABLES*/0xff;i++)
		occurrencecounts[counts[i]] ++;
	double permutations=1;
//	int bits = 0;
	for(i=1;i<NUMCLAUSES*3;i++){
/*
		int cmpto = 1;
		int bitshere = 0;
		for(;cmpto < occurrencecounts[i];cmpto <<= 1)
			bitshere ++;//THIS IS NOT RIGHT
		bits += bitshere;
*/
		double permshere = 1;
		double j_;
		for(j_=1;j_<occurrencecounts[i];j_++)//n! permutations
			permshere *= j;//this gets slow with a butload of permutations
		//permutations *= (permshere+1)/2;//????????????????
		if(occurrencecounts[i] > 1)//things used one time are interchangeable and so don't add entropy?  maybe?
			permutations += (permshere+1)/2;//should  be at least this, right?
		if(occurrencecounts[i] != 0)
			printf("%d variables occur %d times, ~%.0f permutations %s\n", occurrencecounts[i], i, (float)permshere, occurrencecounts[i] <2?" (VULNERABLE)":"");
	}
	int bits = 0;
	double accum = 1;
	while((accum *= 2)< permutations)
		bits ++;

	printf("%.0f permutations give estimated %d bits total entropy?\n", (float)permutations, bits);
	if(NUMVARIABLES > bits)
		printf("estimated entropy is less than key space (%d bits)!  VULNERABLE TO STATISTICAL ANALYSIS\n", NUMVARIABLES);
	else
		printf("estimated entropy is better than key space (good, but DO NOT TRUST THIS)\n");
	
//	printf("estimated %d bits total entropy?\n", bits);
}


void loadsat3(sat3 *s, char fn[]){
	FILE *f = fopen(fn, "r");
	if(f == NULL){
		printf("could not open file %s\n", fn);
		return;
	}
	fread(s, sizeof(sat3), 1, f);
	fclose(f);
}
void savesat3(sat3 *s, char fn[]){
	FILE *f = fopen(fn, "w");
	if(f == NULL){
		printf("could not open file %s\n", fn);
		return;
	}
	fwrite(s, sizeof(sat3), 1, f);
	fclose(f);
}

void clearanssat3(sat3 *s){
	int i;
	for(i=0;i<NUMVARIABLES;i++)
		s->variables[i] = 0;
}
