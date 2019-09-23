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
#define AWS_TCP "25689"

// get socket address, IPv4 or IPv6 --- code from Beej's Documentation
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)-> sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)-> sin6_addr);
}

int main(int argc, char* argv[]) {
	// Check is linkID, size, power is provided by the user.
	if (argc <= 1) {
		printf("You didn't feed me (linkID, size, power)... I will die now :( \n");
		exit(1);
	} else if (argc > 1 && argc < 3) {
		printf("You miss something in (linkID, size, power)... I will die now :( \n");
		exit(1);
	}
	int linkID = atoi(argv[1]);
	int fsize = atoi(argv[2]);
	int power = atoi(argv[3]);

	// Set up TCP connection --- code from Beej's Documentation
	int sockfd = 0;
	struct addrinfo hints, *servinfo, *p;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(HOST, AWS_TCP, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can --- code from Beej's Documentation
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket\n");
			continue;
		}
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect\n");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	freeaddrinfo(servinfo);	// all done with this structure
	printf("The client is up and running. \n");

	send(sockfd, (char *)&linkID, sizeof linkID, 0);	// sends <input> to AWS
	send(sockfd, (char *)&fsize, sizeof fsize, 0);
	send(sockfd, (char *)&power, sizeof power, 0);
	printf("The client sent link ID=<%d>, size=<%d>, and power=<%d> to AWS\n", linkID, fsize, power);

	int result;
	recv(sockfd, (char *)&result, sizeof result, 0);	// recieve result of propagatin delay for specific link
	if (result == 0) {
		printf("Found no matches for link <%d>\n\n", linkID);
	} else {
		double delay;
		recv(sockfd, (char *)&delay, sizeof delay, 0);
		printf("The delay for link <%d> is <%f>ms\n\n", linkID, delay);

	}
	close(sockfd);
	return 0;
}