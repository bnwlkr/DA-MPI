main: *.c *.h
	mpicc -o main *.c

clean:
	rm -rf main
