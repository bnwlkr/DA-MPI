SRC = migrate.c dampi.c profile.c sendrecv.c
HDR = migrate.h dampi.h profile.h sendrecv.h

CC=mpicc -std=gnu99

dampi_profile: dampi_profile.c $(SRC) $(HDR)
	$(CC) -o dampi_profile dampi_profile.c $(SRC)

prime_dampi: prime_dampi.c $(SRC) $(HDR)
	$(CC) -o prime_dampi prime_dampi.c $(SRC)

prime_dampi_profile: prime_dampi_profile.c $(SRC) $(HDR)
	$(CC) -o prime_dampi_profile prime_dampi_profile.c $(SRC)

prime_dampi_static: prime_dampi_static.c $(SRC) $(HDR)
	$(CC) -o prime_dampi_static prime_dampi_static.c $(SRC)


prime_no_dampi: prime_no_dampi.c $(SRC) $(HDR)
	$(CC) -o prime_no_dampi prime_no_dampi.c $(SRC)

all: prime_dampi_profile prime_dampi prime_no_dampi dampi_profile prime_dampi_static

clean:
	rm -rf prime_dampi_profile prime_dampi prime_no_dampi dampi_profile