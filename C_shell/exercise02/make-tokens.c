#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

char **tokenize(char *line)
{
    char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
    char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
    int i, tokenIndex = 0, tokenNo = 0;

    for(i =0; i < strlen(line); i++){

        char readChar = line[i];

        if (readChar == ' ' || readChar == '\n' || readChar == '\t') {
            token[tokenIndex] = '\0';
            if (tokenIndex != 0){
                tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
                strcpy(tokens[tokenNo++], token);
                tokenIndex = 0; 
            }
        } 
        else {
            token[tokenIndex++] = readChar;
        }
    }
 
    free(token);
    tokens[tokenNo] = NULL ;
    return tokens;
}

int run_cd(char str[])
{
    if(chdir(str) < 0)
    {
        fprintf(stderr,"Directory not found!\n");
    }
}

void main(void)
{
    char  line[MAX_INPUT_SIZE];            
    char  **tokens;              
    int i;

    while (1) {           
       
        printf("Hello>");     
        bzero(line, MAX_INPUT_SIZE);
        gets(line);           
        line[strlen(line)] = '\n'; //terminate with new line
        tokens = tokenize(line);
   
        //do whatever you want with the commands, here we just print them
        int pid_value = fork();
        if(pid_value < 0)
        {
            fprintf(stderr,"child creation failed!\n");
        }
        if(pid_value == 0)
        {
            if(strcmp(tokens[0],"cd") != 0)
            {
                if(execvp(tokens[0],tokens) < 0)
                {
                    fprintf(stderr,"Not a valid command!\n");
                }
                exit(0);
            }
            else
            {
                run_cd(tokens[1]);
            }
        }
        else
        {
            wait(NULL);
        }
    
       
        // Freeing the allocated memory	
        for(i=0;tokens[i]!=NULL;i++){
            free(tokens[i]);
        }
        free(tokens);
    }
     

}

                
