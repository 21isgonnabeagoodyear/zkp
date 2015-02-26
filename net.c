#include "sat.h"
#include <sys/socket.h>
//???
//#include <sys/types.h>
//for htonl, etc
#include <netinet/in.h>



#include <stdio.h>
#include <errno.h>

//for system
#include <stdlib.h>

//for sleep
#include <unistd.h>

#define NUMTRIES 20

#define FAILDELAY 1

//let prover try to prove to us once
//returns nonzero if prover is lying to us
int give_prover_try(int fd, sat3 *pubkey){
	sat3 thistry;
	int bytesread = read(fd, &(thistry.clauses), sizeof(thistry.clauses));
	ntohsat3(&thistry);
	if(bytesread != sizeof(thistry.clauses)){
		printf("prover did not send the whole set of clauses!\n");
		return -1;
	}
	if(goodrand()%2){
//printf("ask for variables\n");
		if(write(fd, "v", 1) != 1){//request variables
			printf("failed to send v!\n");
			return -1;
		}
		bytesread = read(fd, &(thistry.variables), sizeof(thistry.variables));
		if(bytesread != sizeof(thistry.variables)){
			printf("prover did not send the whole set of variables!\n");
			return -1;
		}

		if(checksat3(&thistry) == 0){
printsat3(&thistry);
			printf("prover is a trickser! (variables don't work)\n");
			return 1;
		}
	}else{
//printf("ask for transform\n");
		if(write(fd, "t", 1) != 1){//request transform
			printf("failed to send t!\n");
			return -1;
		}
		sat3 shouldbethistry = *pubkey;
		bytesread = read(fd, &(shouldbethistry.replacements), sizeof(shouldbethistry.replacements));
		if(bytesread != sizeof(shouldbethistry.replacements)){
			printf("prover did not send the whole transform!");
			return -1;
		}
		applytransformsat3(&shouldbethistry);
		//printsat3(&shouldbethistry);
		int j;
		for(j=0;j<NUMCLAUSES;j++){
			if(thistry.clauses[j] != shouldbethistry.clauses[j]){
				printf("prover is a trickser! (transform doesn't work for %d)\n", j);
				return 1;
			}

		}
	//	if(memcmp(&shouldbethistry.clauses[0], &pubkey->clauses[0], sizeof(pubkey->clauses)) != 0){
	//		printf("prover is a trickser! (transform doesn't work)\n");
	//		return 1;
	//	}
		

	}
	return 0;
}


//listen for incoming provers
int listen_for_provers(short port, sat3 *goal){
	int i;
	struct sockaddr_in listenaddr = {0};
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1){
		printf("could not create socket\n");
		return -1;
	}
	listenaddr.sin_family = AF_INET;
	listenaddr.sin_addr.s_addr = INADDR_ANY;//listen on all interfaces?
	listenaddr.sin_port = htons(port);
	if(bind(sock, (struct sockaddr *)&listenaddr, sizeof(listenaddr)) <0){
		printf("could not bind socket\n");
		close(sock);
		return -1;
	}
	listen(sock, -1);//wait forever?
	while(1){
		struct sockaddr_in remoteaddress;
		int addrsize = sizeof(listenaddr);
		printf("waiting for provers...\n");
		int connfd = accept(sock, ((struct sockaddr *)(&remoteaddress)), &addrsize);
		if(connfd <0){
			printf("could not accept connection\n");
			close(sock);
			return -1;
		}
		printf("accepted connection from %d.%d.%d.%d\n", ((unsigned char*)&remoteaddress.sin_addr)[0], ((unsigned char*)&remoteaddress.sin_addr)[1], ((unsigned char*)&remoteaddress.sin_addr)[2], ((unsigned char*)&remoteaddress.sin_addr)[3]);
		for(i=0;i<NUMTRIES;i++){
			if(give_prover_try(connfd, goal) !=0)
				break;
		}
		close(connfd);
		if(i == NUMTRIES){
			printf("this fellow passes!\nrunning ./zkpsuccess.sh...\n");
			system("./zkpsuccess.sh");
		}
		else{
			printf("failed after %d attempts, rate limiting\n", i);
			sleep(FAILDELAY);//rate limit to prevent brute force
		}
	}
	close(sock);
}

