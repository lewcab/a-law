.phony all:
all: clean muLaw

muLaw: muLaw.c
	gcc -o muLaw muLaw.c

.PHONY clean:
clean:
	-rm -rf *.o *.exe mts
