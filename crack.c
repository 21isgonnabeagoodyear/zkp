#include "sat.h"
#include <stdlib.h>
#include <stdio.h>

sat3 crack(sat3 tocheck, unsigned long long startat, /*this is pretty terrible*/unsigned long long *endedat){
	int i;
	//FIXME: for some reason this doesn't stop
/*k
	if(startat >= (unsigned long long)(((unsigned long long)(1)) << NUMVARIABLES)){
		//if(endedat != NULL)*endedat = startat;
		return tocheck;
	}
*/
	for(; startat < (unsigned long long)(((unsigned long long)(1)) << NUMVARIABLES)/*NUMVARIABLES can be > 64 but fuck it*/;startat++)
	{
//printf("%d\n", startat);
		for(i=0;i<NUMVARIABLES;i++)
			tocheck.variables[i] = (startat >> i)&1;
		if(checksat3(&tocheck) != 0)
		{
			if(endedat != NULL)*endedat = startat;
			return tocheck;
		}
		if((startat+1) % 1000000000 == 0)
			printf("passing %d billion\n", (startat+1)/1000000000);


	}

	printf("failed to crack key\n");
	//if(endedat != NULL)*endedat = startat;
	return tocheck;//magnets
}

/*
search the entire keyspace and count the solutions
*/
long long countsolutions(sat3 tocheck, unsigned long long startat){
	unsigned long long next = startat;
	unsigned long long prev = -1;
	long long count = 0;
	while(prev != next){
		prev = next;
		crack(tocheck, next+1, &next);
		if(prev != next)
			printf("found solution: 0x%lx\n",next);
		count ++;
	}
	return count-1;
}


void main(int argc, char *argv[]){
	if(argc <=2){
		printf("usage: %s filename startat");
		return;
	}
	unsigned long long startat = atoi(argv[2]);
	//startat <<= 32;//because atoi can't handle 64 bit ints
	sat3 tocrack;
	if(loadsat3(&tocrack, argv[1])){printf("failed to load keyfile\n");return;}
	printf("starting at %d billion\n", startat);
	printf("cracking via brute force method\n");
/*
	sat3 cracked = crack(tocrack, startat*1000000000, NULL);
	printsat3(&cracked);
*/
	printf("%d solutions\n", countsolutions(tocrack, startat*(unsigned long long)(1000000000)));

	

}
