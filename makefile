myshell:main.c
	gcc main.c -o myshell -std=c99 -g -w


.PHONY:clean

clean:
	rm -rf myshell
