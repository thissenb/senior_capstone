/*********************************************
*	Bradley Thissen
*	8/5/2016
*	otp_dec_d.c <v1.0>
*	
*	A daemon program that will always run in the
*	background waiting for an input/key to be
*	sent in via socket from otp_dec to be
*	decrypted and feed back to otp_dec for output.
*	
*	NOTE: Portions of this code were taken from
*	server.c found in the canvas page for
*	assignment 4 as well as otp_enc_d.c
************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

int loop(int temp) { //loops through an algorithm till number in range found
	
	int letter = 0;
	
	while (temp < 65) { //find the number in ascii range
		temp = (temp + 27);
	}
	letter = temp; //give letter the number found
	
	return letter;
}

int main(int argc, char *argv[]) {
//input should only be otp_dec_d <port#>

    int sockfd, newsockfd, portno; 	//socket FD's and port#
    socklen_t clilen; 				//for socket manipulation
	int i = 0;						//general purpose int variable
	int tmp = 0;					//for key cypher addition
	 
    char input[75000];				//holds the input plain-text
	char key[75000];				//holds the cypher key input
	char output[75000];				//holds the output cypher-text
	
	pid_t pid;						//holds process id of child/parent
	
    struct sockaddr_in serv_addr, cli_addr;
	int n;
	 
    if (argc < 2) { //see if port number was passed in
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
	 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	//open socket file descriptor
	 
    if (sockfd < 0) { //make sure the file descriptor worked
		fprintf(stderr, "otp_dec_d: Error opening socket");
	}
	
	//clear the socket structure for input
    bzero((char *) &serv_addr, sizeof(serv_addr));
	 
    portno = atoi(argv[1]); //grab the port number from input
	
	//set up socket connection
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
	 
	//check if the connection worked
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		fprintf(stderr, "otp_dec_d: ERROR on binding");
		exit(1);
	}
	
do { //run in infinite loop for concurrent connections
	
	//wait for socket input from otp_dec
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
	
	//create a new socket file desciptor to handle
	//the input if needed
    newsockfd = accept(sockfd, 
                (struct sockaddr *) &cli_addr, 
                &clilen);
	
	//check to see if new socket fd was successfully created
    if (newsockfd < 0) {
        fprintf(stderr,"otp_dec_d: ERROR on accept");
	}
	
	int input_length; //hold the length of input for multiple
					  //data parsing
	
	//FORK() HERE !!!! ****************************
	pid = fork();
	
	if (pid == 0) {//in the child's process
	
		//read in the size of input file
		n = read(newsockfd, (char*)&input_length, sizeof(input_length));
		
		if (n < 0) { //check if successfully read
			fprintf(stderr,"otp_dec_d: ERROR reading from socket");
			close(newsockfd);
			close(sockfd);
			_exit(1);
		}
		
		//if successful make next read size of the input to be read
		input_length = ntohl(input_length);
		n = read(newsockfd,input,input_length); //read plain-text + key
		
		if (n < 0) { //check if successfully read
			fprintf(stderr,"otp_dec_d: ERROR reading from socket");
			close(newsockfd);
			close(sockfd);
			_exit(1);
		}
		
		//now read in the key, size of key not needed, (last data input)
		n = read(newsockfd, key, 74999);
		
		if (n < 0) { //check if successfully read
			fprintf(stderr,"otp_dec_d: ERROR reading from socket");
			close(newsockfd);
			close(sockfd);
			_exit(1);
		}
		
		//check if input is ONLY FROM opt_enc, NOT opt_dec!!!
		const char* compare = "ABCDEFGHIJKLMNOPQRSTUVWXYZ @";
		
		//(This is set up so having '%' is from otp_enc, and '@' is from otp_dec)
		for(i = 0; i < strlen(input); i++){
			if(strchr(compare, input[i]) == NULL) {
				//input file is from otp_dec, must be rejected!
				fprintf(stderr, "Error: otp_enc cannot use otp_dec_d!\n");
				close(newsockfd);
			close(sockfd);
				_exit(1);
			}
		}
		
		//loop through message and subtract the key
		for(i = 0; i < (strlen(input)-1); i++){ //-1 to ignore the '%' character
		
			if(input[i] == ' ') { //if the character is a space
				input[i] = (char)91; //replace with character '[' in ascii table
				//this is to make the key addition easier
			}
			
			if(key[i] == ' ') { //if the character is a space
				key[i] = (char)91; //replace with character '[' in ascii table
				//this is to make the key addition easier
			}
		
			//subtract the key to cypher to decrypt it
			tmp = ((int)input[i] - (int)key[i]);
			
			if (tmp < 92 && tmp > 64) { //in ascii range
				output[i] = (char)tmp; //give output the cypher character
			}
			
			else if( tmp < 65 ) { //character is beyond accepted range
				//send to looping function
				output[i] = (char)loop(tmp);
			}
			
			if(output[i] == '[') { //if the character is '[' change to ' '
				output[i] = (char)32; //replace with character ' ' in ascii table
			}
		}
		
		//write back to otp_dec
		n = write(newsockfd, output, strlen(output));
		
		if (n < 0) { //check if successfully written
			fprintf(stderr, "opt_end_d: ERROR writing to socket");
			close(newsockfd);
			close(sockfd);
			_exit(1);
		}
		
		//close socket file descriptors
		close(newsockfd);
		close(sockfd);
		
		_exit(0); //close the child process
	}
	
	else { //in the parent process
	
		//set up signal handler to kill any zombie children
		struct sigaction sigchld_action = {
		.sa_handler = SIG_DFL,
		.sa_flags = SA_NOCLDWAIT
		};
		
		sigaction(SIGCHLD, &sigchld_action, NULL);
	}
	
} while (1); //run continuously in background until killed
	
return 0; 
}
