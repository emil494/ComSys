#GCC=gcc -O3 -g -Wall -Wextra -pedantic -std=gnu11
GCC=gcc -g -Wall -Wextra -pedantic -std=gnu11
LD_FLAGS= -lpthread

all: networking

rebuild: clean all

compsys_helpers.o: compsys_helpers.c compsys_helpers.h
	$(GCC) -c $< -o $@

common.o: common.c common.h
	$(GCC) -c $< -o $@

networking: networking.c networking.h common.o compsys_helpers.o sha256.o
	$(GCC) $< *.o -o $@ $(LD_FLAGS)

sha256.o : sha256.c sha256.h
	$(CC) $(CFLAGS) -c $< -o $@

zip: ../../src.zip

../../src.zip:
	make clean
	cd ../.. && zip -r A3/src.zip A3/src/Makefile A3/src/*.c A3/src/*.h A3/python/*.py A3/python/*/*.py A3/config.yaml

clean:
	rm -rf *.o networking sha256 vgcore*
