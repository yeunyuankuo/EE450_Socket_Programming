a. Yeun-Yuan Kuo

b. USC ID: 4542702689

c. What I have done in this assignment?
	In this assignment, I've successfully written TCP and UDP socket simulations amongst a client, a monitor, 3 backend-servers and a aws server.

d. What my code files are and what each one of them do?
    aws.c:
	AWS server that is responsible for the communication between backend-servers and client and monitor. It basically grabs what information it got from client and give it to backend-servers and monitor, then the aws will return whatever the backend-server gives to the client and monitor. In other words, aws is the middle man for client, monitor, and backend-servers.

    client.c:
	This program grabs information from the users (from command line) and sends it to AWS. Also , it will display the final result of what the backend-servers compute.

	serverA.c:
	ServerA is the storage server. This program goes into database A, database assigned to serverA, and searches for a match with the information given by the client from the AWS. It will print our whether it finds a match or not.

	serverB.c:
	Server B is the storage server. This program goes into database B, database assigned to serverB, and searches for a match with the information given by the client from the AWS. It will print our whether it finds a match or not.

	serverC.c:
	Server C is the server that does all the computation work. It uses information given by client from the AWS and the search result from the storage servers to correctly calculate the transmission delay, propagation delay, and end-to-end delay. Then it will send the result back to AWS server.

	monitor.c:
	Monitor basically outputs all the information that comes in from the client side and output from the server side.

	Makefile:
	Makefile is the file with all the functions that executes and run all my code files. You need to run my codes through these make calls:
		make all
		make serverA
		make serverB
		make serverC
		make aws
		make monitor
		./client <linkID> <SIZE> <POWER>

e. The format of all the messages exchanged:
	AWS:
	The AWS is up and running.
	The AWS recieved link ID=<3>, size=<10000>, and power=<-30> from the client using TCP over port <25689>.
	The AWS sent link ID=<3>, size=<10000>, and power=<-30> to the monitor using TCP over port <26689>.
	The AWS sent link ID=<3> to Backend-Server <A> using UDP over port <21689>
	The AWS sent link ID=<3> to Backend-Server <B> using UDP over port <22689>
	The AWS recieved <0> matches from Backend-Server <A> using UDP over port <21689>
	The AWS recieved <1> matches from Backend-Server <B> using UDP over port <22689>
	The AWS sent link ID=<3>, size=<10000>, power=<-30>, and link information to Backend-Server C using UDP over port <23689>
	The AWS recieved outputs from Backend-Server C using UDP over port <23689>
	The AWS send delay=<0.050149>ms to the client using TCP over port <25689>
	The AWS send detailed results to the monitor using TCP over port <26689>
	The AWS sent No Match to the monitor and the client using TCP over ports <26689> and <25689>, respectively

	serverA:
	The Server A is up and running using UDP on port <21689>.
	The Server A recieved input <3>
	The server A has found <0> match
	The Server A finished sending the output to AWS

	serverB:
	The Server B is up and running using UDP on port <22689>.
	The Server B recieved input <3>
	The server B has found <1> match
	The Server B finished sending the output to AWS

	serverC:
	The Server C is up and running using UDP on port <23689>.
	The Server C recieved link information of link ID=<3>, size=<10000>, and signal power=<-30>
	The Server C finished the calculation for link <3>
	The Server C finished sending the output to AWS

	monitor:
	The monitor is up and running. 
	The monitor recieved input link ID=<3>, size=<10000>, and power=<-30> from the AWS
	The result for link <3>:
	Tt = <0.000907>ms, 
	Tp = <0.049242>ms, 
	Delay = <0.050149>ms
	Found no matches for link <0>

	client:
	The client is up and running. 
	The client sent link ID=<3>, size=<10000>, and power=<-30> to AWS
	The delay for link <3> is <0.050149>ms
	Found no matches for link <0>
	
f. Any idiosyncrasy of my project (under what conditions the project fails, if any):
	I've didn't encounter any problem when running my program in Ubuntu.

g. Reused Code: 
	Some parts of my code are based on the tutorial from Beej's Guide to Network Programming.
