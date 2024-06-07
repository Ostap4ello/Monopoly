all: run

setup:
	cmake -S . -B ./build
	cmake --build ./build/.

run:
	./build/main.so
