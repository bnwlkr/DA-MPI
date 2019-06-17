main: main.c profile.c send.c
	mpicc -o main main.c profile.c send.c


clean:
	rm -rf *.o main
