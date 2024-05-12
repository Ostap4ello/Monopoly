all: build run

build: 
	gcc main.c -lncurses -o main.so
run:
	./main.so # -b gameboard/gameboard6.txt -t properties/properties3.txt
clean:
	rm main.so

nc: buildnc runnc

buildnc:
	gcc nctest.c -lncurses -o nctest.so
runnc:
	./nctest.so
cleannc: 
	rm nctest.so

