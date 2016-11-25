CC=g++
CFLAGS=-g -Wall

SRCS=$(shell ls *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))
BIN=zk_test

all:$(BIN)

zk_test:$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lzookeeper_mt -lpthread

%.o:%.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f zk_test *.o
