#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define HOST "localhost"
#define UDP_A "21689"			// UDP for serverA
#define UDP_B "22689"			// UDP for serverB
#define UDP_C "23689"			// UDP for serverC
#define UDP_AWS "24689"			// TCP for AWS
#define TCP_CLIENT "25689"		// TCP for client
#define TCP_MONITOR "26689"		// TCP for monitor
#define BACKLOG 10				// how many pending connections queue will hold

struct finalResult {
	double Tt;
	double Tp;
	double delay;
};

struct linkInformation{
	int linkID;
	int BW;
	float length;
	float velocity;
	float NP;
};

void sigchld_handler(int s) {
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;
	while (waitpid(-1, NULL, WNOHANG) > 0) {
		errno = saved_errno;
	}
}

// get socket address, IPv4 or IPv6 --- code from Beej's Documentation
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)-> sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)-> sin6_addr);
}

int searchResult(int linkID, char ch, struct linkInformation *l) {
	// Set up UDP --- code from Beej's Documentation
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char * serverPort;

	if (ch == 'A') {
		serverPort = UDP_A;
	} else if (ch == 'B') {
		serverPort = UDP_B;
	}
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(HOST, serverPort, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("talker: socket\n");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		exit(1);
	}
	sendto(sockfd, (char *)&linkID, sizeof linkID, 0, p->ai_addr, p->ai_addrlen);
	printf("The AWS sent link ID=<%d> to Backend-Server <%c> using UDP over port <%s>\n", linkID, ch, serverPort);
	
	int match = 0;
	int BW;
	float length;
	float velocity;
	float NP;
	recvfrom(sockfd, (char *)&match, sizeof match, 0, NULL, NULL);
	//printf("The AWS recieved <%d> matches from Backend-Server <%c> using UDP over port <%s>\n", match, ch, serverPort);
	if (match == 1) {
		recvfrom(sockfd, (char *)&BW, sizeof BW, 0, NULL, NULL);
		recvfrom(sockfd, (char *)&length, sizeof length, 0, NULL, NULL);
		recvfrom(sockfd, (char *)&velocity, sizeof velocity, 0, NULL, NULL);
		recvfrom(sockfd, (char *)&NP, sizeof NP, 0, NULL, NULL);
		l->linkID = linkID;
		l->BW = BW;
		l->length = length;
		l->velocity = velocity;
		l->NP = NP;
	}
	return match;
}


int calcInput(int linkID, int fsize, int power, struct linkInformation *l,struct finalResult *r) {
	// Set up UDP --- code from Beej's Documentation
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(HOST, UDP_C, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("talker: socket\n");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "talker: failed to bind socket\n");
		exit(1);
	}

	sendto(sockfd, (char *)&linkID, sizeof linkID, 0, p->ai_addr, p->ai_addrlen);
	sendto(sockfd, (char *)&fsize, sizeof fsize, 0, p->ai_addr, p->ai_addrlen);
	sendto(sockfd, (char *)&power, sizeof power, 0, p->ai_addr, p->ai_addrlen);
	sendto(sockfd, (char *)&l->BW, sizeof l->BW, 0, p->ai_addr, p->ai_addrlen);
	sendto(sockfd, (char *)&l->length, sizeof l->length, 0, p->ai_addr, p->ai_addrlen);
	sendto(sockfd, (char *)&l->velocity, sizeof l->velocity, 0, p->ai_addr, p->ai_addrlen);
	sendto(sockfd, (char *)&l->NP, sizeof l->NP, 0, p->ai_addr, p->ai_addrlen);
	printf("The AWS sent link ID=<%d>, size=<%d>, power=<%d>, and link information to Backend-Server C using UDP over port <%s>\n", linkID, fsize, power, UDP_C);
	double Tt;
	double Tp;
	double delay;
	recvfrom(sockfd, (char *)&Tt, sizeof Tt, 0, NULL, NULL);
	recvfrom(sockfd, (char *)&Tp, sizeof Tp, 0, NULL, NULL);
	recvfrom(sockfd, (char *)&delay, sizeof delay, 0, NULL, NULL);
	printf("The AWS recieved outputs from Backend-Server C using UDP over port <%s>\n", UDP_C);
	r->Tt = Tt;
	r->Tp = Tp;
	r->delay = delay;
}

