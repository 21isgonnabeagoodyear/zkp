all: testlisten_ testsend_
	gcc -g satmain.c sat.c rng.c -o sat
testlisten_:
	gcc -g net.c sat.c rng.c -o testlisten
testsend_:
	gcc -g -DCLIENT net.c sat.c rng.c -o testsend
