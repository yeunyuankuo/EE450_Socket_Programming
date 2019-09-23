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
#define AWS_TCP "26689"	// aws_tcp for monitor

// get socket address, IPv4 or IPv6 --- code from Beej's Documentation
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)-> sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)-> sin6_addr);
}

int main() {
	// Set up TCP connection --- code from Beej's Documentation
	int sockfd2 = 0;
	struct addrinfo hints2, *servinfo2, *p2;
	int rv2;

	memset(&hints2, 0, sizeof hints2);
	hints2.ai_family = AF_UNSPEC;
	hints2.ai_socktype = SOCK_STREAM;

	if ((rv2 = getaddrinfo(HOST, AWS_TCP, &hints2, &servinfo2)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv2));
		return 1;
	}

	// loop through all the results and connect to the first we can --- code from Beej's Documentation
	for (p2 = servinfo2; p2 != NULL; p2 = p2->ai_next) {
		if ((sockfd2 = socket(p2->ai_family, p2->ai_socktype, p2->ai_protocol)) == -1) {
			perror("monitor: socket");
			continue;
		}
		if (connect(sockfd2, p2->ai_addr, p2->ai_addrlen) == -1) {
			close(sockfd2);
			perror("monitor: connect");
			continue;
		}
		break;
	}

	if (p2 == NULL) {
		fprintf(stderr, "monitor: failed to connect\n");
		return 2;
	}
	freeaddrinfo(servinfo2);	// all done with this structure
	printf("The monitor is up and running. \n");

	while(1) {
		int linkID, fsize, power;
		recv(sockfd2, (char *)&linkID, sizeof linkID, 0);	// recieve <input> from AWS
		recv(sockfd2, (char *)&fsize, sizeof fsize, 0);
		recv(sockfd2, (char *)&power, sizeof power, 0);

		printf("The monitor recieved input link ID=<%d>, size=<%d>, and power=<%d> from the AWS\n", linkID, fsize, power);

		int result;
		recv(sockfd2, (char *)&result, sizeof result, 0);

		if (result == 0) {
			printf("Found no matches for link <%d>\n", linkID);
		} else if (result == 1) {
			double Tt, Tp, delay;
			recv(sockfd2, (char *)&Tt, sizeof Tt, 0);
			recv(sockfd2, (char *)&Tp, sizeof Tp, 0);
			recv(sockfd2, (char *)&delay, sizeof delay, 0);
			printf("The result for link <%d>:\nTt = <%f>ms, \nTp = <%f>ms, \nDelay = <%f>ms\n\n", linkID, Tt, Tp, delay);
		}
	}
	close(sockfd2);
	return 0;

}