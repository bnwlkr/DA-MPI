main: main.c profile.c
	mpicc -o main main.c profile.c


clean:
	rm -rf *.o main
