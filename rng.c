#include "rng.h"
#include <stdio.h>

int goodrand(){
	static FILE *f = NULL;
	if(f == NULL)
		f = fopen("/dev/urandom", "r");
	int rv;
	fread(&rv, 4, 1, f);
	return rv;
}
