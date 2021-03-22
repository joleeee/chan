default:
	gcc main.c -o main

debug:
	gcc -g main.c -o main

release:
	gcc -O2 main.c -o main

run: default
	./run &

clean:
	rm main
