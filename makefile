#makefile

cs352proxy: cs352proxy.c
	gcc -Wall -o cs352proxy cs352proxy.c

clean:
	rm -f *.o
	rm cs352proxy

#	usage:	./cs352proxy <host> <port> <local interface>
#			./cs352proxy 192.168.2.2 80 tun1
