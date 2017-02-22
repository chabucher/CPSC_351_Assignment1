INC_DIR=/Users/charlesbucher/Documents/School/Spring_2017/CPSC_351/Homework/mq_example

mq: mq_example.c $(INC_DIR)/util/util.c
	gcc main.cpp $(INC_DIR)/util/util.c -I$(INC_DIR) -lpthread -lrt -o mq_example.o

all: mq

clean:
	rm -rf *.o	
