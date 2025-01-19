#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
int  reverse_string(char *, int, int);
int  word_print(char *, int, int);
int  replace_word(char *, int, int, char *, char *);
//add additional prototypes here


int setup_buff(char *buff, char *user_str, int len){
    //the following implementation fills the buffer with the user string, removing leading, trailing and 
    //duplicated whitespace characters and replacing them with a single space character. If the user string 
    //is too long, the function returns -1. If the buffer is successfully filled, any extra space is filled 
    //with dots and the function returns the length of the buffer.

    int b = 0;
    bool whitespace = false;

    int i = 0;
    while (user_str[i] == ' ' || user_str[i] == '\t') {
        i++;
    }

    for (; user_str[i] !='\0' && b < len; i++){
        if (user_str[i] == ' ' || user_str[i] == '\t'){
            if (!whitespace) {
                buff[b++] = ' ';
                whitespace = true;
            }  
        } else{
            buff[b++] = user_str[i];
            whitespace = false;
        }
    }

    if (b > 0 && buff[b-1] == ' '){
        b--;
    }

    if (user_str[i] != '\0') {
        return -1;
    }

    memset(buff+b, '.', BUFFER_SZ-b);
    return b;
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
    //the following implementation counts the number of words in the buffer by checking 
    //for spaces to identify word endings. At the end, the function returns the number of words in the buffer.

    (void)len; //mark len as unused

    int wrd_count = 0;

    for (int i = 0; i < str_len; i++) {
        if (buff[i] != ' ' && (i == 0 || buff[i - 1] == ' ')) {
            wrd_count++;
        }
    }

    return wrd_count;
}

int reverse_string(char *buff, int len, int str_len) {
    //the following implementation reverses the string in the buffer by swapping the first and last characters. Successful 
    //completion of the function returns 0.

    (void)len; //mark len as unused

    int start = 0;
    int end = str_len - 1;
    char temp;

    while (end > start){
        temp = buff[start];
        buff[start] = buff[end];
        buff[end] = temp;
        start++;
        end--;
    }
    return 0;
}

int word_print(char *buff, int len, int str_len) {
    //the following implementation prints the words in the buffer by checking for spaces to identify word endings and
    //printing each word followed by its length. The function returns the number of words in the buffer.

    int start = 0;
    int idx = 1;
    
    printf("Word Print\n");
    printf("----------\n");
    printf("%d. ", idx++);

    for (int i=0; i<str_len; i++){
        if (buff[i] == ' '){
            printf("(%d)\n", i-start);
            printf("%d. ", idx++);
            start = i+1;
        } else {
            if (i == str_len-1) {
                putchar(buff[i]);
                printf("(%d)\n\n", i-start+1);
            }
            else {
                putchar(buff[i]);
            }
        }
    }

    int wrd_count = count_words(buff, len, str_len);
    return wrd_count;
}

int replace_word(char *buff, int len, int str_len, char *old_word, char *new_word) {
    //the following implementation replaces a word in the buffer with another word by searching for a word match in the 
    //buffer. if a match is found, a check is made to ensure that replacement does not cause a buffer overflow. If 
    //that check is passed, the function moves the words to the left or right as needed to accomodate the replacement
    //and then switches out the old word for the new word before printing out the modified string.

    int old_word_len = 0;
    int new_word_len = 0;

    while (old_word[old_word_len] != '\0') {
        old_word_len++;
    }

    while (new_word[new_word_len] != '\0') {
        new_word_len++;
    }

    if (new_word_len > old_word_len && (str_len - old_word_len + new_word_len) >= len) {
        return -1; 
    }

    for (int i = 0; i <= str_len-old_word_len; i++) {
        bool match_found = true;

        for (int j = 0; j < old_word_len; j++) {
            if (buff[i+j] != old_word[j]) {
                match_found = false;
                break;
            }
        }

        if (match_found && (i == 0 || buff[i-1] == ' ') && 
            (buff[i+old_word_len] == ' ' || buff[i+old_word_len] == '.')) {
            
            if (new_word_len > old_word_len) {
                int move = new_word_len - old_word_len;
                for (int k = str_len; k >= i + old_word_len; k--) {
                    buff[k + move] = buff[k];
                }
            } else if (new_word_len < old_word_len) {
                int move = old_word_len - new_word_len;
                for (int k = i + new_word_len; k < str_len; k++) {
                    buff[k] = buff[k + move];
                }
            }

            for (int j = 0; j < new_word_len; j++) {
                buff[i+j] = new_word[j];
            }

            str_len += (new_word_len - old_word_len);
            i += new_word_len-1;
            
            } else {
                i++;
            }

        }
    printf("Modified String: %.*s\n", str_len, buff);
    return 0;
}

int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    //      The if stament checks the number of arguments passed to the program, in a ssituation where argc<2, which is
    //when it would lead to argv[1] not existing, the program will exit with a return code of 1 before checking the value of argv[1]. 
    //Therefore, this is safe because checking the value of argc prevents the program from accessing an argument that does not exist.
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

    //TODO:  #2 Document the purpose of the if statement below
    //      The if statement ensures the user provides a string to operate on, if the user does not provide a string, the program will
    //      exit with a return code of 1.
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    buff = (char *)malloc(BUFFER_SZ);
    if (buff == NULL){
        exit(99);
    }
    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options

        case 'r':
            rc = reverse_string(buff, BUFFER_SZ, user_str_len);
            if (rc < 0){
                printf("Error reversing words, rc = %d", rc);
                exit(2);
            }
            //printf("Reversed String: %.*s\n", user_str_len, buff);
            break;

        case 'w':
            rc = word_print(buff, BUFFER_SZ, user_str_len);
            if (rc < 0){
                printf("Error printing words, rc = %d", rc);
                exit(2);
            }

            printf("Number of words returned: %d\n", rc);
            break;

        case 'x':
            /* if (argc == 5){
                printf("Not Implemented!");
                exit(3);
            }
            */ //commented out to allow for extra credit implementation

            rc = replace_word(buff, BUFFER_SZ, user_str_len, argv[3], argv[4]);
            if (rc < 0){
                printf("Error printing words, rc = %d", rc);
                exit(2);
            }
            break;
        
        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    free(buff);
    return 0;
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
//          Because the setup_buff function handles restrictions on the length of the user
// string to ensure it fits in the buffer, passing the length of the buffer allows the 
//function to be used in contexts where the buffer size may be different from what is 
//declared in main by passing the needed buffer size as a parameter.