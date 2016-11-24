CC=gcc
CFLAGS=-g

zk_test:logger.o zk_test.o 
	$(CC) $(CFLAGS) -o $@ $^ -lzookeeper_mt -lpthread

%.o:%.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f zk_test *.o
