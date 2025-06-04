.phony all:
all: clean muLaw muLaw.s

muLaw: muLaw.c
	gcc -o muLaw muLaw.c

muLaw.s: muLaw.c
	gcc -S muLaw.c

.PHONY clean:
clean:
	rm -rf *.o *.exe *.s muLaw
