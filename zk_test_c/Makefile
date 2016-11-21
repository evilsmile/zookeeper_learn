CC=gcc
CFLAGS=-g

zk_test:zk_test.c
	$(CC) $(CFLAGS) -o $@ $< -lzookeeper_mt

clean:
	rm -f zk_test
