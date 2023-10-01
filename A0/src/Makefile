# Your GCC compiler
CC=gcc
# On Macs gcc actually calls clang (another C compiler).
# To use gcc, install gcc (with brew) and point to the spcific version
# For the latest uncomment the following line
# CC=gcc-13

# All the flags in the (CompSys) world
CFLAGS=-std=c11 -Wall -Werror -Wextra -pedantic -g3

# Compile your program (make og make file)
file: file.c Makefile
	$(CC) $(CFLAGS) -o file file.c

# Used for generating a zip (make ../src.zip)
../src.zip: file.c Makefile test.sh
	cd .. && zip src.zip src/file.c src/Makefile src/test.sh

# Used for removing generated files (make clean)
clean:
	rm -f *.o
	rm -f file
	rm -f file.c~     # For emacs uses
	rm -rf file.dSYM  # For Mac users

.PHONY: clean
