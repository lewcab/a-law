.phony all:
all: clean aLaw aLaw.s

aLaw: aLaw.c
	gcc -o aLaw aLaw.c

aLaw.s: aLaw.c
	gcc -S aLaw.c

.PHONY clean:
clean:
	rm -rf *.o *.exe *.s aLaw
