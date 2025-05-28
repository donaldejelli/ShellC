#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>

void lsh_loop(void)
{
    char* line;
    char** args;
    int status;

    do {
        printf("> ");
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);
        free(line);
        free(args);
    } while (status);
}
#define LSH_RL_BUFSIZE 1024 //every occurence of LSH_RL_BUFSIZE will be 1024
char *lsh_read_line(void)
{
    int bufsize = LSH_RL_BUFSIZE; //bufsize=1024
    int position = 0;
    char* buffer = (char*)malloc(sizeof(char) * bufsize);  //the malloc will always be 1024 (casting needed because c++ is dogshit)
    int c;

    if (!buffer)  //checks if malloc had an error and exits
    {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);

    }
    while (1) {
        //read a character
        c = getchar();
        if (c == EOF || c == '\n') //EOF is ctrl+d, the other is ENTER
        {
            buffer[position] = '\0'; //marks end of the string
            return buffer;

        }
        else
        {
            buffer[position] = c; //store character
        }
        position++; //go ahead by one position

        //if buffer is exceeded we reallocate
        if (position >= bufsize) {
            bufsize += LSH_RL_BUFSIZE;
            buffer = (char*)realloc(buffer, bufsize); //casting is needed because c++ is dogshit
            if (!buffer)
            {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
}

}

//we take a string input and split it into tokens based on delimiters like spaces
#define LSH_TOK_BUFSIZE 64 //64 tokens
#define LSH_TOK_DELIM " \t\r\n\a" 
char** lsh_split_line(char* line)
{
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char** tokens = (char**)malloc(bufsize * sizeof(char*));
    char* token;

    if (!token)
    {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;
        if (position >= bufsize)
        {
            bufsize += LSH_TOK_BUFSIZE;
            tokens = (char**)realloc(tokens, bufsize * sizeof(char*));
            if (!tokens)
            {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, LSH_TOK_DELIM);

    }
    tokens[position] = NULL;
    return tokens;
}
int lsh_launch(char** args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) {
        // Error forking
        perror("lsh");
    }
    else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}
/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char** args);
int lsh_help(char** args);
int lsh_exit(char** args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
const char* builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char**) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit
};

int lsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char*);
}

/*
  Builtin function implementations.
*/
int lsh_cd(char** args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    }
    else {
        if (chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}

int lsh_help(char** args)
{
    int i;
    printf("Stephen Brennan's LSH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < lsh_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int lsh_exit(char** args)
{
    return 0;
}
int lsh_execute(char** args)
{
    int i;

    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    for (i = 0; i < lsh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return lsh_launch(args);
}
int main(int argc, char **argv)
{
    //load config files if any

    //loop
    lsh_loop();

    //shutdown
    std::cout << "Hello World!\n";
}
