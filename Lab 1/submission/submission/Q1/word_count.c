#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>

// Use Library function such as fopen, geline, fread etc
// use manpages as mentioned in the writeup. 
// -----------------------
//
//
// Your solution begins here





int main(int argc, char *argv[]) {

if(argc != 3) return 0;


char* fileptr = argv[2];
char* type = argv[1];
size_t size = 1000;
size_t charecters;
FILE* fptr = fopen(fileptr, "r");


if(!fptr) return 0;
size_t lines = 0, words = 0, chars = 0;

int c; int in_word = 0;
while((c = fgetc(fptr)) != EOF) {

	chars++;
	if(c == '\n') lines++;
	if(isspace((unsigned char)c)) {
		in_word = 0;
          
 

	} else  {

if(!in_word) {
            words++; in_word = 1;
          }


	}


}


fclose(fptr);
if(strcmp(type, "-l") == 0) printf("%ld", lines);
else if(strcmp(type, "-w") == 0) printf("%ld", words);
else if(strcmp(type, "-c") == 0) printf("%ld", chars);
else return 1;

return 0;


/*
char str[100];
scanf("%s", str);

int num_chars = cnt_c(str);
int num_words = cnt_w(str);
int num_lines = cnt_l(str);

printf("chars: %d, words: %d, lines: %d", num_chars, num_words, num_lines);
*/


}




