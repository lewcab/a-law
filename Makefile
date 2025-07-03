.phony all:
all: clean main asm test

main: main.c
	gcc -o main main.c aLaw.c

asm: main.c
	gcc -S main.c

test: aLawTest.c
	gcc -o aLawTest aLawTest.c aLaw.c

.PHONY clean:
clean:
	rm -rf *.o *.exe *.s main aLawTest
