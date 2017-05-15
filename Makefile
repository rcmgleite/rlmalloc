all:
	gcc -std=c11 -O0 -g rlmalloc.c main.c -o main

clean:
	$(RM) main
