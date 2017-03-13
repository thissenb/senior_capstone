/*************************
*	Bradley Thissen
*	8/4/2016
*	CS344 E-Campus
*	keygen.c <v1.0>
*	
*	a simple program that generates a random
*	string of characters (1-27)<A-Z + " "> as
*	the cypher key to OTP assignment
**************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main( int argc, char *argv[]) {

	srand(time(NULL)); //generate the random seed for the key
	int r = 0;

	if (argc == 2) { //the correct number of arguments passed in

		char *ptr;
		long x = 0;
		long i = 0;
		
		x = strtol(argv[1], &ptr, 10); //grab the decimal value passed in
		
		char key[x+1]; //create a key string with size input + '\n' + NULL
		
		for(i = 0; i < x; i++) { //loop through key and populate with random char.
			
			r = (rand() % (27)) + 65; //number is bounded by ascii table A->Z + [
			key[i] = (char)r;
			
			//check if '[', change to ' '
			if (key[i] == '[') {
				key[i] = (char)32; //decimal value of space character
			}
			
			printf("%c", key[i]); //print out the key string to stdout
			//NOTE: this will print out string + '\n' but not NULL!!!
			
		}
		
		key[x+1] = '\0'; //assert that last character in key is NULL.
		
		printf("\n"); //add a newline character to end of string
		
	}

	else if (argc > 2 ) { //more than key length was passed in	
		perror("Too many arguments");
		exit(1);
	}
	
	else { //not enough arguments passed in
		perror("one argument expected");
		exit(1);
	}
	
return 0;
}
