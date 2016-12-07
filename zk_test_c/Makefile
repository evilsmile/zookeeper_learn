CC=g++
CFLAGS=-g -Wall -std=c++0x
LDFLAGS=-lzookeeper_mt -lpthread -lssl

SRCS=$(shell ls *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))
BIN=zk_test

all:$(BIN)

zk_test:$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o:%.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f zk_test *.o
