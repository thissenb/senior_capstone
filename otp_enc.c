/******************************************
*	Bradley Thissen
*	8/5/2016
*	otp_enc.c <v1.0>
*	
*	A program that reads in text, as
*	well as a key and sends via socket
*	connection to otp_enc_d for encryption
*	once data is returned, the resulting
*	cypher is outputed to stdout.
*
*	NOTE: Portions of this code were taken
*	from client.c found in the canvas page
*	for assignment 4
*******************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

int main(int argc, char *argv[]) { //input should be opt_enc <input_file> key <port#>

    int sockfd, portno, n; 			//file descriptor for socket and number from port input
	int i = 0; 						//general purpose integer value
    struct sockaddr_in serv_addr; 	//needed for socket manipulation
    struct hostent *server; 		//needed for socket manipulation

    char buffer[75000]; 			//stores temp string for I/O
	char masterkey[75000];			//holds the input key for encryption
	char input[150000];				//holds the plain-text file input
	
    if (argc < 4 || argc > 4) { //check if correct number of inputs passed in
       fprintf(stderr,"otp_enc: usage %s hostname port\n", argv[0]);
       exit(0);
    }
	
	//check and see if key passed in is at least equal size to plain-text file
	FILE * text;
	FILE * key;
	
	text = fopen(argv[1], "r"); //open plain-text file for reading
	key  = fopen(argv[2], "r"); //open key file for reading
	
	//should only need the first line that holds the needed data for both!
	fgets(input, sizeof input, text);
	fgets(masterkey, sizeof masterkey, key);
	
	//close file descriptors from passed in arguments
	fclose(text);
	fclose(key);
	
	//check if key is compatible with message
	if (strlen(masterkey) < strlen(input)) { //the key is too small
		fprintf(stderr, "otp_enc: key %s is too short\n", argv[2]);
		exit(1);
	}
	
	//take off newline character before error checking an passing into otp_enc_d
	input[strlen(input)+1] = '\0';
	masterkey[strlen(masterkey)-1] = '\0';
	
	//need to add unusual character to input to parse input/key over in otp_enc_d
	input[strlen(input)-1] = '%';
	//this should move NULL one more space to allow for the '%' symbol
	
	//Create a copmarison string to check the input
	const char* compare = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
	
	//check if passed in message doesn't contain invalid characters
	for(i = 0; i < (strlen(input)-1); i++){
	
		if(strchr(compare, input[i]) == NULL){
			fprintf(stderr, "otp_enc: Input contains bad characters\n");
			exit(1);
		}
	}
	
    portno = atoi(argv[3]); //grab the portnumber from argument 3
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //initialize socket file descriptor
	
    if (sockfd < 0) { //if port input is invalid
        fprintf(stderr, "otp_enc: ERROR opening socket\n");
		exit(1);
	}
		
    server = gethostbyname("localhost"); //set to always connect to same computer
	
    if (server == NULL) { //if localhost can't connect
        fprintf(stderr,"otp_enc: ERROR, no such host\n");
        exit(1);
    }
	
	//set up socket connection
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);	 
    serv_addr.sin_port = htons(portno);
	
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) { 
        fprintf(stderr, "otp_enc: could not contact otp_enc_d on port %d\n", portno); 
		//failed to connect to socket
		exit(2);
	}
	
	//current method of concatenating won't work for plaintext4, need to send data
	//separately rather than all at once!!!
	
	//socket is now connecting and waiting for input
	
	int input_length = strlen(input);
	int tmp = htonl(input_length);
	
	//write twice to the socket first the size of input
	n = write(sockfd, (char*)&tmp, sizeof(tmp));
	
	if (n < 0) {
        fprintf(stderr, "otp_enc: ERROR writing to socket\n");
		close(sockfd);
		exit(1);
	}
	
	//then actuall input, so that the key can input next
	n = write(sockfd, input, strlen(input));
	
	if (n < 0) {
        fprintf(stderr, "otp_enc: ERROR writing to socket\n");
		close(sockfd);
		exit(1);
	}
	
	//now send the key, don't need to worry about length, as don't need to send anything else
	n = write(sockfd,masterkey, strlen(masterkey)); //send the key to encrypt with
	
    if (n < 0) {
        fprintf(stderr, "otp_enc: ERROR writing to socket\n");
		close(sockfd);
		exit(1);
	}
    bzero(buffer,75000); //clear the strings
	
    n = read(sockfd,buffer,74999); //read in the data sent from otp_enc_d
								 //should only be the returned cypher-text
	
    if (n < 0) {
        fprintf(stderr, "otp_enc: ERROR reading from socket\n");
		close(sockfd);
		exit(1);
	}
	
	//IF WIERD SYMBOLS APPEAR HERE BUT NOT ON DAEMON, TRY PRINTING INDIVIDUALLY LIKE IN KEYGEN
    printf("%s\n", buffer); //print out the text sent from otp_enc_d
	
    close(sockfd);
	
    return 0;
}
