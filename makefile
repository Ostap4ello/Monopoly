all: build run

build: 
	gcc main.c -lncurses -o main.so
run:
	./main.so 
clean:
	rm main.so







