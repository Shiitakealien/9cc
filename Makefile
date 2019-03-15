9cc: 9cc.c 9cc.h

test: 9cc
	./9cc -test
	./test.sh

clean:
	rm -f 9cc *.o *~ tmp*
