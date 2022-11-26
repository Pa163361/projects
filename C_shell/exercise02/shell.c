#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

void run_cd(char str[])
{
    if(chdir(str) < 0)
    {
        fprintf(stderr,"Directory not found!\n");
    }
}

void basic_commands_execution(char **tokens)
{
    if(strcmp(tokens[0],"cd") != 0)
    {
        if(execvp(tokens[0],tokens) < 0)
        {
            fprintf(stderr,"Not a valid command!\n");
        }
    }
    else
    {
        run_cd(tokens[1]);
        char str[1000];
        printf("%s\n",getcwd(str,1000));
    }
    exit(0);
}

int redirect(char **tokens_1, char **tokens_2)
{
    int fd = open(tokens_2[0], O_CREAT |  O_RDWR, S_IRWXU);
    int fd_stdout = dup(1);
    close(1);
    dup2(fd,1);
    basic_commands_execution(tokens_1);
    close(1);
    dup2(fd_stdout,1);
    close(fd);
    close(fd_stdout);
    exit(0);
    return 0;
}

int split_into_tokens(char line[], char line_1[], char line_2[], char divider)
{
    int ind_1 = 0, ind_2 = 0, flag = 0;
    for(int i=0;line[i]!='\0';i++)
    {
        if(line[i] == divider)
        {
            flag = 1;
            continue;
        }
        if(flag != 1)
        {
            line_1[ind_1] = line[i];
            ind_1++;
        }
        else
        {
            line_2[ind_2] = line[i];
            ind_2++;
        }
    }
    line_1[ind_1] = '\0';
    line_2[ind_2] = '\0';
    return 0;
}

int piping(char **tokens_1, char **tokens_2)
{
    int p[2];
    pipe(p);
    if(fork() == 0)
    {
        close(1);
        dup(p[1]);
        close(p[0]);
        close(p[1]);
        execvp(tokens_1[0],tokens_1);
    }
    if(fork() == 0)
    {
        close(0);
        dup(p[0]);
        close(p[0]);
        close(p[1]);
        execvp(tokens_2[0],tokens_2);
    }
    close(p[0]);
    close(p[1]);
    wait(NULL);
    wait(NULL);
    // exit(0);
    return 0;
}

char **split_into_commands(char line[])
{
    char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
    char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
    int i, tokenIndex = 0, tokenNo = 0;

    for(i =0; i < strlen(line) - 1; i++){

        char readChar = line[i];

        if ((line[i] == ';') && (line[i + 1] == ';')) {
            i++;
            token[tokenIndex] = '\n';
            tokenIndex++;
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

void main(void)
{
    char  line[MAX_INPUT_SIZE];            
    char  **tokens;              

    while (1) {           
       
        printf("@Pavan>");    
        bzero(line, MAX_INPUT_SIZE);
        gets(line);
        
        line[strlen(line)] = ';';
        line[strlen(line)] = ';';
        line[strlen(line)] = '\0'; //terminate with new line

        // Checking whether the given command is a redirection 

        char **aoc = split_into_commands(line);
        for(int c=0;aoc[c]!=NULL;c++)
        {
            strcpy(line,aoc[c]);
            int flag_redirection = 0, flag_piping = 0;
            for(int i=0;line[i]!='\0';i++)
            {
                if(line[i] == '>')
                {
                    flag_redirection = 1;
                    break;
                }
                else if(line[i] == '|')
                {
                    flag_piping = 1;
                    break;
                }
            }

            char **tokens_1, **tokens_2;
            char line_1[MAX_INPUT_SIZE];
            char line_2[MAX_INPUT_SIZE];
            if(flag_redirection)
                split_into_tokens(line,line_1,line_2,'>');
            if(flag_piping)
                split_into_tokens(line,line_1,line_2,'|');
        
            tokens_1 = tokenize(line_1);
            tokens_2 = tokenize(line_2);
            tokens = tokenize(line);
        
            //do whatever you want with the commands, here we just print them
            int pid_value = fork();
            if(pid_value < 0)
            {
                fprintf(stderr,"child creation failed!\n");
            }
            if(pid_value == 0)
            {
                if((flag_redirection != 1) && (flag_piping != 1))
                {
                    basic_commands_execution(tokens);
                }
                else if(flag_redirection == 1)
                {
                    redirect(tokens_1,tokens_2);
                }
                else if(flag_piping == 1)
                {
                    piping(tokens_1,tokens_2);
                }
                exit(0);
            }
            else
            {
                wait(NULL);
            }
        
        
            // Freeing the allocated memory	
            for(int i=0;tokens[i]!=NULL;i++){
                free(tokens[i]);
            }
            for(int i=0;tokens_1[i]!=NULL;i++){
                free(tokens_1[i]);
            }
            for(int i=0;tokens_2[i]!=NULL;i++){
                free(tokens_2[i]);
            }
            free(tokens);
            free(tokens_1);
            free(tokens_2);

        } 
    }
}

                
