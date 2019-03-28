CFLAGS=-Wall -std=c11 -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

icc: $(OBJS)
	$(CC) -o icc $(OBJS) $(LDFLAGS)

$(OBJS): icc.h

test: icc
	./icc -test
	./test.sh

clean:
	rm -f icc *.o *~ tmp*
