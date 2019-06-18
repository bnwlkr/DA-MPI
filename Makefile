main: *.c *.h
	mpicc -Wno-deprecated-declarations -o main *.c

clean:
	rm -rf *.o main