int send_prove_try(int fd, sat3 *privkey){
	sat3 permuted = *privkey;
	char whichtheywant = 0;
	permutesat3(&permuted);
	htonsat3(&permuted);//prep for sending
	//printsat3(&permuted);
	if(write(fd, &permuted.clauses[0], sizeof(permuted.clauses)) != sizeof(permuted.clauses)){
		printf("failed to send clauses!\n");
		return -1;
	}
	if(read(fd, &whichtheywant, 1) != 1){
		printf("failed to read which they want! (error %d)\n", errno);
		return -1;
	}
	if(whichtheywant == 'v'){
printf("they asked for variables\n");
		if(write(fd, &permuted.variables[0], sizeof(permuted.variables)) != sizeof(permuted.variables)){
			printf("failed to send variables!\n");
			return -1;
		}
	}else{
printf("they asked for transform\n");
		if(write(fd, &permuted.replacements[0], sizeof(permuted.replacements)) != sizeof(permuted.replacements)){
			printf("failed to send replacements!\n");
			return -1;
		}
	}
	return 0;
}

int send_prove_request(char host[], short port, sat3 *privkey){
	int i;
	struct sockaddr_in sendtoaddr = {0};
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1){
		printf("could not create socket\n");
		return -1;
	}
	sendtoaddr.sin_family = AF_INET;
	sendtoaddr.sin_port = htons(port);
	if(inet_pton(AF_INET, host, &sendtoaddr.sin_addr) <=0){
		printf("could not parse host address\n");
		close(sock);
		return -1;
	}
	if(connect(sock, (struct sockaddr*)(&sendtoaddr), sizeof(sendtoaddr)) != 0){
		printf("could not connect to host\n");
		close(sock);
		return -1;
	}
	//write(sock, "hello world", 11);
	for(i=0;i<NUMTRIES;i++){
		if(send_prove_try(sock, privkey) != 0)
			break;
	}
	if(i != NUMTRIES){
		printf("quit working after %d tries\n", i);
	}
	close(sock);

}



int cmain(int argc, char *argv[]){
	char *destip = "127.0.0.1";
	if(argc > 1)
		destip = argv[1];
	sat3 mysecret;
	loadsat3(&mysecret, "privkey.bin");
	printsat3(&mysecret);
	printstatsat3(&mysecret);

	//send_prove_request("127.0.0.1", 8600, &mysecret);
	
	send_prove_request(destip, 8600, &mysecret);
	printf("sat3 is %d bytes\n", sizeof(sat3));

}
int smain(int argc, char *argv[]){
	sat3 pubkey;
//	newsat3(&pubkey);//generate private key
//	savesat3(&pubkey, "privkey.bin");//write for client to use
	loadsat3(&pubkey, "pubkey.bin");
//	clearanssat3(&pubkey);//forget the private parts
	printsat3(&pubkey);

	listen_for_provers(8600, &pubkey);
}
int gmain(int argc, char *argv[]){
	sat3 pubkey;
	newsat3(&pubkey);//generate private key
	savesat3(&pubkey, "privkey.bin");//write for client to use
	clearanssat3(&pubkey);//forget the private parts
	savesat3(&pubkey, "pubkey.bin");//write for client to use
//	clearanssat3(&pubkey);//forget the private parts
//	printsat3(&pubkey);
//	listen_for_provers(8600, &pubkey);
}

int main(int argc, char *argv[]){
#ifdef CLIENT
	cmain(argc, argv);
#else
#ifdef GENERATOR
	gmain(argc, argv);
#else
	smain(argc, argv);
#endif
#endif
}
