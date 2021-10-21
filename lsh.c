/***************************************************************************//**

  @file         main.c

  @author       Stephen Brennan + yarvin

  @date         Thursday,  8 January 2015

  @brief        LSH (Libstephen SHell)

*******************************************************************************/

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int lsh_mv(char **args);
int lsh_ls(char **ls);
void sighandler(int sig);
/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "mv",
  "ls",
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit,
  &lsh_mv,
  &lsh_ls
};

int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

void sighandler(int sig) {
    printf("\nYou press ctrl + c!!\n");
    printf("If you want to exit, use exit command.\n");
    printf(">");
    return 1;
}

/**
   @brief Bultin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */
int lsh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int lsh_help(char **args)
{
  int i;
  printf("yarv.in's LSH\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < lsh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int lsh_exit(char **args)
{
  return 0;
}

/**
   @brief Builtin command: move
   @param args List of args. args[0] is "mv". args[1] is original directory or file. args[2] is target directory or file.
   @return Always returns 1, to continue executing.
 */
int lsh_mv(char **args)
{
    DIR* directory = NULL;
    struct dirent* dir_info = NULL;
    struct stat buf;
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: missing file operand\n");
        return 1;
    } else if (args[2] == NULL) {
        fprintf(stderr, "mv: missing destination file operand after '%s'\n", args[1]);
        return 1;
    } else if (args[3] == NULL) {\
        directory = opendir(".");
        while(dir_info = readdir(directory)) {
            lstat(dir_info->d_name, &buf);
            if (args[2][strlen(args[2])-1] == 0x2F) {
                args[2][strlen(args[2])-1] = NULL;
            }
            if(!strcmp(dir_info->d_name, args[2])) {
                if(S_ISDIR(buf.st_mode)) {
                    strcat(args[2], "/");
                    strcat(args[2], args[1]);
                    rename(args[1], args[2]);
                    closedir(directory);
                    return 1;
                } 
            }
        }
        rename(args[1], args[2]);
        closedir(directory);
        return 1;
    } /*else {
        printf("before opendir");
        directory = opendir(".");
        while(dir_info = readdir(directory)) {
            printf("before strcmp");
            if(!strcmp(dir_info->d_name, args[strlen(args)])) {
                lstat(dir_info->d_name, &buf); 
                printf("before ISDIR");
                if(S_ISDIR(buf.st_mode)) {
                    char * target = args[strlen(args)];
                    if (args[strlen(args)][strlen(args[strlen(args)])-1] == 0x2F) {
                        args[strlen(args)][strlen(args[strlen(args)])-1] = NULL;
                    }
                    printf("%s", target);
                    printf("%s", args[strlen(args)][strlen(args[strlen(args)])-1]);
                    return 1;
                } else {
                    fprintf(stderr, "mv: target '%s' is not a directory\n", args[strlen(args)]);
                    return 1;
                }
            }
        }
    }*/
}   

/**
   @brief Builtin command: list directory contents
   @param args List of args. args[0] is "ls". args[1] is option, directory or file. if args[1] is option, args[2] is directory or file
   @return Always returns 1, to continue executing.
 */
int lsh_ls(char **args)
{
    DIR* directory = NULL;
    struct dirent* dir_info = NULL;
    const char * help = "--help";
    struct stat buf;
    if (args[1] == NULL) {
        directory = opendir(".");
        while (dir_info = readdir(directory)) {
            if (dir_info->d_name[0] == 0x2E) { // 숨겨진 파일 없애기
                continue;
            }
            else {
                printf("%s ", dir_info->d_name);
            }
        }
        printf("\n");
        closedir(directory);
        return 1;
    } else if(!strncmp(args[1], help, 6)) { // -- help
        printf("Usage: ls [OPTION]... [FILE]...\n");
        printf("List information about the FILEs (the current directory by default).\n");
        printf("Sort entries alphabetically if none of -cftuvSUX nor --sort is specified.\n");
        printf("-a, %-10s do not ignore entries starting with .\n", "--all");
        printf("-l, %-10s use a long listing format\n", "");
        return 1;
    }
    else if (!strncmp(args[1], help, 1)) {
        if(strchr(args[1], 'a')){ // -a
            if(strchr(args[1], 'l')) { // -al
                if(args[2]) {
                    directory = opendir(".");
                    while(dir_info = readdir(directory)) {
                        lstat(dir_info->d_name, &buf);
                        if(!strcmp(dir_info->d_name, args[2])) {
                            if(S_ISDIR(buf.st_mode)) {
                                directory = opendir(args[2]);
                                while (dir_info = readdir(directory)) {
                                    lstat(dir_info->d_name, &buf);
                                    print_detail(dir_info, buf);
                                }
                                printf("\n");
                                closedir(directory);
                                return 1;
                            } else {
                                print_detail(dir_info, buf);
                            }
                        }   
                    }
                    printf("No such file or directory\n");
                    closedir(directory);
                    return 1; 
                }
                directory = opendir(".");
                while(dir_info = readdir(directory)) {
                    lstat(dir_info->d_name, &buf);
                    print_detail(dir_info, buf);
                }
                closedir(directory);
                return 1;
            }
            if(args[2]) {
                directory = opendir(".");
                while(dir_info = readdir(directory)) {
                    lstat(dir_info->d_name, &buf);
                    if(!strcmp(dir_info->d_name, args[2])) {
                        if(S_ISDIR(buf.st_mode)) {
                            directory = opendir(args[2]);
                            while (dir_info = readdir(directory)) {
                                printf("%s ", dir_info->d_name);
                            }
                            printf("\n");
                            closedir(directory);
                            return 1;
                        } else {
                            printf("%s\n", args[2]);
                            closedir(directory);
                            return 1;
                        }
                    }   
                }
                printf("No such file or directory\n");
                closedir(directory);
                return 1; 
            }
            directory = opendir(".");
            while (dir_info = readdir(directory)) {
                printf("%s ", dir_info->d_name);
            }
            printf("\n");
            closedir(directory);
            return 1;
        } 
        else if(strchr(args[1], 'l')) { // -l
            if (args[2]) {
                directory = opendir(".");
                while(dir_info = readdir(directory)) {
                    lstat(dir_info->d_name, &buf);
                    if(!strcmp(dir_info->d_name, args[2])) {
                        if(S_ISDIR(buf.st_mode)) {
                            directory = opendir(args[2]);
                            while (dir_info = readdir(directory)) {
                                if (dir_info->d_name[0] == 0x2E) { // 숨겨진 파일 없애기
                                    continue;
                                } else {
                                    lstat(dir_info->d_name, &buf);
                                    print_detail(dir_info, buf);
                                }
                            }
                            printf("\n");
                            closedir(directory);
                            return 1;
                        } else {
                            lstat(dir_info->d_name, &buf);
                            print_detail(dir_info, buf);
                            closedir(directory);
                            return 1;
                        }
                    }
                }
                printf("No such file or directory\n");
                closedir(directory);
                return 1;
            }
            directory = opendir(".");
            while (dir_info = readdir(directory)) {
                if (dir_info->d_name[0] == 0x2E) { // 숨겨진 파일 없애기
                    continue;
                }
                else {
                    lstat(dir_info->d_name, &buf);
                    print_detail(dir_info, buf);
                }
            }
            return 1;
        }
    } else { // ls 뒤에 file or directory
        directory = opendir(".");
        while(dir_info = readdir(directory)) {
            lstat(dir_info->d_name, &buf);
            if(!strcmp(dir_info->d_name, args[1])) {
                if(S_ISDIR(buf.st_mode)) {
                    directory = opendir(args[1]);
                    while (dir_info = readdir(directory)) {
                        if (dir_info->d_name[0] == 0x2E) { // 숨겨진 파일 없애기
                            continue;
                        } else {
                            printf("%s ", dir_info->d_name);
                        }
                    }
                    printf("\n");
                    closedir(directory);
                    return 1;
                } else {
                    printf("%s\n", args[1]);
                    closedir(directory);
                    return 1;
                }
            }
        }
        printf("No such file or directory\n");
        closedir(directory);
        return 1; 
    }
}

int print_detail(struct dirent* dir_info, struct stat buf) {
    char* stream;
    struct passwd * uid = getpwuid(getuid());
    struct group * gid = getgrgid(getgid());
    stream = ctime(&buf.st_ctime);
    stream[strlen(stream)-1] = 0; // 줄바꿈 제거
    check_permition(buf);
    printf(" %d %s %s %5d %s %s\n", buf.st_nlink, uid->pw_name, gid->gr_name, buf.st_size, stream, dir_info->d_name); // 시간까지 형식을 맞추고 싶은데 힘들어서 포기,,
}

int check_permition(struct stat buf) {
    if(S_ISDIR(buf.st_mode)) {
        printf("d");
    } else if (S_ISFIFO(buf.st_mode)) {
        printf("p");
    } else if (S_ISLNK(buf.st_mode)) {
        printf("l");
    } else {
        printf("-");
    }
    if((buf.st_mode & S_IRUSR)) printf("r");
    else printf("-");
    if((buf.st_mode & S_IWUSR)) printf("w");
    else printf("-");
    if((buf.st_mode & S_IXUSR)) printf("x");
    else printf("-");
    if((buf.st_mode & S_IRGRP)) printf("r");
    else printf("-");
    if((buf.st_mode & S_IWGRP)) printf("w");
    else printf("-");
    if((buf.st_mode & S_IXGRP)) printf("x");
    else printf("-");
    if((buf.st_mode & S_IROTH)) printf("r");
    else printf("-");
    if((buf.st_mode & S_IWOTH)) printf("w");
    else printf("-");
    if((buf.st_mode & S_IXOTH)) printf("x");
    else printf("-");
    return 1;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int lsh_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("lsh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("lsh");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int lsh_execute(char **args)
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

/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *lsh_read_line(void)
{
#ifdef LSH_USE_STD_GETLINE
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  if (getline(&line, &bufsize, stdin) == -1) {
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We received an EOF
    } else  {
      perror("lsh: getline\n");
      exit(EXIT_FAILURE);
    }
  }
  return line;
#else
#define LSH_RL_BUFSIZE 1024
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += LSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
#endif
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void lsh_loop(void)
{
  char *line;
  char **args;
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

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
  signal(SIGINT, sighandler);
  lsh_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}

