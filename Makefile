default:
	gcc main.c -o main

debug:
	gcc -g main.c -o main

release:
	gcc -O2 main.c -o main

clean:
	rm main
