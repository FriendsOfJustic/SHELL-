myshell:main.c
	gcc main.c -o myshell -std=c99 -O


.PHONY:clean

clean:
	rm -rf myshell
