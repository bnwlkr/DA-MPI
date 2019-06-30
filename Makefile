export TMPDIR=/tmp
export OMPI_MCA_btl=self,tcp

SRC = migrate.c dampi.c profile.c sendrecv.c
HDR = migrate.h dampi.h profile.h sendrecv.h

prime_dampi: prime_dampi.c $(SRC) $(HDR)
	mpicc -o prime_dampi prime_dampi.c $(SRC)

prime_no_dampi: prime_no_dampi.c $(SRC) $(HDR)
	mpicc -o prime_no_dampi prime_no_dampi.c $(SRC)


main: main.c $(SRC) $(HDR)
	mpicc -o main main.c $(SRC)

all: prime_dampi prime_no_dampi main

clean:
	rm -rf main prime_dampi prime_no_dampi
