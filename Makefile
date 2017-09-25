mq: main.cpp
	rm -rf *.o
	g++ main.cpp functions.cpp -lrt -o ipc_all
	echo "Executable: ipc_all"

all: mq
