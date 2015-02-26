all: testlisten_ testsend_ testgen_
#	gcc -g satmain.c sat.c rng.c -o sat
testlisten_:
	gcc -g net.c sat.c rng.c -o testlisten
testsend_:
	gcc -g -DCLIENT net.c sat.c rng.c -o testsend
testgen_:
	gcc -g -DGENERATOR net.c sat.c rng.c -o testkeygen

#export PATH=$PATH:/mnt/huge/openwrt_glinet_buildroot/openwrt/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/bin
#export STAGING_DIR=/mnt/huge/openwrt_glinet_buildroot/openwrt/staging_dir/

glinet: testlisten_glinet_ testsend_glinet_ testgen_glinet_
#	mips-openwrt-linux-gcc -g satmain.c sat.c rng.c -o sat_glinet
testlisten_glinet_:
	mips-openwrt-linux-gcc -g net.c sat.c rng.c -o testlisten_glinet
testsend_glinet_:
	mips-openwrt-linux-gcc -g -DCLIENT net.c sat.c rng.c -o testsend_glinet
testgen_glinet_:
	mips-openwrt-linux-gcc -g -DGENERATOR net.c sat.c rng.c -o testkeygen_glinet