int main() {
	printf("The AWS is up and running.\n");

	// Set up TCP connection for Client--- code from Beej's Documentation
	int sockfd, client_sock;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr;
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;	// use my IP

	if ((rv = getaddrinfo(HOST, TCP_CLIENT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	// loop through all the results and connect to the first we can --- code from Beej's Documentation
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket\n");
			//continue;
			exit(1);
		}
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt\n");
			exit(1);
		}
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind\n");
			//continue;
			exit(1);
		}
		break;
	}

	freeaddrinfo(servinfo);	// all done with this structure
	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen\n");
		exit(1);
	}

	// Set up TCP connection for Monitor--- code from Beej's Documentation
	int sockfd2, monitor_sock;
	struct addrinfo hints2, *servinfo2, *p2;
	struct sockaddr_storage their_addr2;
	socklen_t sin_size2;
	struct sigaction sa2;
	int yes2=1;
	char s2[INET6_ADDRSTRLEN];
	int rv2;

	memset(&hints2, 0, sizeof hints2);
	hints2.ai_family = AF_UNSPEC;
	hints2.ai_socktype = SOCK_STREAM;
	hints2.ai_flags = AI_PASSIVE;	// use my IP

	if ((rv2 = getaddrinfo(HOST, TCP_MONITOR, &hints2, &servinfo2)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv2));
		return 1;
	}

	// loop through all the results and connect to the first we can --- code from Beej's Documentation
	for (p2 = servinfo2; p2 != NULL; p2 = p2->ai_next) {
		if ((sockfd2 = socket(p2->ai_family, p2->ai_socktype, p2->ai_protocol)) == -1) {
			perror("server: socket\n");
			//continue;
			exit(1);
		}
		if (setsockopt(sockfd2, SOL_SOCKET, SO_REUSEADDR, &yes2, sizeof(int)) == -1) {
			perror("setsockopt\n");
			exit(1);
		}
		if (bind(sockfd2, p2->ai_addr, p2->ai_addrlen) == -1) {
			close(sockfd2);
			perror("server: bind\n");
			//continue;
			exit(1);
		}
		break;
	}

	freeaddrinfo(servinfo2);	// all done with this structure
	if (p2 == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}
	if (listen(sockfd2, BACKLOG) == -1) {
		perror("listen\n");
		exit(1);
	}

	sin_size2 = sizeof their_addr;
	monitor_sock = accept(sockfd2, (struct sockaddr *)&their_addr, &sin_size2);
	if (monitor_sock == -1) {
		perror("accept2\n");
		//continue;
		exit(1);
	}

	while (1) {
		sin_size = sizeof their_addr;
		client_sock = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (client_sock == -1) {
			perror("accept\n");
			//continue;
			exit(1);
		}
		
		// get port of client
		/*inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
		struct sockaddr_in addrTheir;
		memset(&addrTheir, 0, sizeof(addrTheir));
		int len = sizeof(addrTheir);
		getpeername(client_sock, (struct sockaddr *) &addrTheir, (socklen_t *) &len);
		int client_port = addrTheir.sin_port;*/

		// recieve inputs from client
		int linkID, fsize, power;
		recv(client_sock, (char *)&linkID, sizeof linkID, 0);
		recv(client_sock, (char *)&fsize, sizeof fsize, 0);
		recv(client_sock, (char *)&power, sizeof power, 0);
		printf("The AWS recieved link ID=<%d>, size=<%d>, and power=<%d> from the client using TCP over port <%s>.\n", linkID, fsize, power, TCP_CLIENT);

		//sent info to monitor
		printf("The AWS sent link ID=<%d>, size=<%d>, and power=<%d> to the monitor using TCP over port <%s>.\n", linkID, fsize, power, TCP_MONITOR);
		send(monitor_sock, (char *)&linkID, sizeof linkID, 0);
		send(monitor_sock, (char *)&fsize, sizeof fsize, 0);
		send(monitor_sock, (char *)&power, sizeof power, 0);

		struct linkInformation resultA;
		int matchA = searchResult(linkID, 'A', &resultA);	// send input to serverA
		struct linkInformation resultB;	
		int matchB = searchResult(linkID, 'B', &resultB);	// send input to serverB
		printf("The AWS recieved <%d> matches from Backend-Server <%c> using UDP over port <%s>\n", matchA, 'A', UDP_A);
		printf("The AWS recieved <%d> matches from Backend-Server <%c> using UDP over port <%s>\n", matchB, 'B', UDP_B);

		int result;
		if (matchA == 0 && matchB == 0) {
			result = 0;

			//send result to client
			send(client_sock, (char *)&result, sizeof result, 0);

			//send result to monitor
			send(monitor_sock, (char *)&result, sizeof result, 0);
			printf("The AWS sent No Match to the monitor and the client using TCP over ports <%s> and <%s>, respectively\n\n", TCP_MONITOR, TCP_CLIENT);
		} else if (matchA == 1) {
			result = 1;
			/*printf("linkID: %d\n", resultA.linkID);
			printf("BW: %d\n", resultA.BW);
			printf("length: %.1f\n", resultA.length);
			printf("velocity: %.1f\n", resultA.velocity);
			printf("NP: %.1f\n", resultA.NP);*/

			//Send linkInformation to C
			struct finalResult fResultA;
			calcInput(linkID, fsize, power, &resultA ,&fResultA);			

			//send result to client
			send(client_sock, (char *)&result, sizeof result, 0);
			double delay = fResultA.delay;
			send(client_sock, (char *)&delay, sizeof delay, 0);
			printf("The AWS send delay=<%f>ms to the client using TCP over port <%s>\n", delay, TCP_CLIENT);

			//sent result to monitor
			send(monitor_sock, (char *)&result, sizeof result, 0);
			double Tt = fResultA.Tt;
			double Tp = fResultA.Tp;
			send(monitor_sock, (char *)&Tt, sizeof Tt, 0);
			send(monitor_sock, (char *)&Tp, sizeof Tp, 0);
			send(monitor_sock, (char *)&delay, sizeof delay, 0);
			printf("The AWS send detailed results to the monitor using TCP over port <%s>\n\n", TCP_MONITOR);
		} else if (matchB == 1) {
			result = 1;
			/*printf("linkID: %d\n", resultB.linkID);
			printf("BW: %d\n", resultB.BW);
			printf("length: %.1f\n", resultB.length);
			printf("velocity: %.1f\n", resultB.velocity);
			printf("NP: %.1f\n", resultB.NP);*/

			//Send linkInformation to C
			struct finalResult fResultB;
			calcInput(linkID, fsize, power, &resultB, &fResultB);

			//send result to client
			send(client_sock, (char *)&result, sizeof result, 0);
			double delay = fResultB.delay;
			send(client_sock, (char *)&delay, sizeof delay, 0);
			printf("The AWS send delay=<%f>ms to the client using TCP over port <%s>\n", delay, TCP_CLIENT);

			//send result to monitor
			send(monitor_sock, (char *)&result, sizeof result, 0);
			double Tt = fResultB.Tt;
			double Tp = fResultB.Tp;
			send(monitor_sock, (char *)&Tt, sizeof Tt, 0);
			send(monitor_sock, (char *)&Tp, sizeof Tp, 0);
			send(monitor_sock, (char *)&delay, sizeof delay, 0);
			printf("The AWS send detailed results to the monitor using TCP over port <%s>\n\n", TCP_MONITOR);
		} 
		close(client_sock);
	}
	return 0;
}