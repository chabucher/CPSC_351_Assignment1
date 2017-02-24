mq: main.cpp
	rm -rf *.o
	g++ main.cpp functions.cpp -lrt -o main.o
	echo "Executable: main.o"

all: mq
