SRC = migrate.c dampi.c profile.c sendrecv.c
HDR = migrate.h dampi.h profile.h sendrecv.h

dampi_profile: dampi_profile.c $(SRC) $(HDR)
	mpicc -o dampi_profile dampi_profile.c $(SRC)

prime_dampi: prime_dampi.c $(SRC) $(HDR)
	mpicc -o prime_dampi prime_dampi.c $(SRC)

prime_no_dampi: prime_no_dampi.c $(SRC) $(HDR)
	mpicc -o prime_no_dampi prime_no_dampi.c $(SRC)

all: prime_dampi prime_no_dampi dampi_profile

clean:
	rm -rf prime_dampi prime_no_dampi dampi_profile dampi_p_*
