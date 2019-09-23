all:
	gcc -o serverA.o serverA.c -lnsl -lresolv
	gcc -o serverB.o serverB.c -lnsl -lresolv
	gcc -o serverC.o serverC.c -lm -lnsl -lresolv
	gcc -o aws.o aws.c -lnsl -lresolv
	gcc -o monitor.o monitor.c -lnsl -lresolv
	gcc -o client client.c -lnsl -lresolv

.PHONY: serverA serverB serverC aws monitor clean

serverA:
	./serverA.o

serverB:
	./serverB.o

serverC:
	./serverC.o

aws:
	./aws.o

monitor:
	./monitor.o

clean:
	rm serverA.o serverB.o serverC.o aws.o monitor.o client



