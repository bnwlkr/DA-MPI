SRC = migrate.c dampi.c profile.c sendrecv.c
HDR = migrate.h dampi.h profile.h sendrecv.h

prime: prime.c $(SRC) $(HDR)
	mpicc -o prime prime.c $(SRC)

main: main.c $(SRC) $(HDR)
	mpicc -o main main.c $(SRC)

all: prime main

clean:
	rm -rf main prime
