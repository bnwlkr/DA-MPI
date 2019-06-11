main: main.c profiler.c
	mpicc -o main main.c profiler.c


clean:
	rm -rf *.o main