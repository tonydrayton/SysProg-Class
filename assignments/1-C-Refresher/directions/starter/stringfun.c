#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here
void word_print(char *, int);


int setup_buff(char *buff, char *user_str, int len) {
	if (buff == NULL || user_str == NULL || len <= 0) {
		return -2;
	}

	if (len > BUFFER_SZ) {
		return -2;
	}

	int i = 0;
	int j = 0;
	int last_space = 0;

	// skip leading whitespace
	while(user_str[i] == ' ' || user_str[i] == '\t') {
		i++;
	}

	// find the end of the string (disregard trailing whitespace)
	int end = i;
	for(int k = i; user_str[k] != '\0'; k++) {
		if(user_str[k] != ' ' && user_str[k] != '\t') {
			end = k; // only change the end if the character is not whitespace
		}
	}

	// loop through the string and copy characters to the buffer
	while(i <= end) {
		if(j >= BUFFER_SZ) {
			return -1;
		}

		char c = user_str[i];
		if(c == ' ' || c == '\t') {
			if(!last_space && j < len) {
				buff[j++] = ' ';
				last_space = 1; // set to true
			}
		} else {
			if(j < len) {
				buff[j++] = c;
			}
			last_space = 0; // set to false
		}
		i++;
	}

	// fill with .
	while (j < len) {
		buff[j++] = '.';
	}

	return i;
}

void print_buff(char *buff, int len){
    printf("Buffer:  [");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    printf("]\n");
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}


int count_words(char *buff, int len, int str_len) {
	if (buff == NULL || str_len == 0 || len == 0) {
		return 0;
	}

	int wordCount = 0;
	int inWord = 0;

	for (int i = 0; i < str_len && i < len; i++) {
		if (isalnum(buff[i])) {
			if (!inWord) {
				inWord = 1; // inside a word
				wordCount++;
			}
		} else {
			inWord = 0; // outside a word
		}
	}

	return wordCount;
}

void word_print(char *buff, int str_len) {
	if (str_len == 0) {
		return;
	}

	int wordCount = 0;
	int state = 0; // 0 = not word, 1 = in word
	int wordStart = 0;
	int wordEnd = 0;

	for (int i = 0; i < str_len; i++) {
		if (buff[i] == '\\') {
			i++;
			continue;
		}

		if (isalnum(buff[i])) {
			if (state == 0) {
				wordStart = i;
				state = 1;
			}

			if(buff[i + 1] == NULL || !isalnum(buff[i + 1])) {
				wordEnd = i + 1;
				state = 0;
				printf("%d. ", wordCount + 1);
				for (int j = wordStart; j < wordEnd; j++) {
					putchar(buff[j]);
				}
				printf("(%d)\n", wordEnd - wordStart);
				wordCount++;
			}
		} else {
			state = 0;
		}
	}

	printf("\nNumber of words returned: %d\n", wordCount);

	return;
}

void reverse_buff(char *buff, int len) {
	if (buff == NULL || len == 0) {
		return;
	}

	for (int i = 0; i < len / 2; i++) {
		char temp = buff[i];
		buff[i] = buff[len - i - 1];
		buff[len - i - 1] = temp;
	}
}

int replace_word(char *buff, int len, int str_len, char *old_word, char *new_word) {
	if (!buff || !old_word || !new_word || str_len == 0 || len == 0) {
		return -1;
	}

	int oldLength = strlen(old_word);
	int newLength = strlen(new_word);

	if (oldLength == 0 || oldLength > str_len) {
		return -1;
	}

	int max_new_size = len;

	char *new_buff = (char *)malloc(max_new_size);
	if (!new_buff) {
		return -1;
	}

	int temp_index = 0;
	int i = 0;
	int replaced = 0;

	while (i < str_len && temp_index < len) {
		if (strncmp(&buff[i], old_word, oldLength) == 0) {
			if (temp_index + newLength < len - 1) {
				// copy the new_word to the new buffer at the index of the old_word
				memcpy(&new_buff[temp_index], new_word, newLength);
				temp_index += newLength;
				replaced = 1;
			}
			i += oldLength; // skip past the old_word
		} else {
			// just copy the character from the old buffer to the new buffer (ignore it)
			new_buff[temp_index++] = buff[i++];
		}
	}

	while (temp_index < len) {
		new_buff[temp_index++] = '.';
	}

	new_buff[len] = '\0';
	memcpy(buff, new_buff, len);
	free(new_buff);
	return replaced;
}
//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS

int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:✅  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
	/*
		This is safe because argc is checked to be at least 2 arguments before argv is checked.
		If argc is 0 or 1, the body of the if statement will be evaluated and the check for argv will be skipped.
		If argc is 2 or more, argv will have at least an element at indexes 1 and 2 guaranteed.
	*/
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:✅  #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
	/*
		If there are less than 3 arguments, run on the first argument
		This is a check for if there are no options supplied, because ideally there should be at least 2 arguments, the	command and the string.
		If there is no other arguments (options), the program will just output the usage.
	*/
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

	// TODO:✅  #3 Allocate space for the buffer using malloc and
	//          handle error if malloc fails by exiting with a
	//          return code of 99
	// CODE GOES HERE FOR #3
	char *str_buff = malloc(BUFFER_SZ);
	if(str_buff == NULL) {
		printf("Failed to allocate storage.");
		exit(99);
	}

    user_str_len = setup_buff(str_buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
		if(user_str_len == -1) {
			printf("Error: Provided string is too long.\n");
			exit(3);
		} else {
			printf("Error setting up buffer, error = %d", user_str_len);
     		exit(2);
		}
    }

    switch (opt){
        case 'c':
            rc = count_words(str_buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:✅  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
		case 'r':
			reverse_buff(str_buff, user_str_len);
			break;
		case 'w':
			printf("Word Print\n----------\n");
			word_print(str_buff, user_str_len);
			break;
		case 'x':
			int replaced = replace_word(str_buff, BUFFER_SZ, user_str_len, argv[3], argv[4]);
			if(replaced == 0) {
				printf("Word not found.\n");
				exit(1);
			}
			break;
        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:✅  #6 Dont forget to free your buffer before exiting
    print_buff(str_buff, BUFFER_SZ);
	free(str_buff);
    exit(0);
}

//TODO:✅ #7  Notice all of the helper functions provided in the
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that
//          the buff variable will have exactly 50 bytes?
//
//          PLACE YOUR ANSWER HERE
/**
 * Providing both the pointer and the length is a good practice because it allows for the function to be more flexible.
 * Since the length is passed as an argument, the function can be used with any buffer of the same length.
 * And the function does not have to be modified if the buffer size changes.
 */
