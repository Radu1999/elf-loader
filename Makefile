build:
	gcc -m32 main.c loader.c exec_parser.c -o main

clean:
	rm main