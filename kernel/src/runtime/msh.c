#include "minemu/uart.h"
#define LINE_BUFFER_CAPACITY 20

char* space = " ";
char* new_line = "\n";
char cmd_not_found[] = "command not found: ";
char echo_error[] = "echo cannot have 0 args\n";

struct line_buffer{
        char buffer[LINE_BUFFER_CAPACITY + 1]; //is the buffer capacity +1 so that you can add a \0 to signify the end, makes processing easier 
        uint32_t next_write; //tracks the next byte (index) to be written in
        uint32_t length; 
    };

void clear_line_buffer(struct line_buffer *lb){
    lb->length = 0;
    lb->next_write = 0;
}

 /**Writes to an array of strings, output, with each "token" in it's own null terminated string. Delim is the character that separates sections.
    Ex: cd f hi -> [[cd\0],[f\0],[hi\0]]
    Doesn't yet implement checking for literalls, like cd "f hi". That just gives [[cd\0],["f\0],[hi"\0]]
 **/
void tokenize_string(char* string, int str_len, const char* delim, char* output[LINE_BUFFER_CAPACITY +1], char tokens[LINE_BUFFER_CAPACITY + 1][LINE_BUFFER_CAPACITY + 1]){
    //check: are the pointers going to disappear when the function ends since they're local? May need to allocate space for them... agdkajhdj why is C like this
    //I want to go back home to Java
    //I COULD also make it ignore extra delims between tokens, but not sure. I should do something about the case where someone accidentally enters space 2x though
    char character;
    int tokenIndex = 0; 
    int outputIndex = 0;
    for (int i = 0; i < str_len; i++){
        character = string[i];
        if(character == *delim && tokenIndex == 0){//ignore leading whitespace
            continue; 
        } else if (character == *delim && tokenIndex > 0){
            tokens[outputIndex][tokenIndex] = '\0';
            tokenIndex = 0;
            output[outputIndex] = tokens[outputIndex];
            outputIndex++;
        } else {
            tokens[outputIndex][tokenIndex] = character;
            tokenIndex++;
        } if (i + 1 == str_len && tokenIndex > 0){ //still a token waiting to be saved and this is the last character
            tokens[outputIndex][tokenIndex] = '\0';
            tokenIndex = 0;
            output[outputIndex] = tokens[outputIndex];
            outputIndex++;
        }
    }
        output[outputIndex] = NULL; //mark the end of teh array for other functions 
}

/** move to str library later
 *  accepts 2 null terminated strings, returns 1 if equal, 0 if not
 * **/
int string_equal(char* str1, char* str2){
    int i = 0;
    while(1){
        if (str1[i] == str2[i]){ //regular character equals the other
            i++;
            continue;
        } else if ((str1[i] == '\0' && str2[i] != '\0') || (str2[i] == '\0' && str1[i] != '\0')){ //either end prematurely
            return 0;
        } else if (str1[i] == '\0' && str2[i] == '\0'){ //both end at the same time
            return 1;
        } else { //regular character does not equal the other
            return 0;
        }
    }
}

//checks the length (# args) of a command
int len_command(char* command[LINE_BUFFER_CAPACITY + 1]){
    int i = 0;
    while (command[i] != NULL){
        i++;
    }
    return i;
}

//returns the length of null terminated string str
int len_str(char* str){
    int i = 0;
    while (str[i] != '\0'){
        i++;
    }
    return i;
}


//---------------commands----------------


void run_command(char* command[LINE_BUFFER_CAPACITY + 1]){
    if(command[0] == NULL){
        //do nothing
    } else if (string_equal(command[0], "echo") == 1){
        echo(command);
    } else {
        uart_print(cmd_not_found, 19);
        uart_print(command[0], len_str(command[0]));
        uart_print(new_line, 1);
    }
}

void echo(char* command[LINE_BUFFER_CAPACITY]){
    int len = len_command(command);
    if (len == 1){ //user entered only "echo"
        uart_print(new_line, 1);
    } else {
        for (int i = 1; i < len; i++){
            uart_print(command[i], len_str(command[i]));
            if(i+1 < len){
                uart_print(space, 1);
            }
        }
        uart_print(new_line, 1);
    }
}

//-------------end commands---------------

void start_msh(){
    char ready[] = "msh> ";
    int lenReady = 5;
    int running = 1;
    int done = 0;
    uint32_t byte; 
    char* command[LINE_BUFFER_CAPACITY + 1];
    char tokens[LINE_BUFFER_CAPACITY + 1][LINE_BUFFER_CAPACITY + 1];


    //init line buffer
    struct line_buffer lb;
    lb.next_write = 0;
    lb.length = 0;

    while(running == 1){
        uart_print(ready, lenReady);
        done = 0;

        while (done == 0){
            int fail = readb_uart_rx_buffer(&byte);
            if(fail == 1){ //case where there is nothing to read in the buffer
                continue;
            }
                switch (byte){
                    case '\n':
                    //line entered, move to logic for handling it. 
                    tokenize_string(lb.buffer, lb.length, space, command, tokens);
                    run_command(command);
                    clear_line_buffer(&lb);
                    done = 1;
                    break;
                    case 0x08: 
                    //edit buffer, recieved backspace.
                    if(lb.next_write != 0){ //check that we aren't about to go into negative indexes
                        lb.next_write--;
                        lb.length--;
                    }
                    break;
                    case 0x7f: 
                    //edit buffer, recieved backspace.
                    if(lb.next_write != 0){ //check that we aren't about to go into negative indexes
                        lb.next_write--;
                        lb.length--;
                    }
                    break;
                    default:
                    //regular character, write it into the buffer
                    if(lb.next_write < 20){
                        lb.buffer[lb.next_write] = (char)byte;
                        lb.next_write++;
                        lb.length++;
                    } 
                    //else, the byte is discarded. Sorry :( sucks to suck
                    break;
                }
            }
        }
    }