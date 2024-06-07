all: run

setup:
	cmake -S ./CMakeLists.txt -B ./build/
	cmake --build ./build/.

run:
	./build/main.so
