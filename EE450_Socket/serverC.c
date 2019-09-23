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
#include <math.h>

#define HOST "localhost"
#define UDP_C "23689"


void compute(int linkID, int fsize, int power, int BW, float length, float velocity, float NP, int sockfd, struct sockaddr_storage their_addr, socklen_t addr_len) {

	/*printf("linkID: %d\n", linkID);
	printf("fsize: %d\n", fsize);
	printf("power: %d\n", power);

	printf("BW: %d\n", BW);
	printf("length: %.1f\n", length);
	printf("velocity: %.1f\n", velocity);
	printf("NP: %.1f\n", NP);*/

	double s = pow(10, ((double)power/10)) / 1000;
	//printf("s: %f\n", s);

	double np = pow(10, ((double)NP/10)) / 1000;
	//printf("np: %f\n", np);

	double c = ((double)BW*1000000)*(log(1 + (s/np))/log(2));
	//printf("c: %f\n", c);

	double Tt = (double)fsize/c * 1000;
	//printf("Tt: %f\n", Tt);

	double Tp= ((double)length*1000)/((double)velocity*10000000) * 1000;
	//printf("Tp: %f\n", Tp);

	double delay = Tt + Tp;
	//printf("delay: %f\n", delay);


	printf("The Server C finished the calculation for link <%d>\n", linkID);
	//printf("Tt: %f\n", Tt);
	//printf("Tp: %f\n", Tp);
	//printf("delay: %f\n", delay);

	sendto(sockfd, (char *)&Tt, sizeof Tt, 0, (struct sockaddr *) &their_addr, addr_len);
	sendto(sockfd, (char *)&Tp, sizeof Tp, 0, (struct sockaddr *) &their_addr, addr_len);
	sendto(sockfd, (char *)&delay, sizeof delay, 0, (struct sockaddr *) &their_addr, addr_len);
	printf("The Server C finished sending the output to AWS\n\n");

}

int main() {
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

	if ((rv = getaddrinfo(HOST, UDP_C, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("serverC: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("serverC: bind");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "serverC: failed to bind socket\n");
		exit(1);
	}
	freeaddrinfo(servinfo);	// all done with this structure
	printf("The Server C is up and running using UDP on port <%s>.\n", UDP_C);

	while (1) {
		addr_len =  sizeof their_addr;
		int linkID, fsize, power, BW;
		float length, velocity, NP;
		recvfrom(sockfd, (char *)&linkID, sizeof linkID, 0, (struct sockaddr *)&their_addr, &addr_len);
		recvfrom(sockfd, (char *)&fsize, sizeof fsize, 0, (struct sockaddr *)&their_addr, &addr_len);
		recvfrom(sockfd, (char *)&power, sizeof power, 0, (struct sockaddr *)&their_addr, &addr_len);
		recvfrom(sockfd, (char *)&BW, sizeof BW, 0, (struct sockaddr *)&their_addr, &addr_len);
		recvfrom(sockfd, (char *)&length, sizeof length, 0, (struct sockaddr *)&their_addr, &addr_len);
		recvfrom(sockfd, (char *)&velocity, sizeof velocity, 0, (struct sockaddr *)&their_addr, &addr_len);
		recvfrom(sockfd, (char *)&NP, sizeof NP, 0, (struct sockaddr *)&their_addr, &addr_len);
		printf("The Server C recieved link information of link ID=<%d>, size=<%d>, and signal power=<%d>\n", linkID, fsize, power);
		compute(linkID, fsize, power, BW, length, velocity, NP, sockfd, their_addr, addr_len);
		//close(sockfd);
	}
}