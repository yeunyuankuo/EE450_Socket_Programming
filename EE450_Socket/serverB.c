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
#define UDP_B "22689"

int linkID, BW;
float length, velocity, NP;

int search(int givenLinkID, int sockfd, struct sockaddr_storage their_addr, socklen_t addr_len) {
	FILE* fpointer = NULL;
	fpointer = fopen("database_b.csv", "r");
	if (fpointer == NULL) {
		printf("Couldn't open file.\n");
		exit(0);
	}
	int match = 0;
	while (!feof(fpointer)) {
		fscanf(fpointer, "%d%*[,] %d%*[,] %f%*[,] %f%*[,] %f%*[,]", &linkID, &BW, &length, &velocity, &NP);
		if (linkID == givenLinkID) {
			match = 1;
			printf("The server B has found <%d> match\n", match);
			//printf("Search results: %d, %d, %.1f, %.1f, %.1f \n", linkID, BW, length, velocity, NP);
			sendto(sockfd, (char *)&match, sizeof match, 0, (struct sockaddr *) &their_addr, addr_len);
			sendto(sockfd, (char *)&BW, sizeof BW, 0, (struct sockaddr *) &their_addr, addr_len);
			sendto(sockfd, (char *)&length, sizeof length, 0, (struct sockaddr *) &their_addr, addr_len);
			sendto(sockfd, (char *)&velocity, sizeof velocity, 0, (struct sockaddr *) &their_addr, addr_len);
			sendto(sockfd, (char *)&NP, sizeof NP, 0, (struct sockaddr *) &their_addr, addr_len);
		}
	}
	if (match == 0) {
		printf("The server B has found <%d> match\n", match);
		sendto(sockfd, (char *)&match, sizeof match, 0, (struct sockaddr *) &their_addr, addr_len);
	}
	printf("The Server B finished sending the output to AWS\n\n");
	fclose(fpointer);
	return match;
}

int main(void) {
	// Set up UDP --- code from Beej's Documentation
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(HOST, UDP_B, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("serverB: socket\n");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("serverB: bind\n");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "serverB: failed to bind socket\n");
		exit(1);
	}
	freeaddrinfo(servinfo);	// all done with this structure
	printf("The Server B is up and running using UDP on port <%s>.\n", UDP_B);

	while (1) {
		addr_len =  sizeof their_addr;
		int givenLinkID;
		recvfrom(sockfd, (char *)&givenLinkID, sizeof givenLinkID, 0, (struct sockaddr *)&their_addr, &addr_len);
		printf("The Server B recieved input <%d>\n", givenLinkID);
		int match = search(givenLinkID, sockfd, their_addr, addr_len);
		//close(sockfd);
	}
}