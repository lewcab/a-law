.phony all:
all: clean main asm test

main: main.c
	gcc -o main.exe main.c aLaw.c

asm: main.c
	gcc -S main.c

test: aLawTest.c
	gcc -o aLawTest.exe aLawTest.c aLaw.c

.PHONY clean:
clean:
	rm -rf *.o *.exe *.s
